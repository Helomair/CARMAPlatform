#pragma once
#include <lanelet2_core/primitives/RegulatoryElement.h> 
#include <boost/algorithm/string.hpp> 
#include "RegulatoryHelpers.h"

namespace lanelet {

/**
 * TODO comments
 * @brief Represents a traffic light restriction on the lanelet
 * @ingroup RegulatoryElementPrimitives
 * @ingroup Primitives
 */
class DigitalSpeedLimit : public RegulatoryElement {
 public:
  using Ptr = std::shared_ptr<DigitalSpeedLimit>; // TODO needed?
  static constexpr char RuleName[] = "digital_speed_limit";
  Velocity speed_limit_;
  std::unordered_set<std::string> participants_;

  ConstLineString3d startLine() const {
    auto line_strings = getParameters<ConstLineString3d>(RoleName::RefLine);
    return line_strings[0];
  }

  LineString3d startLine() {
    auto line_strings = getParameters<LineString3d>(RoleName::RefLine);
    return line_strings[0];
  }

  ConstLineString3d endLine() const {
    auto line_strings = getParameters<ConstLineString3d>(RoleName::CancelLine);
    return line_strings[0];
  }

  LineString3d endLine() {
    auto line_strings = getParameters<LineString3d>(RoleName::CancelLine);
    return line_strings[0];
  }

  Velocity getSpeedLimit() const {
    return speed_limit_;
  }

  void setSpeedLimit(Velocity speed_limit) {
    speed_limit_ = speed_limit;
  }

  bool appliesTo(const std::string& participant) const {
    return setContainsParticipant(participants_, participant);
  }

 protected:

  // TODO some work might be required to make this loadable from a file
  DigitalSpeedLimit(Id id, Velocity speed_limit, LineString3d start_line, LineString3d end_line, std::vector<std::string> participants) 
    : RegulatoryElement{std::make_shared<lanelet::RegulatoryElementData>(id)},
      speed_limit_(speed_limit),
      participants_(participants.begin(), participants.end())
  {
    parameters().insert({lanelet::RoleNameString::RefLine, {start_line}});
    parameters().insert({lanelet::RoleNameString::CancelLine, {end_line}});
  }

  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class RegisterRegulatoryElement<DigitalSpeedLimit>;
  explicit DigitalSpeedLimit(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data) {};
};


// TODO WHAT THE HECK IS THIS STUFF?
#if __cplusplus < 201703L
constexpr char DigitalSpeedLimit::RuleName[];  // instanciate string in cpp file
#endif
}  // namespace example

// namespace {
// // this object actually does the registration work for us
// lanelet::RegisterRegulatoryElement<lanelet::DigitalSpeedLimit> reg;
// }  

//HERE
/*
OK We have created this 
*/