//
// Created by consti10 on 23.01.23.
//

#include "LastKnowPosition.h"
#include "openhd_util_filesystem.h"
#include "openhd_spdlog.h"

#include <iomanip>

static constexpr auto LAST_KNOWN_POSITION_DIRECTORY="/home/openhd/LastKnownPosition/";

static std::string get_this_flight_directory(){
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream ss;
  ss<<LAST_KNOWN_POSITION_DIRECTORY<<std::put_time(&tm, "%d-%m-%Y %H-%M-%S")<<"/";
  return ss.str();
}

static std::string get_position_filename(int index){
  return fmt::format("pos{}.txt",index);
}

LastKnowPosition::LastKnowPosition()
    : m_directory(get_this_flight_directory())

{
  openhd::log::get_default()->debug("Writing position to {}",m_directory);
  OHDFilesystemUtil::create_directories(m_directory);
}


void LastKnowPosition::on_new_position(double latitude, double longitude,
                                       double altitude) {
  // bad data - do not pollute
  if(latitude==0.0 || longitude==0.0){
    return ;
  }
  // We limit updates to once every X ms to not pollute the disk too much
  const auto delta=std::chrono::steady_clock::now()-m_last_position_update;
  if(delta<std::chrono::milliseconds(500)){
    return ;
  }
  // Update the position in a rolling fashion, overwriting the oldest file
  m_last_position_update=std::chrono::steady_clock::now();
  const auto filename= m_directory+get_this_flight_directory();
  OHDFilesystemUtil::write_file(filename,fmt::format("Lat:{},Lon:{},Alt:{}",latitude,longitude,altitude));
  m_update_index++;
  m_update_index = m_update_index % 10;
}
