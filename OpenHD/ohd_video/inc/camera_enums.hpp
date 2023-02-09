//
// Created by consti10 on 01.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_

#include "include_json.hpp"
#include "openhd_util.h"
#include "openhd_spdlog.hpp"

// Helper for all this json bloat

enum class CameraType {
  // only exists to have a default value, properly discovered cameras must not be of type unknown
  UNKNOWN,
  // Dummy camera, is created fully in sw, for debugging purposes.
  // NOTE: The semi-static gstreamer test video is not ideal, but by using a file or similar we'd lose the option to test different
  // resolutions, framerate(s), bitrate(s) usw.
  DUMMY_SW,
  // Rpi foundation standard/original CSI cameras,old MMAL / BROADCOM stack
  RPI_CSI_MMAL,
  // RPI VEYE on the RPI using the newer V4l2 stack, but we need to handle it manually
  // (E.g. not like a v4l2 camera, but like a special camera type with custom pipeline)
  RPI_VEYE_CSI_V4l2,
  // Any CSI camera on jetson
  JETSON_CSI,
  // Any CSI camera on rockchip
  ROCKCHIP_CSI,
  // Any CSI camera on Allwinner
  ALLWINNER_CSI,
  // UVC / V4l2 USB Camera
  UVC,
  // this is not just a UVC camera that happens to support h264, it's the
  // standard UVC H264 that only a few cameras support, like the older models of
  // the Logitech C920. Other UVC cameras may support h264, but they do it in a
  // completely different way so we keep them separate
  UVC_H264,
  // IP camera that connects via ethernet and provides a video feed at special network address
  // Cannot be auto-detected, therefore needs to be forced manually by the user (and in general, IP camera(s) are really different
  // compared to the other camera types and therefore do not integrate well in OpenHD/ohd_video)
  IP,
  // Raspberry pi only right now, a CSI camera that uses the newer libcamera stack
  RPI_CSI_LIBCAMERA,
  // rk3588 specific, incomplete
  ROCKCHIP_HDMI,
  // This camera is for developing purposes and/or for users that want to create more or less esoteric camera pipelines, e.g. by using
  // a custom script. In this mode, ohd_video acts as a completely agnostic passthrough for a camera stream that is neither managed
  // by openhd main executable nor created by openhd main executable (note: by that you loose any openhd-provided functionalities,
  // e.g. change camera settings and/or parameters.
  // To keep this API somewhat stable we only define the following:
  // Data needs to be provided by feeding h264,h265 or mjpeg encapsulated in RTP to udp port 5500 (localhost)
  CUSTOM_UNMANAGED_CAMERA
};
NLOHMANN_JSON_SERIALIZE_ENUM( CameraType, {
     {CameraType::UNKNOWN, nullptr},
     {CameraType::DUMMY_SW, "DUMMY_SW"},
     {CameraType::RPI_CSI_MMAL, "RPI_CSI_MMAL"},
     {CameraType::RPI_VEYE_CSI_V4l2, "RPI_VEYE_CSI_V4l2"},
     {CameraType::JETSON_CSI, "JETSON_CSI"},
     {CameraType::ROCKCHIP_CSI, "ROCKCHIP_CSI"},
     {CameraType::ALLWINNER_CSI, "ALLWINNER_CSI"},
     {CameraType::UVC, "UVC"},
     {CameraType::UVC_H264, "UVC_H264"},
     {CameraType::IP, "IP"},
     {CameraType::RPI_CSI_LIBCAMERA, "RPI_CSI_LIBCAMERA"},
     {CameraType::ROCKCHIP_HDMI, "ROCKCHIP_HDMI"},
     {CameraType::CUSTOM_UNMANAGED_CAMERA, "CUSTOM_UNMANAGED_CAMERA"}
 });

static std::string camera_type_to_string(const CameraType &camera_type) {
  switch (camera_type) {
    case CameraType::DUMMY_SW:
      return "DUMMY_SW";
    case CameraType::RPI_CSI_MMAL:
      return "RPI_CSI_MMAL";
    case CameraType::RPI_VEYE_CSI_V4l2:
      return "RPI_VEYE_CSI_V4l2";
    case CameraType::JETSON_CSI:
      return "JETSON_CSI";
    case CameraType::ROCKCHIP_CSI:
      return "ROCKCHIP_CSI";
    case CameraType::ALLWINNER_CSI:
      return "ALLWINNER_CSI";
    case CameraType::UVC:
      return "UVC";
    case CameraType::UVC_H264:
      return "UVC_H264";
    case CameraType::IP:
      return "IP";
    case CameraType::RPI_CSI_LIBCAMERA:
      return "RPI_CSI_LIBCAMERA";
    case CameraType::ROCKCHIP_HDMI:
      return "ROCKCHIP_HDMI";
    case CameraType::CUSTOM_UNMANAGED_CAMERA:
      return "CUSTOM_UNMANAGED_CAMERA";
    default:
      return "UNKNOWN";
  }
}

enum class VideoCodec {
  H264=0,
  H265,
  MJPEG
};

static std::string video_codec_to_string(VideoCodec codec) {
  switch (codec) {
    case VideoCodec::H264:
      return "h264";
    case VideoCodec::H265:
      return "h265";
    case VideoCodec::MJPEG:
      return "mjpeg";
  }
  return "h264";
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
    return width > 0 && height > 0 &&
           framerate > 0 && width <= 4096 && height <= 2160 && framerate <= 240;
  }
  /**
   * Convert the VideoFormat into a readable string, in this format it can be
   * parsed back by regex.
   * @return the video format in a readable form.
   */
  [[nodiscard]] std::string toString() const {
    return fmt::format("{}|{}x{}@{}",video_codec_to_string(videoCodec),width,height,framerate);
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

enum class RateControlMode {
  RC_VBR, // variable bitrate
  RC_CBR, // constant bitrate
  RC_CRF  // constnat rate factor
};

struct EndpointFormat{
  // pixel format as string, never empty
  std::string format;
  int width;
  int height;
  int fps;
  std::string debug()const{
    return fmt::format("{}|{}x{}@{}",format,width,height,fps);
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EndpointFormat,format,width,height,fps)

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
