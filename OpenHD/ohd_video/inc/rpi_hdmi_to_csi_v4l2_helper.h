//
// Created by consti10 on 04.03.24.
//

#ifndef OPENHD_RPI_HDMI_TO_CSI_V4L2_HELPER_H
#define OPENHD_RPI_HDMI_TO_CSI_V4L2_HELPER_H

#include "openhd_spdlog.h"

namespace openhd::rpi::hdmi {

static void initialize_resolution(const int width, const int height,int fps){
  openhd::log::get_default()->debug("rpi hdmi initialize_resolution {} {} {}",width,height,fps);
}

}
#endif  // OPENHD_RPI_HDMI_TO_CSI_V4L2_HELPER_H
