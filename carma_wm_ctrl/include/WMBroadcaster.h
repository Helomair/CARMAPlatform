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

#include <functional>
#include <mutex>
#include <ros/ros.h>
#include <ros/callback_queue.h>
#include <carma_wm/WorldModel.h>
#include <carma_utils/CARMAUtils.h>

namespace carma_wm
{
class WMBroadcasterWorker;  // Forward declaration of worker class

/*!
 * \brief Class which provies exposes map publication and carma_wm update logic
 *
 * The WMBroadcaster handles updating the lanelet2 base map and publishing the new versions to the rest of the CARMA Platform ROS network.
 * The broadcaster also provides functions for adding or removing geofences from the map and notifying the rest of the system. 
 *
 */
class WMBroadcaster
{
public:
  /*!
   * \brief Constructor 
   */
  WMBroadcaster();

  /*!
   * \brief Returns a pointer to an intialized world model instance
   *
   * \return Const pointer to a world model object
   */
  void getWorldModel();

  /*!
   * \brief Allows user to set a callback to be triggered when a map update is received
   *        NOTE: If operating in multi-threaded mode the world model will remain locked until the user function
   * completes.
   *
   * \param callback A callback function that will be triggered after the world model receives a new map update
   */
  void setMapCallback(std::function<void()> callback);

  /*!
   * \brief Allows user to set a callback to be triggered when a route update is received
   *        NOTE: If operating in multi-threaded mode the world model will remain locked until the user function
   * completes.
   *
   * \param callback A callback function that will be triggered after the world model is updated with a new route
   */
  void setRouteCallback(std::function<void()> callback);

  /*!
   * \brief Returns a unique_lock which can be used to lock world model updates until the user finishes a desired
   * operation. This function should be used when multiple queries are needed and this object is operating in
   * multi-threaded mode
   *
   * \param pre_locked If true the returned lock will already be locked. If false the lock will be deferred with
   * std::defer_lock Default is true
   *
   * \return std::unique_lock for thread safe access to world model data
   */
  std::unique_lock<std::mutex> getLock(bool pre_locked = true);

private:
  std::unique_ptr<WMListenerWorker> worker_;
  ros::CARMANodeHandle nh_;
  ros::CallbackQueue async_queue_;
  std::unique_ptr<ros::AsyncSpinner> wm_spinner_;
  ros::Subscriber map_sub_;
  ros::Subscriber route_sub_;
  const bool multi_threaded_;
  std::mutex mw_mutex_;
};
}  // namespace carma_wm