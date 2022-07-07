//
// Created by consti10 on 27.04.22.
//

#ifndef XMAVLINKSERVICE_WBSTATISTICSCONVERTER_HPP
#define XMAVLINKSERVICE_WBSTATISTICSCONVERTER_HPP

//#include "../endpoints/wb_include.h"
#include "../mav_include.h"
// wifibroadcast header-only
#include "OpenHDStatisticsWriter.hpp"
#include <vector>

// Helper for converting the WB statistics raw struct to mavlink package
namespace WBStatisticsConverter {
/**
 * Convert a WB statistics message to mavlink
 * @param data the wb statistics data
 * @param sys_id this way the receiver knows if this data was generated on the air or ground pi
 * @return message containing the statistics data
 */
static MavlinkMessage convertWbStatisticsToMavlink(const OpenHDStatisticsWriter::Data &data, const uint8_t sys_id,const uint8_t comp_id) {
  MavlinkMessage msg;
  mavlink_msg_openhd_wifibroadcast_statistics_pack(sys_id,
                                                   comp_id,
                                                   &msg.m,
                                                   data.radio_port,
                                                   data.count_p_all,
                                                   data.count_p_bad,
                                                   data.count_p_dec_ok,
                                                   data.count_p_dec_ok,
                                                   data.count_p_fec_recovered,
                                                   data.count_p_lost);
  return msg;
}

static MavlinkMessage createDummyMessage(const uint8_t sys_id,const uint8_t comp_id){
  OpenHDStatisticsWriter::Data dummyData;
  dummyData.count_p_all=22;
  return convertWbStatisticsToMavlink(dummyData,sys_id,comp_id);
}

// statistics come as raw data from UDP. We assume that one "read" from UDP (which is what this is called with)
// contains one or more wb statistics data packets.
// This method returns parsed data, if anything could successfully be parsed.
static std::vector<OpenHDStatisticsWriter::Data> parseRawDataSafe(const uint8_t *payload, const std::size_t payloadSize){
  //std::cout << "OHDTelemetryGenerator::processNewWifibroadcastStatisticsMessage: " << payloadSize << "\n";
  std::vector<OpenHDStatisticsWriter::Data> ret;
  const auto MSG_SIZE = sizeof(OpenHDStatisticsWriter::Data);
  if (payloadSize >= MSG_SIZE && (payloadSize % MSG_SIZE == 0)) {
    // safe to do so due to the prevous check
    //const int nMessages=(int)payloadSize / MSG_SIZE;
    // we got new properly aligned data
    OpenHDStatisticsWriter::Data data;
    memcpy((uint8_t *)&data, payload, MSG_SIZE);
    ret.push_back(data);
    //lastWbStatisticsMessage[data.radio_port] = data;
  } else {
    std::cerr << "Cannot parse WB statistics due to size mismatch\n";
  }
  return ret;
}

/*struct LinkStatistics{
  int radio_port; // can be for one of the video links or for telemetry or ...
  int n_injected_packets;
  int n_received_packets_ok;
  int n_received_packets_bad;
};*/
}
#endif //XMAVLINKSERVICE_WBSTATISTICSCONVERTER_HPP
