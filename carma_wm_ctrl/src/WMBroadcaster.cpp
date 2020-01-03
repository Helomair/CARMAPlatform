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

#include <functional>
#include <mutex>
#include <carma_wm_ctrl/WMBroadcaster.h>
#include <lanelet2_extension/utility/message_conversion.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <type_traits>

// TODO add ros logging

// TODO consider applying the lanelet2_extension for overwriting lanelet centerlines

// TODO remove includes below here
#include <carma_wm_ctrl/ROSTimerFactory.h>
#include <carma_wm_ctrl/GeofenceScheduler.h>

namespace carma_wm_ctrl  // TODO should this be carma_wm or carma_wm_ctrl?
{


// TODO HERE 1/2/20 The most pressing issue is that logic be added for adding geofences (non-geometric) to the map. Then broadcasting an update. 
// This has the following requirements
// 1. New update message be created
// 2. Updates be applied to local map copy and tracked
// 3. Updates be published when applied.
// 4. Updates be removed when applied. 

WMBroadcaster::WMBroadcaster(PublishMapCallback map_pub, std::unique_ptr<TimerFactory> timer_factory) 
  : map_pub_(map_pub), scheduler_(std::move(timer_factory)) {  
  
  scheduler_.onGeofenceActive(std::bind(&WMBroadcaster::addGeofence, this, _1));
  scheduler_.onGeofenceInactive(std::bind(&WMBroadcaster::removeGeofence, this, _1));

};

void WMBroadcaster::baseMapCallback(const autoware_lanelet2_msgs::MapBinConstPtr& map_msg) {
  std::lock_guard<std::mutex> guard(map_mutex_);

  lanelet::LaneletMapPtr new_map(new lanelet::LaneletMap);

  lanelet::utils::conversion::fromBinMsg(*map_msg, new_map);

  // TODO consider saving origianal map as well as updated map for geometry updates
  base_map_ = new_map;
  //current_map_ = lanelet::createMap();
  // TODO warning if this is called multiple times?
};

void WMBroadcaster::geofenceCallback(/*TODO*/){
  std::lock_guard<std::mutex> guard(map_mutex_);
  Geofence gf;
  scheduler_.addGeofence(gf); // Add the geofence to the schedule
};

void WMBroadcaster::addGeofence(const Geofence& gf){
  std::lock_guard<std::mutex> guard(map_mutex_);
  // Add the geofence to the map
  // 1. Identify elements to be modified
  // -- Use geofence centerline to find impacted elemtents

  for (auto basic_point : gf.centerline) {
    auto vec_of_pair_dist_lanelet = lanelet::geometry::findNearest(base_map_->laneletLayer, lanelet::utils::to2D(basic_point), 1);
    if (vec_of_pair_dist_lanelet.empty()) {
      // TODO throw exception
    }
    double distance = vec_of_pair_dist_lanelet[0].first;
    auto nearest_lanelet = vec_of_pair_dist_lanelet[0].second;
    // Check for intersection
    if (!lanelet::geometry::inside(nearest_lanelet, lanelet::utils::to2D(basic_point))) {
      // No intersection so do not add
      // TODO what to do here
    }
    // Intersection so add this lanelet to set of lanelets
    
  }

  // 2. Add them to tracked geofences
  // 3. Update the map TODO this implies value in using carma_wm 
  // 4. Publish update message 
};

void WMBroadcaster::removeGeofence(const Geofence& gf){
  std::lock_guard<std::mutex> guard(map_mutex_);
};

}  // namespace carma_wm_ctrl
