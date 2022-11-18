//
// Created by consti10 on 25.10.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_
#define OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_

#include <string>
#include <sstream>
#include "openhd-util-filesystem.hpp"
#include "openhd-util.hpp"

namespace openhd::video{

static constexpr auto RECORDINGS_PATH="/home/openhd/Videos/";

/**
 * Creates a new not yet used filename (aka the file does not yet exists) to be used for air recording.
 * @param suffix the suffix of the filename,e.g. ".avi" or ".mp4"
 */
static std::string create_unused_recording_filename(const std::string& suffix){
  if(!OHDFilesystemUtil::exists(RECORDINGS_PATH)){
    OHDFilesystemUtil::create_directories(RECORDINGS_PATH);
  }
  for(int i=0;i<10000;i++){
    std::stringstream filename;
    filename<<RECORDINGS_PATH;
    filename<<"recording"<<i<<suffix;
    if(!OHDFilesystemUtil::exists(filename.str().c_str())){
      return filename.str();
    }
  }
  openhd::log::get_default()->warn("Cannot create new filename, overwriting already existing");
  std::stringstream filename;
  filename<<RECORDINGS_PATH;
  filename<<"recording"<<0<<suffix;
  return filename.str();
}

}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_
