#include <openhd-settings.hpp>
#include "DProfile.h"

#include "openhd-util.hpp"

std::shared_ptr<OHDProfile>  DProfile::discover(int camera_count) {
  std::cout << "Profile::discover()\n";
  // We read the unit id from the persistent storage, later write it to the tmp storage json
  auto unit_id = getOrCreateUnitId();
  // We are air pi if there is at least one camera
  auto ret=std::make_shared<OHDProfile>(camera_count > 0);
  return ret;
}

