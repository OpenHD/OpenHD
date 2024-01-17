//
// Created by consti10 on 19.09.23.
//
#include "ohd_video_air_generic_settings.h"
#include "include_json.hpp"
#include "openhd_platform.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AirCameraGenericSettings,switch_primary_and_secondary,
                                   dualcam_primary_video_allocated_bandwidth_perc,
                                   primary_camera_type,
                                   secondary_camera_type);

std::optional<AirCameraGenericSettings>
AirCameraGenericSettingsHolder::impl_deserialize(const std::string &file_as_string) const {
    return openhd_json_parse<AirCameraGenericSettings>(file_as_string);
}

std::string AirCameraGenericSettingsHolder::imp_serialize(const AirCameraGenericSettings &data) const {
    const nlohmann::json tmp=data;
    return tmp.dump(4);
}

AirCameraGenericSettings AirCameraGenericSettingsHolder::create_default() const {
    AirCameraGenericSettings ret{};
    ret.primary_camera_type=X_CAM_TYPE_DUMMY_SW;
    ret.secondary_camera_type=X_CAM_TYPE_DISABLED;
    if(OHDPlatform::instance().platform_type==PlatformType::RaspberryPi){
        // TODO IMAGE WRITER
        ret.primary_camera_type=X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI;
    }else if(OHDPlatform::instance().platform_type==PlatformType::Allwinner){
        ret.primary_camera_type=X_CAM_TYPE_X20_RUNCAM_NANO;
    }else if(OHDPlatform::instance().platform_type==PlatformType::Rockchip){
        ret.primary_camera_type=X_CAM_TYPE_ROCK_HDMI_IN;
    }
    return ret;
}