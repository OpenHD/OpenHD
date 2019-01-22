/** @file
 *	@brief MAVLink comm testsuite protocol generated from icarous.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <gtest/gtest.h>
#include "icarous.hpp"

#ifdef TEST_INTEROP
using namespace mavlink;
#undef MAVLINK_HELPER
#include "mavlink.h"
#endif


TEST(icarous, ICAROUS_HEARTBEAT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::icarous::msg::ICAROUS_HEARTBEAT packet_in{};
    packet_in.status = 5;

    mavlink::icarous::msg::ICAROUS_HEARTBEAT packet1{};
    mavlink::icarous::msg::ICAROUS_HEARTBEAT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.status, packet2.status);
}

#ifdef TEST_INTEROP
TEST(icarous_interop, ICAROUS_HEARTBEAT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_icarous_heartbeat_t packet_c {
         5
    };

    mavlink::icarous::msg::ICAROUS_HEARTBEAT packet_in{};
    packet_in.status = 5;

    mavlink::icarous::msg::ICAROUS_HEARTBEAT packet2{};

    mavlink_msg_icarous_heartbeat_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.status, packet2.status);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(icarous, ICAROUS_KINEMATIC_BANDS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::icarous::msg::ICAROUS_KINEMATIC_BANDS packet_in{};
    packet_in.numBands = 125;
    packet_in.type1 = 192;
    packet_in.min1 = 17.0;
    packet_in.max1 = 45.0;
    packet_in.type2 = 3;
    packet_in.min2 = 73.0;
    packet_in.max2 = 101.0;
    packet_in.type3 = 70;
    packet_in.min3 = 129.0;
    packet_in.max3 = 157.0;
    packet_in.type4 = 137;
    packet_in.min4 = 185.0;
    packet_in.max4 = 213.0;
    packet_in.type5 = 204;
    packet_in.min5 = 241.0;
    packet_in.max5 = 269.0;

    mavlink::icarous::msg::ICAROUS_KINEMATIC_BANDS packet1{};
    mavlink::icarous::msg::ICAROUS_KINEMATIC_BANDS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.numBands, packet2.numBands);
    EXPECT_EQ(packet1.type1, packet2.type1);
    EXPECT_EQ(packet1.min1, packet2.min1);
    EXPECT_EQ(packet1.max1, packet2.max1);
    EXPECT_EQ(packet1.type2, packet2.type2);
    EXPECT_EQ(packet1.min2, packet2.min2);
    EXPECT_EQ(packet1.max2, packet2.max2);
    EXPECT_EQ(packet1.type3, packet2.type3);
    EXPECT_EQ(packet1.min3, packet2.min3);
    EXPECT_EQ(packet1.max3, packet2.max3);
    EXPECT_EQ(packet1.type4, packet2.type4);
    EXPECT_EQ(packet1.min4, packet2.min4);
    EXPECT_EQ(packet1.max4, packet2.max4);
    EXPECT_EQ(packet1.type5, packet2.type5);
    EXPECT_EQ(packet1.min5, packet2.min5);
    EXPECT_EQ(packet1.max5, packet2.max5);
}

#ifdef TEST_INTEROP
TEST(icarous_interop, ICAROUS_KINEMATIC_BANDS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_icarous_kinematic_bands_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 125, 192, 3, 70, 137, 204
    };

    mavlink::icarous::msg::ICAROUS_KINEMATIC_BANDS packet_in{};
    packet_in.numBands = 125;
    packet_in.type1 = 192;
    packet_in.min1 = 17.0;
    packet_in.max1 = 45.0;
    packet_in.type2 = 3;
    packet_in.min2 = 73.0;
    packet_in.max2 = 101.0;
    packet_in.type3 = 70;
    packet_in.min3 = 129.0;
    packet_in.max3 = 157.0;
    packet_in.type4 = 137;
    packet_in.min4 = 185.0;
    packet_in.max4 = 213.0;
    packet_in.type5 = 204;
    packet_in.min5 = 241.0;
    packet_in.max5 = 269.0;

    mavlink::icarous::msg::ICAROUS_KINEMATIC_BANDS packet2{};

    mavlink_msg_icarous_kinematic_bands_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.numBands, packet2.numBands);
    EXPECT_EQ(packet_in.type1, packet2.type1);
    EXPECT_EQ(packet_in.min1, packet2.min1);
    EXPECT_EQ(packet_in.max1, packet2.max1);
    EXPECT_EQ(packet_in.type2, packet2.type2);
    EXPECT_EQ(packet_in.min2, packet2.min2);
    EXPECT_EQ(packet_in.max2, packet2.max2);
    EXPECT_EQ(packet_in.type3, packet2.type3);
    EXPECT_EQ(packet_in.min3, packet2.min3);
    EXPECT_EQ(packet_in.max3, packet2.max3);
    EXPECT_EQ(packet_in.type4, packet2.type4);
    EXPECT_EQ(packet_in.min4, packet2.min4);
    EXPECT_EQ(packet_in.max4, packet2.max4);
    EXPECT_EQ(packet_in.type5, packet2.type5);
    EXPECT_EQ(packet_in.min5, packet2.min5);
    EXPECT_EQ(packet_in.max5, packet2.max5);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif
