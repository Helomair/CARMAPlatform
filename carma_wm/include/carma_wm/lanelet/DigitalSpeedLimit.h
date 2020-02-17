#pragma once
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
#include <unordered_set>

namespace lanelet
{
/**
 * @brief Represents a speed limit which can be set dynamically either through a V2X communications service.
 *        In a standard used case a digital speed limit would be expected to have precedence over a speed limit from a sign
 *
 * A digital speed limit is dynamic and is normally provided through a communications service. This means the speed
 * limit is stored directly in the regulatory element rather than a TrafficSign element. A speed limit is applied uniformly accross all affected lanelets.
 *
 * @ingroup RegulatoryElementPrimitives
 * @ingroup Primitives
 */
class DigitalSpeedLimit : public RegulatoryElement
{
public:
  static constexpr char RuleName[] = "digital_speed_limit";
  Velocity speed_limit_;
  std::unordered_set<std::string> participants_;

  /**
   * @brief Returns the lanelets this rule applies to
   *
   * @return Lanelets affected by this access rule
   */
  ConstLanelets getLanelets() const;

  /**
   * @brief Returns the areas this rule applies to
   *
   * @return The areas affected by this rule
   */
  ConstAreas getAreas() const;

  /**
   * @brief Returns the speed limit defined by this regulation
   *
   * @return The speed limit as a velocity object
   */
  Velocity getSpeedLimit() const;

  /**
   * @brief Sets the speed limit defined by this regulation
   *
   * @param The speed limit as a velocity object
   */
  void setSpeedLimit(Velocity speed_limit);

  /**
   * @brief Returns true if the given participant must follow this speed limit
   *
   * @return True if this speed limit should apply to the given participant
   */
  bool appliesTo(const std::string& participant) const;

  // TODO some work might be required to make this loadable from a file
  /**
   * @brief Constructor creates a speed limit based on the provided velocity, start, end lines, and the affected
   * participants
   *
   * @param id The lanelet::Id of this object
   * @param speed_limit The velocity which will be treated as the speed limit in this region
   * @param lanelets The lanelets this speed limit applies to
   * @param areas The areas this speed limit applies to
   * @param participants The set of participants which this speed limit will apply to
   */
  DigitalSpeedLimit(Id id, Velocity speed_limit, Lanelets lanelets, Areas areas,
                    std::vector<std::string> participants);

protected:
  // the following lines are required so that lanelet2 can create this object when loading a map with this regulatory
  // element
  friend class RegisterRegulatoryElement<DigitalSpeedLimit>;
  explicit DigitalSpeedLimit(const lanelet::RegulatoryElementDataPtr& data) : RegulatoryElement(data){};
};

// Convienace Ptr Declarations
using DigitalSpeedLimitPtr = std::shared_ptr<DigitalSpeedLimit>;
using DigitalSpeedLimitConstPtr = std::shared_ptr<const DigitalSpeedLimit>;

}  // namespace lanelet