#include "OHDDiscovery.h"
#include "openhd-settings.hpp"
#include "openhd-camera.hpp"
#include "tests.hpp"

#include <iostream>

/**
 * Test the discover process of OpenHD. After running this executable, look at the json's generated under /tmp
 */
int main(int argc, char *argv[]) {

    OHDCommonTests::test_video_format_regex();

    generateSettingsDirectoryIfNonExists();
    const auto id1=getOrCreateUnitId();
    const auto id2=getOrCreateUnitId();
    assert(id1==id2);

    // putting it here, since where else


    OHDDiscovery::runOnceOnStartup(false);

    return 0;
}