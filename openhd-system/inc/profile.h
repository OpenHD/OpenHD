#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <cstdint>

#include "platform.h"

class Profile {
public:
    Profile(PlatformType platform_type, BoardType board_type, CarrierType carrier_type);
    
    virtual ~Profile() {}

    void discover();

    std::string generate_manifest();

private:
    PlatformType m_platform_type;
    BoardType m_board_type;
    CarrierType m_carrier_type;

    uint8_t m_profile = 1;
    std::string m_settings_file = "/boot/openhd-settings-1.txt";
};

#endif

