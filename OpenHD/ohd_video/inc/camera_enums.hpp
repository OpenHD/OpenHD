//
// Created by consti10 on 01.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_

#include <sstream>
#include <string>

enum class VideoCodec { H264 = 0, H265 };

static std::string video_codec_to_string(VideoCodec codec) {
  switch (codec) {
    case VideoCodec::H264:
      return "h264";
    case VideoCodec::H265:
      return "h265";
  }
  return "h264";
}
static VideoCodec video_codec_from_int(const int video_codec) {
  if (video_codec == 0) return VideoCodec::H264;
  if (video_codec == 1) return VideoCodec::H265;
  return VideoCodec::H264;  // default to h264 here
}
static_assert(static_cast<int>(VideoCodec::H264) == 0);
static_assert(static_cast<int>(VideoCodec::H265) == 1);
static int video_codec_to_int(const VideoCodec video_codec) {
  return static_cast<int>(video_codec);
}

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
  // values <0 mean something went wrong during parsing or similar.
  // 0 means auto (in some cases9
  // And for simplicity, I go with 4k and 240fps max here.
  [[nodiscard]] bool isValid() const {
    return width >= 0 && height >= 0 && framerate >= 0 && width <= 4096 &&
           height <= 2160 && framerate <= 240;
  }
  /**
   * Convert the VideoFormat into a readable string, in this format it can be
   * parsed back by regex.
   * @return the video format in a readable form.
   */
  [[nodiscard]] std::string toString() const {
    std::stringstream ss;
    ss << video_codec_to_string(videoCodec) << "|";
    ss << width << "x" << height << "@" << framerate;
    return ss.str();
  }
};

static constexpr int AIR_RECORDING_OFF = 0;
static constexpr int AIR_RECORDING_ON = 1;
static constexpr int AIR_RECORDING_AUTO_ARM_DISARM = 2;

enum class RateControlMode {
  RC_VBR,  // variable bitrate
  RC_CBR,  // constant bitrate
  RC_CRF   // constnat rate factor
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_OPENHD_CAMERA_ENUMS_H_
