#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_

#include <vector>
#include <optional>

#include <boost/filesystem.hpp>
#include <fstream>
#include "openhd-spdlog.hpp"

#include <sys/types.h>
#include <sys/stat.h>

// boost::filesystem or std::filesystem, what a pain
// If possible, one should not use boost::filesystem or anything from boost::
// inside the project, but quickly write a wrapper here.
// We want to get rid of boost at some time.
// which should reduce build time and dependency issues on all the platforms.
// However, for std::filesystem we need c++17 AND support from the compiler for std::filesystem
// which is lagging behind immensely.

namespace OHDFilesystemUtil{

/**
 * Quite common during the discovery step, find all directory entries in a directory.
 * @param directory
 * @return the full paths of each entry in the directory.
 */
static std::vector<std::string> getAllEntriesFullPathInDirectory(const std::string& directory){
  boost::filesystem::path dev(directory.c_str());
  std::vector<std::string> ret;
  for (auto &entry: boost::filesystem::directory_iterator(dev)) {
	auto device_file = entry.path().string();
	ret.push_back(device_file);
  }
  return ret;
}

// Same as above, but returns the filenames only.
static std::vector<std::string> getAllEntriesFilenameOnlyInDirectory(const std::string& directory){
  boost::filesystem::path net(directory.c_str());
  std::vector<std::string> ret;
  for (auto &entry: boost::filesystem::directory_iterator(net)) {
	const auto interface_name = entry.path().filename().string();
	ret.push_back(interface_name);
  }
  return ret;
}

// same as boost::filesystem::exists
static bool exists(const std::string& file){
  return boost::filesystem::exists(file);
}

// These don't create top level directories recursively
static void create_directory(const std::string& directory){
  boost::filesystem::create_directory(directory.c_str());
}

// creates top level directories recursively
static void create_directories(const std::string& directory){
  boost::filesystem::create_directories(directory.c_str());
  assert(exists(directory));
}

static void safe_delete_directory(const std::string& directory){
  if(exists(directory)){
    boost::filesystem::remove_all(directory.c_str());
  }
}

// Write a text file. If the file already exists, its content is overwritten
static void write_file(const std::string& path,const std::string& content){
  try{
    std::ofstream t(path);
    t << content;
    t.close();
  }catch (std::exception& e){
    openhd::log::get_default()->warn("Cannot write file [{}]",path);
  }
}

// Read a file as text and return its content as a string.
// If the file doesn't exist, return std::nullopt
static std::optional<std::string> opt_read_file(const std::string& filename,bool log_debug=true){
  if(!exists(filename)){
    if(log_debug)openhd::log::get_default()->warn("File [{}] doesn't exist",filename);
    return std::nullopt;
  }
  try{
    std::ifstream f(filename);
    std::string str((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
    return str;
  }catch (std::exception& e){
    if(log_debug)openhd::log::get_default()->warn("Cannot read file [{}]",filename);
    return std::nullopt;
  }
}

// Similar to above, but returns empty string if file doesn't exist
static std::string read_file(const std::string& filename){
  const auto content= opt_read_file(filename);
  if(!content.has_value())return "";
  return content.value();
}

static void remove_if_existing(const std::string& filename){
  if(exists(filename)){
    boost::filesystem::remove(filename);
  }
}

static void make_file_read_write_everyone(const std::string& filename){
  if(exists(filename)){
    try{
      chmod("./myfile", S_IROTH | S_IWOTH); // enables owner to rwx file
    }catch (std::exception& e){
      openhd::log::get_default()->debug("Cannot change permissions on [{}]",filename);
    }
  }
}

// Return: remaining space in root directory
static int get_remaining_space_in_mb(){
  boost::filesystem::space_info info = boost::filesystem::space("/");
  return info.available / 1024 / 1024;
}

}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_
