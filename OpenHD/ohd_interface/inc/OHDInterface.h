//
// Created by consti10 on 02.05.22.
//

#ifndef OPENHD_OPENHD_INTERFACE_H
#define OPENHD_OPENHD_INTERFACE_H

#include <memory>

#include "WBStreams.h"
#include "WifiCards.h"

class OHDInterface {
public:
    /**
     * Note: due to json, needs to be crated after OHDSystem has been run once.
     * The constructor reads the generated json by OHDSystem and tries to initialize all its members.
     * NOTE: We need to wire link - related settings into here.
     * For example, changing the wifi frequency (needs to be synced between air and ground) but also
     * settings that can be unsinced (like enable / disable wifi hotspot, which - as an example - is only
     * a setting that affects the ground pi.
     * @param is_air true if we run on the air pi, ground pi otherwise. Note that some (TODO settings) might be different for
     * air or ground pi, for example wifi hotspot can only be enabled on the ground pi.
     * @param unit_id ?? Stephen no idea ?
     */
    OHDInterface(bool is_air,std::string unit_id);
    std::unique_ptr<WifiCards> wifi;
    // temprarily removed
    //std::unique_ptr<EthernetCards> ethernet;
    std::unique_ptr<WBStreams> streams;
private:
    const bool is_air;
    const std::string unit_id;
};


#endif //OPENHD_OPENHD_INTERFACE_H
