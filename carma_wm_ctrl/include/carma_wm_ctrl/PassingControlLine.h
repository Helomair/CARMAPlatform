
#include <lanelet2_core/primitives/RegulatoryElement.h> 
#include <boost/algorithm/string.hpp> 
#include "RegulatoryHelpers.h"

namespace lanelet {

// TODO should this regulatory element and others be applyable to multiple parameters?
/**
 * TODO comments
 * @brief Represents a traffic light restriction on the lanelet
 * @ingroup RegulatoryElementPrimitives
 * @ingroup Primitives
 */
class PassingControlLine : public RegulatoryElement {
 public:
  using Ptr = std::shared_ptr<PassingControlLine>; // TODO needed?
  static constexpr char RuleName[] = "passing_control_line";
  std::unordered_set<std::string> left_participants_;
  std::unordered_set<std::string> right_participants_;

  /**
   * @brief get the line this regulation applies to
   * @return the line as a line string
   */
  ConstLineString3d controlLine() const {
    auto line_strings = getParameters<ConstLineString3d>(RoleName::RefLine);
    return line_strings[0];
  }
  LineString3d controlLine() {
    auto line_strings = getParameters<LineString3d>(RoleName::RefLine);
    return line_strings[0];
  }

  bool passableFromLeft(const std::string& participant) {
    return setContainsParticipant(left_participants_, participant);
  }

  bool passableFromRight(const std::string& participant) {
    return setContainsParticipant(right_participants_, participant);
  }

 protected:

  // TODO some work might be required to make this loadable from a file
  PassingControlLine(Id id, LineString3d controlLine, std::vector<std::string> left_participants, std::vector<std::string> right_participants) 
    : RegulatoryElement{std::make_shared<lanelet::RegulatoryElementData>(id)},
      left_participants_(left_participants.begin(), left_participants.end()),
      right_participants_(right_participants.begin(), right_participants.end())
  {
    parameters().insert({lanelet::RoleNameString::RefLine, {controlLine}});
  }

  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class RegisterRegulatoryElement<PassingControlLine>;
  explicit PassingControlLine(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data) {};
};


// TODO WHAT THE HECK IS THIS STUFF?
#if __cplusplus < 201703L
constexpr char PassingControlLine::RuleName[];  // instanciate string in cpp file
#endif
}  // namespace example

namespace {
// this object actually does the registration work for us
lanelet::RegisterRegulatoryElement<lanelet::PassingControlLine> reg_pcl; // TODO figure this out. If it is named the same between files it will cause duplicate declaration errors
}  

//HERE
/*
OK We have created this 
*/