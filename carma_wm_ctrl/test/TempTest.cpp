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
#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>
#include <lanelet2_core/Attribute.h>

#include <exception>
#include <memory>
#include <tuple>
#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/Area.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/primitives/Point.h>
#include <lanelet2_routing/Route.h>
#include <lanelet2_routing/RoutingGraph.h>
#include "TestHelpers.h"

using ::testing::_;
using ::testing::A;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnArg;

using LaneletRoutePtr = std::shared_ptr<lanelet::routing::Route>;
using LaneletRouteConstPtr = std::shared_ptr<const lanelet::routing::Route>;
using LaneletRouteUPtr = std::unique_ptr<lanelet::routing::Route>;
using LaneletRouteUConstPtr = std::unique_ptr<const lanelet::routing::Route>;

using LaneletRoutingGraphPtr = std::shared_ptr<lanelet::routing::RoutingGraph>;
using LaneletRoutingGraphConstPtr = std::shared_ptr<const lanelet::routing::RoutingGraph>;
using LaneletRoutingGraphUPtr = std::unique_ptr<lanelet::routing::RoutingGraph>;
using LaneletRoutingGraphConstUPtr = std::unique_ptr<const lanelet::routing::RoutingGraph>;

namespace carma_wm
{
TEST(Temp, Temp1)
{

  auto pl1 = getPoint(0, 0, 0);
  auto pl2 = getPoint(0, 1, 0);
  auto pl3 = getPoint(0, 2, 0);
  auto pr1 = getPoint(1, 0, 0);
  auto pr2 = getPoint(1, 1, 0);
  auto pr3 = getPoint(1, 2, 0);
  std::vector<lanelet::Point3d> left_1 = { pl1, pl2 };
  std::vector<lanelet::Point3d> right_1 = { pr1, pr2 };
  auto ll_1 = getLanelet(left_1, right_1);

  std::vector<lanelet::Point3d> left_2 = { pl2, pl3 };
  std::vector<lanelet::Point3d> right_2 = { pr2, pr3 };
  auto ll_2 = getLanelet(left_2, right_2);

  // 2. Build map but do not assign
  // Create basic map and verify that the map and routing graph can be build, but the route remains false
  lanelet::LaneletMapPtr map = lanelet::utils::createMap({ ll_1, ll_2 }, {});

  // 3. Build routing graph but do not assign
  // Build routing graph from map
  lanelet::traffic_rules::TrafficRulesUPtr traffic_rules = lanelet::traffic_rules::TrafficRulesFactory::create(
      lanelet::Locations::Germany, lanelet::Participants::VehicleCar);
  lanelet::routing::RoutingGraphUPtr map_graph = lanelet::routing::RoutingGraph::build(*map, *traffic_rules);

  std::cerr << map->laneletLayer[0].regulatoryElements() << 

  // 4. Generate route
  auto optional_route = map_graph->getRoute(ll_1, ll_2);
  ASSERT_TRUE((bool)optional_route);
  lanelet::routing::Route route = std::move(*optional_route);
  LaneletRoutePtr route_ptr = std::make_shared<lanelet::routing::Route>(std::move(route));
}
}