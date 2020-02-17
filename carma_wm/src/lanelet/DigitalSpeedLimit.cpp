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
#include <boost/algorithm/string.hpp>
#include <carma_wm/lanelet/DigitalSpeedLimit.h>
#include "RegulatoryHelpers.h"

namespace lanelet
{

ConstLanelets DigitalSpeedLimit::getLanelets() const
{
  return getParameters<ConstLanelet>(RoleName::Refers);
}

ConstAreas DigitalSpeedLimit::getAreas() const
{
  return getParameters<ConstArea>(RoleName::Refers);
}

Velocity DigitalSpeedLimit::getSpeedLimit() const
{
  return speed_limit_;
}

void DigitalSpeedLimit::setSpeedLimit(Velocity speed_limit)
{
  speed_limit_ = speed_limit;
}

bool DigitalSpeedLimit::appliesTo(const std::string& participant) const
{
  return setContainsParticipant(participants_, participant);
}

// TODO some work might be required to make this loadable from a file
DigitalSpeedLimit::DigitalSpeedLimit(Id id, Velocity speed_limit, Lanelets lanelets, Areas areas,
                                     std::vector<std::string> participants)
  : RegulatoryElement( id, RuleParameterMap(), {{AttributeNamesString::Type, AttributeValueString::RegulatoryElement},{AttributeNamesString::Subtype, RuleName}})
  , speed_limit_(speed_limit)
  , participants_(participants.begin(), participants.end())
{

  parameters()[lanelet::RoleNameString::Refers].insert(parameters()[lanelet::RoleNameString::Refers].end(), lanelets.begin(), lanelets.end());
  parameters()[lanelet::RoleNameString::Refers].insert(parameters()[lanelet::RoleNameString::Refers].end(), areas.begin(), areas.end());
}

// C++ 14 vs 17 constant defintion
#if __cplusplus < 201703L
constexpr char DigitalSpeedLimit::RuleName[];  // instantiate string in cpp file
#endif

namespace
{
// this object actually does the registration work for us
lanelet::RegisterRegulatoryElement<lanelet::DigitalSpeedLimit> reg;
}  // namespace

}  // namespace lanelet