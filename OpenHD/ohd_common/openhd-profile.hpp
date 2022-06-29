#ifndef OPENHD_PROFILE_H
#define OPENHD_PROFILE_H

#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include <fstream>

#include "json.hpp"
#include "openhd-util.hpp"
#include "openhd-log.hpp"
#include "openhd-settings.hpp"

/**
 * The profile is created on startup and then doesn't change during run time.
 * Note that while the unit id never changes between successive re-boots of OpenHD,
 * the is_air variable might change, but not during run time
 * (aka a ground pi might become an air pi when the user switches the SD card around).
 */
class OHDProfile {
 public:
  explicit OHDProfile(bool is_air=false,std::string unit_id1="0"):is_air(is_air),unit_id(std::move(unit_id1)){};
  // Weather we run on an air or ground "pi" (air or ground system).
  // R.n this is determined by checking if there is at least one camera connected to the system
  // or by using the force_air (development) variable.
  const bool is_air;
  // The unique id of this system, it is created once then never changed again.
  const std::string unit_id;
  [[nodiscard]] std::string to_string()const{
	std::stringstream ss;
	ss<<"OHDProfile{"<<(is_air ? "Air":"Ground")<<":"<<unit_id<<"}";
	return ss.str();
  }
};

static nlohmann::json profile_to_json(const OHDProfile &ohdProfile) {
  nlohmann::json j;
  j["unit-id"] = ohdProfile.unit_id;
  j["is-air"] = ohdProfile.is_air;
  return j;
}

static constexpr auto PROFILE_MANIFEST_FILENAME = "/tmp/profile_manifest";

static void write_profile_manifest(const OHDProfile &ohdProfile) {
  auto manifest = profile_to_json(ohdProfile);
  std::ofstream _t(PROFILE_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}

namespace DProfile{

static std::shared_ptr<OHDProfile>  discover(int camera_count) {
  std::cout << "Profile::discover()\n";
  // We read the unit id from the persistent storage, later write it to the tmp storage json
  auto unit_id = getOrCreateUnitId();
  // We are air pi if there is at least one camera
  auto ret=std::make_shared<OHDProfile>(camera_count > 0);
  write_profile_manifest(*ret);
  return ret;
}

}
#endif

