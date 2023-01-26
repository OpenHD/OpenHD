//
// Created by consti10 on 26.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_GST_BITRATE_CONTROLL_WRAPPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_GST_BITRATE_CONTROLL_WRAPPER_H_

#include <gst/gst.h>
#include <optional>
#include "openhd-spdlog.hpp"

// Bitrate is one of the few params we want to support changing dynamically at run time
// without the need for a pipeline restart.
// This just wraps the differences for those pipelines nicely
struct GstBitrateControlElement {
  // Some elements take kbit/s, some take bit/s
  bool takes_kbit=false;
  // the encoder (or similar) element, must not be null
  GstElement *encoder;
  // Not all encoders / elements call the bitrate property "bitrate"
  std::string property_name="bitrate";
};

static std::optional<GstBitrateControlElement> get_dynamic_bitrate_control_element_in_pipeline(GstElement *gst_pipeline,CameraType camera_type){
  GstBitrateControlElement ret{};
  ret.encoder= nullptr;
  if(camera_type==CameraType::RPI_CSI_MMAL){
    ret.encoder= gst_bin_get_by_name(GST_BIN(gst_pipeline), "rpicamsrc");
    ret.property_name="bitrate";
    ret.takes_kbit= false;
  }else if(camera_type==CameraType::DUMMY_SW || camera_type==CameraType::UVC){
    // NOTE: With UVC we might not do sw encode - in which case we cannot do variable bitrate control
    ret.encoder= gst_bin_get_by_name(GST_BIN(gst_pipeline), "swencoder");
    ret.property_name="bitrate";
    ret.takes_kbit= true;
  }else if(camera_type==CameraType::RPI_CSI_LIBCAMERA || camera_type==CameraType::RPI_VEYE_CSI_V4l2){
    // ARGH - cannot change extra-controls without restart
    //ret.encoder= gst_bin_get_by_name(GST_BIN(gst_pipeline), "rpi_v4l2_encoder");
    //ret.property_name="video_bitrate";
    //ret.takes_kbit= false;
  }
  if(ret.encoder== nullptr){
    openhd::log::get_default()->debug("Cannot find dynamic bitrate control element for camera {}", camera_type_to_string(camera_type));
    return std::nullopt;
  }
  // try fetching the value for testing if it actually works
  gint actual_bits_per_second=-1;
  g_object_get(ret.encoder, ret.property_name.c_str(),&actual_bits_per_second,NULL);
  if(actual_bits_per_second==-1){
    openhd::log::get_default()->warn("dynamic bitrate control element doesn't work");
    return std::nullopt;
  }
  openhd::log::get_default()->info("Got bitrate control for camera {}, current:{}",camera_type_to_string(camera_type),actual_bits_per_second);
  return ret;
}

static bool change_bitrate(const GstBitrateControlElement& ctrl_el,int bitrate_kbits){
  const auto bitrate=ctrl_el.takes_kbit ? bitrate_kbits : kbits_to_bits_per_second(bitrate_kbits);
  g_object_set(ctrl_el.encoder, ctrl_el.property_name.c_str(), bitrate, NULL);
  gint actual_bits_per_second=-1;
  g_object_get(ctrl_el.encoder, ctrl_el.property_name.c_str(),&actual_bits_per_second,NULL);
  if(actual_bits_per_second!=bitrate){
    openhd::log::get_default()->warn("Cannot change bitrate to {} kbit/s, got {}",bitrate_kbits,actual_bits_per_second);
    return false;
  }
  openhd::log::get_default()->debug("Changed bitrate to {} kbit/s",bitrate_kbits);
  return true;
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_GST_BITRATE_CONTROLL_WRAPPER_H_
