//
// Created by consti10 on 04.03.24.
//

#ifndef OPENHD_RPI_HDMI_TO_CSI_V4L2_HELPER_H
#define OPENHD_RPI_HDMI_TO_CSI_V4L2_HELPER_H

#include "openhd_spdlog.h"
#include "openhd_util.h"

namespace openhd::rpi::hdmi {

static void initialize_resolution(const int width, const int height, int fps) {
  openhd::log::get_default()->debug("rpi hdmi initialize_resolution {} {} {}",
                                    width, height, fps);
  // https://forums.raspberrypi.com/viewtopic.php?f=38&t=281972
  // TODO: Create a script that (Installs ?) the correct timings (file)
  OHDUtil::run_command(
      fmt::format("bash /usr/local/bin/ohd_rpi_hdmi_v4l2_setup.sh {} {} {}",
                  width, height, fps),
      {});
  // to select the currently detected timings
  // v4l2-ctl --set-dv-bt-timings query
  OHDUtil::run_command("v4l2-ctl --set-dv-bt-timings query", {});
}

}  // namespace openhd::rpi::hdmi
#endif  // OPENHD_RPI_HDMI_TO_CSI_V4L2_HELPER_H
