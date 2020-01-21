#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <boost/algorithm/string.hpp>
#include <carma_wm/lanelet/DigitalSpeedLimit.h>
#include "RegulatoryHelpers.h"

namespace lanelet
{
ConstLineString3d DigitalSpeedLimit::startLine() const
{
  auto line_strings = getParameters<ConstLineString3d>(RoleName::RefLine);
  return line_strings[0];
}

LineString3d DigitalSpeedLimit::startLine()
{
  auto line_strings = getParameters<LineString3d>(RoleName::RefLine);
  return line_strings[0];
}

ConstLineString3d DigitalSpeedLimit::endLine() const
{
  auto line_strings = getParameters<ConstLineString3d>(RoleName::CancelLine);
  return line_strings[0];
}

LineString3d DigitalSpeedLimit::endLine()
{
  auto line_strings = getParameters<LineString3d>(RoleName::CancelLine);
  return line_strings[0];
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
DigitalSpeedLimit::DigitalSpeedLimit(Id id, Velocity speed_limit, LineString3d start_line, LineString3d end_line,
                                     std::vector<std::string> participants)
  : RegulatoryElement{ std::make_shared<lanelet::RegulatoryElementData>(id) }
  , speed_limit_(speed_limit)
  , participants_(participants.begin(), participants.end())
{

  parameters().insert({ lanelet::RoleNameString::RefLine, { start_line } });
  parameters().insert({ lanelet::RoleNameString::CancelLine, { end_line } });
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