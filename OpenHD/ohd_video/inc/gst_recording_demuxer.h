//
// Created by consti10 on 20.06.23.
//

#ifndef OPENHD_GST_RECORDING_DEMUXER_H
#define OPENHD_GST_RECORDING_DEMUXER_H

#include <memory>
#include <thread>

class GstRecordingDemuxer {
 public:
  static void demux_all_mkv_files();
  void demux_all_mkv_files_async();
 private:
  std::unique_ptr<std::thread> m_demux_thread=nullptr;
};

#endif  // OPENHD_GST_RECORDING_DEMUXER_H
