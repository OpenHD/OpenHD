#include "OHDDiscovery.h"
#include "openhd-settings.hpp"

#include <iostream>

/**
 * Test the discover process of OpenHD. After running this executable, look at the json's generated under /tmp
 */
int main(int argc, char *argv[]) {

    generateSettingsDirectoryIfNonExists();
    const auto id1=getOrCreateUnitId();
    const auto id2=getOrCreateUnitId();
    assert(id1==id2);

    std::cerr<<"OHDSystem begin\n";
    OHDDiscovery::runOnceOnStartup(false);
    std::cerr<<"OHDSystem end\n";

    return 0;
}