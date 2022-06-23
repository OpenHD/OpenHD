#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <cstdint>

#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

namespace DProfile{
  static std::shared_ptr<OHDProfile> discover(int camera_count);
};

#endif

