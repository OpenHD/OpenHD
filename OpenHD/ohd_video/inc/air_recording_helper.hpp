// Created by consti10 on 25.10.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_
#define OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_

#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>  // Added for debugging output

#include "config_paths.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

namespace openhd::video {

// TODO what about the following:
// During streaming, we record to a file in /home/openhd/Videos/tmp with a
// container that supports playback even when the streaming fails unexpectedly.
// Once we are done writing to this file (for example, camera is stopped
// properly) we demux it into a more commonly used format (e.g. from .mkv to
// .mp4)

static std::string get_localtime_string() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream ss;
  ss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S");
  std::string localtime_str = ss.str();
  std::cout << "[DEBUG] Local time string: " << localtime_str << std::endl;  // Debug
  return localtime_str;
}

static std::string format_track_count(int count) {
  std::ostringstream oss;
  oss << std::setw(4) << std::setfill('0')
      << count;  // Format count with leading zeros to make it 4 digits
  std::string formatted_count = oss.str();
  std::cout << "[DEBUG] Formatted track count: " << formatted_count << std::endl;  // Debug
  return formatted_count;
}

static int get_recording_index_track_count() {
  const std::string recording_track_filename =
      std::string(getVideoPath()) + "track_count.txt";
  int track_count = 1;
  std::cout << "[DEBUG] Recording track filename: " << recording_track_filename << std::endl;  // Debug
  const auto opt_content =
      OHDFilesystemUtil::opt_read_file(recording_track_filename);

  if (opt_content.has_value()) {
    std::cout << "[DEBUG] Found existing track count file." << std::endl;  // Debug
    const auto last_track_count = OHDUtil::string_to_int(opt_content.value());
    if (last_track_count.has_value()) {
      track_count = last_track_count.value() + 1;
      std::cout << "[DEBUG] Last track count: " << last_track_count.value() << ", new track count: " << track_count << std::endl;  // Debug
    } else {
      std::cout << "[DEBUG] Failed to convert track count from file content." << std::endl;  // Debug
    }
  } else {
    std::cout << "[DEBUG] No existing track count file found. Starting at 1." << std::endl;  // Debug
  }

  OHDFilesystemUtil::write_file(recording_track_filename,
                                format_track_count(track_count));
  std::cout << "[DEBUG] Updated track count file with: " << track_count << std::endl;  // Debug
  return track_count;
}

/**
 * Creates a new not yet used filename (aka the file does not yet exists) to be
 * used for air recording.
 * @param suffix the suffix of the filename,e.g. ".avi" or ".mp4"
 */
static std::string create_unused_recording_filename(const std::string& suffix) {
  if (!OHDFilesystemUtil::exists(std::string(getConfigBasePath()))) {
    std::cout << "[DEBUG] Config base path does not exist. Creating directories." << std::endl;  // Debug
    OHDFilesystemUtil::create_directories(std::string(getConfigBasePath()));
  }
  // TEMPORARY - considering how many users use RPI (where date is not reliable)
  // we just name the files ascending
  // recording-1, recording-2 ...
  // we also make sure that we always use ascending numbers, even if the user
  // deletes a video
  const int track_index = get_recording_index_track_count();
  std::stringstream ss;
  ss << std::string(getConfigBasePath()) << "recording_" << track_index
     << suffix;
  std::string recording_filename = ss.str();
  std::cout << "[DEBUG] Created unused recording filename: " << recording_filename << std::endl;  // Debug
  return recording_filename;
  /*for(int i=0;i<10000;i++){
    // Suffix might be either .
    std::stringstream filename;
    filename<<VIDEO_PATH;
    filename<<"recording"<<i<<suffix;
    if(!OHDFilesystemUtil::exists(filename.str())){
      return filename.str();
    }
  }
  openhd::log::get_default()->warn("Cannot create new filename, overwriting
  already existing"); std::stringstream filename; filename<<VIDEO_PATH;
  filename<<"recording"<<0<<suffix;
  return filename.str();*/
}

}  // namespace openhd::video

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_
