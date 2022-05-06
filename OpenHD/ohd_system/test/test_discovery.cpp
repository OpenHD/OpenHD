#include "OHDSystem.h"

/**
 * Test the discover process of OpenHD. After running this executable, look at the json's generated under /tmp
 */
int main(int argc, char *argv[]) {

    OHDSystem::runOnceOnStartup();

    return 0;
}