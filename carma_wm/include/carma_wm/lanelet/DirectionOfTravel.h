#pragma once
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
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_core/primitives/LaneletOrArea.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/primitives/Area.h>
#include <boost/algorithm/string.hpp>
#include <lanelet2_core/Forward.h>
#include <unordered_set>

namespace lanelet
{
/**
 * @brief A direction of travel regulation defines if a lanelet is One Way or Bi-Directional
 *
 * @ingroup RegulatoryElementPrimitives
 * @ingroup Primitives
 */
class DirectionOfTravel : public RegulatoryElement
{
public:
  static constexpr char RuleName[] = "direction_of_travel";

  static constexpr char OneWay[] = "one_way";
  static constexpr char BiDirectional[] = "bi_directional";
  static constexpr char DirectionAttribute[] = "direction";
  
  std::unordered_set<std::string> participants_;
  std::string direction;

  /**
   * @brief Returns the lanelets this rule applies to
   *
   * @return Lanelets affected by this access rule
   */
  ConstLanelets getLanelets() const;

  /**
   * @brief Returns true if this regulation is one way
   * 
   * @return True if DirectionAttribute is equal to DirectionOfTravel::OneWay. False otherwise
   */ 
  bool isOneWay() const;

  /**
   * @brief Returns true if the given participant must follow this speed limit
   *
   * @return True if this speed limit should apply to the given participant
   */
  bool appliesTo(const std::string& participant) const;

  // TODO some work might be required to make this loadable from a file

  /**
   * @brief Constructor creates an access rule for the provided lanelets, areas, and participants
   *
   * @param id The lanelet::Id to give this regulation
   * @param lanelets The lanelets impacted by this regulation
   * @param direction_of_travel The direction of travel expected to be one of DirectionOfTravel::OneWay or DirectionOfTravel::BiDirectional
   * @param participants The participants which can access the provided lanelets and areas
   */
  DirectionOfTravel(Id id, Lanelets lanelets, std::string direction_of_travel, std::vector<std::string> participants);

protected:

  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class RegisterRegulatoryElement<DirectionOfTravel>;
  explicit DirectionOfTravel(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data){};
};

// Convienace Ptr Declarations
using DirectionOfTravelPtr = std::shared_ptr<DirectionOfTravel>;
using DirectionOfTravelConstPtr = std::shared_ptr<const DirectionOfTravel>;

}  // namespace lanelet