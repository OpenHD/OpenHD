#include <cstdio>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include "json.hpp"

#include "platform.h"
#include "profile.h"

extern "C" {
    #include <bcm2835.h>
}


Profile::Profile(PlatformType platform_type, BoardType board_type, CarrierType carrier_type) : 
    m_platform_type(platform_type),
    m_board_type(board_type),
    m_carrier_type(carrier_type) {}


void Profile::discover() {
    std::cout << "Profile::discover()" << std::endl;

    int input0 = true;
    int input1 = true;

    switch (m_platform_type) {
        case PlatformTypeRaspberryPi: {
            if (!bcm2835_init()) {
                throw std::runtime_error("bcm2835 gpio init failed");
            }
            bcm2835_gpio_fsel(20, BCM2835_GPIO_FSEL_INPT);
            bcm2835_gpio_fsel(21, BCM2835_GPIO_FSEL_INPT);

            bcm2835_gpio_set_pud(20, BCM2835_GPIO_PUD_UP);
            bcm2835_gpio_set_pud(21, BCM2835_GPIO_PUD_UP);

            input0 = bcm2835_gpio_lev(20);
            input1 = bcm2835_gpio_lev(21);

            break;
        }
        case PlatformTypeJetson: {

            break;
        }
        case PlatformTypeNanoPi: {
            break;
        }
        case PlatformTypeRockchip: {
            break;
        }
        case PlatformTypePC: {
            break;
        }
        case PlatformTypeUnknown: {
            break;
        }
    }

    if (input0 == 0 && input1 == 0) {
        m_settings_file = "/boot/openhd-settings-4.txt";
        m_profile = 4;
    } else if (input0 == 0 && input1 == 1) {
        m_settings_file = "/boot/openhd-settings-3.txt";
        m_profile = 3;
    } else if (input0 == 1 && input1 == 0) {
        m_settings_file = "/boot/openhd-settings-2.txt";
        m_profile = 2;
    } else if (input0 == 1 && input1 == 1) {
        m_settings_file = "/boot/openhd-settings-1.txt";
        m_profile = 1;
    }
}


std::string Profile::generate_manifest() {
    nlohmann::json j;
    
    j["profile"] = m_profile;
    j["settings-file"] = m_settings_file;

    return j.dump(4);
}
