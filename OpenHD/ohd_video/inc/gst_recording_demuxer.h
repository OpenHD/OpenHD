//
// Created by consti10 on 20.06.23.
//

#ifndef OPENHD_GST_RECORDING_DEMUXER_H
#define OPENHD_GST_RECORDING_DEMUXER_H

#include <memory>
#include <thread>

// Simple util class / namespace for demuxing .mkv air recording files
// uses gstreamer & command line util
class GstRecordingDemuxer {
 public:
  // Find all files that end in .mkv in the openhd videos (air recording) directory and demuxes
  // them to .mp4 for easier use.
  void demux_all_mkv_files_async();
  static GstRecordingDemuxer& instance();
 private:
  std::unique_ptr<std::thread> m_demux_thread=nullptr;
};

#endif  // OPENHD_GST_RECORDING_DEMUXER_H
