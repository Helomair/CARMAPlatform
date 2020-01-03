#include "carma_wm_ctrl/WMBroadcaster.h"
#include "carma_wm_ctrl/ROSTimerFactory.h"

namespace carma_wm_ctrl {

  using std::placeholders::_1;

  void publishMap(const autoware_lanelet2_msgs::MapBin&) {
    // do publish
  }



void myFunc() {
 // Publisher p;
  std::unique_ptr<ROSTimerFactory> factory = std::make_unique<ROSTimerFactory>();
  WMBroadcaster wm(std::bind(publishMap, _1), std::move(factory));
}

}