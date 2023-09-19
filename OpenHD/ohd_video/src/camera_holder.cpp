//
// Created by consti10 on 19.09.23.
//
#include "camera_holder.h"
#include "include_json.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM( CameraType, {
    {CameraType::UNKNOWN, nullptr},
    {CameraType::DUMMY_SW, "DUMMY_SW"},
    {CameraType::RPI_CSI_MMAL, "RPI_CSI_MMAL"},
    {CameraType::RPI_CSI_VEYE_V4l2, "RPI_CSI_VEYE_V4l2"},
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

NLOHMANN_JSON_SERIALIZE_ENUM( VideoCodec, {
    {VideoCodec::H264, "h264"},
    {VideoCodec::H265, "h265"},
    {VideoCodec::MJPEG, "mjpeg"},
});

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VideoFormat,videoCodec,width,height,framerate)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EndpointFormat,format,width,height,fps)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IPCameraSettings,location,extraparam_1)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraEndpointV4l2, v4l2_device_node,bus,formats_h264,formats_h265,formats_mjpeg,formats_raw)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,type,name,vendor,sensor_name,bus,index,rpi_csi_mmal_is_csi_to_hdmi, v4l2_endpoints)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraSettings, enable_streaming,
                                   streamed_video_format, h26x_bitrate_kbits,
                                   h26x_keyframe_interval, h26x_intra_refresh_type, mjpeg_quality_percent, ip_cam_url, air_recording,
                                   camera_rotation_degree, horizontal_flip, vertical_flip,
                                   awb_mode, exposure_mode, brightness_percentage, rpi_rpicamsrc_iso, rpi_rpicamsrc_metering_mode,
                                    // rpi libcamera specific IQ params begin
                                   rpi_libcamera_sharpness_as_int,rpi_libcamera_contrast_as_int,rpi_libcamera_saturation_as_int,rpi_libcamera_ev_value,
                                   rpi_libcamera_denoise_index, rpi_libcamera_awb_index, rpi_libcamera_metering_index, rpi_libcamera_exposure_index,
                                   rpi_libcamera_shutter_microseconds,
                                    // rpi libcamera specific IQ params end
                                   force_sw_encode)

std::optional<CameraSettings> CameraHolder::impl_deserialize(const std::string &file_as_string) const {
    return openhd_json_parse<CameraSettings>(file_as_string);
}

std::string CameraHolder::imp_serialize(const CameraSettings &data) const {
    const nlohmann::json tmp=data;
    return tmp.dump(4);
}

static nlohmann::json cameras_to_json(const std::vector<Camera> &cameras) {
  nlohmann::json j;
  for (const auto &camera : cameras) {
    nlohmann::json _camera = camera;
    j.push_back(_camera);
  }
  return j;
}

static constexpr auto CAMERA_MANIFEST_FILENAME = "/tmp/camera_manifest";

void write_camera_manifest(const std::vector<Camera> &cameras) {
    auto manifest = cameras_to_json(cameras);
    OHDFilesystemUtil::write_file(CAMERA_MANIFEST_FILENAME,manifest.dump(4));
}
