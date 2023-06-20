//
// Created by consti10 on 20.06.23.
//

#include "gst_recording_demuxer.h"

#include <cassert>
#include <string>

#include "openhd_util_filesystem.h"
#include "openhd_util.h"
#include "openhd_spdlog.h"
#include "air_recording_helper.hpp"

static std::string create_gst_demux_pipeline(const std::string& in_file,const std::string& out_file){
  return fmt::format("filesrc location={} ! matroskademux  ! qtmux ! filesink location={}",in_file,out_file);
}

static bool run_gst_pipeline(const std::string& pipeline){
  return true;
}


// NOTE: Requires gstreamer to be installed !
// Takes a .mkv (matroska) file as input and converts it into more manageable
// .mp4

static void demux_mkv(const std::string& in_file){
  auto console=openhd::log::create_or_get("gst_demuxer");
  console->debug("Demuxing {}", in_file);
  assert(OHDFilesystemUtil::exists(in_file));
  assert(OHDUtil::endsWith(in_file,".mkv"));
  assert(in_file.size()>=4);
  const std::string file_without_suffix= in_file.substr(0, in_file.size()-4);
  const std::string out_file_mp4=file_without_suffix+".mp4";
  console->debug("New file name: {}",out_file_mp4);
  // convert the file
  OHDUtil::run_command("gst-launch-1.0", {create_gst_demux_pipeline(in_file,out_file_mp4)});
  if(!OHDFilesystemUtil::exists(out_file_mp4)){
    // conversion probably not successful
    console->warn("Cannot convert {} to {}", in_file,out_file_mp4);
    return ;
  }
  // Now we can safely delete the old file
  OHDFilesystemUtil::remove_if_existing(in_file);
  // and make the new file rw everybody
  OHDFilesystemUtil::make_file_read_write_everyone(out_file_mp4);
  console->debug("Demuxing {} done", in_file);
}

static void demux_all_mkv_files_in_video_directory(){
  auto console=openhd::log::create_or_get("gst_demuxer");
  const auto files=OHDFilesystemUtil::getAllEntriesFullPathInDirectory(openhd::video::RECORDINGS_PATH);
  std::vector<std::string> files_to_convert{};
  for(const auto& file: files){
    if(OHDUtil::endsWith(file,".mkv")){
      files_to_convert.push_back(file);
    }
  }
  console->debug("Need to convert {} .mkv files",files_to_convert.size());
  for(const auto& file: files_to_convert){
    demux_mkv(file);
  }
}

void GstRecordingDemuxer::demux_all_mkv_files_async() {
  auto console=openhd::log::create_or_get("gst_demuxer");
  if(m_demux_thread!= nullptr){
    console->warn("Waiting for previous demuxing to end");
    if(m_demux_thread->joinable()){
      m_demux_thread->join();
    }
    m_demux_thread= nullptr;
  }
  m_demux_thread=std::make_unique<std::thread>([this](){
    demux_all_mkv_files_in_video_directory();
  });
}

GstRecordingDemuxer& GstRecordingDemuxer::instance() {
  static GstRecordingDemuxer demuxer;
  return demuxer;
}
