#pragma once
#include <lanelet2_core/Forward.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/geometry/Area.h>
#include <lanelet2_core/utility/Units.h>
#include <lanelet2_traffic_rules/TrafficRules.h>
#include <lanelet2_traffic_rules/Exceptions.h>
#include <lanelet2_core/primitives/LaneletOrArea.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>
#include "RegionAccessRule.h"
#include "DigitalSpeedLimit.h"
#include "PassingControlLine.h"

namespace lanelet {
namespace traffic_rules {

//! Class for inferring traffic rules for lanelets and areas
class CarmaUSTrafficRules : public TrafficRules {  // NOLINT
 public:
  //using Configuration = std::map<std::string, Attribute>; TODO remove

  virtual ~CarmaUSTrafficRules() {};

  bool canAccessRegion(const ConstLaneletOrArea& region) const {

    auto accessRestrictions = region.regulatoryElementsAs<RegionAccessRule>();
    for (auto access_rule : accessRestrictions) {
      if (!access_rule->accessable(participant())) {
        return false;
      }
    }
    return true; // TODO should we support attributes as fallback?
  }

  /**
   * @brief returns whether it is allowed to pass/drive on this lanelet
   *
   * The result can differ by the type of the traffic participant. A sidewalk
   * lanelet is passable for a pedestrian but not for a vehicle. Orientation is
   * important. It is not possible to pass an inverted oneWay Lanelet.
   */
  bool canPass(const ConstLanelet& lanelet) const {
    ConstLaneletOrArea region(lanelet);
    return canAccessRegion(region);
  }

  //! returns whether it is allowed to pass/drive on this area
  bool canPass(const ConstArea& area) const{
    ConstLaneletOrArea region(area);
    return canAccessRegion(region);
  }

  /**
   * @brief returns whether it is allowed to pass/drive from a lanelet to
   * another lanelet.
   *
   * The orientation of the lanelets is important. The function first checks if
   * lanelets are direcly adjacent, then checks if both lanelets are passable
   * and finally checks if any traffic rule prevents to pass between the
   * lanelets.
   */
  // TODO NOTE: Basic on the GenericTrafficRules object this function is for non-lanechange passing
  bool canPass(const ConstLanelet& from, const ConstLanelet& to) const {
    return geometry::follows(from, to) && canPass(from) && canPass(to);
  }

  bool boundPassable(const ConstLineString3d& bound, const std::vector<std::shared_ptr<const PassingControlLine>>& controlLines, bool fromLeft) const {
    for (auto control_line : controlLines) {
      for (auto sub_line : control_line->controlLine()) {

        if (bound.id() == sub_line.id()) {

          if ((fromLeft && !bound.inverted()) || (!fromLeft && bound.inverted())) { // If from the left or coming from the right and the bound is inverted
            return control_line->passableFromLeft(participant());
          } else {
            return control_line->passableFromRight(participant());
          }
          
        }
      }
    }
    // TODO throw exception or revert to attributes or return true????
  }

  Optional<ConstLineString3d> determineCommonLine(const ConstLanelet& ll, const ConstArea& ar) const {
    return utils::findIf(ar.outerBound(), [p1 = ll.leftBound().back(), p2 = ll.rightBound().back()](auto& boundLs) {
      return (boundLs.back() == p1 && boundLs.front() == p2);
    });
  }
  Optional<ConstLineString3d> determineCommonLine(const ConstArea& ar1, const ConstArea& ar2) const {
    return utils::findIf(ar1.outerBound(), [&ar2](auto& ar1Bound) {
      return !!utils::findIf(ar2.outerBound(),
                            [ar1Bound = ar1Bound.invert()](auto& ar2Bound) { return ar2Bound == ar1Bound; });
    });
  }

  bool canPass(const ConstLanelet& from, const ConstArea& to) const {
    if (!canPass(from) || !canPass(to)) {
      return false;
    }

    if (geometry::leftOf(from, to)) {
      return boundPassable(from.rightBound(), from.regulatoryElementsAs<PassingControlLine>(), true);
    }
    if (geometry::rightOf(from, to)) {
      return boundPassable(from.leftBound(), from.regulatoryElementsAs<PassingControlLine>(), false);
    }

    auto line = determineCommonLine(from, to);
    if (!!line) {
      auto val = to.regulatoryElementsAs<PassingControlLine>();
      auto to_regs = utils::transformSharedPtr<const PassingControlLine>(val);
      return boundPassable(*line, to_regs, true);
    }
    return false;
  }

  bool canPass(const ConstArea& from, const ConstLanelet& to) const {
    if (!canPass(from) || !canPass(to)) {
      return false;
    }

    if (geometry::leftOf(to, from)) {
      return boundPassable(to.rightBound(), from.regulatoryElementsAs<PassingControlLine>(), false);
    }
    if (geometry::rightOf(to, from)) {
      return boundPassable(to.leftBound(), from.regulatoryElementsAs<PassingControlLine>(), true);
    }
    auto line = determineCommonLine(to.invert(), from); // TODO verify this inversion
    if (!!line) {
      return boundPassable(*line, from.regulatoryElementsAs<PassingControlLine>(), false);
    }
    return false;
  }

