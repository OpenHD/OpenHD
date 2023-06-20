//
// Created by consti10 on 23.01.23.
//

#include "LastKnowPosition.h"
#include "openhd_util_filesystem.h"
#include "openhd_spdlog.h"

static constexpr auto LAST_KNOWN_POSITION_DIRECTORY="/home/openhd/LastKnownPosition/";

static std::string get_position_filename(int index){
    return LAST_KNOWN_POSITION_DIRECTORY+fmt::format("pos{}.txt",index);
}

LastKnowPosition::LastKnowPosition() {
  OHDFilesystemUtil::create_directories(LAST_KNOWN_POSITION_DIRECTORY);
}


void LastKnowPosition::on_new_position(double latitude, double longitude,
                                       double altitude) {
  // We limit updates to once every X ms to not pollute the disk too much
  const auto delta=std::chrono::steady_clock::now()-m_last_position_update;
  if(delta<std::chrono::milliseconds(500)){
    return ;
  }
  // Update the position in a rolling fashion, overwriting the oldest file
  m_last_position_update=std::chrono::steady_clock::now();
  const auto filename= get_position_filename(m_update_index);
  OHDFilesystemUtil::write_file(filename,fmt::format("Lat:{},Lon:{},Alt:{}",latitude,longitude,altitude));
  m_update_index++;
  m_update_index = m_update_index % 10;
}
