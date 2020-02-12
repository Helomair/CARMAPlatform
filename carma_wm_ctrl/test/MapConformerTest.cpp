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

  //lanelet::MapConformer::ensureCompliance(map);
}
}  // namespace carma_wm_ctrl