#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H

#include "camera_enums.hpp"
#include "v_validate_settings.h"

// Mutable data for a discovered camera
// See camera_holder for how the settings are created the first time a camera is detected and
// changed via mavlink / openhd mavlink.

static constexpr int DEFAULT_BITRATE_KBITS = 5000;
// Quite low, at 30fps a keyframe interval of 15 means a single lost keyframe results in max. 500ms
// of interruption (assuming the next keyframe is properly received).
// Noe that if you double the FPS, you can also double the keyframe interval if you want to keep those interruptions
// at roughly 500ms
static constexpr int DEFAULT_KEYFRAME_INTERVAL = 15;
static constexpr int DEFAULT_MJPEG_QUALITY_PERCENT = 50;

static constexpr int DEFAULT_RECORDING_KBITS = 10000;
static constexpr int DEFAULT_RECORDING_QP = 26;
static constexpr RateControlMode DEFAULT_RC_MODE = RateControlMode::RC_CBR;


// Return true if the bitrate is considered sane, false otherwise
static bool check_bitrate_sane(const int bitrateKBits) {
  if (bitrateKBits <= 100 || bitrateKBits > (1000 * 1000 * 50)) {
    return false;
  }
  return true;
}

// User-selectable camera options
// These values are settings that can change dynamically at run time
// (non-deterministic)
struct CameraSettings {
  // Enable / Disable streaming for this camera
  // This can be usefully for debugging, but also when the there is suddenly a really high interference,
  // and the user wants to fly home without video, using only telemetry / HUD.
  // Default to true, otherwise we'd have conflicts with the "always a picture without changing any settings" paradigm.
  bool enable_streaming=true;
  // The video format selected by the user. If the user sets a video format that
  // isn't supported (for example, he might select h264|1920x1080@120 but the
  // camera can only do 60fps) The stream might default to the first available
  // video format. We generally default to h264|640x480@30, except for the rare case when
  // a camera cannot do this exact resolution (e.g. veye). In this case, these default params are overridden
  // the first time settings are created for a specific discovered camera (see create_default() below)
  VideoFormat streamed_video_format{VideoCodec::H264, 640, 480, 30};
  // The settings below can only be implemented on a "best effort" manner -
  // changing them does not necessarily mean the camera supports changing them. Unsupported settings have to
  // be ignored during pipeline construction
  // In general, we only try to expose these values as mavlink parameters if the camera supports them, to not
  // confuse the user.
  //
  // ----------------------------------------------------------------------------------------------------------
  //
  // The bitrate the generated stream should have. Note that not all cameras /
  // encoders support a constant bitrate, and not all encoders support all
  // bitrates, especially really low ones. How an encoder handles a specific constant bitrate is vendor specific.
  // Note that we always use a constant bitrate in OpenHD, since it is the only way to properly adjust the bitrate
  // depending on the link quality (once we have that wired up).
  // Also note that his param is for h264 and h265 - mjpeg normally does not have a bitrate param, only a quality param.
  int h26x_bitrate_kbits = DEFAULT_BITRATE_KBITS;
  // Interval (in frames) between I frames. -1 = automatic, 0 = single-keyframe , else positive values up to 2147483647
  // note that with 0 and/or sometimes -1, you can create issues like no decoded image at all,
  // since wifibroadcast relies on keyframes in regular intervals.
  // Also, some camera(s) might use a different mapping in regard to the keyframe interval than what's defined here,
  // supporting them needs different setting validation methods.
  // only valid for h264 / h265, mjpeg has no keyframe interval
  int h26x_keyframe_interval =DEFAULT_KEYFRAME_INTERVAL;
  // Type of Intra Refresh to use, -1 to disable intra refresh. R.n only supported on gst-rpicamsrc.
  // see gst-rpicamsrc for more info.
  int h26x_intra_refresh_type =-1;
  // MJPEG has no bitrate parameter, only a "quality" param. This value is only used if the
  // user selected MJPEG as its video codec
  int mjpeg_quality_percent=DEFAULT_MJPEG_QUALITY_PERCENT;
  // Only for network cameras (CameraTypeIP) URL in the rtp:// ... or similar
  std::string ip_cam_url="rtsp://admin:admin@192.168.1.168:554/0";
  // enable/disable recording to file
  Recording air_recording=Recording::DISABLED;
  //
  // Below are params that most often only affect the ISP, not the encoder
  //
  // camera rotation, only supported on rpicamsrc at the moment
  // 0 nothing, 90° to the right, 180° to the right, 270° to the right
  int camera_rotation_degree=0;
  // horizontal / vertical flip, r.n only supported on rpicamsrc
  bool horizontal_flip= false;
  bool vertical_flip=false;
  // R.n only for rpi camera, see https://gstreamer.freedesktop.org/documentation/rpicamsrc/index.html?gi-language=c
  int awb_mode=1; //default 1 (auto)
  int exposure_mode=1; //default 1 (auto)
  // default 50, range [0,100]
  int brightness_percentage=50;
  // ISO value to use (0 = Auto)
  // Integer. Range: 0 - 3200 Default: 0
  int rpi_rpicamsrc_iso=0;

  // only used on RK3588, dirty, r.n not persistent
  VideoFormat recordingFormat{VideoCodec::H264, 0, 0, 0}; // 0 means copy
  int recordingKBits = DEFAULT_RECORDING_KBITS;
  int recordingQP = DEFAULT_RECORDING_QP;
  RateControlMode recordingRCMode = DEFAULT_RC_MODE;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSettings,enable_streaming,
                                   streamed_video_format, h26x_bitrate_kbits,
                                   h26x_keyframe_interval, h26x_intra_refresh_type,mjpeg_quality_percent, ip_cam_url,air_recording,
                                   camera_rotation_degree,horizontal_flip,vertical_flip,
                                   awb_mode,exposure_mode,brightness_percentage,rpi_rpicamsrc_iso)


#endif
