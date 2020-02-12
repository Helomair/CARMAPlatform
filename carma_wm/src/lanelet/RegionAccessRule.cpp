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
  : RegulatoryElement( id, RuleParameterMap(), {{AttributeNamesString::Type, AttributeValueString::RegulatoryElement},{AttributeNamesString::Subtype, RuleName}})
  , participants_(participants.begin(), participants.end())
{

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
