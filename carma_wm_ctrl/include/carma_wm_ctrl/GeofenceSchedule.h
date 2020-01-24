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
#include <ros/time.h>
#include <unordered_set>
#include <boost/date_time/gregorian/greg_weekday.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>

namespace carma_wm_ctrl
{
class GeofenceSchedule
{
  ros::Time schedule_start;
  ros::Time schedule_end;

  ros::Duration control_start; // Duration from start of day
  ros::Duration control_end; // Duration from start of day
  ros::Duration control_duration;
  ros::Duration control_interval;

  std::unordered_set<boost::gregorian::greg_weekday, std::hash<int>> week_day_set;  // NOTE: If no day of week is
                                                                                    // included then all should be
  public: 

  /**
   * @brief Returns true if the schedule has expired by the provided time
   * 
   * @param time UTC time to compare
   * 
   * @return True if time > schedule_end
   */ 
  bool scheduleExpired(const ros::Time& time = ros::Time::now()) const;

  /**
   * @brief Returns true if the schedule has started by the provided time
   * 
   * @param time UTC time to compare
   * 
   * @return True if time > schedule_start
   */ 
  bool scheduleStarted(const ros::Time& time = ros::Time::now()) const;

  // returns ros::Time(0) when the schedule is expired or the next interval will be on a different day of the week
  // Argument provided as absolute time (since 1970)
  /**
   * @brief Returns the start time of the next active interval defined by this schedule
   * 
   * @param time UTC time to compare
   * 
   * @return Return the time of the next scheduled interval within the current day. Returns ros::Time(0) when the schedule is expired or the next interval will be on a different day of the week or after schedule end
   * 
   * TODO the UTC offset is provided in the geofence spec but for now we will ignore and assume all times are UTC
   */ 
  ros::Time getNextInterval(const ros::Time& time) const;
};
}