  bool canPass(const ConstArea& from, const ConstArea& to) const {
    if (!canPass(from) || !canPass(to)) {
      return false;
    }

    auto line = determineCommonLine(from, to);
    if (!!line) {
      return boundPassable(*line, from.regulatoryElementsAs<PassingControlLine>(), true);
    }
    
    return false;
  }

  

  /**
   * @brief determines if a lane change can be made between two lanelets
   *
   * The orientation of the lanelets is important here as well.
   */
  bool canChangeLane(const ConstLanelet& from, const ConstLanelet& to) const {
    if (!canPass(from) || !canPass(to)) {
      return false;
    }
    if (geometry::leftOf(from, to)) {
      return boundPassable(from.rightBound(), from.regulatoryElementsAs<PassingControlLine>(), true);
    }
    if (geometry::rightOf(from, to)) {
      return boundPassable(from.leftBound(), from.regulatoryElementsAs<PassingControlLine>(), false);
    }
  }

  Velocity trafficSignToVelocity(const std::string& typeString) const {
    using namespace lanelet::units::literals;
    // MUTCD Code plus a - for the speed limit value
    const static std::map<std::string, Velocity> StrToVelocity{
        {"R2-1-5mph", 5_mph},   {"R2-1-10mph", 10_mph}, {"R2-1-15mph", 15_mph}, {"R2-1-20mph", 20_mph},
        {"R2-1-25mph", 25_mph}, {"R2-1-30mph", 30_mph}, {"R2-1-35mph", 35_mph}, {"R2-1-40mph", 40_mph},
        {"R2-1-45mph", 45_mph}, {"R2-1-50mph", 50_mph}, {"R2-1-55mph", 55_mph}, {"R2-1-60mph", 60_mph},
        {"R2-1-65mph", 65_mph}, {"R2-1-70mph", 70_mph}, {"R2-1-75mph", 75_mph}, {"R2-1-80mph", 80_mph}};
    try {
      return StrToVelocity.at(typeString);
    } catch (std::out_of_range&) {
      // try to interpret typeString directly as velocity
      Attribute asAttribute(typeString);
      auto velocity = asAttribute.asVelocity();
      if (!!velocity) {
        return *velocity;
      }
      throw lanelet::InterpretationError("Unable to interpret the velocity information from " + typeString);
    }
  }


  SpeedLimitInformation speedLimit(const ConstLaneletOrArea& lanelet_or_area) const {
    auto sign_speed_limits = lanelet_or_area.regulatoryElementsAs<SpeedLimit>();
    auto digital_speed_limits = lanelet_or_area.regulatoryElementsAs<DigitalSpeedLimit>();
    Velocity speed_limit;
    for (auto sign_speed_limit : sign_speed_limits) {
      speed_limit = trafficSignToVelocity(sign_speed_limit->type());
    }
    for (auto dig_speed_limit : digital_speed_limits) {
      if (dig_speed_limit->appliesTo(participant())) {
        speed_limit = dig_speed_limit->getSpeedLimit();
      }
    }

    return SpeedLimitInformation{speed_limit, true};
  }

  //! returns speed limit on this lanelet.
  SpeedLimitInformation speedLimit(const ConstLanelet& lanelet) const {
    ConstLaneletOrArea lanelet_or_area(lanelet);
    return speedLimit(lanelet_or_area);
  }

  //! returns speed limit on this area.
  SpeedLimitInformation speedLimit(const ConstArea& area) {
    ConstLaneletOrArea lanelet_or_area(area);
    return speedLimit(lanelet_or_area);
  }

  //! returns whether a lanelet can be driven in one direction only
  // TODO This function always returns true meaning CARMA cannot handle non-directed roadways
  // To add that functionality a new regulatory element called direction should be added
  bool isOneWay(const ConstLanelet& lanelet) const {
    return true;
  } 

  /**
   * @brief returns whether dynamic traffic rules apply to this lanelet.
   *
   * This can be a case if e.g. a speed limit is controlled by digital signs.
   * If this returns true, additional handling must be done to find which rules
   * are dynamic and how to handle them. This makes the values returned by the
   * other functions unreliable. Handling of dynamic rules is not covered here.
   */
  bool hasDynamicRules(const ConstLanelet& lanelet) const {
    return true; // All regulations are considered dynamic in CARMA
  }

  /**
   * @brief configuration for this traffic rules object
   */
  const Configuration& configuration() const { return config_; }

  /**
   * @brief the traffic participant the rules are valid for (e.g. vehicle, car,
   * pedestrian, etc)
   */
  const std::string& participant() const { return config_.at("participant").value(); }

  /**
   * @brief the the location the rules are valid for
   *
   * Should be ISO country code
   */
  const std::string& location() const { return config_.at("location").value(); }

 private:
  Configuration config_;
};

std::ostream& operator<<(std::ostream& stream, const SpeedLimitInformation& obj); // TODO are these needed?

std::ostream& operator<<(std::ostream& stream, const TrafficRules& obj);

}  // namespace traffic_rules
}  // namespace lanelet
