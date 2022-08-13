//
// Created by consti10 on 13.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_VEYE_HELPER_H_
#define OPENHD_OPENHD_OHD_COMMON_VEYE_HELPER_H_

#include "openhd-util.hpp"

// NOTE: This is a bad software design, but r.n the veye is kinda special and I hacked it in "working" in the quickest way possible.
// Please do not use this as an example on how to do cameras, look at gstreamerstream ;)

namespace openhd::veye{

// TODO: does this also kill OpenHD, since it is started from OpenHD ?!
static void kill_all_running_veye_instances(){
  OHDUtil::run_command("killall ",{"\"/usr/local/share/veye-raspberrypi/veye_raspivid\""});
}

}
#endif //OPENHD_OPENHD_OHD_COMMON_VEYE_HELPER_H_
