//
// Created by consti10 on 14.02.24.
//

#ifndef OPENHD_X20_IMAGE_QUALITY_HELPER_H
#define OPENHD_X20_IMAGE_QUALITY_HELPER_H

#include "camera_settings.hpp"
#include "openhd_util.h"
#include "openhd_spdlog.h"

namespace openhd::x20{
// TODO: Right now we do not have a proper mapping between ranges, only none / some
static std::optional<int> get_x20_contrast(const CameraSettings& settings){
  if(settings.openhd_contrast==OPENHD_CONTRAST_DEFAULT)return std::nullopt;
  if(settings.openhd_contrast>OPENHD_CONTRAST_DEFAULT)return 3;
}
static std::optional<int> get_x20_saturation(const CameraSettings& settings){
  if(settings.openhd_saturation==OPENHD_SATURATION_DEFAULT)return std::nullopt;
  if(settings.openhd_contrast>OPENHD_CONTRAST_DEFAULT)return 5;
}
static std::optional<int> get_x20_flip(const CameraSettings& settings){
  if(settings.openhd_flip==OPENHD_FLIP_NONE)return std::nullopt;
  return settings.openhd_flip;
}

// On allwinner / X20 we set IQ params with scripts
static void apply_x20_runcam_iq_settings(const CameraSettings& settings){
 openhd::log::get_default()->debug("apply_x20_runcam_iq_settings begin");
 const auto flip= get_x20_flip(settings);
 if(flip.has_value()){
   std::stringstream ss;
   ss<<"bash /usr/local/bin/x20/runcam_v2/runcam_flip.sh "<<flip.value();
   OHDUtil::run_command(ss.str(),{});
 }
 const auto contrast= get_x20_contrast(settings);
 if(contrast.has_value()){
   std::stringstream ss;
   ss<<"bash /usr/local/bin/x20/runcam_v2/runcam_contrast.sh "<<contrast.value();
   OHDUtil::run_command(ss.str(),{});
 }
 const auto saturation= get_x20_saturation(settings);
 if(saturation.has_value()){
   std::stringstream ss;
   ss<<"bash /usr/local/bin/x20/runcam_v2/runcam_saturation.sh "<<saturation.value();
   OHDUtil::run_command(ss.str(),{});
 }
 openhd::log::get_default()->debug("apply_x20_runcam_iq_settings end");
}

}

#endif  // OPENHD_X20_IMAGE_QUALITY_HELPER_H
