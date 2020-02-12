/*
 * Copyright (C) 2020 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <carma_wm/WorldModel.h>
#include <lanelet2_core/Attribute.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>
#include <lanelet2_traffic_rules/GermanTrafficRules.h>
#include <carma_wm/lanelet/RegionAccessRule.h>
#include <carma_wm/lanelet/PassingControlLine.h>
#include <carma_wm_ctrl/MapConformer.h>


namespace lanelet
{
/**
 * The map conformer is responsible for ensuring that a loaded lanelet2 map meets the expectations of
 * CarmaUsTrafficRules Rather than simply validating the map this class will actually modify the map when possible to
 * ensure compliance.
 */
namespace MapConformer
{
namespace
{  // Private namespace
// Enum defining types of lane change
enum class LaneChangeType
{
  ToRight,
  ToLeft,
  Both,
  None
};

/**
 * @brief Helper function to get a value from a map or return a default value when key is not present
 * 
 * This function has been copied from lanelet2_traffic_rules/src/GenericTrafficRules.cpp as it was not exposed by the Lanelet2 libraray.
 * The function therefore has the same functionality as the GenericTrafficRules class
 * 
 * https://github.com/fzi-forschungszentrum-informatik/Lanelet2/blob/master/lanelet2_traffic_rules/src/GenericTrafficRules.cpp
 * The source function is copyrighted under BSD 3-Clause "New" or "Revised" License a copy of that notice has been
 * included with this package
 * 
 * @param map The map to search
 * @param key The key to evaluate
 * @param defaultVal The default to return if key is not in map
 * 
 * @return Value at key in map or defaultVal if key is not in map
 */ 
template <typename Map, typename Key, typename Value>
Value getMapOrDefault(const Map& map, Key key, Value defaultVal)
{
  auto elem = map.find(key);
  if (elem == map.end())
  {
    return defaultVal;
  }
  return elem->second;
}

/**
 * @brief Helper function to check if a string starts with another string
 * 
 * This function has been copied from lanelet2_traffic_rules/src/GenericTrafficRules.cpp as it was not exposed by the Lanelet2 libraray.
 * The function therefore has the same functionality as the GenericTrafficRules class
 * 
 * https://github.com/fzi-forschungszentrum-informatik/Lanelet2/blob/master/lanelet2_traffic_rules/src/GenericTrafficRules.cpp
 * The source function is copyrighted under BSD 3-Clause "New" or "Revised" License a copy of that notice has been
 * included with this package
 * 
 * @param str Base string
 * @param substr Starting string to check for
 * 
 * @return True if str starts with substr
 */ 
bool startswith(const std::string& str, const std::string& substr)
{
  return str.compare(0, substr.size(), substr) == 0;
}

/**
 * @brief Helper function to determine the type of lane change implicitly allowed by the markings on the road.
 * 
 * This function has been copied from lanelet2_traffic_rules/src/GenericTrafficRules.cpp as it was not exposed by the Lanelet2 libraray.
 * The function therefore has the same functionality as the GenericTrafficRules class
 * 
 * https://github.com/fzi-forschungszentrum-informatik/Lanelet2/blob/master/lanelet2_traffic_rules/src/GenericTrafficRules.cpp
 * The source function is copyrighted under BSD 3-Clause "New" or "Revised" License a copy of that notice has been
 * included with this package
 * 
 * @param type The type of the line string marking
 * @param subtype The sub-type of the line string marking
 * @param participant The type of participant the rules apply too
 * 
 * @return An enum describing the type of lane change allowed by the road markings
 */ 
LaneChangeType getChangeType(const std::string& type, const std::string& subtype, const std::string& participant)
{
  using LaneChangeMap = std::map<std::pair<std::string, std::string>, LaneChangeType>;
  const static LaneChangeMap VehicleChangeType{
    { { AttributeValueString::LineThin, AttributeValueString::Dashed }, LaneChangeType::Both },
    { { AttributeValueString::LineThick, AttributeValueString::Dashed }, LaneChangeType::Both },
    { { AttributeValueString::LineThin, AttributeValueString::DashedSolid }, LaneChangeType::ToRight },
    { { AttributeValueString::LineThick, AttributeValueString::DashedSolid }, LaneChangeType::ToRight },
    { { AttributeValueString::LineThin, AttributeValueString::SolidDashed }, LaneChangeType::ToLeft },
    { { AttributeValueString::LineThick, AttributeValueString::SolidDashed }, LaneChangeType::ToLeft }
  };
  const static LaneChangeMap PedestrianChangeType{ { { AttributeValueString::Curbstone, AttributeValueString::Low },
                                                     LaneChangeType::Both } };

  if (startswith(participant, Participants::Vehicle))
  {
    return getMapOrDefault(VehicleChangeType, std::make_pair(type, subtype), LaneChangeType::None);
  }
  if (participant == Participants::Pedestrian)
  {
    return getMapOrDefault(PedestrianChangeType, std::make_pair(type, subtype), LaneChangeType::None);
  }
  if (participant == Participants::Bicycle)
  {
    auto asVehicle = getMapOrDefault(VehicleChangeType, std::make_pair(type, subtype), LaneChangeType::None);
    if (asVehicle != LaneChangeType::None)
    {
      return asVehicle;
    }
    return getMapOrDefault(PedestrianChangeType, std::make_pair(type, subtype), LaneChangeType::None);
  }
  return LaneChangeType::None;
}

/**
 * @brief Helper function constructs a PassingControlLine based off the provided bound and lane change type
 * 
 * @param bound The bound which will mark the control line
 * @param type The type of lane change permitted by this bound's implied regulations
 * @param participant The participant that is allowed to cross this control line
 * 
 * @return A new PasssingControlLine which was created based off the bound
 */ 
PassingControlLinePtr buildControlLine(LineString3d& bound, const LaneChangeType type, const std::string& participant)
{
  if (bound.inverted())
  {
    switch (type)
    {
      case LaneChangeType::ToRight:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound.invert() }, {}, { participant }));
      case LaneChangeType::ToLeft:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound.invert() }, { participant }, {}));
      case LaneChangeType::Both:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound.invert() }, { participant }, { participant }));
      case LaneChangeType::None:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound.invert() }, {}, {}));
    }
  }
  else
  {
    switch (type)
    {
      case LaneChangeType::ToRight:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound }, { participant }, {}));
      case LaneChangeType::ToLeft:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound }, {}, { participant }));
      case LaneChangeType::Both:
        return std::shared_ptr<PassingControlLine>(
            new PassingControlLine(lanelet::utils::getId(), { bound }, { participant }, { participant }));
      case LaneChangeType::None:
        return std::shared_ptr<PassingControlLine>(new PassingControlLine(lanelet::utils::getId(), { bound }, {}, {}));
    }
  }
}

