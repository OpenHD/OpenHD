//
// Created by consti10 on 19.09.23.
//
#include "ohd_video_air_generic_settings.h"
#include "include_json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AirCameraGenericSettings,switch_primary_and_secondary,n_cameras_to_wait_for,
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
