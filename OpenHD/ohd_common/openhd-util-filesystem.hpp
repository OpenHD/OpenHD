#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_

#include <vector>

#include <boost/filesystem.hpp>

// boost::filesystem or std::filesystem, what a pain

namespace OHDFilesystemUtil{

/**
 * Quite common during the discovery step, find all directory entries in a directory.
 * @param directory
 * @return the full paths of each entry in the directory.
 */
static std::vector<std::string> getAllEntriesFullPathInDirectory(const char* directory){
  boost::filesystem::path dev(directory);
  std::vector<std::string> ret;
  for (auto &entry: boost::filesystem::directory_iterator(dev)) {
	auto device_file = entry.path().string();
	ret.push_back(device_file);
  }
  return ret;
}

// Same as above, but returns the filenames only.
static std::vector<std::string> getAllEntriesFilenameOnlyInDirectory(const char* directory){
  boost::filesystem::path net(directory);
  std::vector<std::string> ret;
  for (auto &entry: boost::filesystem::directory_iterator(net)) {
	const auto interface_name = entry.path().filename().string();
	ret.push_back(interface_name);
  }
  return ret;
}


}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_
