#pragma once
#include <lanelet2_core/primitives/RegulatoryElement.h>
#include <boost/algorithm/string.hpp>
#include <unordered_set>

namespace lanelet
{
/**
 * @brief Represents an access restriction in the form of a line laying on the roadway. Restricts whether a given
 * participant can cross the line from the left or right. General usage is as lane boundaries.
 *
 * A PassingControlLine is created from a list of contiguous LineString3d and participants who are allowed to cross from
 * the left or right. If the control line is representing a lane boundary, each LineString3d parameter should exactly
 * match a right or left bound of an adjacent lanelet. In this fashion, a single regulatory element can represent the
 * lane change restrictions of multiple lanelets while still allowing each lanelet to be associated individually.
 *
 * @ingroup RegulatoryElementPrimitives
 * @ingroup Primitives
 */
class PassingControlLine : public RegulatoryElement
{
public:
  static constexpr char RuleName[] = "passing_control_line";
  std::unordered_set<std::string> left_participants_;
  std::unordered_set<std::string> right_participants_;

  /**
   * @brief get the list of contigious line strings that from this control line
   * @return the lines as a list of line strings
   */
  ConstLineStrings3d controlLine() const;
  /**
   * @brief Same as ConstLineStrings3d controlLine() const but without the const modifier
   * However, the implementation of this method is expected to be const
   */
  LineStrings3d controlLine();

  /**
   * @brief Returns true if the provided participant is allowed to cross this line from the left
   *
   * @param participant The string classification of the participant type
   *
   * @return True if participant can cross from the left
   */
  bool passableFromLeft(const std::string& participant) const;

  /**
   * @brief Returns true if the provided participant is allowed to cross this line from the right
   *
   * @param participant The string classification of the participant type
   *
   * @return True if participant can cross from the right
   */
  bool passableFromRight(const std::string& participant) const;
  
  /**
   * @brief Helper function to match a given bound with a control line regulatory element then determine if it can be passed on the right or left
   * 
   * The set of line strings contained in each of the provided control lines is searched until a sub-line is found that matches the provided lanelet or area bound. 
   * Then the inverted ness of that line is evaluated to determine whether the passableFromLeft or passableFromRight function should be called. 
   * The returned value indicates if the control line can be crossed from the direction specified by the fromLeft parameter where the left/rightness relates to the provided bound not the control line
   * 
   * @param bound The bound to try passing. The fromLeft is treated relative to this bound
   * @param controlLines The set of possible control lines which this bound might be a part of
   * @param fromLeft True if the user is trying to check if the bound is passable from its left. False if the user is trying to check if the bound is passable from its right
   * @param participant The participant being evaluated
   * 
   * @return True if the bound can be crossed from the specified direction or if none of the controlLines match the provided bound
   */ 
  static bool boundPassable(const ConstLineString3d& bound, const std::vector<std::shared_ptr<const PassingControlLine>>& controlLines, bool fromLeft, const std::string& participant);

  // TODO some work might be required to make this loadable from a file
  /**
   * @brief Constructor takes in an Id and list of contigious line strings that form this PassingControlLine as well as
   * the sets of participants with access
   *
   * If this control line is a lanelet boundary it is expected that each LineString will exactly match the left or right
   * bound of a lanelet in the map
   *
   * @param id The lanelet::Id of this element
   * @param controlLine The line strings which represent this regularoty elements geometry
   * @param left_participants The set of participants which can cross this line from the left
   * @param right_participants The set of participants which can cross this line from the right
   *
   */
  PassingControlLine(Id id, LineStrings3d controlLine, std::vector<std::string> left_participants,
                     std::vector<std::string> right_participants);
                     
protected:
  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class RegisterRegulatoryElement<PassingControlLine>;
  explicit PassingControlLine(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data){};
};

// Convienace Ptr Declarations
using PassingControlLinePtr = std::shared_ptr<PassingControlLine>;
using PassingControlLineConstPtr = std::shared_ptr<const PassingControlLine>;

}  // namespace lanelet
