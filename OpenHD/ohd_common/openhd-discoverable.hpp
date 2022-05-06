//
// Created by consti10 on 06.05.22.
//

#ifndef OPENHD_OPENHD_DISCOVERABLE_HPP
#define OPENHD_OPENHD_DISCOVERABLE_HPP

#include "json.hpp"

namespace OHD{
    /**
     * This is a pure virtual interface for classes that perform a "discovery" step.
     * The discovery step should be performed for all subclasses at startup, and we run OpenHD
     * under the assumption that the connected hardware does not change during run time.
     * (If a user wants to connect new hardware, like a new camera or wifi card, he needs to de-power the system first).
     * Note: One cannot assume that the connected hardware stays the same between reboot.
     */
    class IDiscoverable{
    public:
        /**
         * Discover the connected hardware at startup.
         */
        virtual void discover()=0;
        /**
         * Write out a manifest that contains the information about the discovered hardware.
         * @return a json file with key-value pairs, the key-value-pairs are sub-class specific.
         */
        virtual nlohmann::json generate_manifest()=0;
    };
}
#endif //OPENHD_OPENHD_DISCOVERABLE_HPP
