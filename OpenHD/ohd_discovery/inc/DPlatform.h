#ifndef PLATFORM_H
#define PLATFORM_H

#include <array>
#include <chrono>

#include "json.hpp"

#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover the platform we are running on and write it to json.
 * Note: One should not use a instance of this class for anything else than discovery, to pass around the discovered
 * data use the struct from ohd_platform.
 */
class DPlatform: public OHD::IDiscoverable{
public:
    DPlatform()=default;
    virtual ~DPlatform() = default;

    void discover() override;

    void write_manifest() override;

    PlatformType platform_type() {
        return m_platform_type;
    }

    BoardType board_type() {
        return m_board_type;
    }

    CarrierType carrier_type() {
        return m_carrier_type;
    }
private:
    void detect_raspberrypi();
    void detect_jetson();
    void detect_pc();
    // This data is written by this class as information is gathered.
    //OHDPlatform ohdPlatform;
    PlatformType m_platform_type = PlatformTypeUnknown;
    BoardType m_board_type = BoardTypeUnknown;
    CarrierType m_carrier_type = CarrierTypeNone;
};

#endif
