#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <cstdint>

#include "platform.h"

#include "json.hpp"

#include "openhd-platform.hpp"


class Profile {
public:
    Profile(PlatformType platform_type, BoardType board_type, CarrierType carrier_type, int camera_count);
    
    virtual ~Profile() {}

    void discover();

    nlohmann::json generate_manifest();

private:
    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;

    int m_camera_count = 0;

    uint8_t m_profile = 1;
    std::string m_settings_file = "/boot/openhd-settings-1.txt";
};

#endif

