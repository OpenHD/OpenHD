
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

//#include <boost/filesystem.hpp>
#include <filesystem>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

// from https://superuser.com/questions/631859/preferred-place-to-store-configuration-files-that-change-often
// All persistent settings are written into this directory.
static constexpr auto BASE_PATH="/etc/openhd/";
// for example, the unique id
static const auto UNIT_ID_FILE = std::string(BASE_PATH) + "unit.id";

/**
 * If the directory does not exist yet,
 * generate the directory where all persistent settings of OpenHD are stored.
 */
static void generateSettingsDirectoryIfNonExists() {
  if (!std::filesystem::exists(BASE_PATH)) {
	std::cout << "Creating settings directory\n";
	if (!std::filesystem::create_directory(BASE_PATH)) {
	  std::cerr << "Cannot create settings directory\n";
	}
  }
  assert(std::filesystem::exists(BASE_PATH));
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
	const boost::uuids::uuid _uuid = boost::uuids::random_generator()();
	unit_id = to_string(_uuid);
	std::cout << "Created new unit id:[" << unit_id << "]\n";
	// and write it ot to the right file
	std::ofstream of(UNIT_ID_FILE);
	of << unit_id;
	of.close();
  } else {
	//std::cout<<"Unit id exists, reading\n";
	unit_id = std::string((std::istreambuf_iterator<char>(unit_id_file)),
						  std::istreambuf_iterator<char>());
	std::cout << "Read unit id:[" << unit_id << "]\n";
	return unit_id;
  }
  assert(!unit_id.empty());
  return unit_id;
}

/**
 * The settings are stored in a directory called air_$unit_id or ground_$unit_id.
 * @return the settings directory, created newly if non existent. As an example, it will return a path like
 * this: BASE_PATH/air_8bfff348-c17e-4833-af66-cef83f90c208/
 */
static std::string findOrCreateSettingsDirectory(bool is_air) {
  generateSettingsDirectoryIfNonExists();
  std::stringstream settingsPath;
  settingsPath << BASE_PATH;
  settingsPath << (is_air ? "air_" : "ground_");
  const auto unit_id = getOrCreateUnitId();
  settingsPath << unit_id;
  auto str = settingsPath.str();
  std::cout << "SettingsDirectory:[" << str << "]\n";
  // create the directory if it is non existing
  if (!std::filesystem::exists(str.c_str())) {
	std::filesystem::create_directory(str.c_str());
  }
  assert(std::filesystem::exists(str.c_str()));
  return str;
}

#endif
