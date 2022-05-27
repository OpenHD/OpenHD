#include "OHDDiscovery.h"
#include "openhd-settings.hpp"
#include "openhd-camera.hpp"
#include "openhd-common-tests.hpp"

#include <iostream>

/**
 * Test the discover process of OpenHD. After running this executable, look at the json's generated under /tmp
 */
int main(int argc, char *argv[]) {

  const auto id1 = getOrCreateUnitId();
  const auto id2 = getOrCreateUnitId();
  assert(id1 == id2);

  OHDDiscovery::runOnceOnStartup(false, false);

  return 0;
}