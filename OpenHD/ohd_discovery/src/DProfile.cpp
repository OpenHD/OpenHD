#include <iostream>
#include <sstream>

#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"
#include "openhd-settings.hpp"

#include "DProfile.h"

DProfile::DProfile(bool is_air) :
	m_is_air(is_air) {}

std::shared_ptr<OHDProfile>  DProfile::discover(int camera_count) {
  std::cout << "Profile::discover()\n";
  // We read the unit id from the persistent storage, later write it to the tmp storage json
  auto unit_id = getOrCreateUnitId();
  auto ret=std::make_shared<OHDProfile>(camera_count > 0);
  write_profile_manifest(*ret);
  return ret;
}

