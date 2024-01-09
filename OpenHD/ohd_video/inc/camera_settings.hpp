#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H

#include "camera_enums.hpp"
#include "v_validate_settings.h"

// Mutable data for a discovered camera
// See camera_holder for how the settings are created the first time a camera is detected and
// changed via mavlink / openhd mavlink.

// For the default value, we assume a fec overhead of 20% - 8MBit/s before FEC fits well into MCS index 3, even
// on highly polluted channels (we account for the worst here)
static constexpr int DEFAULT_BITRATE_KBITS = 8000;
// The ideal value is not definitive, and depends on the rf environment, the FEC percentage, and the camera fps
// Higher values result in less key frames, and better image quality at the same bitrate, but increases the risk for
// "stuttering" in case frames are lost.
static constexpr int DEFAULT_KEYFRAME_INTERVAL = 5;
static constexpr int DEFAULT_MJPEG_QUALITY_PERCENT = 50;

static constexpr int DEFAULT_RECORDING_KBITS = 10000;
static constexpr int DEFAULT_RECORDING_QP = 26;
static constexpr RateControlMode DEFAULT_RC_MODE = RateControlMode::RC_CBR;

// Minimum amount of free space required to enable air video recording.
// Also, If the free space becomes less than that, air recording (if running) should be stopped.
// This feature is r.n already implemented for all cameras (in gstreamerstream)
static constexpr auto MINIMUM_AMOUNT_FREE_SPACE_FOR_AIR_RECORDING_MB=300;

static constexpr int RPI_LIBCAMERA_DEFAULT_CONTRAST_AS_INT=100;
static constexpr int RPI_LIBCAMERA_DEFAULT_SATURATION_AS_INT=100;
static constexpr int RPI_LIBCAMERA_DEFAULT_SHARPNESS_AS_INT=100;
static constexpr int RPI_LIBCAMERA_DEFAULT_EV=0;

// Return true if the bitrate is considered sane, false otherwise
static bool check_bitrate_sane(const int bitrateKBits) {
  if (bitrateKBits <= 100 || bitrateKBits > (1000 * 1000 * 50)) {
    return false;
  }
  return true;
}

// IP cams are a bit different regarding their settings.
// These just map to corresponding gst rtsp settings
struct IPCameraSettings{
  std::string location="rtsp://admin:admin@192.168.2.176:554/stream=0";
  // Whatever the user wants
  std::string extraparam_1;
};

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
  // camera can only do 60fps) the camera might stop streaming, and the user has to set a different resolution manually
  // (In general, we cannot really check if a camera supports a given resolution / framerate properly yet)
  // Note that this default value is overridden in case we know more about the camera(s).
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
  // Type of Intra Refresh to use, -1 to disable intra refresh. R.n only supported on gst-rpicamsrc and sw encoder
  // See gst-rpicamsrc for more info on mmal (there we have different intra options)
  // sw encoder only has off (-1) and on (anything not -1)
  int h26x_intra_refresh_type =-1;
  // MJPEG has no bitrate parameter, only a "quality" param. This value is only used if the
  // user selected MJPEG as its video codec
  int mjpeg_quality_percent=DEFAULT_MJPEG_QUALITY_PERCENT;
  // Only for network cameras (CameraTypeIP) URL in the rtp:// ... or similar
  std::string ip_cam_url="rtsp://admin:admin@192.168.1.168:554/0";
  // enable/disable recording to file
  int air_recording=AIR_RECORDING_OFF;
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
  // Camera exposure metering mode to use
  // Default 0 (average)
  int rpi_rpicamsrc_metering_mode=0;
  // libcamera params
  // The following 3 params are float in libcamera - but floats suck, int or percentage is much more verbose to the user
  // and easier to map as parameters. Which is why we have a different mapping on those parameters.
  int rpi_libcamera_sharpness_as_int=RPI_LIBCAMERA_DEFAULT_SHARPNESS_AS_INT; //NOTE: float in libcamera
  int rpi_libcamera_contrast_as_int=RPI_LIBCAMERA_DEFAULT_CONTRAST_AS_INT; //NOTE: float in libcamera
  int rpi_libcamera_saturation_as_int=RPI_LIBCAMERA_DEFAULT_SATURATION_AS_INT; //NOTE: float in libcamera
  int rpi_libcamera_ev_value=RPI_LIBCAMERA_DEFAULT_EV;
  int rpi_libcamera_denoise_index=0;
  int rpi_libcamera_awb_index=0; // 0=Auto
  int rpi_libcamera_metering_index=0; // 0=centre
  int rpi_libcamera_exposure_index=0; // 0=normal
  int rpi_libcamera_shutter_microseconds=0; // 0= auto

  // Depending on the cam type, openhd uses hw-accelerated encoding whenever possible.
  // However, in some cases (e.g. when using a USB camera that outputs raw and h264, but the hw encoder of the cam is bad)
  // or for experimenting (e.g. when using libcamera / rpicamsrc and RPI4) one might prefer to use SW encode.
  // Enabling this is no guarantee a sw encoded pipeline exists for this camera.
  bool force_sw_encode=false;

  // only used on RK3588, dirty, r.n not persistent
  VideoFormat recordingFormat{VideoCodec::H264, 0, 0, 0}; // 0 means copy
  int recordingKBits = DEFAULT_RECORDING_KBITS;
  int recordingQP = DEFAULT_RECORDING_QP;
  RateControlMode recordingRCMode = DEFAULT_RC_MODE;
  // OpenHD WB supports changing encryption on the fly per camera stream
  bool enable_ultra_secure_encryption= false;
  // these are customizable settings
  int custom_script_value0=0;
  int custom_script_value1=0;
};



#endif