/**
 * @brief Generate RegionAccessRules from the inferred regulations in the provided map and lanelet
 * 
 * @param lanelet The lanelet to generate the rules for
 * @param map The map which the lanelet is part of
 */ 
void addInferredAccessRule(Lanelet& lanelet, lanelet::LaneletMapPtr map)
{
  auto access_rules = lanelet.regulatoryElementsAs<RegionAccessRule>();
  // If the lanelet does not have an access rule then add one based on the generic traffic rules
  if (access_rules.size() == 0)
  {  // No access rule detected so add one

    // Get generic german traffic rules object as this follows the GenericTrafficRules the closest
    std::string participant(lanelet.attribute(AttributeNamesString::Participant).value());

    lanelet::traffic_rules::TrafficRulesUPtr traffic_rules =
        lanelet::traffic_rules::TrafficRulesFactory::create(lanelet::Locations::Germany, participant);

    // Use german traffic rules to interperest passability of lanelet
    bool accessable = traffic_rules->canPass(lanelet);
    // If the lanelet is accessable then add a new rule describing that fact
    if (accessable)
    {
      std::shared_ptr<RegionAccessRule> rar(
          new RegionAccessRule(lanelet::utils::getId(), { lanelet }, {}, { participant }));
      lanelet.addRegulatoryElement(rar);
      map->add(rar);
    }
  }
}

/**
 * @brief Generate RegionAccessRules from the inferred regulations in the provided map and area
 * 
 * @param area The area to generate the rules for
 * @param map The map which the area is part of
 */ 
void addInferredAccessRule(Area& area, lanelet::LaneletMapPtr map)
{
  auto access_rules = area.regulatoryElementsAs<RegionAccessRule>();
  // If the lanelet does not have an access rule then add one based on the generic traffic rules
  if (access_rules.size() == 0)
  {  // No access rule detected so add one

    // Get generic german traffic rules object as this follows the GenericTrafficRules the closest
    std::string participant(area.attribute(AttributeNamesString::Participant).value());

    lanelet::traffic_rules::TrafficRulesUPtr traffic_rules =
        lanelet::traffic_rules::TrafficRulesFactory::create(lanelet::Locations::Germany, participant);

    // Use german traffic rules to interperest passability of lanelet
    bool accessable = traffic_rules->canPass(area);
    // If the lanelet is accessable then add a new rule describing that fact
    if (accessable)
    {
      std::shared_ptr<RegionAccessRule> rar(
          new RegionAccessRule(lanelet::utils::getId(), {}, { area }, { participant }));
      area.addRegulatoryElement(rar);
      map->add(rar);
    }
  }
}

/**
 * @brief Generate PassingControlLines from the inferred regulations in the provided map and lanelet
 * 
 * @param lanelet The lanelet to generate control lines for
 * @param map The map which the lanelet is part of
 */ 
