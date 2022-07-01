//
// Created by consti10 on 01.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_

#include "openhd-util.hpp"

enum class CameraType {
  RaspberryPiCSI,  // Rpi foundation CSI camera
  RaspberryPiVEYE,
  JetsonCSI,    // Any CSI camera on jetson
  RockchipCSI,  // Any CSI camera on rockchip
  // I think this is a V4l2 camera so to say, too.
  UVC,
  // this is not just a UVC camera that happens to support h264, it's the
  // standard UVC H264 that only a few cameras support, like the older models of
  // the Logitech C920. Other UVC cameras may support h264, but they do it in a
  // completely different way so we keep them separate
  UVCH264,
  IP,     // IP camera that connects via ethernet and provides a video feet at special network address
  Dummy,  // Dummy camera, is created fully in sw, for debugging ppurposes.
  Unknown
};

static std::string camera_type_to_string(const CameraType &camera_type) {
  switch (camera_type) {
    case CameraType::RaspberryPiCSI:
      return "pi-csi";
    case CameraType::RaspberryPiVEYE:
      return "pi-veye";
    case CameraType::JetsonCSI:
      return "jetson-csi";
    case CameraType::RockchipCSI:
      return "rockchip-csi";
    case CameraType::UVC:
      return "uvc";
    case CameraType::UVCH264:
      return "uvch264";
    case CameraType::IP:
      return "ip";
    case CameraType::Dummy:
      return "dummy";
    default:
      return "unknown";
  }
}

enum class VideoCodec {
  H264,
  H265,
  MJPEG,
  Unknown
};
static std::string video_codec_to_string(VideoCodec codec) {
  switch (codec) {
    case VideoCodec::H264:
      return "h264";
    case VideoCodec::H265:
      return "h265";
    case VideoCodec::MJPEG:
      return "mjpeg";
    default:
      return "unknown";
  }
}
static VideoCodec string_to_video_codec(const std::string &codec) {
  if (OHDUtil::to_uppercase(codec).find(OHDUtil::to_uppercase("h264")) !=
      std::string::npos) {
    return VideoCodec::H264;
  } else if (OHDUtil::to_uppercase(codec).find(OHDUtil::to_uppercase("h265")) !=
             std::string::npos) {
    return VideoCodec::H265;
  } else if (OHDUtil::to_uppercase(codec).find(
                 OHDUtil::to_uppercase("mjpeg")) != std::string::npos) {
    return VideoCodec::MJPEG;
  }
  return VideoCodec::Unknown;
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
