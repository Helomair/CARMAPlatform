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
#include <carma_wm_ctrl/Geofence.h>
#include <carma_wm_ctrl/GeofenceScheduler.h>
#include <carma_wm_ctrl/ROSTimerFactory.h>
#include <memory>
#include <chrono>
#include <ctime> 
#include <atomic>
#include "TestTimer.h"
#include "TestTimerFactory.h"

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
  GeofenceScheduler scheduler(std::make_unique<TestTimerFactory>()); // Create scheduler with test timers. Having this check helps verify that the timers do not crash on destruction
}

// This doesn't wait the right amount of time
bool waitForEqOrTimeout(double timeout_s, uint32_t expected, std::atomic<uint32_t>& actual) {
    auto start = std::chrono::system_clock::now();
    std::chrono::duration<double,  std::ratio<1,1>> sec(timeout_s);
    auto elapsed_seconds = std::chrono::duration<double>(std::chrono::system_clock::now()-start);

    while (elapsed_seconds < sec) {
      //std::cerr << " Expected:  " << expected << " Actual: " << actual << std::endl;

      if (actual.load() == expected){
        return true;
      }
      elapsed_seconds = std::chrono::system_clock::now()-start;
    }

    return false;
}

TEST(GeofenceScheduler, addGeofence)
{
  // Test adding then evaulate if the calls to active and inactive are done correctly
  // Finally test cleaing the timers
  Geofence gf;
  gf.id_ = 1;
  gf.schedule = GeofenceSchedule(
    ros::Time(1), // Schedule between 1 and 6
    ros::Time(8),
    ros::Duration(2), // Start's at 2
    ros::Duration(6), // Ends at by 6
    ros::Duration(1), // Duration of 1 and interval of two so active durations are (2-3 and 4-5)
    ros::Duration(2) 
  );
  ros::Time::setNow(ros::Time(0)); // Set current time

  GeofenceScheduler scheduler(std::make_unique<TestTimerFactory>()); // Create scheduler

  std::atomic<uint32_t> active_call_count(0);
  std::atomic<uint32_t> inactive_call_count(0);
  std::atomic<uint32_t> last_active_gf(0);
  std::atomic<uint32_t> last_inactive_gf(0);
  scheduler.onGeofenceActive([&](const Geofence& gf) {
    active_call_count.store(active_call_count.load() + 1);
    last_active_gf.store(gf.id_);
    std::cerr << "Active called for: " << gf.id_ << std::endl;
  });

  scheduler.onGeofenceInactive([&](const Geofence& gf) {
    inactive_call_count.store(inactive_call_count.load() + 1);
    last_inactive_gf.store(gf.id_);
    std::cerr << "Inactive called for: " << gf.id_ << std::endl;
  });

  ASSERT_EQ(0, active_call_count.load());
  ASSERT_EQ(0, inactive_call_count.load());
  ASSERT_EQ(0, last_active_gf.load());
  ASSERT_EQ(0, last_inactive_gf.load());

  scheduler.addGeofence(gf);

  ros::Time::setNow(ros::Time(1.0)); // Set current time

  ASSERT_EQ(0, active_call_count.load());
  ASSERT_EQ(0, inactive_call_count.load());
  ASSERT_EQ(0, last_active_gf.load());
  ASSERT_EQ(0, last_inactive_gf.load());

  ros::Time::setNow(ros::Time(2.1)); // Set current time

  std::cerr << " Here 1" << std::endl;

  ASSERT_TRUE(waitForEqOrTimeout(3.0, 1, last_active_gf));
  ASSERT_EQ(1, active_call_count.load());
  ASSERT_EQ(0, inactive_call_count.load());
  ASSERT_EQ(0, last_inactive_gf.load());

  ros::Time::setNow(ros::Time(3.1)); // Set current time

  std::cerr << " Here 2" << std::endl;

  ASSERT_TRUE(waitForEqOrTimeout(3.0, 1, last_inactive_gf));
  ASSERT_EQ(1, active_call_count.load());
  ASSERT_EQ(1, inactive_call_count.load());
  ASSERT_EQ(1, last_active_gf.load());

  ros::Time::setNow(ros::Time(3.5)); // Set current time

  std::cerr << " Here 2" << std::endl;

  ASSERT_EQ(1, active_call_count.load());
  ASSERT_EQ(1, inactive_call_count.load());
  ASSERT_EQ(1, last_active_gf.load());
  ASSERT_EQ(1, last_inactive_gf.load());

  ros::Time::setNow(ros::Time(4.2)); // Set current time

  ASSERT_TRUE(waitForEqOrTimeout(3.0, 2, active_call_count));
  ASSERT_EQ(1, inactive_call_count.load());
  ASSERT_EQ(1, last_active_gf.load());

}

}  // namespace carma_wm_ctrl