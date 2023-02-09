#ifndef OPENHD_PROFILE_H
#define OPENHD_PROFILE_H

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "include_json.hpp"
#include "openhd-settings-directories.hpp"
#include "openhd-spdlog.hpp"
#include "openhd-util.hpp"

/**
 * The profile is created on startup and then doesn't change during run time.
 * Note that while the unit id never changes between successive re-boots of OpenHD,
 * the is_air variable might change, but not during run time
 * (aka a ground pi might become an air pi when the user switches the SD card around).
 */
class OHDProfile {
 public:
  explicit OHDProfile(bool is_air,std::string unit_id1):
  is_air(is_air),unit_id(std::move(unit_id1)){};
  // Weather we run on an air or ground "pi" (air or ground system).
  // R.n this is determined by checking if there is at least one camera connected to the system
  // or by using the force_air (development) variable.
  const bool is_air;
  // The unique id of this system, it is created once then never changed again.
  const std::string unit_id;
  [[nodiscard]] bool is_ground()const{
    return !is_air;
  }
  [[nodiscard]] std::string to_string()const{
	std::stringstream ss;
	ss<<"OHDProfile{"<<(is_air ? "Air":"Ground")<<":"<<unit_id<<"}";
	return ss.str();
  }
};

namespace DProfile{

static std::shared_ptr<OHDProfile>  discover(bool is_air) {
  openhd::log::get_default()->debug("Profile::discover()");
  // We read the unit id from the persistent storage, later write it to the tmp storage json
  const auto unit_id = openhd::getOrCreateUnitId();
  // We are air pi if there is at least one camera
  auto ret=std::make_shared<OHDProfile>(is_air,unit_id);
  return ret;
}

}
#endif

