//
// Created by consti10 on 30.01.24.
//

#include "wifi_client.h"
#include "wifi_card.h"
#include <openhd_spdlog.h>

static constexpr auto OHD_WIFI_CLIENT_CONNECTION_NAME ="ohd_wfi_client";

static std::string get_ohd_wifi_hotspot_connection_nm_filename(){
    return fmt::format("/etc/NetworkManager/system-connections/{}.nmconnection",OHD_WIFI_CLIENT_CONNECTION_NAME);
}

static constexpr auto WIFI_CLIENT_CONFIG_FILE="/boot/openhd/wifi_client.txt";

std::string WiFiClient::create_command(const WiFiClient::Configuration &configuration) {
    return fmt::format("sudo nmcli dev wifi connect \"{}\" password \"{}\"",configuration.network_name,configuration.password);
}

static std::shared_ptr<spdlog::logger> get_console(){
    return openhd::log::create_or_get("WiFiClient");
}

std::optional<WiFiClient::Configuration> WiFiClient::get_configuration() {
    const auto content=OHDFilesystemUtil::opt_read_file(WIFI_CLIENT_CONFIG_FILE);
    if(!content.has_value())return std::nullopt;
    const auto lines=OHDUtil::split_string_by_newline(content.value());
    if(lines.size()<2)return std::nullopt;
    const auto ssid=lines[0];
    const auto pw=lines[1];
    if(ssid.length()<3){
        get_console()->debug("Invalid SSID:{}",ssid);
        return std::nullopt;
    }
    if(pw.length()<6){
        get_console()->debug("Invalid PW:{}",pw);
        return std::nullopt;
    }
    return WiFiClient::Configuration{ssid,pw};
}

bool WiFiClient::create_if_enabled() {
    const auto config=get_configuration();
    if(!config.has_value()){
        openhd::log::get_default()->debug("WiFi Client disabled");
        return false;
    }
    const auto command= create_command(config.value());
    OHDUtil::run_command(command,{}, true);
    return true;
}
