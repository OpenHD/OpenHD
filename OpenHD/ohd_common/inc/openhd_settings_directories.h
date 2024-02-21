
#ifndef OPENHD_SETTINGS_H
#define OPENHD_SETTINGS_H

#include <string>

namespace openhd {

// from
// https://superuser.com/questions/631859/preferred-place-to-store-configuration-files-that-change-often
// All persistent settings are written into this directory.
// Jan 28 / v2.3.1 : A lot of (rpi) users complained that they cannot change
// settings manually anymore. Even though this is not recommended, we want to
// support that - and since on rpi image only /boot shows up under windows in
// the file reader, we had to change the path in this regard. Shouldn't create
// any issues on linux, since we are root, we can just cretae the directory at
// run time
// !!!! Had to be reverted - writing to /boot on rpi is too prone to file system
// corruption !!!
// static constexpr auto SETTINGS_BASE_PATH ="/boot/openhd/settings/";
static constexpr auto SETTINGS_BASE_PATH = "/usr/local/share/openhd/";
// for example, the unique id
static std::string get_unit_id_file_path() {
  return std::string(SETTINGS_BASE_PATH) + "unit.id";
}

// Interface, telemetry and video each have their own directory for settings
// to separate them logically like also done in code
static std::string get_interface_settings_directory() {
  return std::string(SETTINGS_BASE_PATH) + "interface/";
}
static std::string get_telemetry_settings_directory() {
  return std::string(SETTINGS_BASE_PATH) + "telemetry/";
}
static std::string get_video_settings_directory() {
  return std::string(SETTINGS_BASE_PATH) + "video/";
}

/**
 * If the directory does not exist yet,
 * generate the directory where all persistent settings of OpenHD are stored.
 */
void generateSettingsDirectoryIfNonExists();

// fucking boost, random bugged on allwinner. This is a temporary solution
static std::string create_unit_it_temporary() { return "01234566789"; }

/**
 * If no unit id file exists, this is the first boot of this OpenHD image on the
 * platform. In this case, generate a new random unit id, and store it
 * persistently. Then return the unit id. If a unit id file already exists, read
 * and return the unit id.
 * @return the unit id, it doesn't change during reboots of the same system.
 */
std::string getOrCreateUnitId();

// Clean up the directory where OpenHD persistent settings are stored
// Which in turn means that all modules that follow the "create default settings
// when no settings are found by (HW)-id" will create full new default settings.
void clean_all_settings();

// Helper for development - we catch 2 things with the following pattern:
// 1) When openhd is started - check if the file exists, in which case either a
// develoer started openhd twice (which most likely was a mistake) or the
// previous openhd execution did not terminate properly (which is only a soft
// error, since properly terminating is a nice to have but not necessarily
// required) 2) When openhd is stopped (SIGTERM) - remove the file
static std::string get_openhd_is_running_filename() {
  return "/tmp/openhd_is_running.txt";
}

void check_currently_running_file_and_write();

void remove_currently_running_file();

}  // namespace openhd

#endif
