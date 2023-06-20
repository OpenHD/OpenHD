//
// Created by consti10 on 09.02.23.
//

#include "openhd_util_filesystem.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>
#include <fstream>
#include <optional>
#include <vector>

#include "openhd_spdlog.h"

std::vector<std::string> OHDFilesystemUtil::getAllEntriesFullPathInDirectory(
    const std::string &directory) {
  if(!OHDFilesystemUtil::exists(directory)){
    return {};
  }
  boost::filesystem::path dev(directory.c_str());
  std::vector<std::string> ret;
  for (auto &entry: boost::filesystem::directory_iterator(dev)) {
    auto device_file = entry.path().string();
    ret.push_back(device_file);
  }
  return ret;
}

std::vector<std::string>
OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory(
    const std::string &directory) {
  boost::filesystem::path net(directory.c_str());
  std::vector<std::string> ret;
  for (auto &entry: boost::filesystem::directory_iterator(net)) {
    const auto interface_name = entry.path().filename().string();
    ret.push_back(interface_name);
  }
  return ret;
}

bool OHDFilesystemUtil::exists(const std::string &file) {
  return boost::filesystem::exists(file);
}

void OHDFilesystemUtil::create_directory(const std::string &directory) {
  boost::filesystem::create_directory(directory.c_str());
}

void OHDFilesystemUtil::create_directories(const std::string &directory) {
  boost::filesystem::create_directories(directory.c_str());
  assert(exists(directory));
}

void OHDFilesystemUtil::safe_delete_directory(const std::string &directory) {
  if(exists(directory)){
    boost::filesystem::remove_all(directory.c_str());
  }
}

void OHDFilesystemUtil::write_file(const std::string &path,
                                   const std::string &content) {
  try{
    std::ofstream t(path);
    if(!t.good()){
      openhd::log::get_default()->warn("Cannot open file [{}]",path);
    }
    t << content;
    t.close();
    if(!t.good()){
      openhd::log::get_default()->warn("Cannot write file [{}]",path);
    }
  }catch (std::exception& e){
    openhd::log::get_default()->warn("Cannot write file [{}] {}",path,e.what());
  }
}

std::optional<std::string> OHDFilesystemUtil::opt_read_file(
    const std::string &filename, bool log_debug) {
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

std::string OHDFilesystemUtil::read_file(const std::string &filename) {
  const auto content= opt_read_file(filename);
  if(!content.has_value())return "";
  return content.value();
}

void OHDFilesystemUtil::remove_if_existing(const std::string &filename) {
  if(exists(filename)){
    boost::filesystem::remove(filename);
  }
}

void OHDFilesystemUtil::make_file_read_write_everyone(
    const std::string &filename) {
  if(!exists(filename)){
    openhd::log::get_default()->warn("Cannot change file {} rw anybody, file does not exist",filename);
    return ;
  }
  const auto res= chmod(filename.c_str(), S_IROTH | S_IWOTH);
  if(res!=0){
    openhd::log::get_default()->warn("Cannot change file {} rw anybody, ret:{}",filename,res);
  }
}

int OHDFilesystemUtil::get_remaining_space_in_mb() {
  boost::filesystem::space_info info = boost::filesystem::space("/");
  return info.available / 1024 / 1024;
}
