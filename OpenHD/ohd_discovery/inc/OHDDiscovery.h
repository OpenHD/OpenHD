//
// Created by consti10 on 02.05.22.
//

#ifndef OPENHD_OHDDISCOVERY_H
#define OPENHD_OHDDISCOVERY_H


class OHDDiscovery {
public:
    /**
     * Writes all the system information into json files.
     * Needs to be run on every startup of OpenHD air or ground unit.
     * NOTE: Since Stephen had his microservice approach, looking at this as a whole
     * doesn't really make sense (writing json first, then reading it) but for now it will stay this way.
     * @param forceAir forces the pi to boot as air pi, even though no cameras are found.
     */
    static void runOnceOnStartup(bool forceAir);
};


#endif //OPENHD_OHDDISCOVERY_H
