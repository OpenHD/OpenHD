//
// Created by consti10 on 08.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_LOG_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_LOG_H_

#include "openhd-log.hpp"

//namespace mavsd{

static OpenHDLogger LogDebug(){
  return OpenHDLogger();
}

static OpenHDLogger LogWarn(){
  return OpenHDLogger();
}

static OpenHDLogger LogErr(){
  return OpenHDLogger();
}

//}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_LOG_H_
