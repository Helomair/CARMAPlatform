#include "carma_wm_ctrl/WMBroadcaster.h"

namespace carma_wm_ctrl {

  using std::placeholders::_1;

  void publishMap(const autoware_lanelet2_msgs::MapBin&) {
    // do publish
  }



void myFunc() {
 // Publisher p;
  WMBroadcaster wm(std::bind(publishMap, _1));
}

}