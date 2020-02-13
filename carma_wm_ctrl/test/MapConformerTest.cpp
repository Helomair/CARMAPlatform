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
#include <carma_wm_ctrl/MapConformer.h>
#include <carma_wm/lanelet/CarmaUSTrafficRules.h>
#include "TestHelpers.h"

using ::testing::_;
using ::testing::A;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnArg;

namespace carma_wm_ctrl
{
/**
 * @brief Function modifies an existing map to make a best effort attempt at ensuring the map confroms to the expectations of CarmaUSTrafficRules
 * 
 * Map is updated by ensuring all lanelet and area bounds are marked with PassingControlLines
 * In addition, lanelets and areas are updated to have their accessability marked with a RegionAccessRule.
 * At the moment the creation of DigitalSpeedLimits for all lanelets/areas is not performed. This is because CarmaUSTrafficRules supports the existing SpeedLimit definition and allows DigitalSpeedLimits to be overlayed on that.
 * 
 * @param map A pointer to the map which will be modified in place
 */ 
//void ensureCompliance(lanelet::LaneletMapPtr map);
TEST(MapConformer, ensureCompliance)
{
  auto map = carma_wm::getDisjointRouteMap();

  for (auto reg : map->regulatoryElementLayer) {
    FAIL() << "There should be no regulations in the map at this point";
  }

  // // TODO we are here
  // // This runs without exceptions now, but we need at least a bit more validation on the functionality then this
  // lanelet::MapConformer::ensureCompliance(map);

  // // First verify that each lanelet has a left and right control line
  // ASSERT_EQ(3, map->laneletLayer.size());
  // for (auto ll : map->laneletLayer) {
  //   auto control_lines = ll.regulatoryElementsAs<lanelet::PassingControlLine>();
  //   ASSERT_EQ(2, control_lines.size());

  //   if (ll.id() == 10000) { // First lanelet in disjoint route
  //     ASSERT_FALSE(lanelet::PassingControlLine::boundPassable(ll.leftBound(),
  //                         control_lines, false,
  //                         lanelet::Participants::Vehicle));

  //     ASSERT_TRUE(lanelet::PassingControlLine::boundPassable(ll.rightBound(),
  //                          control_lines, true,
  //                          lanelet::Participants::Vehicle));
  //   } else if (ll.id() == 10001) {

  //     ASSERT_TRUE(lanelet::PassingControlLine::boundPassable(ll.leftBound(),
  //                         control_lines, false,
  //                         lanelet::Participants::Vehicle));

  //     ASSERT_FALSE(lanelet::PassingControlLine::boundPassable(ll.rightBound(),
  //                          control_lines, true,
  //                          lanelet::Participants::Vehicle));
  //   } else if (ll.id() == 10002) {
  //     ASSERT_FALSE(lanelet::PassingControlLine::boundPassable(ll.leftBound(),
  //                         control_lines, false,
  //                         lanelet::Participants::Vehicle));

  //     ASSERT_FALSE(lanelet::PassingControlLine::boundPassable(ll.rightBound(),
  //                          control_lines, true,
  //                          lanelet::Participants::Vehicle)); 
  //   } else {
  //     FAIL() << "The base map used in TEST(MapConformer, ensureCompliance) has changed. The unit test must be updated";
  //   }
  // }
  
  // Then verify that routing can still be done properly over this map
  // Build routing graph from map
  lanelet::traffic_rules::TrafficRulesUPtr traffic_rules = lanelet::traffic_rules::TrafficRulesFactory::create(
      lanelet::Locations::Germany, lanelet::Participants::VehicleCar);
  // TODO how to build CarmaUSTrafficRules?
    
  lanelet::routing::RoutingGraphUPtr map_graph = lanelet::routing::RoutingGraph::build(*map, *traffic_rules);
  map_graph->exportGraphViz("my_graph");

  // 4. Generate route
  auto ll_1 = map->laneletLayer.find(10000);
  auto ll_3 = map->laneletLayer.find(10002);
  std::cerr << "It1 Id: " << (*ll_1).id() << "  It3 Id: " << (*ll_3).id() << std::endl;
  auto optional_route = map_graph->getRoute(*ll_1, *ll_3);
  ASSERT_TRUE((bool)optional_route); // Routing is possible
  // map_graph->exportGraphViz("my_graph"); // Uncomment to visualize route graph

}
}  // namespace carma_wm_ctrl