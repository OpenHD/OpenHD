//
// Created by consti10 on 01.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_

#include "json.hpp"
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
NLOHMANN_JSON_SERIALIZE_ENUM( CameraType, {
     {CameraType::Unknown, nullptr},
     {CameraType::RaspberryPiCSI, "RaspberryPiCSI"},
     {CameraType::RaspberryPiVEYE, "RaspberryPiVEYE"},
     {CameraType::JetsonCSI, "JetsonCSI"},
     {CameraType::RockchipCSI, "RockchipCSI"},
     {CameraType::UVC, "UVC"},
     {CameraType::UVCH264, "UVCH264"},
     {CameraType::IP, "IP"},
     {CameraType::Dummy, "Dummy"},
 });

static std::string camera_type_to_string(const CameraType &camera_type) {
  switch (camera_type) {
    case CameraType::RaspberryPiCSI:
      return "RaspberryPiCSI";
    case CameraType::RaspberryPiVEYE:
      return "RaspberryPiVEYE";
    case CameraType::JetsonCSI:
      return "JetsonCSI";
    case CameraType::RockchipCSI:
      return "RockchipCSI";
    case CameraType::UVC:
      return "UVC";
    case CameraType::UVCH264:
      return "UVCH264";
    case CameraType::IP:
      return "IP";
    case CameraType::Dummy:
      return "Dummy";
    default:
      return "unknown";
  }
}

enum class VideoCodec {
  H264=0,
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
static VideoCodec video_codec_from_int(const int video_codec){
  if(video_codec==0)return VideoCodec::H264;
  if(video_codec==1)return VideoCodec::H265;
  if(video_codec==2)return VideoCodec::MJPEG;
  return VideoCodec::H264; // default to h264 here
}
static int video_codec_to_int(const VideoCodec video_codec){
    return static_cast<int>(video_codec);
}

NLOHMANN_JSON_SERIALIZE_ENUM( VideoCodec, {
  {VideoCodec::Unknown, nullptr},
  {VideoCodec::H264, "h264"},
  {VideoCodec::H265, "h265"},
  {VideoCodec::MJPEG, "mjpeg"},
});

// A video format refers to a selected configuration supported by OpenHD.
// It is possible that a camera cannot do the selected configuration in HW,
// In this case a sw encoder can be used (in case of low res streams, that will
// work even on the pi). Example string: h264|1280x720@60 This class also
// provides a safe way to cast from/to a readable string. However, in case
// someone manually enters a wrong string (for example h264OOPS|1280x720@60) the
// behaviour is undefined.
struct VideoFormat {
  // The video codec, we default to h264
  VideoCodec videoCodec = VideoCodec::H264;
  // The width of this stream, in pixels
  int width = 640;
  // The height of this stream, in pixels
  int height = 480;
  // The framerate of this stream, in frames per second.
  int framerate = 30;
  // For checking if 2 video formats are exactly the same
  bool operator==(const VideoFormat &o) const {
    return this->width == o.width && this->height == o.height &&
           this->framerate == o.framerate;
  }
  // Return true if the Video format is valid, aka the values set "make sense".
  // values <=0 mean something went wrong during parsing or similar. And for
  // simplicity, I go with 4k and 240fps max here.
  [[nodiscard]] bool isValid() const {
    return videoCodec != VideoCodec::Unknown && width > 0 && height > 0 &&
           framerate > 0 && width <= 4096 && height <= 2160 && framerate <= 240;
  }
  /**
   * Convert the VideoFormat into a readable string, in this format it can be
   * parsed back by regex.
   * @return the video format in a readable form.
   */
  [[nodiscard]] std::string toString() const {
    std::stringstream ss;
    ss << video_codec_to_string(videoCodec) << "|" << width << "x" << height
       << "@" << framerate;
    return ss.str();
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoFormat,videoCodec,width,height,framerate)

enum class Recording{
  DISABLED,
  ENABLED
};
NLOHMANN_JSON_SERIALIZE_ENUM( Recording, {
  {Recording::DISABLED, nullptr},
  {Recording::ENABLED, "ENABLED"},
});
static Recording recording_from_int(int32_t value){
  if(value==1)return Recording::ENABLED;
  return Recording::DISABLED;
}
static int32_t recording_to_int(const Recording& recording){
  if(recording==Recording::ENABLED)return 1;
  return 0;
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
