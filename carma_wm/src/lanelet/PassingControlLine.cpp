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
#include <carma_wm/lanelet/PassingControlLine.h>
#include "RegulatoryHelpers.h"

namespace lanelet
{
ConstLineStrings3d PassingControlLine::controlLine() const
{
  return getParameters<ConstLineString3d>(RoleName::RefLine);
}

LineStrings3d PassingControlLine::controlLine()
{
  return getParameters<LineString3d>(RoleName::RefLine);
}

bool PassingControlLine::passableFromLeft(const std::string& participant) const
{
  return setContainsParticipant(left_participants_, participant);
}

bool PassingControlLine::passableFromRight(const std::string& participant) const
{
  return setContainsParticipant(right_participants_, participant);
}

bool PassingControlLine::boundPassable(const ConstLineString3d& bound,
                          const std::vector<std::shared_ptr<const PassingControlLine>>& controlLines, bool fromLeft,
                          const std::string& participant)
{
  for (auto control_line : controlLines)
  {
    for (auto sub_line : control_line->controlLine())
    {
      if (bound.id() == sub_line.id())
      {
        if ((fromLeft && !bound.inverted()) || (!fromLeft && bound.inverted()))
        {  // If from the left or coming from the right and the bound is inverted
          return control_line->passableFromLeft(participant);
        }
        else
        {
          return control_line->passableFromRight(participant);
        }
      }
    }
  }
  return true;
}

PassingControlLine::PassingControlLine(Id id, LineStrings3d controlLine, std::vector<std::string> left_participants,
                                       std::vector<std::string> right_participants)
  : RegulatoryElement( id, RuleParameterMap(), {{AttributeNamesString::Type, AttributeValueString::RegulatoryElement},{AttributeNamesString::Subtype, RuleName}})
  , left_participants_(left_participants.begin(), left_participants.end())
  , right_participants_(right_participants.begin(), right_participants.end())
{

  auto ref_line_list = parameters()[lanelet::RoleNameString::RefLine];

  ref_line_list.insert(ref_line_list.end(), controlLine.begin(), controlLine.end());
  // TODO validate that provided control line is contigious
}

// C++ 14 vs 17 parameter export
#if __cplusplus < 201703L
constexpr char PassingControlLine::RuleName[];  // instantiate string in cpp file
#endif

namespace
{
// this object actually does the registration work for us
lanelet::RegisterRegulatoryElement<lanelet::PassingControlLine> reg;
}  // namespace

}  // namespace lanelet
