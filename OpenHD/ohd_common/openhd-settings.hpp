
#ifndef OPENHD_SETTINGS_H
#define OPENHD_SETTINGS_H

#include <exception>
#include <stdexcept>
#include <optional>
#include <string>
#include <fstream>
#include <streambuf>
#include <utility>
#include <optional>
#include <iostream>
#include <fstream>

#include "openhd-util-filesystem.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <string_view>

// from https://superuser.com/questions/631859/preferred-place-to-store-configuration-files-that-change-often
// All persistent settings are written into this directory.
//static constexpr auto BASE_PATH="/etc/openhd/";
static constexpr auto BASE_PATH="/usr/local/share/openhd/";
// for example, the unique id
static constexpr auto UNIT_ID_FILE = "/usr/local/share/openhd/unit.id";

/**
 * If the directory does not exist yet,
 * generate the directory where all persistent settings of OpenHD are stored.
 */
static void generateSettingsDirectoryIfNonExists() {
  if(!OHDFilesystemUtil::exists(BASE_PATH)){
	OHDFilesystemUtil::create_directories(BASE_PATH);
  }
  assert(OHDFilesystemUtil::exists(BASE_PATH));
}

// fucking boost, random bugged on allwinner. This is a temporary solution
static std::string create_unit_it_temporary(){
  return "01234566789";
}

/**
 * If no unit id file exists, this is the first boot of this OpenHD image on the platform.
 * In this case, generate a new random unit id, and store it persistently.
 * Then return the unit id.
 * If a unit id file already exists, read and return the unit id.
 * @return the unit id, it doesn't change during reboots of the same system.
 */
static std::string getOrCreateUnitId() {
  generateSettingsDirectoryIfNonExists();
  std::ifstream unit_id_file(UNIT_ID_FILE);
  std::string unit_id;
  if (!unit_id_file.is_open()) {
	//std::cout<<"Generating new unit id\n";
	// generate new unit id
	// See https://www.boost.org/doc/libs/1_62_0/libs/uuid/uuid.html
	//const boost::uuids::uuid _uuid = boost::uuids::random_generator()();
	//unit_id = to_string(_uuid);
        unit_id = create_unit_it_temporary();
	std::cout << "Created new unit id:[" << unit_id << "]\n";
	// and write it ot to the right file
	std::ofstream of(UNIT_ID_FILE);
	of << unit_id;
	of.close();
  } else {
	//std::cout<<"Unit id exists, reading\n";
	unit_id = std::string((std::istreambuf_iterator<char>(unit_id_file)),std::istreambuf_iterator<char>());
	std::cout << "Read unit id:[" << unit_id << "]\n";
	return unit_id;
  }
  assert(!unit_id.empty());
  return unit_id;
}

// Clean up the directory where OpenHD persistent settings are stored
// Which in turn means that all modules that follow the "create default settings when no settings are found by (HW)-id"
// will create full new default settings.
static void clean_all_settings(){
  OHDFilesystemUtil::safe_delete_directory(BASE_PATH);
  generateSettingsDirectoryIfNonExists();
}

#endif
