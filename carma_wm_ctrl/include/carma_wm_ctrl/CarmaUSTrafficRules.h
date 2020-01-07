#pragma once
#include <lanelet2_core/Forward.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_traffic_rules/TrafficRules.h>
#include <lanelet2_core/primitives/LaneletOrArea.h>  
#include "RegionAccessRule.h"
#include "DigitalSpeedLimit.h"
#include "PassingControlLine.h"

namespace lanelet {
namespace traffic_rules {

//! Class for inferring traffic rules for lanelets and areas
class CarmaUSTrafficRules : public TrafficRules {  // NOLINT
 public:
  //using Configuration = std::map<std::string, Attribute>; TODO remove

  /**
   * @brief Constructor
   * @param config a configuration for traffic rules. This can be necessary for
   * very specialized rule objects (ie considering the time of day or the
   * weather). The config must have at least two entries: participant and location.
   */
  explicit TrafficRules(Configuration config = Configuration()) : config_{std::move(config)} {}
  virtual ~TrafficRules(); // TODO

  bool canAccessRegion(const ConstLaneletOrArea& region) const {
    auto accessRestrictions = region.regulatoryElementsAs<RegionAccessRule>();
    for (auto access_rule : accessRestrictions) {
      if (!access_rule->accessable(participant())) {
        return false;
      }
    }
    return true;
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
  bool canPass(const ConstLanelet& from, const ConstLanelet& to) const {
    // TODO verify the difference between canPass and canChangeLane (is can pass only without lanechange?)
    // 1. Determine if from is to left or right and adjacent
    // 2. Check if from lanelet is accessable
    // 3. Check if to lanelet is accessable
    // 4. Get passingline control regulatory element
    // 5. Check if can cross line
  }
  bool canPass(const ConstLanelet& from, const ConstArea& to) const = 0; // TODO basically the same as above
  bool canPass(const ConstArea& from, const ConstLanelet& to) const = 0; // TODO basically the same as above
  bool canPass(const ConstArea& from, const ConstArea& to) const = 0; // TODO basically the same as above

  /**
   * @brief determines if a lane change can be made between two lanelets
   *
   * The orientation of the lanelets is important here as well.
   */
  bool canChangeLane(const ConstLanelet& from, const ConstLanelet& to) const = 0; // TODO same as what is currently written for canPass above

  //! returns speed limit on this lanelet.
  SpeedLimitInformation speedLimit(const ConstLanelet& lanelet) const = 0; // TODO Get both speedlimit and digital speed limit regulations. Digital speed limit always take preference

  //! returns speed limit on this area.
  SpeedLimitInformation speedLimit(const ConstArea& area) const = 0;  // TODO Get both speedlimit and digital speed limit regulations. Digital speed limit always take preference

  //! returns whether a lanelet can be driven in one direction only
  bool isOneWay(const ConstLanelet& lanelet) const = 0; // TODO need new direction regulatory element

  /**
   * @brief returns whether dynamic traffic rules apply to this lanelet.
   *
   * This can be a case if e.g. a speed limit is controlled by digital signs.
   * If this returns true, additional handling must be done to find which rules
   * are dynamic and how to handle them. This makes the values returned by the
   * other functions unreliable. Handling of dynamic rules is not covered here.
   */
  bool hasDynamicRules(const ConstLanelet& lanelet) const = 0; // TODO Always return true

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
