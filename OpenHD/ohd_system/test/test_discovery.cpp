#include "OHDSystem.h"

#include <iostream>

/**
 * Test the discover process of OpenHD. After running this executable, look at the json's generated under /tmp
 */
int main(int argc, char *argv[]) {

    std::cerr<<"OHDSystem begin\n";
    OHDSystem::runOnceOnStartup();
    std::cerr<<"OHDSystem end\n";

    return 0;
}