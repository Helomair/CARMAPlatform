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
  // TODO undo comment out
  deletion_timer_ =
      timerFactory_->buildTimer(nextId(), ros::Duration(1), std::bind(&GeofenceScheduler::clearTimers, this));
}

uint32_t GeofenceScheduler::nextId()
{
  next_id_++;
  return next_id_;
}

void GeofenceScheduler::clearTimers()
{
  std::cerr << "Clearing" << std::endl;
  std::lock_guard<std::mutex> guard(mutex_);
  std::cerr << "Clearing 2" << std::endl;
  auto it = timers_.begin();
 
	// Erase all expired timers_ using iterators to ensure operation is safe
	while (it != timers_.end()) {
		// Check if timer is marked for deletion
		if (it->second.second) {
			// erase() function returns the iterator of the next
			// to last deleted element.
      std::cerr << "Removed timer" << std::endl;
			it = timers_.erase(it);
		} else {
			it++;
    }
	}
  std::cerr << "Done Clear" << std::endl;
}

void GeofenceScheduler::addGeofence(Geofence geofence)
{std::cerr << "Enter: 0" << std::endl;
  std::lock_guard<std::mutex> guard(mutex_);
  // Create timer for next start time
  ros::Time startTime = geofence.schedule.getNextInterval(ros::Time::now());
  std::cerr << "StartTime: " << startTime.toSec() << std::endl;
  std::cerr << "ROS Time: " << ros::Time::now().toSec() << std::endl;
  std::cerr << "Duration: " << (startTime.toSec() - ros::Time::now().toSec()) << std::endl;
  // TODO check startTime is valid
  int32_t timer_id = nextId();
  std::cerr << "Factory: " << (!!timerFactory_) << std::endl;
  TimerPtr timer = timerFactory_->buildTimer(
      timer_id, startTime - ros::Time::now(),
      std::bind(&GeofenceScheduler::startGeofenceCallback, this, _1, geofence, timer_id), true, true);
  std::cerr << "timer: " << (!!timer) << std::endl;

std::cerr << "ID --" << std::endl;
  auto id = timer->getId();

  std::cerr << "ID -- " << id << std::endl;

std::cerr << "Pair --" << std::endl;

  //auto pair = std::make_pair(std::move(timer), false);

std::cerr << "Access --" << std::endl;

  timers_[timer_id] = std::make_pair(std::move(timer), false);
  //timers_.insert({timer_id, std::make_pair(std::move(timer), false)});  // Add start timer to map by Id
  std::cerr << "Exit 1: " << std::endl;
}

void GeofenceScheduler::startGeofenceCallback(const ros::TimerEvent& event, const Geofence& gf, const int32_t timer_id)
{
  std::cerr << "Enter 2: " << std::endl;
  std::lock_guard<std::mutex> guard(mutex_);

  active_callback_(gf);
  ros::Time endTime = gf.schedule.getNextInterval(ros::Time::now());  // TODO there might be some challenges with this
                                                                      // call and small intervals
  int32_t ending_timer_id = nextId();
  TimerPtr timer = timerFactory_->buildTimer(
      ending_timer_id, endTime - ros::Time::now(),
      std::bind(&GeofenceScheduler::endGeofenceCallback, this, _1, gf, ending_timer_id), true, true);
  timers_[ending_timer_id] = std::make_pair(std::move(timer), false);  // Add end timer to map by Id

  timers_[timer_id].second = true;  // Mark start timer for deletion
  std::cerr << "Exit 3: " << std::endl;
}

void GeofenceScheduler::endGeofenceCallback(const ros::TimerEvent& event, const Geofence& gf, const int32_t timer_id)
{
  std::cerr << "Enter 4: "<< std::endl;
  std::lock_guard<std::mutex> guard(mutex_);
  inactive_callback_(gf);
  timers_[timer_id].second = true;  // Mark timer for deletion
  std::cerr << "Exit 5: " << std::endl;
}

void GeofenceScheduler::onGeofenceActive(std::function<void(const Geofence&)> active_callback)
{
  std::cerr << "Enter 8: " << std::endl;
  std::lock_guard<std::mutex> guard(mutex_);
  active_callback_ = active_callback;
  std::cerr << "Exit 9: " << std::endl;
}

void GeofenceScheduler::onGeofenceInactive(std::function<void(const Geofence&)> inactive_callback)
{
  std::cerr << "Enter 10: " << std::endl;
  std::lock_guard<std::mutex> guard(mutex_);
  inactive_callback_ = inactive_callback;
  std::cerr << "Exit 11: " << std::endl;
}
}  // namespace carma_wm_ctrl