#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <cstdint>

#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-discoverable.hpp"

class DProfile{
 public:
  static std::shared_ptr<OHDProfile> discover(int camera_count);
};

#endif

