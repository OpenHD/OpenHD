//
// Created by consti10 on 25.10.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_
#define OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_

#include <iomanip>
#include <sstream>
#include <string>

#include "openhd_util.h"
#include "openhd_util_filesystem.h"

namespace openhd::video {

// TODO what about the following:
// During streaming, we record to a file in /home/openhd/Videos/tmp with a
// container that supports playback even when the streaming fails unexpectedly.
// Once we are done writing to this file (for example, camera is stopped
// properly) we demux it into a more commonly used format (e.g. from .mkv to
// .mp4)

static constexpr auto RECORDINGS_PATH = "/home/openhd/Videos/";

static std::string get_localtime_string() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream ss;
  ss << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S");
  return ss.str();
}

static int
get_recording_index_track_count() {  // File to keep track of recording numbers
  const std::string recording_track_filename =
      std::string(RECORDINGS_PATH) + "track_count.txt";
  int track_count = 1;
  const auto opt_content =
      OHDFilesystemUtil::opt_read_file(recording_track_filename);
  if (opt_content.has_value()) {
    const auto last_track_count = OHDUtil::string_to_int(opt_content.value());
    if (last_track_count.has_value()) {
      track_count = last_track_count.value() + 1;
    }
  }
  OHDFilesystemUtil::write_file(recording_track_filename,
                                OHDUtil::int_as_string(track_count));
  return track_count;
}

/**
 * Creates a new not yet used filename (aka the file does not yet exists) to be
 * used for air recording.
 * @param suffix the suffix of the filename,e.g. ".avi" or ".mp4"
 */
static std::string create_unused_recording_filename(const std::string& suffix) {
  if (!OHDFilesystemUtil::exists(RECORDINGS_PATH)) {
    OHDFilesystemUtil::create_directories(RECORDINGS_PATH);
  }
  // TEMPORARY - considering how many users use RPI (where date is not reliable)
  // we just name the files ascending
  // recording-1, recording-2 ...
  // we also make sure that we always use ascending numbers, even if the user
  // deletes a video
  const int track_index = get_recording_index_track_count();
  std::stringstream ss;
  ss << RECORDINGS_PATH << "recording_" << track_index << suffix;
  return ss.str();
  /*for(int i=0;i<10000;i++){
    // Suffix might be either .
    std::stringstream filename;
    filename<<RECORDINGS_PATH;
    filename<<"recording"<<i<<suffix;
    if(!OHDFilesystemUtil::exists(filename.str())){
      return filename.str();
    }
  }
  openhd::log::get_default()->warn("Cannot create new filename, overwriting
  already existing"); std::stringstream filename; filename<<RECORDINGS_PATH;
  filename<<"recording"<<0<<suffix;
  return filename.str();*/
}

}  // namespace openhd::video

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_AIRRECORDINGFILEHELPER_HPP_
