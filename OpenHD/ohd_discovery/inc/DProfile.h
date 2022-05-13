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
    /**
     *
     * @param is_air weather we run as air or ground pi is decided elsewhere, we just take it as a param here
     * and write this param out to json.
     */
    explicit DProfile(bool is_air);
    virtual ~DProfile() = default;

    void discover() override;

    void write_manifest() override;

    std::string unit_id() {
        return m_unit_id;
    }

private:
    const bool m_is_air;
    std::string m_unit_id;
};

#endif

