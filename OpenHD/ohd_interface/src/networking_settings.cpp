//
// Created by consti10 on 19.09.23.
//

#include "networking_settings.h"

#include "include_json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NetworkingSettings, wifi_hotspot_mode,
                                   ethernet_operating_mode);

std::optional<NetworkingSettings> NetworkingSettingsHolder::impl_deserialize(
    const std::string &file_as_string) const {
  return openhd_json_parse<NetworkingSettings>(file_as_string);
}

std::string NetworkingSettingsHolder::imp_serialize(
    const NetworkingSettings &data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}
