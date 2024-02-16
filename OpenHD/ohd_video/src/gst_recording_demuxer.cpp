//
// Created by consti10 on 20.06.23.
//

#include "gst_recording_demuxer.h"

#include <cassert>
#include <string>

#include "air_recording_helper.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

static std::string create_gst_demux_pipeline(const std::string& in_file,
                                             const std::string& out_file) {
  return fmt::format(
      "filesrc location={} ! matroskademux  ! qtmux ! filesink location={}",
      in_file, out_file);
}

static bool run_gst_pipeline(const std::string& pipeline) { return true; }

// NOTE: Requires gstreamer to be installed !
// Takes a .mkv (matroska) file as input and converts it into more manageable
// .mp4

static void demux_mkv(const std::string& in_file) {
  // if(true) return ;
  auto console = openhd::log::create_or_get("gst_demuxer");
  console->debug("Demuxing {}", in_file);
  assert(OHDFilesystemUtil::exists(in_file));
  assert(OHDUtil::endsWith(in_file, ".mkv"));
  assert(in_file.size() >= 4);
  const std::string file_without_suffix = in_file.substr(0, in_file.size() - 4);
  const std::string out_file_mp4 = file_without_suffix + ".mp4";
  console->debug("New file name: {}", out_file_mp4);
  // convert the file
  OHDUtil::run_command("gst-launch-1.0",
                       {create_gst_demux_pipeline(in_file, out_file_mp4)});
  if (!OHDFilesystemUtil::exists(out_file_mp4)) {
    // conversion probably not successful
    console->warn("Cannot convert {} to {}", in_file, out_file_mp4);
    return;
  }
  if (OHDFilesystemUtil::get_file_size_bytes(out_file_mp4) == 0) {
    // something must have gone wrong during conversion
    console->warn("Cannot demux,{} is empty", out_file_mp4);
    OHDFilesystemUtil::remove_if_existing(out_file_mp4);
    return;
  }
  // Now we can safely delete the old file
  OHDFilesystemUtil::remove_if_existing(in_file);
  // and make the new file rw everybody
  OHDFilesystemUtil::make_file_read_write_everyone(out_file_mp4);
  console->debug("Demuxing {} done", in_file);
}

// Returns all files ending in .mkv in the video recordings directory
static std::vector<std::string> get_all_mkv_video_files() {
  const auto files = OHDFilesystemUtil::getAllEntriesFullPathInDirectory(
      openhd::video::RECORDINGS_PATH);
  std::vector<std::string> files_to_convert{};
  for (const auto& file : files) {
    if (OHDUtil::endsWith(file, ".mkv")) {
      files_to_convert.push_back(file);
    }
  }
  return files_to_convert;
}

GstRecordingDemuxer::~GstRecordingDemuxer() {
  auto console = openhd::log::create_or_get("gst_demuxer");
  console->debug("~GstRecordingDemuxer, Terminating {} demux ops",
                 m_demux_ops.size());
  for (auto& demux : m_demux_ops) {
    auto demux_thread = demux.thread;
    if (demux_thread->joinable()) {
      console->debug("Waiting for demuxing to end");
      demux_thread->join();
    }
  }
}

void GstRecordingDemuxer::demux_all_remaining_mkv_files_async() {
  auto console = openhd::log::create_or_get("gst_demuxer");
  auto files_to_demux = get_all_mkv_video_files();
  for (auto& file : files_to_demux) {
    demux_mkv_file_async_threadsafe(file);
  }
}

GstRecordingDemuxer& GstRecordingDemuxer::instance() {
  static GstRecordingDemuxer demuxer;
  return demuxer;
}

void GstRecordingDemuxer::demux_mkv_file_async_threadsafe(
    std::string filename) {
  auto console = openhd::log::create_or_get("gst_demuxer");
  if (!OHDUtil::endsWith(filename, ".mkv")) {
    console->debug("{} not a .mkv file", filename);
    return;
  }
  // Check if we are already demuxing file X
  std::lock_guard<std::mutex> guard(m_demux_ops_mutex);
  auto already_demuxing = std::find_if(
      m_demux_ops.begin(), m_demux_ops.end(),
      [&filename](const DeMuxOperation& x) { return x.filename == filename; });
  if (already_demuxing == m_demux_ops.end()) {
    // not yet demuxed
    auto demux_thread = std::make_shared<std::thread>(
        [this, filename]() { demux_mkv(filename); });
    m_demux_ops.push_back({filename, demux_thread});
  } else {
    // aldrady demuxed / currently demuxing
    console->debug("Already demuxed {}", filename);
  }
}
