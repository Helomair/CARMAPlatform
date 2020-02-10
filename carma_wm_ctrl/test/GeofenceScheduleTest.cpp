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
#include <carma_wm_ctrl/GeofenceSchedule.h>

using ::testing::_;
using ::testing::A;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnArg;

namespace carma_wm_ctrl
{

  //   /**
  //  * @brief Constructor which takes in a TimerFactory. Timers from this factory will be used to generate the triggers
  //  * for goefence activity.
  //  *
  //  * @param timerFactory A pointer to a TimerFactory which can be used to generate timers for geofence triggers.
  //  */
  // GeofenceScheduler(std::unique_ptr<TimerFactory> timerFactory);

  // /**
  //  * @brief Add a geofence to the scheduler. This will cause it to trigger an event when it becomes active or goes
  //  * inactive according to its schedule
  //  *
  //  * @param geofence The geofence to be added
  //  */
  // void addGeofence(Geofence geofence);

  // /**
  //  * @brief The callback which is triggered when a geofence becomes active
  //  *        This will call the user set active_callback set from the onGeofenceActive function
  //  *
  //  * @param event The record of the timer event causing this to trigger
  //  * @param gf The geofence which is being activated
  //  * @param timer_id The id of the timer which caused this callback to occur
  //  */
  // void startGeofenceCallback(const ros::TimerEvent& event, const Geofence& gf, const int32_t timer_id);
  // /**
  //  * @brief The callback which is triggered when a geofence becomes in-active
  //  *        This will call the user set inactive_callback set from the onGeofenceInactive function
  //  *
  //  * @param event The record of the timer event causing this to trigger
  //  * @param gf The geofence which is being un-activated
  //  * @param timer_id The id of the timer which caused this callback to occur
  //  */
  // void endGeofenceCallback(const ros::TimerEvent& event, const Geofence& gf, const int32_t timer_id);
  // /**
  //  * @brief Method which allows the user to set a callback which will be triggered when a geofence becomes active
  //  *
  //  * @param active_callback The callback which will be triggered
  //  */
  // void onGeofenceActive(std::function<void(const Geofence&)> active_callback);
  // /**
  //  * @brief Method which allows the user to set a callback which will be triggered when a geofence becomes in-active
  //  *
  //  * @param inactive_callback The callback which will be triggered
  //  */
  // void onGeofenceInactive(std::function<void(const Geofence&)> inactive_callback);

  // /**
  //  * @brief Clears the expired timers from the memory of this scheduler
  //  */
  // void clearTimers();
TEST(GeofenceScheduler, Constructor)
{

}

TEST(GeofenceSchedule, scheduleStarted)
{
  GeofenceSchedule sch;
  sch.schedule_start_ = ros::Time(0);
  sch.schedule_end_ = ros::Time(1);
  sch.control_start_ = ros::Duration(0);
  sch.control_end_ = ros::Duration(1);
  sch.control_duration_ = ros::Duration(1);
  sch.control_interval_ = ros::Duration(2);

  ASSERT_TRUE(sch.scheduleStarted(ros::Time(0)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(0.9)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(1.0)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(1.1)));

  sch.schedule_start_ = ros::Time(1579882740.000);  // EST Mon Jan 24 1970 11:19:00
  sch.schedule_end_ = ros::Time(1579886340.000);    // 1 hr total duration

  ASSERT_FALSE(sch.scheduleStarted(ros::Time(1579882739.000)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(1579882740.000)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(1579882741.000)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(1579886340.000)));
  ASSERT_TRUE(sch.scheduleStarted(ros::Time(1579886341.000)));
}

TEST(GeofenceSchedule, getNextInterval)
{
  // Test before start

  GeofenceSchedule sch(
    ros::Time(1),
    ros::Time(6),
    ros::Duration(2),
    ros::Duration(3),
    ros::Duration(1),
    ros::Duration(2)  // This means the next schedule is a 4 (2+2)
  );

  // Test before control start
  ASSERT_NEAR(2.0, sch.getNextInterval(ros::Time(0)).toSec(), 0.00001);
  // Test after start but before control_start
  ASSERT_NEAR(2.0, sch.getNextInterval(ros::Time(1.5)).toSec(), 0.00001);
  // Test between first control_start and control_end
  ASSERT_NEAR(0.0, sch.getNextInterval(ros::Time(2.5)).toSec(), 0.00001);
  // Test after control ends
  ASSERT_NEAR(0.0, sch.getNextInterval(ros::Time(3.5)).toSec(), 0.00001);

  sch = GeofenceSchedule(
    ros::Time(1),
    ros::Time(6),
    ros::Duration(2),
    ros::Duration(5),
    ros::Duration(1),
    ros::Duration(2)  // This means the next schedule is a 4 (2+2)
  );
  // Test between end of first control and start of second
  ASSERT_NEAR(4.0, sch.getNextInterval(ros::Time(3.5)).toSec(), 0.00001);
  // Test between 2nd control start and control end
  ASSERT_NEAR(0.0, sch.getNextInterval(ros::Time(4.5)).toSec(), 0.00001);
  // Test after control_end
  ASSERT_NEAR(0.0, sch.getNextInterval(ros::Time(5.5)).toSec(), 0.00001);
  // Test other day of the week
  ASSERT_NEAR(0.0, sch.getNextInterval(ros::Time(90000)).toSec(), 0.00001);
  // Test after schedule end
  ASSERT_NEAR(0.0, sch.getNextInterval(ros::Time(7.0)).toSec(), 0.00001);
}
}  // namespace carma_wm_ctrl