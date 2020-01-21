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
#include <carma_wm_ctrl/GeofenceSchedule.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace carma_wm_ctrl
{
bool GeofenceSchedule::scheduleExpired(const ros::Time& time) const
{
  return schedule_end < time;
}

bool GeofenceSchedule::scheduleStarted(const ros::Time& time) const
{
  return schedule_start < time;
}

// returns ros::Time(0) when the schedule is expired or the next interval will be on a different day of the week
// Argument provided as absolute time (since 1970)
ros::Time GeofenceSchedule::getNextInterval(ros::Time time) const
{
  if (scheduleExpired(time))
  {
    return ros::Time(0);  // If the schedule has expired or was never started
  }

  boost::posix_time::ptime boost_time = time.toBoost();
  boost::gregorian::date date = boost_time.date();

  if (week_day_set.find(date.day_of_week()) == week_day_set.end())
  {
    return ros::Time(0);  // This geofence is not active on this day
  }

  auto time_of_day = boost_time.time_of_day();

  // TODO
  // If time of day is between control_start and control_end
  boost::posix_time::ptime t(date, boost::posix_time::hours(0));  // Get absolute start time of the day

  ros::Time ros_time_of_day = ros::Time::fromBoost(time_of_day);
  ros::Time abs_day_start = ros::Time::fromBoost(t);
  ros::Duration cur_start = control_start;
  ros::Duration cur_end = control_end;

  constexpr int num_sec_in_day = 86400;
  const ros::Duration full_day(num_sec_in_day);

  while (cur_start < full_day && ros_time_of_day > ros::Time(cur_start.toSec()))
  {
    cur_start += control_interval;
  }

  // check if the only next interval is after the schedule end or past the end of the day
  if (abs_day_start + cur_start > schedule_end || cur_start > full_day)
  {
    return ros::Time(0);
  }

  // At this point we should have the next start time which is still within the schedule and day
  return abs_day_start + cur_start;
}
// TODO the UTC offset is provided in the geofence spec but for now we will ignore and assume all times are UTC
}  // namespace carma_wm_ctrl
