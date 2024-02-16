//
// Created by consti10 on 20.06.23.
//

#ifndef OPENHD_GST_RECORDING_DEMUXER_H
#define OPENHD_GST_RECORDING_DEMUXER_H

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Simple util class / namespace for demuxing .mkv air recording files
// uses gstreamer & command line util
class GstRecordingDemuxer {
 public:
  ~GstRecordingDemuxer();
  // Find all files that end in .mkv in the openhd videos (air recording)
  // directory and then spawns a new thread to demux the file (unless it is
  // already being demuxed)
  void demux_all_remaining_mkv_files_async();
  // demux a specific .mkv file (async) unless it is already being demuxed
  // thread-safe
  void demux_mkv_file_async_threadsafe(std::string filename);
  static GstRecordingDemuxer& instance();

 private:
  struct DeMuxOperation {
    std::string filename;
    std::shared_ptr<std::thread> thread;
  };
  std::vector<DeMuxOperation> m_demux_ops{};
  // makes sure we spawn exactly one demux thread per file
  std::mutex m_demux_ops_mutex;
};

#endif  // OPENHD_GST_RECORDING_DEMUXER_H
