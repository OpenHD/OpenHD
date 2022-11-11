//
// Created by consti10 on 06.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_

#include <queue>

#include "openhd-spdlog-tele-sink.h"

namespace StatusTextAccumulator{

static void convert(mavlink_message_t& mavlink_message,const openhd::loggers::sink::TelemetryForwardSink::StoredLogMessage& msg,uint8_t sys_id,uint8_t comp_id){
  mavlink_statustext_t mavlink_statustext;
  mavlink_statustext.id=0;
  mavlink_statustext.chunk_seq=0;
  mavlink_statustext.severity=(int)msg.m_level;
  std::strncpy((char *)mavlink_statustext.text,msg.m_message.c_str(),50);
  mavlink_msg_statustext_encode(sys_id,comp_id,&mavlink_message,&mavlink_statustext);
}

}



#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_STATUSTEXT_H_
