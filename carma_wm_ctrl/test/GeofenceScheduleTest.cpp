/*
 * Copyright (C) 2019 LEIDOS.
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

#include <gmock/gmock.h>
#include <iostream>
#include <carma_wm_ctrl/GeofenceSchedule.h>

#include "TestHelpers.h"

using ::testing::_;
using ::testing::A;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnArg;

namespace carma_wm_ctrl
{
  // ros::Time schedule_start;
  // ros::Time schedule_end;

  // ros::Duration control_start; // Duration from start of day
  // ros::Duration control_end; // Duration from start of day
  // ros::Duration control_duration;
  // ros::Duration control_interval;

TEST(GeofenceSchedule, scheduleExpired)
{
  GeofenceSchedule sch;
  sch.schedule_start = ros::Time(0);
  sch.schedule_end = ros::Time(1);
  sch.control_start = ros::Duration(0);
  sch.control_end = ros::Duration(1);
  sch.control_duration = ros::Duration(1);
  sch.control_interval = ros::Duration(2);

  ASSERT_TRUE(sch.scheduleExpired(ros::Time(1.1)));

bool scheduleExpired(const ros::Time& time = ros::Time::now()) const;
}

TEST(GeofenceSchedule, scheduleStarted)
{
//bool scheduleStarted(const ros::Time& time = ros::Time::now()) const;
}

TEST(GeofenceSchedule, getNextInterval)
{
//ros::Time getNextInterval(ros::Time time) const;
}
}