void addInferredPassingControlLine(Lanelet& lanelet, lanelet::LaneletMapPtr map)
{
  std::string participant(lanelet.attribute(AttributeNamesString::Participant).value());

  LineString3d left_bound = lanelet.leftBound();
  LineString3d right_bound = lanelet.rightBound();

  // Determine possibility of lane change for left and right bounds
  LaneChangeType left_type = getChangeType(left_bound.attribute(AttributeName::Type).value(),
                                           left_bound.attribute(AttributeName::Subtype).value(), participant);
  LaneChangeType right_type = getChangeType(right_bound.attribute(AttributeName::Type).value(),
                                            right_bound.attribute(AttributeName::Subtype).value(), participant);



  auto local_control_lines = lanelet.regulatoryElementsAs<PassingControlLine>();

  bool foundLeft = false;
  bool foundRight = false;

  // Iterate over all regulatory elements in the map to determine if there is an existing regulation for this
  // lanelet's bounds
  for (auto reg_elem : map->regulatoryElementLayer)
  {
    auto id = reg_elem->id();
    auto at = AttributeName::Subtype;
    auto val = reg_elem->attribute(AttributeName::Subtype).value();
    if (reg_elem->attribute(AttributeName::Subtype).value() == PassingControlLine::RuleName)
    {
      auto pcl = std::static_pointer_cast<PassingControlLine>(reg_elem);
      for (auto sub_line : pcl->controlLine())
      {
        if (left_bound.id() == sub_line.id())
        {
          foundLeft = true;
          // Check if our lanelet contains this control line
          // If it does not then add it
          // TODO unit test contains call here
          // replace with if it does not work lanelet::utils::find_if(local_control_lines.begin(),
          // local_control_lines.end(), [&reg_elem](auto& elem) { return reg_elem.id() == elem.id(); });
          bool alreadyAdded = lanelet::utils::contains(local_control_lines, pcl);
          if (!alreadyAdded)
          {
            lanelet.addRegulatoryElement(pcl);
          }
        }
        else if (right_bound.id() == sub_line.id())
        {
          foundRight = true;
          // Check if our lanelet contains this control line
          // If it does not then add it
          bool alreadyAdded = lanelet::utils::contains(local_control_lines, pcl);
          if (!alreadyAdded)
          {
            lanelet.addRegulatoryElement(pcl);
          }
        }
      }
    }
  }

  // If no existing regulation was found for this lanelet's right or left bound then create a new one and add it to
  // the lanelet and the map
  if (!foundLeft)
  {
    PassingControlLinePtr pcl_left = buildControlLine(left_bound, left_type, participant);
    lanelet.addRegulatoryElement(pcl_left);
    map->add(pcl_left);
  }
  if (!foundRight)
  {
    PassingControlLinePtr pcl_right = buildControlLine(right_bound, right_type, participant);
    lanelet.addRegulatoryElement(pcl_right);
    map->add(pcl_right);
  }
}

/**
 * @brief Generate PassingControlLines from the inferred regulations in the provided map and area
 * 
 * @param area The area to generate control lines for
 * @param map The map which the area is part of
 */ 
void addInferredPassingControlLine(Area& area, lanelet::LaneletMapPtr map)
{
  std::string participant(area.attribute(AttributeNamesString::Participant).value());

  LineStrings3d outerBounds = area.outerBound();

  auto local_control_lines = area.regulatoryElementsAs<PassingControlLine>();

  std::vector<size_t> not_found_indices;

  // Iterate over all regulatory elements in the map to determine if there is an existing regulation for this area's
  // bounds
  for (auto reg_elem : map->regulatoryElementLayer)
  {
    if (reg_elem->attribute(AttributeName::Subtype).value() == PassingControlLine::RuleName)
    {
      auto pcl = std::static_pointer_cast<PassingControlLine>(reg_elem);
      for (auto sub_line : pcl->controlLine())
      {
        size_t i = 0;
        for (auto sub_bound : outerBounds)
        {
          if (sub_bound.id() == sub_line.id())
          {
            // If an existing regulation applies to this area's bounds then add it to the area
            bool alreadyAdded = lanelet::utils::contains(local_control_lines, pcl);
            if (!alreadyAdded)
            {
              area.addRegulatoryElement(pcl);
            }
          }
          else
          {
            not_found_indices.push_back(i);
          }
          i++;
        }
      }
    }
  }

  // For all the bounds which did not have an existing regulation create a new one
  for (size_t i : not_found_indices)
  {
    LineString3d sub_bound = outerBounds[i];
    LaneChangeType change_type = getChangeType(sub_bound.attribute(AttributeName::Type).value(),
                                               sub_bound.attribute(AttributeName::Subtype).value(), participant);
    PassingControlLinePtr control_line = buildControlLine(sub_bound, change_type, participant);
    area.addRegulatoryElement(control_line);
    map->add(control_line);
  }
}
}  // namespace

void ensureCompliance(lanelet::LaneletMapPtr map)
{
  // Handle lanelets
  for (auto lanelet : map->laneletLayer)
  {
    addInferredAccessRule(lanelet, map);
    addInferredPassingControlLine(lanelet, map);
  }

  // Handle areas
  for (auto area : map->areaLayer)
  {
    addInferredAccessRule(area, map);
    addInferredPassingControlLine(area, map);
  }
}
};  // namespace MapConformer
}  // namespace lanelet