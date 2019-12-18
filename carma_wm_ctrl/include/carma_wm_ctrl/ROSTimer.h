#pragma once
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
#include <ros/node_handle.h>
#include <ros/timer.h>
#include <ros/time.h>
#include "Timer.h"

namespace carma_wm_ctrl
{
class ROSTimer : public Timer
{
  ros::Timer timer_;
  ros::NodeHandle nh_;
  uint32_t id_ = 0;

public:
  ~ROSTimer(){};

  void initializeTimer(ros::Duration duration, std::function<void(const ros::TimerEvent&)> callback,
                       bool oneshot = false, bool autostart = true)
  {
    timer_.stop();
    timer_ = nh_.createTimer(duration, callback, oneshot, autostart);
  }

  void start()
  {
    timer_.start();
  }

  void stop()
  {
    timer_.stop();
  }

  uint32_t getId()
  {
    return id_;
  }

  void setId(uint32_t id)
  {
    id_ = id;
  }

  bool callbacksQueued() {
    return timer_.hasPending();
  }
};

// To support simulation I need to be connected to /clock
// To be connected to /clock the ros::Timer must be used
// The geofence scheduler needs to build ros::Timer objects as needed
// These can be worked with directly but only if created by a factory
// To unit test I need to be able to build ros::Timers which means the factory would need to be able to build them which
// I can't do. So I would need to interface both the factory and the timer lass

// WIP Geofence class described according to "Geofences for Roadway Controls for CARMAcloud Version 1.0" November 18
// 2019
}  // namespace carma_wm_ctrl