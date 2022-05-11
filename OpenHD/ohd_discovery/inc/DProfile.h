#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <cstdint>

#include "DPlatform.h"

#include "json.hpp"

#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

/**
 * Discover the profile we are running on and write it to json.
 */
class DProfile: public OHD::IDiscoverable {
public:
    DProfile(bool is_air);
    virtual ~DProfile() = default;

    void discover() override;

    nlohmann::json generate_manifest() override;

    std::string unit_id() {
        return m_unit_id;
    }

private:
    const bool m_is_air;
    std::string m_unit_id;
};

#endif

