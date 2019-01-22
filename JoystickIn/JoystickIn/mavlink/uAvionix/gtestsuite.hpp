/** @file
 *	@brief MAVLink comm testsuite protocol generated from uAvionix.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <gtest/gtest.h>
#include "uAvionix.hpp"

#ifdef TEST_INTEROP
using namespace mavlink;
#undef MAVLINK_HELPER
#include "mavlink.h"
#endif


TEST(uAvionix, UAVIONIX_ADSB_OUT_CFG)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_CFG packet_in{};
    packet_in.ICAO = 963497464;
    packet_in.callsign = to_char_array("GHIJKLMN");
    packet_in.emitterType = 242;
    packet_in.aircraftSize = 53;
    packet_in.gpsOffsetLat = 120;
    packet_in.gpsOffsetLon = 187;
    packet_in.stallSpeed = 17443;
    packet_in.rfSelect = 254;

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_CFG packet1{};
    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_CFG packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.ICAO, packet2.ICAO);
    EXPECT_EQ(packet1.callsign, packet2.callsign);
    EXPECT_EQ(packet1.emitterType, packet2.emitterType);
    EXPECT_EQ(packet1.aircraftSize, packet2.aircraftSize);
    EXPECT_EQ(packet1.gpsOffsetLat, packet2.gpsOffsetLat);
    EXPECT_EQ(packet1.gpsOffsetLon, packet2.gpsOffsetLon);
    EXPECT_EQ(packet1.stallSpeed, packet2.stallSpeed);
    EXPECT_EQ(packet1.rfSelect, packet2.rfSelect);
}

#ifdef TEST_INTEROP
TEST(uAvionix_interop, UAVIONIX_ADSB_OUT_CFG)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_uavionix_adsb_out_cfg_t packet_c {
         963497464, 17443, "GHIJKLMN", 242, 53, 120, 187, 254
    };

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_CFG packet_in{};
    packet_in.ICAO = 963497464;
    packet_in.callsign = to_char_array("GHIJKLMN");
    packet_in.emitterType = 242;
    packet_in.aircraftSize = 53;
    packet_in.gpsOffsetLat = 120;
    packet_in.gpsOffsetLon = 187;
    packet_in.stallSpeed = 17443;
    packet_in.rfSelect = 254;

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_CFG packet2{};

    mavlink_msg_uavionix_adsb_out_cfg_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.ICAO, packet2.ICAO);
    EXPECT_EQ(packet_in.callsign, packet2.callsign);
    EXPECT_EQ(packet_in.emitterType, packet2.emitterType);
    EXPECT_EQ(packet_in.aircraftSize, packet2.aircraftSize);
    EXPECT_EQ(packet_in.gpsOffsetLat, packet2.gpsOffsetLat);
    EXPECT_EQ(packet_in.gpsOffsetLon, packet2.gpsOffsetLon);
    EXPECT_EQ(packet_in.stallSpeed, packet2.stallSpeed);
    EXPECT_EQ(packet_in.rfSelect, packet2.rfSelect);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(uAvionix, UAVIONIX_ADSB_OUT_DYNAMIC)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_DYNAMIC packet_in{};
    packet_in.utcTime = 963497464;
    packet_in.gpsLat = 963497672;
    packet_in.gpsLon = 963497880;
    packet_in.gpsAlt = 963498088;
    packet_in.gpsFix = 247;
    packet_in.numSats = 58;
    packet_in.baroAltMSL = 963498296;
    packet_in.accuracyHor = 963498504;
    packet_in.accuracyVert = 18483;
    packet_in.accuracyVel = 18587;
    packet_in.velVert = 18691;
    packet_in.velNS = 18795;
    packet_in.VelEW = 18899;
    packet_in.emergencyStatus = 125;
    packet_in.state = 19003;
    packet_in.squawk = 19107;

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_DYNAMIC packet1{};
    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_DYNAMIC packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.utcTime, packet2.utcTime);
    EXPECT_EQ(packet1.gpsLat, packet2.gpsLat);
    EXPECT_EQ(packet1.gpsLon, packet2.gpsLon);
    EXPECT_EQ(packet1.gpsAlt, packet2.gpsAlt);
    EXPECT_EQ(packet1.gpsFix, packet2.gpsFix);
    EXPECT_EQ(packet1.numSats, packet2.numSats);
    EXPECT_EQ(packet1.baroAltMSL, packet2.baroAltMSL);
    EXPECT_EQ(packet1.accuracyHor, packet2.accuracyHor);
    EXPECT_EQ(packet1.accuracyVert, packet2.accuracyVert);
    EXPECT_EQ(packet1.accuracyVel, packet2.accuracyVel);
    EXPECT_EQ(packet1.velVert, packet2.velVert);
    EXPECT_EQ(packet1.velNS, packet2.velNS);
    EXPECT_EQ(packet1.VelEW, packet2.VelEW);
    EXPECT_EQ(packet1.emergencyStatus, packet2.emergencyStatus);
    EXPECT_EQ(packet1.state, packet2.state);
    EXPECT_EQ(packet1.squawk, packet2.squawk);
}

#ifdef TEST_INTEROP
TEST(uAvionix_interop, UAVIONIX_ADSB_OUT_DYNAMIC)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_uavionix_adsb_out_dynamic_t packet_c {
         963497464, 963497672, 963497880, 963498088, 963498296, 963498504, 18483, 18587, 18691, 18795, 18899, 19003, 19107, 247, 58, 125
    };

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_DYNAMIC packet_in{};
    packet_in.utcTime = 963497464;
    packet_in.gpsLat = 963497672;
    packet_in.gpsLon = 963497880;
    packet_in.gpsAlt = 963498088;
    packet_in.gpsFix = 247;
    packet_in.numSats = 58;
    packet_in.baroAltMSL = 963498296;
    packet_in.accuracyHor = 963498504;
    packet_in.accuracyVert = 18483;
    packet_in.accuracyVel = 18587;
    packet_in.velVert = 18691;
    packet_in.velNS = 18795;
    packet_in.VelEW = 18899;
    packet_in.emergencyStatus = 125;
    packet_in.state = 19003;
    packet_in.squawk = 19107;

    mavlink::uAvionix::msg::UAVIONIX_ADSB_OUT_DYNAMIC packet2{};

    mavlink_msg_uavionix_adsb_out_dynamic_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.utcTime, packet2.utcTime);
    EXPECT_EQ(packet_in.gpsLat, packet2.gpsLat);
    EXPECT_EQ(packet_in.gpsLon, packet2.gpsLon);
    EXPECT_EQ(packet_in.gpsAlt, packet2.gpsAlt);
    EXPECT_EQ(packet_in.gpsFix, packet2.gpsFix);
    EXPECT_EQ(packet_in.numSats, packet2.numSats);
    EXPECT_EQ(packet_in.baroAltMSL, packet2.baroAltMSL);
    EXPECT_EQ(packet_in.accuracyHor, packet2.accuracyHor);
    EXPECT_EQ(packet_in.accuracyVert, packet2.accuracyVert);
    EXPECT_EQ(packet_in.accuracyVel, packet2.accuracyVel);
    EXPECT_EQ(packet_in.velVert, packet2.velVert);
    EXPECT_EQ(packet_in.velNS, packet2.velNS);
    EXPECT_EQ(packet_in.VelEW, packet2.VelEW);
    EXPECT_EQ(packet_in.emergencyStatus, packet2.emergencyStatus);
    EXPECT_EQ(packet_in.state, packet2.state);
    EXPECT_EQ(packet_in.squawk, packet2.squawk);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(uAvionix, UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::uAvionix::msg::UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT packet_in{};
    packet_in.rfHealth = 5;

    mavlink::uAvionix::msg::UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT packet1{};
    mavlink::uAvionix::msg::UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.rfHealth, packet2.rfHealth);
}

#ifdef TEST_INTEROP
TEST(uAvionix_interop, UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_uavionix_adsb_transceiver_health_report_t packet_c {
         5
    };

    mavlink::uAvionix::msg::UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT packet_in{};
    packet_in.rfHealth = 5;

    mavlink::uAvionix::msg::UAVIONIX_ADSB_TRANSCEIVER_HEALTH_REPORT packet2{};

    mavlink_msg_uavionix_adsb_transceiver_health_report_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.rfHealth, packet2.rfHealth);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif
