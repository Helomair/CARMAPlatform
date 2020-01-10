#pragma once
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_core/primitives/LaneletOrArea.h>  
#include <lanelet2_core/primitives/Lanelet.h> 
#include <lanelet2_core/primitives/Area.h> 
#include <boost/algorithm/string.hpp> 
#include <lanelet2_core/Forward.h>
#include "RegulatoryHelpers.h"

namespace lanelet {

/**
 * TODO comments
 * @brief Represents a traffic light restriction on the lanelet
 * @ingroup RegulatoryElementPrimitives
 * @ingroup Primitives
 */
class RegionAccessRule : public RegulatoryElement {
 public:
  using Ptr = std::shared_ptr<RegionAccessRule>; // TODO needed?
  static constexpr char RuleName[] = "region_access_rule";
  std::unordered_set<std::string> participants_;

  ConstLanelets getLanelets() const {
    return getParameters<ConstLanelet>(RoleName::Refers);
  }

  ConstAreas getAreas() const {
    return getParameters<ConstArea>(RoleName::Refers);
  }

  bool accessable(const std::string& participant) const {
    return setContainsParticipant(participants_, participant);
  }

 protected:

  // TODO some work might be required to make this loadable from a file
  RegionAccessRule(Id id, Lanelets lanelets, Areas areas, std::vector<std::string> participants) 
    : RegulatoryElement{std::make_shared<lanelet::RegulatoryElementData>(id)},
    participants_(participants.begin(), participants.end())
  {
    auto refers_list = parameters()[lanelet::RoleNameString::Refers];

    refers_list.insert(refers_list.end(), lanelets.begin(), lanelets.end());
    refers_list.insert(refers_list.end(), areas.begin(), areas.end());
  }

  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class RegisterRegulatoryElement<RegionAccessRule>;
  explicit RegionAccessRule(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data) {};
};

// TODO WHAT THE HECK IS THIS STUFF?
#if __cplusplus < 201703L
constexpr char RegionAccessRule::RuleName[];  // instantiate string in cpp file
#endif
}  // namespace example

// namespace {
// // this object actually does the registration work for us
// lanelet::RegisterRegulatoryElement<lanelet::RegionAccessRule> reg_rar; // TODO
// }  

//HERE
/*
OK We have created this 
*/