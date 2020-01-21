#include <lanelet2_core/Forward.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/geometry/Area.h>
#include <lanelet2_core/utility/Units.h>
#include <lanelet2_traffic_rules/TrafficRules.h>
#include <lanelet2_traffic_rules/Exceptions.h>
#include <lanelet2_core/primitives/LaneletOrArea.h>
#include <lanelet2_core/primitives/BasicRegulatoryElements.h>
#include <carma_wm/lanelet/CarmaUSTrafficRules.h>
#include <carma_wm/lanelet/RegionAccessRule.h>
#include <carma_wm/lanelet/DigitalSpeedLimit.h>
#include <carma_wm/lanelet/PassingControlLine.h>

namespace lanelet
{
namespace traffic_rules
{

/**
 * @brief Helper function for determining the common line between a lanelet and area. 
 * Based on the same function in lanelet2_traffic_rules/GenericTrafficRules.cpp. Copied here as it is not exposed by that library
 */ 
Optional<ConstLineString3d> determineCommonLine(const ConstLanelet& ll, const ConstArea& ar)
{
  return utils::findIf(ar.outerBound(), [p1 = ll.leftBound().back(), p2 = ll.rightBound().back()](auto& boundLs) {
    return (boundLs.back() == p1 && boundLs.front() == p2);
  });
}

/**
 * @brief Helper function for determining the common line between a lanelet and area. 
 * Based on the same function in lanelet2_traffic_rules/GenericTrafficRules.cpp. Copied here as it is not exposed by that library.
 */ 
Optional<ConstLineString3d> determineCommonLine(const ConstArea& ar1, const ConstArea& ar2)
{
  return utils::findIf(ar1.outerBound(), [&ar2](auto& ar1Bound) {
    return !!utils::findIf(ar2.outerBound(),
                           [ar1Bound = ar1Bound.invert()](auto& ar2Bound) { return ar2Bound == ar1Bound; });
  });
}

bool CarmaUSTrafficRules::canAccessRegion(const ConstLaneletOrArea& region) const
{
  auto accessRestrictions = region.regulatoryElementsAs<RegionAccessRule>();
  for (auto access_rule : accessRestrictions)
  {
    if (!access_rule->accessable(participant()))
    {
      return false;
    }
  }
  return true;  // TODO should we support attributes as fallback?
}

bool CarmaUSTrafficRules::canPass(const ConstLanelet& lanelet) const
{
  ConstLaneletOrArea region(lanelet);
  return canAccessRegion(region);
}

bool CarmaUSTrafficRules::canPass(const ConstArea& area) const
{
  ConstLaneletOrArea region(area);
  return canAccessRegion(region);
}

// TODO NOTE: Basic on the GenericTrafficRules object this function is for non-lanechange passing
bool CarmaUSTrafficRules::canPass(const ConstLanelet& from, const ConstLanelet& to) const
{
  return geometry::follows(from, to) && canPass(from) && canPass(to);
}

bool CarmaUSTrafficRules::boundPassable(const ConstLineString3d& bound,
                   const std::vector<PassingControlLineConstPtr>& controlLines, bool fromLeft) const
{
  bool result = PassingControlLine::boundPassable(bound, controlLines, fromLeft, participant());
  // TODO throw exception or revert to attributes or return true????
  // We could fallback on generic traffic rules here
  return result;
}

bool CarmaUSTrafficRules::canPass(const ConstLanelet& from, const ConstArea& to) const
{
  if (!canPass(from) || !canPass(to))
  {
    return false;
  }

  if (geometry::leftOf(from, to))
  {
    return boundPassable(from.rightBound(), from.regulatoryElementsAs<PassingControlLine>(), true);
  }
  if (geometry::rightOf(from, to))
  {
    return boundPassable(from.leftBound(), from.regulatoryElementsAs<PassingControlLine>(), false);
  }

  auto line = determineCommonLine(from, to);
  if (!!line)
  {
    auto val = to.regulatoryElementsAs<PassingControlLine>();
    auto to_regs = utils::transformSharedPtr<const PassingControlLine>(val);
    return boundPassable(*line, to_regs, true);
  }
  return false;
}

bool CarmaUSTrafficRules::canPass(const ConstArea& from, const ConstLanelet& to) const
{
  if (!canPass(from) || !canPass(to))
  {
    return false;
  }

  if (geometry::leftOf(to, from))
  {
    return boundPassable(to.rightBound(), from.regulatoryElementsAs<PassingControlLine>(), false);
  }
  if (geometry::rightOf(to, from))
  {
    return boundPassable(to.leftBound(), from.regulatoryElementsAs<PassingControlLine>(), true);
  }
  auto line = determineCommonLine(to.invert(), from);  // TODO verify this inversion
  if (!!line)
  {
    return boundPassable(*line, from.regulatoryElementsAs<PassingControlLine>(), false);
  }
  return false;
}

bool CarmaUSTrafficRules::canPass(const ConstArea& from, const ConstArea& to) const
{
  if (!canPass(from) || !canPass(to))
  {
    return false;
  }

  auto line = determineCommonLine(from, to);
  if (!!line)
  {
    return boundPassable(*line, from.regulatoryElementsAs<PassingControlLine>(), true);
  }

  return false;
}

bool CarmaUSTrafficRules::canChangeLane(const ConstLanelet& from, const ConstLanelet& to) const
{
  if (!canPass(from) || !canPass(to))
  {
    return false;
  }
  if (geometry::leftOf(from, to))
  {
    return boundPassable(from.rightBound(), from.regulatoryElementsAs<PassingControlLine>(), true);
  }
  if (geometry::rightOf(from, to))
  {
    return boundPassable(from.leftBound(), from.regulatoryElementsAs<PassingControlLine>(), false);
  }
}

Velocity CarmaUSTrafficRules::trafficSignToVelocity(const std::string& typeString) const
{
  using namespace lanelet::units::literals;
  // MUTCD Code plus a - for the speed limit value
  const static std::map<std::string, Velocity> StrToVelocity{
    { "R2-1-5mph", 5_mph },   { "R2-1-10mph", 10_mph }, { "R2-1-15mph", 15_mph }, { "R2-1-20mph", 20_mph },
    { "R2-1-25mph", 25_mph }, { "R2-1-30mph", 30_mph }, { "R2-1-35mph", 35_mph }, { "R2-1-40mph", 40_mph },
    { "R2-1-45mph", 45_mph }, { "R2-1-50mph", 50_mph }, { "R2-1-55mph", 55_mph }, { "R2-1-60mph", 60_mph },
    { "R2-1-65mph", 65_mph }, { "R2-1-70mph", 70_mph }, { "R2-1-75mph", 75_mph }, { "R2-1-80mph", 80_mph }
  };
  try
  {
    return StrToVelocity.at(typeString);
  }
  catch (std::out_of_range&)
  {
    // try to interpret typeString directly as velocity
    Attribute asAttribute(typeString);
    auto velocity = asAttribute.asVelocity();
    if (!!velocity)
    {
      return *velocity;
    }
    throw lanelet::InterpretationError("Unable to interpret the velocity information from " + typeString);
  }
}

SpeedLimitInformation CarmaUSTrafficRules::speedLimit(const ConstLaneletOrArea& lanelet_or_area) const
{
  auto sign_speed_limits = lanelet_or_area.regulatoryElementsAs<SpeedLimit>();
  auto digital_speed_limits = lanelet_or_area.regulatoryElementsAs<DigitalSpeedLimit>();
  Velocity speed_limit;
  for (auto sign_speed_limit : sign_speed_limits)
  {
    speed_limit = trafficSignToVelocity(sign_speed_limit->type());
  }
  for (auto dig_speed_limit : digital_speed_limits)
  {
    if (dig_speed_limit->appliesTo(participant()))
    {
      speed_limit = dig_speed_limit->getSpeedLimit();
    }
  }

  return SpeedLimitInformation{ speed_limit, true };
}

SpeedLimitInformation CarmaUSTrafficRules::speedLimit(const ConstLanelet& lanelet) const
{
  ConstLaneletOrArea lanelet_or_area(lanelet);
  return speedLimit(lanelet_or_area);
}

SpeedLimitInformation CarmaUSTrafficRules::speedLimit(const ConstArea& area) const
{
  ConstLaneletOrArea lanelet_or_area(area);
  return speedLimit(lanelet_or_area);
}

// TODO This function always returns true meaning CARMA cannot handle non-directed roadways
// To add that functionality a new regulatory element called direction should be added
bool CarmaUSTrafficRules::isOneWay(const ConstLanelet& lanelet) const
{
  return true;
}

bool CarmaUSTrafficRules::hasDynamicRules(const ConstLanelet& lanelet) const
{
  return true;  // All regulations are considered dynamic in CARMA
}


/**
 * TODO incorporate registration into this class
 * RegisterTrafficRules<GermanVehicle> gvRules(Locations::Germany, Participants::Vehicle);
RegisterTrafficRules<GermanPedestrian> gpRules(Locations::Germany, Participants::Pedestrian);
RegisterTrafficRules<GermanBicycle> gbRules(Locations::Germany, Participants::Bicycle);
*/

}  // namespace traffic_rules
}  // namespace lanelet
