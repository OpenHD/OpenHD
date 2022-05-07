//
// Created by consti10 on 27.04.22.
//

#ifndef XMAVLINKSERVICE_WBSTATISTICSCONVERTER_HPP
#define XMAVLINKSERVICE_WBSTATISTICSCONVERTER_HPP

//#include "../endpoints/wb_include.h"
#include "../mav_include.h"
#include "../../../lib/wifibroadcast/src/OpenHDStatisticsWriter.hpp"

// Helper for converting the WB statistics raw struct to mavlink package
namespace WBStatisticsConverter{
    /**
     * Convert a WB statistics message to mavlink
     * @param data the wb statistics data
     * @param sys_id this way the receiver knows if this data was generated on the air or ground pi
     * @return message containing the statistics data
     */
    static MavlinkMessage convertWbStatisticsToMavlink(const OpenHDStatisticsWriter::Data& data,const uint8_t sys_id){
        MavlinkMessage msg;
        mavlink_msg_openhd_wifibroadcast_statistics_pack(sys_id,MAV_COMP_ID_ALL,&msg.m,data.radio_port,data.count_p_all,data.count_p_bad,data.count_p_dec_ok,
                                                         data.count_p_dec_ok,data.count_p_fec_recovered,data.count_p_lost);
        return msg;
    }
}
#endif //XMAVLINKSERVICE_WBSTATISTICSCONVERTER_HPP
