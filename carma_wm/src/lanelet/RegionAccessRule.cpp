#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <lanelet2_core/primitives/LaneletOrArea.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/primitives/Area.h>
#include <boost/algorithm/string.hpp>
#include <lanelet2_core/Forward.h>
#include <carma_wm/lanelet/RegionAccessRule.h>
#include "RegulatoryHelpers.h"

namespace lanelet
{
ConstLanelets RegionAccessRule::getLanelets() const
{
  return getParameters<ConstLanelet>(RoleName::Refers);
}

ConstAreas RegionAccessRule::getAreas() const
{
  return getParameters<ConstArea>(RoleName::Refers);
}

bool RegionAccessRule::accessable(const std::string& participant) const
{
  return setContainsParticipant(participants_, participant);
}

RegionAccessRule::RegionAccessRule(Id id, Lanelets lanelets, Areas areas, std::vector<std::string> participants)
  : RegulatoryElement{ std::make_shared<lanelet::RegulatoryElementData>(id) }
  , participants_(participants.begin(), participants.end())
{
  //attributes()[AttributeName::Type] = AttributeValueString::RegulatoryElement;
  //attributes()[AttributeName::Subtype] = RuleName;

  auto refers_list = parameters()[lanelet::RoleNameString::Refers];

  refers_list.insert(refers_list.end(), lanelets.begin(), lanelets.end());
  refers_list.insert(refers_list.end(), areas.begin(), areas.end());
}

// C++ 14 vs 17 constent definition
#if __cplusplus < 201703L
constexpr char RegionAccessRule::RuleName[];  // instantiate string in cpp file
#endif

namespace
{
// this object actually does the registration work for us
lanelet::RegisterRegulatoryElement<lanelet::RegionAccessRule> reg;
}  // namespace
}  // namespace lanelet
