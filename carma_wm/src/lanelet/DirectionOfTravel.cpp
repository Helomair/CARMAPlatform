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
#include <carma_wm/lanelet/DirectionOfTravel.h>
#include "RegulatoryHelpers.h"

namespace lanelet
{

// Forward declare static constexpr
constexpr char DirectionOfTravel::OneWay[];
constexpr char DirectionOfTravel::BiDirectional[];
constexpr char DirectionOfTravel::DirectionAttribute[];

ConstLanelets DirectionOfTravel::getLanelets() const
{
  return getParameters<ConstLanelet>(RoleName::Refers);
}

bool DirectionOfTravel::isOneWay() const {
  return direction.compare(OneWay) == 0;
}

bool DirectionOfTravel::appliesTo(const std::string& participant) const
{
  return setContainsParticipant(participants_, participant);
}

DirectionOfTravel::DirectionOfTravel(Id id, Lanelets lanelets, std::string direction_of_travel, std::vector<std::string> participants)
  : RegulatoryElement( id, RuleParameterMap(), {{AttributeNamesString::Type, AttributeValueString::RegulatoryElement},{AttributeNamesString::Subtype, RuleName}})
  , participants_(participants.begin(), participants.end())
{

  if (direction_of_travel.compare(OneWay) != 0 || direction_of_travel.compare(BiDirectional) != 0) {
    throw std::invalid_argument("Failed to build DirectionOfTravel regulation as provided direction_of_travel was not one of [ " + std::string(OneWay) + ", " + std::string(BiDirectional) + " ]");
  }
  
  direction = direction_of_travel;

  parameters()[lanelet::RoleNameString::Refers].insert(parameters()[lanelet::RoleNameString::Refers].end(), lanelets.begin(), lanelets.end());

}

// C++ 14 vs 17 constent definition
#if __cplusplus < 201703L
constexpr char DirectionOfTravel::RuleName[];  // instantiate string in cpp file
#endif

namespace
{
// this object actually does the registration work for us
lanelet::RegisterRegulatoryElement<lanelet::DirectionOfTravel> reg;
}  // namespace
}  // namespace lanelet
