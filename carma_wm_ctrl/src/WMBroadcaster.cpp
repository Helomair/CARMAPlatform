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
#include <carma_wm_ctrl/WMBroadcaster.h>
#include <lanelet2_extension/utility/message_conversion.h>
#include <type_traits>



// TODO remove includes below here
#include <carma_wm_ctrl/ROSTimerFactory.h>
#include <carma_wm_ctrl/GeofenceScheduler.h>

namespace carma_wm_ctrl  // TODO should this be carma_wm or carma_wm_ctrl?
{
WMBroadcaster::WMBroadcaster(PublishMapCallback map_pub) : map_pub_(map_pub) {
  
};

void WMBroadcaster::baseMapCallback(const autoware_lanelet2_msgs::MapBinConstPtr& map_msg) {
  std::lock_guard<std::mutex> guard(map_mutex_);

  lanelet::LaneletMapPtr new_map(new lanelet::LaneletMap);

  lanelet::utils::conversion::fromBinMsg(*map_msg, new_map);

  semantic_map_ = new_map;
  // TODO warning if this is called multiple times?
};

void WMBroadcaster::geofenceCallback(/*TODO*/){
  std::lock_guard<std::mutex> guard(map_mutex_);
};

void WMBroadcaster::addGeofence(/*TODO*/){
  std::lock_guard<std::mutex> guard(map_mutex_);
};

void WMBroadcaster::removeGeofence(/*TODO*/){
  std::lock_guard<std::mutex> guard(map_mutex_);
};

}  // namespace carma_wm_ctrl
