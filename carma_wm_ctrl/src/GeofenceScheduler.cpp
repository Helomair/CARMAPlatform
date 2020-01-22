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

#include <carma_wm_ctrl/GeofenceScheduler.h>

namespace carma_wm_ctrl
{
using std::placeholders::_1;

GeofenceScheduler::GeofenceScheduler(std::unique_ptr<TimerFactory> timerFactory)
  : timerFactory_(std::move(timerFactory))
{
  // Create repeating loop to clear geofence timers which are no longer needed
  deletion_timer_ =
      timerFactory_->buildTimer(nextId(), ros::Duration(1), std::bind(&GeofenceScheduler::clearTimers, this));
}

uint32_t GeofenceScheduler::nextId()
{
  std::lock_guard<std::mutex> guard(mutex_);
  next_id_++;
  return next_id_;
}

void GeofenceScheduler::clearTimers()
{
  std::lock_guard<std::mutex> guard(mutex_);
  for (const auto& key_val_pair : timers)
  {
    if (key_val_pair.second.second)
    {
      // This timer should be erased
      timers.erase(key_val_pair.first);  // TODO ensure this is a safe in loop deletion
    }
  }
}

void GeofenceScheduler::addGeofence(Geofence geofence)
{
  std::lock_guard<std::mutex> guard(mutex_);
  // Create timer for next start time
  ros::Time startTime = geofence.schedule.getNextInterval(ros::Time::now());
  // TODO check startTime is valid
  int32_t timer_id = nextId();
  TimerPtr timer = timerFactory_->buildTimer(
      timer_id, startTime - ros::Time::now(),
      std::bind(&GeofenceScheduler::startGeofenceCallback, this, _1, geofence, timer_id), true, true);
  timers[timer->getId()] = std::make_pair(std::move(timer), false);  // Add start timer to map by Id
}

void GeofenceScheduler::startGeofenceCallback(const ros::TimerEvent& event, const Geofence& gf, const int32_t timer_id)
{
  std::lock_guard<std::mutex> guard(mutex_);

  active_callback_(gf);
  ros::Time endTime = gf.schedule.getNextInterval(ros::Time::now());  // TODO there might be some challenges with this
                                                                      // call and small intervals
  int32_t ending_timer_id = nextId();
  TimerPtr timer = timerFactory_->buildTimer(
      ending_timer_id, endTime - ros::Time::now(),
      std::bind(&GeofenceScheduler::endGeofenceCallback, this, _1, gf, ending_timer_id), true, true);
  timers[timer->getId()] = std::make_pair(std::move(timer), false);  // Add end timer to map by Id

  timers[timer_id].second = true;  // Mark timer for deletion
}

void GeofenceScheduler::endGeofenceCallback(const ros::TimerEvent& event, const Geofence& gf, const int32_t timer_id)
{
  std::lock_guard<std::mutex> guard(mutex_);
  inactive_callback_(gf);
  timers[timer_id].second = true;  // Mark timer for deletion
}

void GeofenceScheduler::onGeofenceActive(std::function<void(const Geofence&)> active_callback)
{
  std::lock_guard<std::mutex> guard(mutex_);
  active_callback_ = active_callback;
}

void GeofenceScheduler::onGeofenceInactive(std::function<void(const Geofence&)> inactive_callback)
{
  std::lock_guard<std::mutex> guard(mutex_);
  inactive_callback_ = inactive_callback;
}
}  // namespace carma_wm_ctrl