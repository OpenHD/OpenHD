/** @file
 *	@brief MAVLink comm testsuite protocol generated from common.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <gtest/gtest.h>
#include "common.hpp"

#ifdef TEST_INTEROP
using namespace mavlink;
#undef MAVLINK_HELPER
#include "mavlink.h"
#endif


TEST(common, HEARTBEAT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HEARTBEAT packet_in{};
    packet_in.type = 17;
    packet_in.autopilot = 84;
    packet_in.base_mode = 151;
    packet_in.custom_mode = 963497464;
    packet_in.system_status = 218;
    packet_in.mavlink_version = 3;

    mavlink::common::msg::HEARTBEAT packet1{};
    mavlink::common::msg::HEARTBEAT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.autopilot, packet2.autopilot);
    EXPECT_EQ(packet1.base_mode, packet2.base_mode);
    EXPECT_EQ(packet1.custom_mode, packet2.custom_mode);
    EXPECT_EQ(packet1.system_status, packet2.system_status);
    EXPECT_EQ(packet1.mavlink_version, packet2.mavlink_version);
}

#ifdef TEST_INTEROP
TEST(common_interop, HEARTBEAT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_heartbeat_t packet_c {
         963497464, 17, 84, 151, 218, 3
    };

    mavlink::common::msg::HEARTBEAT packet_in{};
    packet_in.type = 17;
    packet_in.autopilot = 84;
    packet_in.base_mode = 151;
    packet_in.custom_mode = 963497464;
    packet_in.system_status = 218;
    packet_in.mavlink_version = 3;

    mavlink::common::msg::HEARTBEAT packet2{};

    mavlink_msg_heartbeat_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.autopilot, packet2.autopilot);
    EXPECT_EQ(packet_in.base_mode, packet2.base_mode);
    EXPECT_EQ(packet_in.custom_mode, packet2.custom_mode);
    EXPECT_EQ(packet_in.system_status, packet2.system_status);
    EXPECT_EQ(packet_in.mavlink_version, packet2.mavlink_version);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SYS_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SYS_STATUS packet_in{};
    packet_in.onboard_control_sensors_present = 963497464;
    packet_in.onboard_control_sensors_enabled = 963497672;
    packet_in.onboard_control_sensors_health = 963497880;
    packet_in.load = 17859;
    packet_in.voltage_battery = 17963;
    packet_in.current_battery = 18067;
    packet_in.battery_remaining = -33;
    packet_in.drop_rate_comm = 18171;
    packet_in.errors_comm = 18275;
    packet_in.errors_count1 = 18379;
    packet_in.errors_count2 = 18483;
    packet_in.errors_count3 = 18587;
    packet_in.errors_count4 = 18691;

    mavlink::common::msg::SYS_STATUS packet1{};
    mavlink::common::msg::SYS_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.onboard_control_sensors_present, packet2.onboard_control_sensors_present);
    EXPECT_EQ(packet1.onboard_control_sensors_enabled, packet2.onboard_control_sensors_enabled);
    EXPECT_EQ(packet1.onboard_control_sensors_health, packet2.onboard_control_sensors_health);
    EXPECT_EQ(packet1.load, packet2.load);
    EXPECT_EQ(packet1.voltage_battery, packet2.voltage_battery);
    EXPECT_EQ(packet1.current_battery, packet2.current_battery);
    EXPECT_EQ(packet1.battery_remaining, packet2.battery_remaining);
    EXPECT_EQ(packet1.drop_rate_comm, packet2.drop_rate_comm);
    EXPECT_EQ(packet1.errors_comm, packet2.errors_comm);
    EXPECT_EQ(packet1.errors_count1, packet2.errors_count1);
    EXPECT_EQ(packet1.errors_count2, packet2.errors_count2);
    EXPECT_EQ(packet1.errors_count3, packet2.errors_count3);
    EXPECT_EQ(packet1.errors_count4, packet2.errors_count4);
}

#ifdef TEST_INTEROP
TEST(common_interop, SYS_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_sys_status_t packet_c {
         963497464, 963497672, 963497880, 17859, 17963, 18067, 18171, 18275, 18379, 18483, 18587, 18691, -33
    };

    mavlink::common::msg::SYS_STATUS packet_in{};
    packet_in.onboard_control_sensors_present = 963497464;
    packet_in.onboard_control_sensors_enabled = 963497672;
    packet_in.onboard_control_sensors_health = 963497880;
    packet_in.load = 17859;
    packet_in.voltage_battery = 17963;
    packet_in.current_battery = 18067;
    packet_in.battery_remaining = -33;
    packet_in.drop_rate_comm = 18171;
    packet_in.errors_comm = 18275;
    packet_in.errors_count1 = 18379;
    packet_in.errors_count2 = 18483;
    packet_in.errors_count3 = 18587;
    packet_in.errors_count4 = 18691;

    mavlink::common::msg::SYS_STATUS packet2{};

    mavlink_msg_sys_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.onboard_control_sensors_present, packet2.onboard_control_sensors_present);
    EXPECT_EQ(packet_in.onboard_control_sensors_enabled, packet2.onboard_control_sensors_enabled);
    EXPECT_EQ(packet_in.onboard_control_sensors_health, packet2.onboard_control_sensors_health);
    EXPECT_EQ(packet_in.load, packet2.load);
    EXPECT_EQ(packet_in.voltage_battery, packet2.voltage_battery);
    EXPECT_EQ(packet_in.current_battery, packet2.current_battery);
    EXPECT_EQ(packet_in.battery_remaining, packet2.battery_remaining);
    EXPECT_EQ(packet_in.drop_rate_comm, packet2.drop_rate_comm);
    EXPECT_EQ(packet_in.errors_comm, packet2.errors_comm);
    EXPECT_EQ(packet_in.errors_count1, packet2.errors_count1);
    EXPECT_EQ(packet_in.errors_count2, packet2.errors_count2);
    EXPECT_EQ(packet_in.errors_count3, packet2.errors_count3);
    EXPECT_EQ(packet_in.errors_count4, packet2.errors_count4);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SYSTEM_TIME)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SYSTEM_TIME packet_in{};
    packet_in.time_unix_usec = 93372036854775807ULL;
    packet_in.time_boot_ms = 963497880;

    mavlink::common::msg::SYSTEM_TIME packet1{};
    mavlink::common::msg::SYSTEM_TIME packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_unix_usec, packet2.time_unix_usec);
    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
}

#ifdef TEST_INTEROP
TEST(common_interop, SYSTEM_TIME)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_system_time_t packet_c {
         93372036854775807ULL, 963497880
    };

    mavlink::common::msg::SYSTEM_TIME packet_in{};
    packet_in.time_unix_usec = 93372036854775807ULL;
    packet_in.time_boot_ms = 963497880;

    mavlink::common::msg::SYSTEM_TIME packet2{};

    mavlink_msg_system_time_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_unix_usec, packet2.time_unix_usec);
    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PING)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PING packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.seq = 963497880;
    packet_in.target_system = 41;
    packet_in.target_component = 108;

    mavlink::common::msg::PING packet1{};
    mavlink::common::msg::PING packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.seq, packet2.seq);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
}

#ifdef TEST_INTEROP
TEST(common_interop, PING)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_ping_t packet_c {
         93372036854775807ULL, 963497880, 41, 108
    };

    mavlink::common::msg::PING packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.seq = 963497880;
    packet_in.target_system = 41;
    packet_in.target_component = 108;

    mavlink::common::msg::PING packet2{};

    mavlink_msg_ping_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.seq, packet2.seq);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CHANGE_OPERATOR_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL packet_in{};
    packet_in.target_system = 5;
    packet_in.control_request = 72;
    packet_in.version = 139;
    packet_in.passkey = to_char_array("DEFGHIJKLMNOPQRSTUVWXYZA");

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL packet1{};
    mavlink::common::msg::CHANGE_OPERATOR_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.control_request, packet2.control_request);
    EXPECT_EQ(packet1.version, packet2.version);
    EXPECT_EQ(packet1.passkey, packet2.passkey);
}

#ifdef TEST_INTEROP
TEST(common_interop, CHANGE_OPERATOR_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_change_operator_control_t packet_c {
         5, 72, 139, "DEFGHIJKLMNOPQRSTUVWXYZA"
    };

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL packet_in{};
    packet_in.target_system = 5;
    packet_in.control_request = 72;
    packet_in.version = 139;
    packet_in.passkey = to_char_array("DEFGHIJKLMNOPQRSTUVWXYZA");

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL packet2{};

    mavlink_msg_change_operator_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.control_request, packet2.control_request);
    EXPECT_EQ(packet_in.version, packet2.version);
    EXPECT_EQ(packet_in.passkey, packet2.passkey);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CHANGE_OPERATOR_CONTROL_ACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL_ACK packet_in{};
    packet_in.gcs_system_id = 5;
    packet_in.control_request = 72;
    packet_in.ack = 139;

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL_ACK packet1{};
    mavlink::common::msg::CHANGE_OPERATOR_CONTROL_ACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.gcs_system_id, packet2.gcs_system_id);
    EXPECT_EQ(packet1.control_request, packet2.control_request);
    EXPECT_EQ(packet1.ack, packet2.ack);
}

#ifdef TEST_INTEROP
TEST(common_interop, CHANGE_OPERATOR_CONTROL_ACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_change_operator_control_ack_t packet_c {
         5, 72, 139
    };

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL_ACK packet_in{};
    packet_in.gcs_system_id = 5;
    packet_in.control_request = 72;
    packet_in.ack = 139;

    mavlink::common::msg::CHANGE_OPERATOR_CONTROL_ACK packet2{};

    mavlink_msg_change_operator_control_ack_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.gcs_system_id, packet2.gcs_system_id);
    EXPECT_EQ(packet_in.control_request, packet2.control_request);
    EXPECT_EQ(packet_in.ack, packet2.ack);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, AUTH_KEY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::AUTH_KEY packet_in{};
    packet_in.key = to_char_array("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE");

    mavlink::common::msg::AUTH_KEY packet1{};
    mavlink::common::msg::AUTH_KEY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.key, packet2.key);
}

#ifdef TEST_INTEROP
TEST(common_interop, AUTH_KEY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_auth_key_t packet_c {
         "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE"
    };

    mavlink::common::msg::AUTH_KEY packet_in{};
    packet_in.key = to_char_array("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE");

    mavlink::common::msg::AUTH_KEY packet2{};

    mavlink_msg_auth_key_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.key, packet2.key);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_MODE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_MODE packet_in{};
    packet_in.target_system = 17;
    packet_in.base_mode = 84;
    packet_in.custom_mode = 963497464;

    mavlink::common::msg::SET_MODE packet1{};
    mavlink::common::msg::SET_MODE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.base_mode, packet2.base_mode);
    EXPECT_EQ(packet1.custom_mode, packet2.custom_mode);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_MODE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_mode_t packet_c {
         963497464, 17, 84
    };

    mavlink::common::msg::SET_MODE packet_in{};
    packet_in.target_system = 17;
    packet_in.base_mode = 84;
    packet_in.custom_mode = 963497464;

    mavlink::common::msg::SET_MODE packet2{};

    mavlink_msg_set_mode_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.base_mode, packet2.base_mode);
    EXPECT_EQ(packet_in.custom_mode, packet2.custom_mode);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PARAM_REQUEST_READ)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PARAM_REQUEST_READ packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.param_id = to_char_array("EFGHIJKLMNOPQRS");
    packet_in.param_index = 17235;

    mavlink::common::msg::PARAM_REQUEST_READ packet1{};
    mavlink::common::msg::PARAM_REQUEST_READ packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.param_id, packet2.param_id);
    EXPECT_EQ(packet1.param_index, packet2.param_index);
}

#ifdef TEST_INTEROP
TEST(common_interop, PARAM_REQUEST_READ)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_param_request_read_t packet_c {
         17235, 139, 206, "EFGHIJKLMNOPQRS"
    };

    mavlink::common::msg::PARAM_REQUEST_READ packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.param_id = to_char_array("EFGHIJKLMNOPQRS");
    packet_in.param_index = 17235;

    mavlink::common::msg::PARAM_REQUEST_READ packet2{};

    mavlink_msg_param_request_read_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.param_id, packet2.param_id);
    EXPECT_EQ(packet_in.param_index, packet2.param_index);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PARAM_REQUEST_LIST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PARAM_REQUEST_LIST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::common::msg::PARAM_REQUEST_LIST packet1{};
    mavlink::common::msg::PARAM_REQUEST_LIST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
}

#ifdef TEST_INTEROP
TEST(common_interop, PARAM_REQUEST_LIST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_param_request_list_t packet_c {
         5, 72
    };

    mavlink::common::msg::PARAM_REQUEST_LIST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::common::msg::PARAM_REQUEST_LIST packet2{};

    mavlink_msg_param_request_list_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PARAM_VALUE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PARAM_VALUE packet_in{};
    packet_in.param_id = to_char_array("IJKLMNOPQRSTUVW");
    packet_in.param_value = 17.0;
    packet_in.param_type = 77;
    packet_in.param_count = 17443;
    packet_in.param_index = 17547;

    mavlink::common::msg::PARAM_VALUE packet1{};
    mavlink::common::msg::PARAM_VALUE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.param_id, packet2.param_id);
    EXPECT_EQ(packet1.param_value, packet2.param_value);
    EXPECT_EQ(packet1.param_type, packet2.param_type);
    EXPECT_EQ(packet1.param_count, packet2.param_count);
    EXPECT_EQ(packet1.param_index, packet2.param_index);
}

#ifdef TEST_INTEROP
TEST(common_interop, PARAM_VALUE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_param_value_t packet_c {
         17.0, 17443, 17547, "IJKLMNOPQRSTUVW", 77
    };

    mavlink::common::msg::PARAM_VALUE packet_in{};
    packet_in.param_id = to_char_array("IJKLMNOPQRSTUVW");
    packet_in.param_value = 17.0;
    packet_in.param_type = 77;
    packet_in.param_count = 17443;
    packet_in.param_index = 17547;

    mavlink::common::msg::PARAM_VALUE packet2{};

    mavlink_msg_param_value_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.param_id, packet2.param_id);
    EXPECT_EQ(packet_in.param_value, packet2.param_value);
    EXPECT_EQ(packet_in.param_type, packet2.param_type);
    EXPECT_EQ(packet_in.param_count, packet2.param_count);
    EXPECT_EQ(packet_in.param_index, packet2.param_index);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PARAM_SET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PARAM_SET packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.param_id = to_char_array("GHIJKLMNOPQRSTU");
    packet_in.param_value = 17.0;
    packet_in.param_type = 199;

    mavlink::common::msg::PARAM_SET packet1{};
    mavlink::common::msg::PARAM_SET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.param_id, packet2.param_id);
    EXPECT_EQ(packet1.param_value, packet2.param_value);
    EXPECT_EQ(packet1.param_type, packet2.param_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, PARAM_SET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_param_set_t packet_c {
         17.0, 17, 84, "GHIJKLMNOPQRSTU", 199
    };

    mavlink::common::msg::PARAM_SET packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.param_id = to_char_array("GHIJKLMNOPQRSTU");
    packet_in.param_value = 17.0;
    packet_in.param_type = 199;

    mavlink::common::msg::PARAM_SET packet2{};

    mavlink_msg_param_set_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.param_id, packet2.param_id);
    EXPECT_EQ(packet_in.param_value, packet2.param_value);
    EXPECT_EQ(packet_in.param_type, packet2.param_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_RAW_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_RAW_INT packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.fix_type = 89;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.eph = 18275;
    packet_in.epv = 18379;
    packet_in.vel = 18483;
    packet_in.cog = 18587;
    packet_in.satellites_visible = 156;
    packet_in.alt_ellipsoid = 963499024;
    packet_in.h_acc = 963499232;
    packet_in.v_acc = 963499440;
    packet_in.vel_acc = 963499648;
    packet_in.hdg_acc = 963499856;

    mavlink::common::msg::GPS_RAW_INT packet1{};
    mavlink::common::msg::GPS_RAW_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.fix_type, packet2.fix_type);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.eph, packet2.eph);
    EXPECT_EQ(packet1.epv, packet2.epv);
    EXPECT_EQ(packet1.vel, packet2.vel);
    EXPECT_EQ(packet1.cog, packet2.cog);
    EXPECT_EQ(packet1.satellites_visible, packet2.satellites_visible);
    EXPECT_EQ(packet1.alt_ellipsoid, packet2.alt_ellipsoid);
    EXPECT_EQ(packet1.h_acc, packet2.h_acc);
    EXPECT_EQ(packet1.v_acc, packet2.v_acc);
    EXPECT_EQ(packet1.vel_acc, packet2.vel_acc);
    EXPECT_EQ(packet1.hdg_acc, packet2.hdg_acc);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_RAW_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_raw_int_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 963498296, 18275, 18379, 18483, 18587, 89, 156, 963499024, 963499232, 963499440, 963499648, 963499856
    };

    mavlink::common::msg::GPS_RAW_INT packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.fix_type = 89;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.eph = 18275;
    packet_in.epv = 18379;
    packet_in.vel = 18483;
    packet_in.cog = 18587;
    packet_in.satellites_visible = 156;
    packet_in.alt_ellipsoid = 963499024;
    packet_in.h_acc = 963499232;
    packet_in.v_acc = 963499440;
    packet_in.vel_acc = 963499648;
    packet_in.hdg_acc = 963499856;

    mavlink::common::msg::GPS_RAW_INT packet2{};

    mavlink_msg_gps_raw_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.fix_type, packet2.fix_type);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.eph, packet2.eph);
    EXPECT_EQ(packet_in.epv, packet2.epv);
    EXPECT_EQ(packet_in.vel, packet2.vel);
    EXPECT_EQ(packet_in.cog, packet2.cog);
    EXPECT_EQ(packet_in.satellites_visible, packet2.satellites_visible);
    EXPECT_EQ(packet_in.alt_ellipsoid, packet2.alt_ellipsoid);
    EXPECT_EQ(packet_in.h_acc, packet2.h_acc);
    EXPECT_EQ(packet_in.v_acc, packet2.v_acc);
    EXPECT_EQ(packet_in.vel_acc, packet2.vel_acc);
    EXPECT_EQ(packet_in.hdg_acc, packet2.hdg_acc);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_STATUS packet_in{};
    packet_in.satellites_visible = 5;
    packet_in.satellite_prn = {{ 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91 }};
    packet_in.satellite_used = {{ 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151 }};
    packet_in.satellite_elevation = {{ 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211 }};
    packet_in.satellite_azimuth = {{ 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }};
    packet_in.satellite_snr = {{ 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75 }};

    mavlink::common::msg::GPS_STATUS packet1{};
    mavlink::common::msg::GPS_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.satellites_visible, packet2.satellites_visible);
    EXPECT_EQ(packet1.satellite_prn, packet2.satellite_prn);
    EXPECT_EQ(packet1.satellite_used, packet2.satellite_used);
    EXPECT_EQ(packet1.satellite_elevation, packet2.satellite_elevation);
    EXPECT_EQ(packet1.satellite_azimuth, packet2.satellite_azimuth);
    EXPECT_EQ(packet1.satellite_snr, packet2.satellite_snr);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_status_t packet_c {
         5, { 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91 }, { 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151 }, { 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211 }, { 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, { 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75 }
    };

    mavlink::common::msg::GPS_STATUS packet_in{};
    packet_in.satellites_visible = 5;
    packet_in.satellite_prn = {{ 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91 }};
    packet_in.satellite_used = {{ 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151 }};
    packet_in.satellite_elevation = {{ 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211 }};
    packet_in.satellite_azimuth = {{ 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }};
    packet_in.satellite_snr = {{ 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75 }};

    mavlink::common::msg::GPS_STATUS packet2{};

    mavlink_msg_gps_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.satellites_visible, packet2.satellites_visible);
    EXPECT_EQ(packet_in.satellite_prn, packet2.satellite_prn);
    EXPECT_EQ(packet_in.satellite_used, packet2.satellite_used);
    EXPECT_EQ(packet_in.satellite_elevation, packet2.satellite_elevation);
    EXPECT_EQ(packet_in.satellite_azimuth, packet2.satellite_azimuth);
    EXPECT_EQ(packet_in.satellite_snr, packet2.satellite_snr);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SCALED_IMU)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SCALED_IMU packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.xacc = 17443;
    packet_in.yacc = 17547;
    packet_in.zacc = 17651;
    packet_in.xgyro = 17755;
    packet_in.ygyro = 17859;
    packet_in.zgyro = 17963;
    packet_in.xmag = 18067;
    packet_in.ymag = 18171;
    packet_in.zmag = 18275;

    mavlink::common::msg::SCALED_IMU packet1{};
    mavlink::common::msg::SCALED_IMU packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.xmag, packet2.xmag);
    EXPECT_EQ(packet1.ymag, packet2.ymag);
    EXPECT_EQ(packet1.zmag, packet2.zmag);
}

#ifdef TEST_INTEROP
TEST(common_interop, SCALED_IMU)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_scaled_imu_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 18275
    };

    mavlink::common::msg::SCALED_IMU packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.xacc = 17443;
    packet_in.yacc = 17547;
    packet_in.zacc = 17651;
    packet_in.xgyro = 17755;
    packet_in.ygyro = 17859;
    packet_in.zgyro = 17963;
    packet_in.xmag = 18067;
    packet_in.ymag = 18171;
    packet_in.zmag = 18275;

    mavlink::common::msg::SCALED_IMU packet2{};

    mavlink_msg_scaled_imu_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.xmag, packet2.xmag);
    EXPECT_EQ(packet_in.ymag, packet2.ymag);
    EXPECT_EQ(packet_in.zmag, packet2.zmag);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RAW_IMU)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RAW_IMU packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.xacc = 17651;
    packet_in.yacc = 17755;
    packet_in.zacc = 17859;
    packet_in.xgyro = 17963;
    packet_in.ygyro = 18067;
    packet_in.zgyro = 18171;
    packet_in.xmag = 18275;
    packet_in.ymag = 18379;
    packet_in.zmag = 18483;

    mavlink::common::msg::RAW_IMU packet1{};
    mavlink::common::msg::RAW_IMU packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.xmag, packet2.xmag);
    EXPECT_EQ(packet1.ymag, packet2.ymag);
    EXPECT_EQ(packet1.zmag, packet2.zmag);
}

#ifdef TEST_INTEROP
TEST(common_interop, RAW_IMU)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_raw_imu_t packet_c {
         93372036854775807ULL, 17651, 17755, 17859, 17963, 18067, 18171, 18275, 18379, 18483
    };

    mavlink::common::msg::RAW_IMU packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.xacc = 17651;
    packet_in.yacc = 17755;
    packet_in.zacc = 17859;
    packet_in.xgyro = 17963;
    packet_in.ygyro = 18067;
    packet_in.zgyro = 18171;
    packet_in.xmag = 18275;
    packet_in.ymag = 18379;
    packet_in.zmag = 18483;

    mavlink::common::msg::RAW_IMU packet2{};

    mavlink_msg_raw_imu_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.xmag, packet2.xmag);
    EXPECT_EQ(packet_in.ymag, packet2.ymag);
    EXPECT_EQ(packet_in.zmag, packet2.zmag);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RAW_PRESSURE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RAW_PRESSURE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.press_abs = 17651;
    packet_in.press_diff1 = 17755;
    packet_in.press_diff2 = 17859;
    packet_in.temperature = 17963;

    mavlink::common::msg::RAW_PRESSURE packet1{};
    mavlink::common::msg::RAW_PRESSURE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.press_abs, packet2.press_abs);
    EXPECT_EQ(packet1.press_diff1, packet2.press_diff1);
    EXPECT_EQ(packet1.press_diff2, packet2.press_diff2);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
}

#ifdef TEST_INTEROP
TEST(common_interop, RAW_PRESSURE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_raw_pressure_t packet_c {
         93372036854775807ULL, 17651, 17755, 17859, 17963
    };

    mavlink::common::msg::RAW_PRESSURE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.press_abs = 17651;
    packet_in.press_diff1 = 17755;
    packet_in.press_diff2 = 17859;
    packet_in.temperature = 17963;

    mavlink::common::msg::RAW_PRESSURE packet2{};

    mavlink_msg_raw_pressure_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.press_abs, packet2.press_abs);
    EXPECT_EQ(packet_in.press_diff1, packet2.press_diff1);
    EXPECT_EQ(packet_in.press_diff2, packet2.press_diff2);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SCALED_PRESSURE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SCALED_PRESSURE packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.press_abs = 45.0;
    packet_in.press_diff = 73.0;
    packet_in.temperature = 17859;

    mavlink::common::msg::SCALED_PRESSURE packet1{};
    mavlink::common::msg::SCALED_PRESSURE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.press_abs, packet2.press_abs);
    EXPECT_EQ(packet1.press_diff, packet2.press_diff);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
}

#ifdef TEST_INTEROP
TEST(common_interop, SCALED_PRESSURE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_scaled_pressure_t packet_c {
         963497464, 45.0, 73.0, 17859
    };

    mavlink::common::msg::SCALED_PRESSURE packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.press_abs = 45.0;
    packet_in.press_diff = 73.0;
    packet_in.temperature = 17859;

    mavlink::common::msg::SCALED_PRESSURE packet2{};

    mavlink_msg_scaled_pressure_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.press_abs, packet2.press_abs);
    EXPECT_EQ(packet_in.press_diff, packet2.press_diff);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ATTITUDE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ATTITUDE packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.roll = 45.0;
    packet_in.pitch = 73.0;
    packet_in.yaw = 101.0;
    packet_in.rollspeed = 129.0;
    packet_in.pitchspeed = 157.0;
    packet_in.yawspeed = 185.0;

    mavlink::common::msg::ATTITUDE packet1{};
    mavlink::common::msg::ATTITUDE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet1.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet1.yawspeed, packet2.yawspeed);
}

#ifdef TEST_INTEROP
TEST(common_interop, ATTITUDE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_attitude_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0
    };

    mavlink::common::msg::ATTITUDE packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.roll = 45.0;
    packet_in.pitch = 73.0;
    packet_in.yaw = 101.0;
    packet_in.rollspeed = 129.0;
    packet_in.pitchspeed = 157.0;
    packet_in.yawspeed = 185.0;

    mavlink::common::msg::ATTITUDE packet2{};

    mavlink_msg_attitude_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet_in.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet_in.yawspeed, packet2.yawspeed);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ATTITUDE_QUATERNION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ATTITUDE_QUATERNION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.q1 = 45.0;
    packet_in.q2 = 73.0;
    packet_in.q3 = 101.0;
    packet_in.q4 = 129.0;
    packet_in.rollspeed = 157.0;
    packet_in.pitchspeed = 185.0;
    packet_in.yawspeed = 213.0;

    mavlink::common::msg::ATTITUDE_QUATERNION packet1{};
    mavlink::common::msg::ATTITUDE_QUATERNION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.q1, packet2.q1);
    EXPECT_EQ(packet1.q2, packet2.q2);
    EXPECT_EQ(packet1.q3, packet2.q3);
    EXPECT_EQ(packet1.q4, packet2.q4);
    EXPECT_EQ(packet1.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet1.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet1.yawspeed, packet2.yawspeed);
}

#ifdef TEST_INTEROP
TEST(common_interop, ATTITUDE_QUATERNION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_attitude_quaternion_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0
    };

    mavlink::common::msg::ATTITUDE_QUATERNION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.q1 = 45.0;
    packet_in.q2 = 73.0;
    packet_in.q3 = 101.0;
    packet_in.q4 = 129.0;
    packet_in.rollspeed = 157.0;
    packet_in.pitchspeed = 185.0;
    packet_in.yawspeed = 213.0;

    mavlink::common::msg::ATTITUDE_QUATERNION packet2{};

    mavlink_msg_attitude_quaternion_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.q1, packet2.q1);
    EXPECT_EQ(packet_in.q2, packet2.q2);
    EXPECT_EQ(packet_in.q3, packet2.q3);
    EXPECT_EQ(packet_in.q4, packet2.q4);
    EXPECT_EQ(packet_in.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet_in.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet_in.yawspeed, packet2.yawspeed);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOCAL_POSITION_NED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOCAL_POSITION_NED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;

    mavlink::common::msg::LOCAL_POSITION_NED packet1{};
    mavlink::common::msg::LOCAL_POSITION_NED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOCAL_POSITION_NED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_local_position_ned_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0
    };

    mavlink::common::msg::LOCAL_POSITION_NED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;

    mavlink::common::msg::LOCAL_POSITION_NED packet2{};

    mavlink_msg_local_position_ned_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GLOBAL_POSITION_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GLOBAL_POSITION_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.lat = 963497672;
    packet_in.lon = 963497880;
    packet_in.alt = 963498088;
    packet_in.relative_alt = 963498296;
    packet_in.vx = 18275;
    packet_in.vy = 18379;
    packet_in.vz = 18483;
    packet_in.hdg = 18587;

    mavlink::common::msg::GLOBAL_POSITION_INT packet1{};
    mavlink::common::msg::GLOBAL_POSITION_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.relative_alt, packet2.relative_alt);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.hdg, packet2.hdg);
}

#ifdef TEST_INTEROP
TEST(common_interop, GLOBAL_POSITION_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_global_position_int_t packet_c {
         963497464, 963497672, 963497880, 963498088, 963498296, 18275, 18379, 18483, 18587
    };

    mavlink::common::msg::GLOBAL_POSITION_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.lat = 963497672;
    packet_in.lon = 963497880;
    packet_in.alt = 963498088;
    packet_in.relative_alt = 963498296;
    packet_in.vx = 18275;
    packet_in.vy = 18379;
    packet_in.vz = 18483;
    packet_in.hdg = 18587;

    mavlink::common::msg::GLOBAL_POSITION_INT packet2{};

    mavlink_msg_global_position_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.relative_alt, packet2.relative_alt);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.hdg, packet2.hdg);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RC_CHANNELS_SCALED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RC_CHANNELS_SCALED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.port = 65;
    packet_in.chan1_scaled = 17443;
    packet_in.chan2_scaled = 17547;
    packet_in.chan3_scaled = 17651;
    packet_in.chan4_scaled = 17755;
    packet_in.chan5_scaled = 17859;
    packet_in.chan6_scaled = 17963;
    packet_in.chan7_scaled = 18067;
    packet_in.chan8_scaled = 18171;
    packet_in.rssi = 132;

    mavlink::common::msg::RC_CHANNELS_SCALED packet1{};
    mavlink::common::msg::RC_CHANNELS_SCALED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.port, packet2.port);
    EXPECT_EQ(packet1.chan1_scaled, packet2.chan1_scaled);
    EXPECT_EQ(packet1.chan2_scaled, packet2.chan2_scaled);
    EXPECT_EQ(packet1.chan3_scaled, packet2.chan3_scaled);
    EXPECT_EQ(packet1.chan4_scaled, packet2.chan4_scaled);
    EXPECT_EQ(packet1.chan5_scaled, packet2.chan5_scaled);
    EXPECT_EQ(packet1.chan6_scaled, packet2.chan6_scaled);
    EXPECT_EQ(packet1.chan7_scaled, packet2.chan7_scaled);
    EXPECT_EQ(packet1.chan8_scaled, packet2.chan8_scaled);
    EXPECT_EQ(packet1.rssi, packet2.rssi);
}

#ifdef TEST_INTEROP
TEST(common_interop, RC_CHANNELS_SCALED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rc_channels_scaled_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 65, 132
    };

    mavlink::common::msg::RC_CHANNELS_SCALED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.port = 65;
    packet_in.chan1_scaled = 17443;
    packet_in.chan2_scaled = 17547;
    packet_in.chan3_scaled = 17651;
    packet_in.chan4_scaled = 17755;
    packet_in.chan5_scaled = 17859;
    packet_in.chan6_scaled = 17963;
    packet_in.chan7_scaled = 18067;
    packet_in.chan8_scaled = 18171;
    packet_in.rssi = 132;

    mavlink::common::msg::RC_CHANNELS_SCALED packet2{};

    mavlink_msg_rc_channels_scaled_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.port, packet2.port);
    EXPECT_EQ(packet_in.chan1_scaled, packet2.chan1_scaled);
    EXPECT_EQ(packet_in.chan2_scaled, packet2.chan2_scaled);
    EXPECT_EQ(packet_in.chan3_scaled, packet2.chan3_scaled);
    EXPECT_EQ(packet_in.chan4_scaled, packet2.chan4_scaled);
    EXPECT_EQ(packet_in.chan5_scaled, packet2.chan5_scaled);
    EXPECT_EQ(packet_in.chan6_scaled, packet2.chan6_scaled);
    EXPECT_EQ(packet_in.chan7_scaled, packet2.chan7_scaled);
    EXPECT_EQ(packet_in.chan8_scaled, packet2.chan8_scaled);
    EXPECT_EQ(packet_in.rssi, packet2.rssi);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RC_CHANNELS_RAW)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RC_CHANNELS_RAW packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.port = 65;
    packet_in.chan1_raw = 17443;
    packet_in.chan2_raw = 17547;
    packet_in.chan3_raw = 17651;
    packet_in.chan4_raw = 17755;
    packet_in.chan5_raw = 17859;
    packet_in.chan6_raw = 17963;
    packet_in.chan7_raw = 18067;
    packet_in.chan8_raw = 18171;
    packet_in.rssi = 132;

    mavlink::common::msg::RC_CHANNELS_RAW packet1{};
    mavlink::common::msg::RC_CHANNELS_RAW packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.port, packet2.port);
    EXPECT_EQ(packet1.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet1.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet1.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet1.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet1.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet1.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet1.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet1.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet1.rssi, packet2.rssi);
}

#ifdef TEST_INTEROP
TEST(common_interop, RC_CHANNELS_RAW)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rc_channels_raw_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 65, 132
    };

    mavlink::common::msg::RC_CHANNELS_RAW packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.port = 65;
    packet_in.chan1_raw = 17443;
    packet_in.chan2_raw = 17547;
    packet_in.chan3_raw = 17651;
    packet_in.chan4_raw = 17755;
    packet_in.chan5_raw = 17859;
    packet_in.chan6_raw = 17963;
    packet_in.chan7_raw = 18067;
    packet_in.chan8_raw = 18171;
    packet_in.rssi = 132;

    mavlink::common::msg::RC_CHANNELS_RAW packet2{};

    mavlink_msg_rc_channels_raw_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.port, packet2.port);
    EXPECT_EQ(packet_in.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet_in.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet_in.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet_in.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet_in.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet_in.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet_in.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet_in.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet_in.rssi, packet2.rssi);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SERVO_OUTPUT_RAW)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SERVO_OUTPUT_RAW packet_in{};
    packet_in.time_usec = 963497464;
    packet_in.port = 65;
    packet_in.servo1_raw = 17443;
    packet_in.servo2_raw = 17547;
    packet_in.servo3_raw = 17651;
    packet_in.servo4_raw = 17755;
    packet_in.servo5_raw = 17859;
    packet_in.servo6_raw = 17963;
    packet_in.servo7_raw = 18067;
    packet_in.servo8_raw = 18171;
    packet_in.servo9_raw = 18327;
    packet_in.servo10_raw = 18431;
    packet_in.servo11_raw = 18535;
    packet_in.servo12_raw = 18639;
    packet_in.servo13_raw = 18743;
    packet_in.servo14_raw = 18847;
    packet_in.servo15_raw = 18951;
    packet_in.servo16_raw = 19055;

    mavlink::common::msg::SERVO_OUTPUT_RAW packet1{};
    mavlink::common::msg::SERVO_OUTPUT_RAW packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.port, packet2.port);
    EXPECT_EQ(packet1.servo1_raw, packet2.servo1_raw);
    EXPECT_EQ(packet1.servo2_raw, packet2.servo2_raw);
    EXPECT_EQ(packet1.servo3_raw, packet2.servo3_raw);
    EXPECT_EQ(packet1.servo4_raw, packet2.servo4_raw);
    EXPECT_EQ(packet1.servo5_raw, packet2.servo5_raw);
    EXPECT_EQ(packet1.servo6_raw, packet2.servo6_raw);
    EXPECT_EQ(packet1.servo7_raw, packet2.servo7_raw);
    EXPECT_EQ(packet1.servo8_raw, packet2.servo8_raw);
    EXPECT_EQ(packet1.servo9_raw, packet2.servo9_raw);
    EXPECT_EQ(packet1.servo10_raw, packet2.servo10_raw);
    EXPECT_EQ(packet1.servo11_raw, packet2.servo11_raw);
    EXPECT_EQ(packet1.servo12_raw, packet2.servo12_raw);
    EXPECT_EQ(packet1.servo13_raw, packet2.servo13_raw);
    EXPECT_EQ(packet1.servo14_raw, packet2.servo14_raw);
    EXPECT_EQ(packet1.servo15_raw, packet2.servo15_raw);
    EXPECT_EQ(packet1.servo16_raw, packet2.servo16_raw);
}

#ifdef TEST_INTEROP
TEST(common_interop, SERVO_OUTPUT_RAW)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_servo_output_raw_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 65, 18327, 18431, 18535, 18639, 18743, 18847, 18951, 19055
    };

    mavlink::common::msg::SERVO_OUTPUT_RAW packet_in{};
    packet_in.time_usec = 963497464;
    packet_in.port = 65;
    packet_in.servo1_raw = 17443;
    packet_in.servo2_raw = 17547;
    packet_in.servo3_raw = 17651;
    packet_in.servo4_raw = 17755;
    packet_in.servo5_raw = 17859;
    packet_in.servo6_raw = 17963;
    packet_in.servo7_raw = 18067;
    packet_in.servo8_raw = 18171;
    packet_in.servo9_raw = 18327;
    packet_in.servo10_raw = 18431;
    packet_in.servo11_raw = 18535;
    packet_in.servo12_raw = 18639;
    packet_in.servo13_raw = 18743;
    packet_in.servo14_raw = 18847;
    packet_in.servo15_raw = 18951;
    packet_in.servo16_raw = 19055;

    mavlink::common::msg::SERVO_OUTPUT_RAW packet2{};

    mavlink_msg_servo_output_raw_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.port, packet2.port);
    EXPECT_EQ(packet_in.servo1_raw, packet2.servo1_raw);
    EXPECT_EQ(packet_in.servo2_raw, packet2.servo2_raw);
    EXPECT_EQ(packet_in.servo3_raw, packet2.servo3_raw);
    EXPECT_EQ(packet_in.servo4_raw, packet2.servo4_raw);
    EXPECT_EQ(packet_in.servo5_raw, packet2.servo5_raw);
    EXPECT_EQ(packet_in.servo6_raw, packet2.servo6_raw);
    EXPECT_EQ(packet_in.servo7_raw, packet2.servo7_raw);
    EXPECT_EQ(packet_in.servo8_raw, packet2.servo8_raw);
    EXPECT_EQ(packet_in.servo9_raw, packet2.servo9_raw);
    EXPECT_EQ(packet_in.servo10_raw, packet2.servo10_raw);
    EXPECT_EQ(packet_in.servo11_raw, packet2.servo11_raw);
    EXPECT_EQ(packet_in.servo12_raw, packet2.servo12_raw);
    EXPECT_EQ(packet_in.servo13_raw, packet2.servo13_raw);
    EXPECT_EQ(packet_in.servo14_raw, packet2.servo14_raw);
    EXPECT_EQ(packet_in.servo15_raw, packet2.servo15_raw);
    EXPECT_EQ(packet_in.servo16_raw, packet2.servo16_raw);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_REQUEST_PARTIAL_LIST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_REQUEST_PARTIAL_LIST packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.start_index = 17235;
    packet_in.end_index = 17339;
    packet_in.mission_type = 151;

    mavlink::common::msg::MISSION_REQUEST_PARTIAL_LIST packet1{};
    mavlink::common::msg::MISSION_REQUEST_PARTIAL_LIST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.start_index, packet2.start_index);
    EXPECT_EQ(packet1.end_index, packet2.end_index);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_REQUEST_PARTIAL_LIST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_request_partial_list_t packet_c {
         17235, 17339, 17, 84, 151
    };

    mavlink::common::msg::MISSION_REQUEST_PARTIAL_LIST packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.start_index = 17235;
    packet_in.end_index = 17339;
    packet_in.mission_type = 151;

    mavlink::common::msg::MISSION_REQUEST_PARTIAL_LIST packet2{};

    mavlink_msg_mission_request_partial_list_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.start_index, packet2.start_index);
    EXPECT_EQ(packet_in.end_index, packet2.end_index);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_WRITE_PARTIAL_LIST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_WRITE_PARTIAL_LIST packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.start_index = 17235;
    packet_in.end_index = 17339;
    packet_in.mission_type = 151;

    mavlink::common::msg::MISSION_WRITE_PARTIAL_LIST packet1{};
    mavlink::common::msg::MISSION_WRITE_PARTIAL_LIST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.start_index, packet2.start_index);
    EXPECT_EQ(packet1.end_index, packet2.end_index);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_WRITE_PARTIAL_LIST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_write_partial_list_t packet_c {
         17235, 17339, 17, 84, 151
    };

    mavlink::common::msg::MISSION_WRITE_PARTIAL_LIST packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.start_index = 17235;
    packet_in.end_index = 17339;
    packet_in.mission_type = 151;

    mavlink::common::msg::MISSION_WRITE_PARTIAL_LIST packet2{};

    mavlink_msg_mission_write_partial_list_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.start_index, packet2.start_index);
    EXPECT_EQ(packet_in.end_index, packet2.end_index);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_ITEM)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_ITEM packet_in{};
    packet_in.target_system = 101;
    packet_in.target_component = 168;
    packet_in.seq = 18691;
    packet_in.frame = 235;
    packet_in.command = 18795;
    packet_in.current = 46;
    packet_in.autocontinue = 113;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.x = 129.0;
    packet_in.y = 157.0;
    packet_in.z = 185.0;
    packet_in.mission_type = 180;

    mavlink::common::msg::MISSION_ITEM packet1{};
    mavlink::common::msg::MISSION_ITEM packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seq, packet2.seq);
    EXPECT_EQ(packet1.frame, packet2.frame);
    EXPECT_EQ(packet1.command, packet2.command);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.autocontinue, packet2.autocontinue);
    EXPECT_EQ(packet1.param1, packet2.param1);
    EXPECT_EQ(packet1.param2, packet2.param2);
    EXPECT_EQ(packet1.param3, packet2.param3);
    EXPECT_EQ(packet1.param4, packet2.param4);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_ITEM)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_item_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 18691, 18795, 101, 168, 235, 46, 113, 180
    };

    mavlink::common::msg::MISSION_ITEM packet_in{};
    packet_in.target_system = 101;
    packet_in.target_component = 168;
    packet_in.seq = 18691;
    packet_in.frame = 235;
    packet_in.command = 18795;
    packet_in.current = 46;
    packet_in.autocontinue = 113;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.x = 129.0;
    packet_in.y = 157.0;
    packet_in.z = 185.0;
    packet_in.mission_type = 180;

    mavlink::common::msg::MISSION_ITEM packet2{};

    mavlink_msg_mission_item_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seq, packet2.seq);
    EXPECT_EQ(packet_in.frame, packet2.frame);
    EXPECT_EQ(packet_in.command, packet2.command);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.autocontinue, packet2.autocontinue);
    EXPECT_EQ(packet_in.param1, packet2.param1);
    EXPECT_EQ(packet_in.param2, packet2.param2);
    EXPECT_EQ(packet_in.param3, packet2.param3);
    EXPECT_EQ(packet_in.param4, packet2.param4);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_REQUEST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_REQUEST packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.seq = 17235;
    packet_in.mission_type = 17;

    mavlink::common::msg::MISSION_REQUEST packet1{};
    mavlink::common::msg::MISSION_REQUEST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seq, packet2.seq);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_REQUEST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_request_t packet_c {
         17235, 139, 206, 17
    };

    mavlink::common::msg::MISSION_REQUEST packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.seq = 17235;
    packet_in.mission_type = 17;

    mavlink::common::msg::MISSION_REQUEST packet2{};

    mavlink_msg_mission_request_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seq, packet2.seq);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_SET_CURRENT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_SET_CURRENT packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.seq = 17235;

    mavlink::common::msg::MISSION_SET_CURRENT packet1{};
    mavlink::common::msg::MISSION_SET_CURRENT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seq, packet2.seq);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_SET_CURRENT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_set_current_t packet_c {
         17235, 139, 206
    };

    mavlink::common::msg::MISSION_SET_CURRENT packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.seq = 17235;

    mavlink::common::msg::MISSION_SET_CURRENT packet2{};

    mavlink_msg_mission_set_current_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seq, packet2.seq);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_CURRENT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_CURRENT packet_in{};
    packet_in.seq = 17235;

    mavlink::common::msg::MISSION_CURRENT packet1{};
    mavlink::common::msg::MISSION_CURRENT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.seq, packet2.seq);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_CURRENT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_current_t packet_c {
         17235
    };

    mavlink::common::msg::MISSION_CURRENT packet_in{};
    packet_in.seq = 17235;

    mavlink::common::msg::MISSION_CURRENT packet2{};

    mavlink_msg_mission_current_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.seq, packet2.seq);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_REQUEST_LIST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_REQUEST_LIST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.mission_type = 139;

    mavlink::common::msg::MISSION_REQUEST_LIST packet1{};
    mavlink::common::msg::MISSION_REQUEST_LIST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_REQUEST_LIST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_request_list_t packet_c {
         5, 72, 139
    };

    mavlink::common::msg::MISSION_REQUEST_LIST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.mission_type = 139;

    mavlink::common::msg::MISSION_REQUEST_LIST packet2{};

    mavlink_msg_mission_request_list_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_COUNT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_COUNT packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.count = 17235;
    packet_in.mission_type = 17;

    mavlink::common::msg::MISSION_COUNT packet1{};
    mavlink::common::msg::MISSION_COUNT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_COUNT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_count_t packet_c {
         17235, 139, 206, 17
    };

    mavlink::common::msg::MISSION_COUNT packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.count = 17235;
    packet_in.mission_type = 17;

    mavlink::common::msg::MISSION_COUNT packet2{};

    mavlink_msg_mission_count_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_CLEAR_ALL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_CLEAR_ALL packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.mission_type = 139;

    mavlink::common::msg::MISSION_CLEAR_ALL packet1{};
    mavlink::common::msg::MISSION_CLEAR_ALL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_CLEAR_ALL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_clear_all_t packet_c {
         5, 72, 139
    };

    mavlink::common::msg::MISSION_CLEAR_ALL packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.mission_type = 139;

    mavlink::common::msg::MISSION_CLEAR_ALL packet2{};

    mavlink_msg_mission_clear_all_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_ITEM_REACHED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_ITEM_REACHED packet_in{};
    packet_in.seq = 17235;

    mavlink::common::msg::MISSION_ITEM_REACHED packet1{};
    mavlink::common::msg::MISSION_ITEM_REACHED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.seq, packet2.seq);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_ITEM_REACHED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_item_reached_t packet_c {
         17235
    };

    mavlink::common::msg::MISSION_ITEM_REACHED packet_in{};
    packet_in.seq = 17235;

    mavlink::common::msg::MISSION_ITEM_REACHED packet2{};

    mavlink_msg_mission_item_reached_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.seq, packet2.seq);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_ACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_ACK packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.type = 139;
    packet_in.mission_type = 206;

    mavlink::common::msg::MISSION_ACK packet1{};
    mavlink::common::msg::MISSION_ACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_ACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_ack_t packet_c {
         5, 72, 139, 206
    };

    mavlink::common::msg::MISSION_ACK packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.type = 139;
    packet_in.mission_type = 206;

    mavlink::common::msg::MISSION_ACK packet2{};

    mavlink_msg_mission_ack_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_GPS_GLOBAL_ORIGIN)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_GPS_GLOBAL_ORIGIN packet_in{};
    packet_in.target_system = 41;
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.time_usec = 93372036854776626ULL;

    mavlink::common::msg::SET_GPS_GLOBAL_ORIGIN packet1{};
    mavlink::common::msg::SET_GPS_GLOBAL_ORIGIN packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.latitude, packet2.latitude);
    EXPECT_EQ(packet1.longitude, packet2.longitude);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_GPS_GLOBAL_ORIGIN)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_gps_global_origin_t packet_c {
         963497464, 963497672, 963497880, 41, 93372036854776626ULL
    };

    mavlink::common::msg::SET_GPS_GLOBAL_ORIGIN packet_in{};
    packet_in.target_system = 41;
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.time_usec = 93372036854776626ULL;

    mavlink::common::msg::SET_GPS_GLOBAL_ORIGIN packet2{};

    mavlink_msg_set_gps_global_origin_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.latitude, packet2.latitude);
    EXPECT_EQ(packet_in.longitude, packet2.longitude);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_GLOBAL_ORIGIN)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_GLOBAL_ORIGIN packet_in{};
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.time_usec = 93372036854776563ULL;

    mavlink::common::msg::GPS_GLOBAL_ORIGIN packet1{};
    mavlink::common::msg::GPS_GLOBAL_ORIGIN packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.latitude, packet2.latitude);
    EXPECT_EQ(packet1.longitude, packet2.longitude);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_GLOBAL_ORIGIN)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_global_origin_t packet_c {
         963497464, 963497672, 963497880, 93372036854776563ULL
    };

    mavlink::common::msg::GPS_GLOBAL_ORIGIN packet_in{};
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.time_usec = 93372036854776563ULL;

    mavlink::common::msg::GPS_GLOBAL_ORIGIN packet2{};

    mavlink_msg_gps_global_origin_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.latitude, packet2.latitude);
    EXPECT_EQ(packet_in.longitude, packet2.longitude);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PARAM_MAP_RC)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PARAM_MAP_RC packet_in{};
    packet_in.target_system = 187;
    packet_in.target_component = 254;
    packet_in.param_id = to_char_array("UVWXYZABCDEFGHI");
    packet_in.param_index = 18067;
    packet_in.parameter_rc_channel_index = 113;
    packet_in.param_value0 = 17.0;
    packet_in.scale = 45.0;
    packet_in.param_value_min = 73.0;
    packet_in.param_value_max = 101.0;

    mavlink::common::msg::PARAM_MAP_RC packet1{};
    mavlink::common::msg::PARAM_MAP_RC packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.param_id, packet2.param_id);
    EXPECT_EQ(packet1.param_index, packet2.param_index);
    EXPECT_EQ(packet1.parameter_rc_channel_index, packet2.parameter_rc_channel_index);
    EXPECT_EQ(packet1.param_value0, packet2.param_value0);
    EXPECT_EQ(packet1.scale, packet2.scale);
    EXPECT_EQ(packet1.param_value_min, packet2.param_value_min);
    EXPECT_EQ(packet1.param_value_max, packet2.param_value_max);
}

#ifdef TEST_INTEROP
TEST(common_interop, PARAM_MAP_RC)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_param_map_rc_t packet_c {
         17.0, 45.0, 73.0, 101.0, 18067, 187, 254, "UVWXYZABCDEFGHI", 113
    };

    mavlink::common::msg::PARAM_MAP_RC packet_in{};
    packet_in.target_system = 187;
    packet_in.target_component = 254;
    packet_in.param_id = to_char_array("UVWXYZABCDEFGHI");
    packet_in.param_index = 18067;
    packet_in.parameter_rc_channel_index = 113;
    packet_in.param_value0 = 17.0;
    packet_in.scale = 45.0;
    packet_in.param_value_min = 73.0;
    packet_in.param_value_max = 101.0;

    mavlink::common::msg::PARAM_MAP_RC packet2{};

    mavlink_msg_param_map_rc_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.param_id, packet2.param_id);
    EXPECT_EQ(packet_in.param_index, packet2.param_index);
    EXPECT_EQ(packet_in.parameter_rc_channel_index, packet2.parameter_rc_channel_index);
    EXPECT_EQ(packet_in.param_value0, packet2.param_value0);
    EXPECT_EQ(packet_in.scale, packet2.scale);
    EXPECT_EQ(packet_in.param_value_min, packet2.param_value_min);
    EXPECT_EQ(packet_in.param_value_max, packet2.param_value_max);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_REQUEST_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_REQUEST_INT packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.seq = 17235;
    packet_in.mission_type = 17;

    mavlink::common::msg::MISSION_REQUEST_INT packet1{};
    mavlink::common::msg::MISSION_REQUEST_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seq, packet2.seq);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_REQUEST_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_request_int_t packet_c {
         17235, 139, 206, 17
    };

    mavlink::common::msg::MISSION_REQUEST_INT packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.seq = 17235;
    packet_in.mission_type = 17;

    mavlink::common::msg::MISSION_REQUEST_INT packet2{};

    mavlink_msg_mission_request_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seq, packet2.seq);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SAFETY_SET_ALLOWED_AREA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SAFETY_SET_ALLOWED_AREA packet_in{};
    packet_in.target_system = 77;
    packet_in.target_component = 144;
    packet_in.frame = 211;
    packet_in.p1x = 17.0;
    packet_in.p1y = 45.0;
    packet_in.p1z = 73.0;
    packet_in.p2x = 101.0;
    packet_in.p2y = 129.0;
    packet_in.p2z = 157.0;

    mavlink::common::msg::SAFETY_SET_ALLOWED_AREA packet1{};
    mavlink::common::msg::SAFETY_SET_ALLOWED_AREA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.frame, packet2.frame);
    EXPECT_EQ(packet1.p1x, packet2.p1x);
    EXPECT_EQ(packet1.p1y, packet2.p1y);
    EXPECT_EQ(packet1.p1z, packet2.p1z);
    EXPECT_EQ(packet1.p2x, packet2.p2x);
    EXPECT_EQ(packet1.p2y, packet2.p2y);
    EXPECT_EQ(packet1.p2z, packet2.p2z);
}

#ifdef TEST_INTEROP
TEST(common_interop, SAFETY_SET_ALLOWED_AREA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_safety_set_allowed_area_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 77, 144, 211
    };

    mavlink::common::msg::SAFETY_SET_ALLOWED_AREA packet_in{};
    packet_in.target_system = 77;
    packet_in.target_component = 144;
    packet_in.frame = 211;
    packet_in.p1x = 17.0;
    packet_in.p1y = 45.0;
    packet_in.p1z = 73.0;
    packet_in.p2x = 101.0;
    packet_in.p2y = 129.0;
    packet_in.p2z = 157.0;

    mavlink::common::msg::SAFETY_SET_ALLOWED_AREA packet2{};

    mavlink_msg_safety_set_allowed_area_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.frame, packet2.frame);
    EXPECT_EQ(packet_in.p1x, packet2.p1x);
    EXPECT_EQ(packet_in.p1y, packet2.p1y);
    EXPECT_EQ(packet_in.p1z, packet2.p1z);
    EXPECT_EQ(packet_in.p2x, packet2.p2x);
    EXPECT_EQ(packet_in.p2y, packet2.p2y);
    EXPECT_EQ(packet_in.p2z, packet2.p2z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SAFETY_ALLOWED_AREA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SAFETY_ALLOWED_AREA packet_in{};
    packet_in.frame = 77;
    packet_in.p1x = 17.0;
    packet_in.p1y = 45.0;
    packet_in.p1z = 73.0;
    packet_in.p2x = 101.0;
    packet_in.p2y = 129.0;
    packet_in.p2z = 157.0;

    mavlink::common::msg::SAFETY_ALLOWED_AREA packet1{};
    mavlink::common::msg::SAFETY_ALLOWED_AREA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.frame, packet2.frame);
    EXPECT_EQ(packet1.p1x, packet2.p1x);
    EXPECT_EQ(packet1.p1y, packet2.p1y);
    EXPECT_EQ(packet1.p1z, packet2.p1z);
    EXPECT_EQ(packet1.p2x, packet2.p2x);
    EXPECT_EQ(packet1.p2y, packet2.p2y);
    EXPECT_EQ(packet1.p2z, packet2.p2z);
}

#ifdef TEST_INTEROP
TEST(common_interop, SAFETY_ALLOWED_AREA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_safety_allowed_area_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 77
    };

    mavlink::common::msg::SAFETY_ALLOWED_AREA packet_in{};
    packet_in.frame = 77;
    packet_in.p1x = 17.0;
    packet_in.p1y = 45.0;
    packet_in.p1z = 73.0;
    packet_in.p2x = 101.0;
    packet_in.p2y = 129.0;
    packet_in.p2z = 157.0;

    mavlink::common::msg::SAFETY_ALLOWED_AREA packet2{};

    mavlink_msg_safety_allowed_area_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.frame, packet2.frame);
    EXPECT_EQ(packet_in.p1x, packet2.p1x);
    EXPECT_EQ(packet_in.p1y, packet2.p1y);
    EXPECT_EQ(packet_in.p1z, packet2.p1z);
    EXPECT_EQ(packet_in.p2x, packet2.p2x);
    EXPECT_EQ(packet_in.p2y, packet2.p2y);
    EXPECT_EQ(packet_in.p2z, packet2.p2z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ATTITUDE_QUATERNION_COV)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ATTITUDE_QUATERNION_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.q = {{ 73.0, 74.0, 75.0, 76.0 }};
    packet_in.rollspeed = 185.0;
    packet_in.pitchspeed = 213.0;
    packet_in.yawspeed = 241.0;
    packet_in.covariance = {{ 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0 }};

    mavlink::common::msg::ATTITUDE_QUATERNION_COV packet1{};
    mavlink::common::msg::ATTITUDE_QUATERNION_COV packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet1.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet1.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, ATTITUDE_QUATERNION_COV)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_attitude_quaternion_cov_t packet_c {
         93372036854775807ULL, { 73.0, 74.0, 75.0, 76.0 }, 185.0, 213.0, 241.0, { 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0 }
    };

    mavlink::common::msg::ATTITUDE_QUATERNION_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.q = {{ 73.0, 74.0, 75.0, 76.0 }};
    packet_in.rollspeed = 185.0;
    packet_in.pitchspeed = 213.0;
    packet_in.yawspeed = 241.0;
    packet_in.covariance = {{ 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0 }};

    mavlink::common::msg::ATTITUDE_QUATERNION_COV packet2{};

    mavlink_msg_attitude_quaternion_cov_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet_in.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet_in.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, NAV_CONTROLLER_OUTPUT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::NAV_CONTROLLER_OUTPUT packet_in{};
    packet_in.nav_roll = 17.0;
    packet_in.nav_pitch = 45.0;
    packet_in.nav_bearing = 18275;
    packet_in.target_bearing = 18379;
    packet_in.wp_dist = 18483;
    packet_in.alt_error = 73.0;
    packet_in.aspd_error = 101.0;
    packet_in.xtrack_error = 129.0;

    mavlink::common::msg::NAV_CONTROLLER_OUTPUT packet1{};
    mavlink::common::msg::NAV_CONTROLLER_OUTPUT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.nav_roll, packet2.nav_roll);
    EXPECT_EQ(packet1.nav_pitch, packet2.nav_pitch);
    EXPECT_EQ(packet1.nav_bearing, packet2.nav_bearing);
    EXPECT_EQ(packet1.target_bearing, packet2.target_bearing);
    EXPECT_EQ(packet1.wp_dist, packet2.wp_dist);
    EXPECT_EQ(packet1.alt_error, packet2.alt_error);
    EXPECT_EQ(packet1.aspd_error, packet2.aspd_error);
    EXPECT_EQ(packet1.xtrack_error, packet2.xtrack_error);
}

#ifdef TEST_INTEROP
TEST(common_interop, NAV_CONTROLLER_OUTPUT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_nav_controller_output_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 18275, 18379, 18483
    };

    mavlink::common::msg::NAV_CONTROLLER_OUTPUT packet_in{};
    packet_in.nav_roll = 17.0;
    packet_in.nav_pitch = 45.0;
    packet_in.nav_bearing = 18275;
    packet_in.target_bearing = 18379;
    packet_in.wp_dist = 18483;
    packet_in.alt_error = 73.0;
    packet_in.aspd_error = 101.0;
    packet_in.xtrack_error = 129.0;

    mavlink::common::msg::NAV_CONTROLLER_OUTPUT packet2{};

    mavlink_msg_nav_controller_output_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.nav_roll, packet2.nav_roll);
    EXPECT_EQ(packet_in.nav_pitch, packet2.nav_pitch);
    EXPECT_EQ(packet_in.nav_bearing, packet2.nav_bearing);
    EXPECT_EQ(packet_in.target_bearing, packet2.target_bearing);
    EXPECT_EQ(packet_in.wp_dist, packet2.wp_dist);
    EXPECT_EQ(packet_in.alt_error, packet2.alt_error);
    EXPECT_EQ(packet_in.aspd_error, packet2.aspd_error);
    EXPECT_EQ(packet_in.xtrack_error, packet2.xtrack_error);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GLOBAL_POSITION_INT_COV)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GLOBAL_POSITION_INT_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.estimator_type = 33;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.relative_alt = 963498504;
    packet_in.vx = 185.0;
    packet_in.vy = 213.0;
    packet_in.vz = 241.0;
    packet_in.covariance = {{ 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0, 296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0, 304.0 }};

    mavlink::common::msg::GLOBAL_POSITION_INT_COV packet1{};
    mavlink::common::msg::GLOBAL_POSITION_INT_COV packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.estimator_type, packet2.estimator_type);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.relative_alt, packet2.relative_alt);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, GLOBAL_POSITION_INT_COV)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_global_position_int_cov_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 963498296, 963498504, 185.0, 213.0, 241.0, { 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0, 296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0, 304.0 }, 33
    };

    mavlink::common::msg::GLOBAL_POSITION_INT_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.estimator_type = 33;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.relative_alt = 963498504;
    packet_in.vx = 185.0;
    packet_in.vy = 213.0;
    packet_in.vz = 241.0;
    packet_in.covariance = {{ 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 288.0, 289.0, 290.0, 291.0, 292.0, 293.0, 294.0, 295.0, 296.0, 297.0, 298.0, 299.0, 300.0, 301.0, 302.0, 303.0, 304.0 }};

    mavlink::common::msg::GLOBAL_POSITION_INT_COV packet2{};

    mavlink_msg_global_position_int_cov_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.estimator_type, packet2.estimator_type);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.relative_alt, packet2.relative_alt);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOCAL_POSITION_NED_COV)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOCAL_POSITION_NED_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.estimator_type = 165;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.vx = 157.0;
    packet_in.vy = 185.0;
    packet_in.vz = 213.0;
    packet_in.ax = 241.0;
    packet_in.ay = 269.0;
    packet_in.az = 297.0;
    packet_in.covariance = {{ 325.0, 326.0, 327.0, 328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0, 336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0, 344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0, 352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0, 360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0, 368.0, 369.0 }};

    mavlink::common::msg::LOCAL_POSITION_NED_COV packet1{};
    mavlink::common::msg::LOCAL_POSITION_NED_COV packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.estimator_type, packet2.estimator_type);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.ax, packet2.ax);
    EXPECT_EQ(packet1.ay, packet2.ay);
    EXPECT_EQ(packet1.az, packet2.az);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOCAL_POSITION_NED_COV)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_local_position_ned_cov_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, { 325.0, 326.0, 327.0, 328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0, 336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0, 344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0, 352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0, 360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0, 368.0, 369.0 }, 165
    };

    mavlink::common::msg::LOCAL_POSITION_NED_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.estimator_type = 165;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.vx = 157.0;
    packet_in.vy = 185.0;
    packet_in.vz = 213.0;
    packet_in.ax = 241.0;
    packet_in.ay = 269.0;
    packet_in.az = 297.0;
    packet_in.covariance = {{ 325.0, 326.0, 327.0, 328.0, 329.0, 330.0, 331.0, 332.0, 333.0, 334.0, 335.0, 336.0, 337.0, 338.0, 339.0, 340.0, 341.0, 342.0, 343.0, 344.0, 345.0, 346.0, 347.0, 348.0, 349.0, 350.0, 351.0, 352.0, 353.0, 354.0, 355.0, 356.0, 357.0, 358.0, 359.0, 360.0, 361.0, 362.0, 363.0, 364.0, 365.0, 366.0, 367.0, 368.0, 369.0 }};

    mavlink::common::msg::LOCAL_POSITION_NED_COV packet2{};

    mavlink_msg_local_position_ned_cov_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.estimator_type, packet2.estimator_type);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.ax, packet2.ax);
    EXPECT_EQ(packet_in.ay, packet2.ay);
    EXPECT_EQ(packet_in.az, packet2.az);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RC_CHANNELS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RC_CHANNELS packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.chancount = 125;
    packet_in.chan1_raw = 17443;
    packet_in.chan2_raw = 17547;
    packet_in.chan3_raw = 17651;
    packet_in.chan4_raw = 17755;
    packet_in.chan5_raw = 17859;
    packet_in.chan6_raw = 17963;
    packet_in.chan7_raw = 18067;
    packet_in.chan8_raw = 18171;
    packet_in.chan9_raw = 18275;
    packet_in.chan10_raw = 18379;
    packet_in.chan11_raw = 18483;
    packet_in.chan12_raw = 18587;
    packet_in.chan13_raw = 18691;
    packet_in.chan14_raw = 18795;
    packet_in.chan15_raw = 18899;
    packet_in.chan16_raw = 19003;
    packet_in.chan17_raw = 19107;
    packet_in.chan18_raw = 19211;
    packet_in.rssi = 192;

    mavlink::common::msg::RC_CHANNELS packet1{};
    mavlink::common::msg::RC_CHANNELS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.chancount, packet2.chancount);
    EXPECT_EQ(packet1.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet1.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet1.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet1.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet1.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet1.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet1.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet1.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet1.chan9_raw, packet2.chan9_raw);
    EXPECT_EQ(packet1.chan10_raw, packet2.chan10_raw);
    EXPECT_EQ(packet1.chan11_raw, packet2.chan11_raw);
    EXPECT_EQ(packet1.chan12_raw, packet2.chan12_raw);
    EXPECT_EQ(packet1.chan13_raw, packet2.chan13_raw);
    EXPECT_EQ(packet1.chan14_raw, packet2.chan14_raw);
    EXPECT_EQ(packet1.chan15_raw, packet2.chan15_raw);
    EXPECT_EQ(packet1.chan16_raw, packet2.chan16_raw);
    EXPECT_EQ(packet1.chan17_raw, packet2.chan17_raw);
    EXPECT_EQ(packet1.chan18_raw, packet2.chan18_raw);
    EXPECT_EQ(packet1.rssi, packet2.rssi);
}

#ifdef TEST_INTEROP
TEST(common_interop, RC_CHANNELS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rc_channels_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 18275, 18379, 18483, 18587, 18691, 18795, 18899, 19003, 19107, 19211, 125, 192
    };

    mavlink::common::msg::RC_CHANNELS packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.chancount = 125;
    packet_in.chan1_raw = 17443;
    packet_in.chan2_raw = 17547;
    packet_in.chan3_raw = 17651;
    packet_in.chan4_raw = 17755;
    packet_in.chan5_raw = 17859;
    packet_in.chan6_raw = 17963;
    packet_in.chan7_raw = 18067;
    packet_in.chan8_raw = 18171;
    packet_in.chan9_raw = 18275;
    packet_in.chan10_raw = 18379;
    packet_in.chan11_raw = 18483;
    packet_in.chan12_raw = 18587;
    packet_in.chan13_raw = 18691;
    packet_in.chan14_raw = 18795;
    packet_in.chan15_raw = 18899;
    packet_in.chan16_raw = 19003;
    packet_in.chan17_raw = 19107;
    packet_in.chan18_raw = 19211;
    packet_in.rssi = 192;

    mavlink::common::msg::RC_CHANNELS packet2{};

    mavlink_msg_rc_channels_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.chancount, packet2.chancount);
    EXPECT_EQ(packet_in.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet_in.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet_in.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet_in.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet_in.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet_in.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet_in.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet_in.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet_in.chan9_raw, packet2.chan9_raw);
    EXPECT_EQ(packet_in.chan10_raw, packet2.chan10_raw);
    EXPECT_EQ(packet_in.chan11_raw, packet2.chan11_raw);
    EXPECT_EQ(packet_in.chan12_raw, packet2.chan12_raw);
    EXPECT_EQ(packet_in.chan13_raw, packet2.chan13_raw);
    EXPECT_EQ(packet_in.chan14_raw, packet2.chan14_raw);
    EXPECT_EQ(packet_in.chan15_raw, packet2.chan15_raw);
    EXPECT_EQ(packet_in.chan16_raw, packet2.chan16_raw);
    EXPECT_EQ(packet_in.chan17_raw, packet2.chan17_raw);
    EXPECT_EQ(packet_in.chan18_raw, packet2.chan18_raw);
    EXPECT_EQ(packet_in.rssi, packet2.rssi);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, REQUEST_DATA_STREAM)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::REQUEST_DATA_STREAM packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.req_stream_id = 17;
    packet_in.req_message_rate = 17235;
    packet_in.start_stop = 84;

    mavlink::common::msg::REQUEST_DATA_STREAM packet1{};
    mavlink::common::msg::REQUEST_DATA_STREAM packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.req_stream_id, packet2.req_stream_id);
    EXPECT_EQ(packet1.req_message_rate, packet2.req_message_rate);
    EXPECT_EQ(packet1.start_stop, packet2.start_stop);
}

#ifdef TEST_INTEROP
TEST(common_interop, REQUEST_DATA_STREAM)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_request_data_stream_t packet_c {
         17235, 139, 206, 17, 84
    };

    mavlink::common::msg::REQUEST_DATA_STREAM packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.req_stream_id = 17;
    packet_in.req_message_rate = 17235;
    packet_in.start_stop = 84;

    mavlink::common::msg::REQUEST_DATA_STREAM packet2{};

    mavlink_msg_request_data_stream_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.req_stream_id, packet2.req_stream_id);
    EXPECT_EQ(packet_in.req_message_rate, packet2.req_message_rate);
    EXPECT_EQ(packet_in.start_stop, packet2.start_stop);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, DATA_STREAM)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::DATA_STREAM packet_in{};
    packet_in.stream_id = 139;
    packet_in.message_rate = 17235;
    packet_in.on_off = 206;

    mavlink::common::msg::DATA_STREAM packet1{};
    mavlink::common::msg::DATA_STREAM packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.stream_id, packet2.stream_id);
    EXPECT_EQ(packet1.message_rate, packet2.message_rate);
    EXPECT_EQ(packet1.on_off, packet2.on_off);
}

#ifdef TEST_INTEROP
TEST(common_interop, DATA_STREAM)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_data_stream_t packet_c {
         17235, 139, 206
    };

    mavlink::common::msg::DATA_STREAM packet_in{};
    packet_in.stream_id = 139;
    packet_in.message_rate = 17235;
    packet_in.on_off = 206;

    mavlink::common::msg::DATA_STREAM packet2{};

    mavlink_msg_data_stream_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.stream_id, packet2.stream_id);
    EXPECT_EQ(packet_in.message_rate, packet2.message_rate);
    EXPECT_EQ(packet_in.on_off, packet2.on_off);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MANUAL_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MANUAL_CONTROL packet_in{};
    packet_in.target = 163;
    packet_in.x = 17235;
    packet_in.y = 17339;
    packet_in.z = 17443;
    packet_in.r = 17547;
    packet_in.buttons = 17651;

    mavlink::common::msg::MANUAL_CONTROL packet1{};
    mavlink::common::msg::MANUAL_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target, packet2.target);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.r, packet2.r);
    EXPECT_EQ(packet1.buttons, packet2.buttons);
}

#ifdef TEST_INTEROP
TEST(common_interop, MANUAL_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_manual_control_t packet_c {
         17235, 17339, 17443, 17547, 17651, 163
    };

    mavlink::common::msg::MANUAL_CONTROL packet_in{};
    packet_in.target = 163;
    packet_in.x = 17235;
    packet_in.y = 17339;
    packet_in.z = 17443;
    packet_in.r = 17547;
    packet_in.buttons = 17651;

    mavlink::common::msg::MANUAL_CONTROL packet2{};

    mavlink_msg_manual_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target, packet2.target);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.r, packet2.r);
    EXPECT_EQ(packet_in.buttons, packet2.buttons);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RC_CHANNELS_OVERRIDE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RC_CHANNELS_OVERRIDE packet_in{};
    packet_in.target_system = 53;
    packet_in.target_component = 120;
    packet_in.chan1_raw = 17235;
    packet_in.chan2_raw = 17339;
    packet_in.chan3_raw = 17443;
    packet_in.chan4_raw = 17547;
    packet_in.chan5_raw = 17651;
    packet_in.chan6_raw = 17755;
    packet_in.chan7_raw = 17859;
    packet_in.chan8_raw = 17963;
    packet_in.chan9_raw = 18171;
    packet_in.chan10_raw = 18275;
    packet_in.chan11_raw = 18379;
    packet_in.chan12_raw = 18483;
    packet_in.chan13_raw = 18587;
    packet_in.chan14_raw = 18691;
    packet_in.chan15_raw = 18795;
    packet_in.chan16_raw = 18899;
    packet_in.chan17_raw = 19003;
    packet_in.chan18_raw = 19107;

    mavlink::common::msg::RC_CHANNELS_OVERRIDE packet1{};
    mavlink::common::msg::RC_CHANNELS_OVERRIDE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet1.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet1.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet1.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet1.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet1.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet1.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet1.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet1.chan9_raw, packet2.chan9_raw);
    EXPECT_EQ(packet1.chan10_raw, packet2.chan10_raw);
    EXPECT_EQ(packet1.chan11_raw, packet2.chan11_raw);
    EXPECT_EQ(packet1.chan12_raw, packet2.chan12_raw);
    EXPECT_EQ(packet1.chan13_raw, packet2.chan13_raw);
    EXPECT_EQ(packet1.chan14_raw, packet2.chan14_raw);
    EXPECT_EQ(packet1.chan15_raw, packet2.chan15_raw);
    EXPECT_EQ(packet1.chan16_raw, packet2.chan16_raw);
    EXPECT_EQ(packet1.chan17_raw, packet2.chan17_raw);
    EXPECT_EQ(packet1.chan18_raw, packet2.chan18_raw);
}

#ifdef TEST_INTEROP
TEST(common_interop, RC_CHANNELS_OVERRIDE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rc_channels_override_t packet_c {
         17235, 17339, 17443, 17547, 17651, 17755, 17859, 17963, 53, 120, 18171, 18275, 18379, 18483, 18587, 18691, 18795, 18899, 19003, 19107
    };

    mavlink::common::msg::RC_CHANNELS_OVERRIDE packet_in{};
    packet_in.target_system = 53;
    packet_in.target_component = 120;
    packet_in.chan1_raw = 17235;
    packet_in.chan2_raw = 17339;
    packet_in.chan3_raw = 17443;
    packet_in.chan4_raw = 17547;
    packet_in.chan5_raw = 17651;
    packet_in.chan6_raw = 17755;
    packet_in.chan7_raw = 17859;
    packet_in.chan8_raw = 17963;
    packet_in.chan9_raw = 18171;
    packet_in.chan10_raw = 18275;
    packet_in.chan11_raw = 18379;
    packet_in.chan12_raw = 18483;
    packet_in.chan13_raw = 18587;
    packet_in.chan14_raw = 18691;
    packet_in.chan15_raw = 18795;
    packet_in.chan16_raw = 18899;
    packet_in.chan17_raw = 19003;
    packet_in.chan18_raw = 19107;

    mavlink::common::msg::RC_CHANNELS_OVERRIDE packet2{};

    mavlink_msg_rc_channels_override_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet_in.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet_in.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet_in.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet_in.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet_in.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet_in.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet_in.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet_in.chan9_raw, packet2.chan9_raw);
    EXPECT_EQ(packet_in.chan10_raw, packet2.chan10_raw);
    EXPECT_EQ(packet_in.chan11_raw, packet2.chan11_raw);
    EXPECT_EQ(packet_in.chan12_raw, packet2.chan12_raw);
    EXPECT_EQ(packet_in.chan13_raw, packet2.chan13_raw);
    EXPECT_EQ(packet_in.chan14_raw, packet2.chan14_raw);
    EXPECT_EQ(packet_in.chan15_raw, packet2.chan15_raw);
    EXPECT_EQ(packet_in.chan16_raw, packet2.chan16_raw);
    EXPECT_EQ(packet_in.chan17_raw, packet2.chan17_raw);
    EXPECT_EQ(packet_in.chan18_raw, packet2.chan18_raw);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MISSION_ITEM_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MISSION_ITEM_INT packet_in{};
    packet_in.target_system = 101;
    packet_in.target_component = 168;
    packet_in.seq = 18691;
    packet_in.frame = 235;
    packet_in.command = 18795;
    packet_in.current = 46;
    packet_in.autocontinue = 113;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.x = 963498296;
    packet_in.y = 963498504;
    packet_in.z = 185.0;
    packet_in.mission_type = 180;

    mavlink::common::msg::MISSION_ITEM_INT packet1{};
    mavlink::common::msg::MISSION_ITEM_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seq, packet2.seq);
    EXPECT_EQ(packet1.frame, packet2.frame);
    EXPECT_EQ(packet1.command, packet2.command);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.autocontinue, packet2.autocontinue);
    EXPECT_EQ(packet1.param1, packet2.param1);
    EXPECT_EQ(packet1.param2, packet2.param2);
    EXPECT_EQ(packet1.param3, packet2.param3);
    EXPECT_EQ(packet1.param4, packet2.param4);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.mission_type, packet2.mission_type);
}

#ifdef TEST_INTEROP
TEST(common_interop, MISSION_ITEM_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mission_item_int_t packet_c {
         17.0, 45.0, 73.0, 101.0, 963498296, 963498504, 185.0, 18691, 18795, 101, 168, 235, 46, 113, 180
    };

    mavlink::common::msg::MISSION_ITEM_INT packet_in{};
    packet_in.target_system = 101;
    packet_in.target_component = 168;
    packet_in.seq = 18691;
    packet_in.frame = 235;
    packet_in.command = 18795;
    packet_in.current = 46;
    packet_in.autocontinue = 113;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.x = 963498296;
    packet_in.y = 963498504;
    packet_in.z = 185.0;
    packet_in.mission_type = 180;

    mavlink::common::msg::MISSION_ITEM_INT packet2{};

    mavlink_msg_mission_item_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seq, packet2.seq);
    EXPECT_EQ(packet_in.frame, packet2.frame);
    EXPECT_EQ(packet_in.command, packet2.command);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.autocontinue, packet2.autocontinue);
    EXPECT_EQ(packet_in.param1, packet2.param1);
    EXPECT_EQ(packet_in.param2, packet2.param2);
    EXPECT_EQ(packet_in.param3, packet2.param3);
    EXPECT_EQ(packet_in.param4, packet2.param4);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.mission_type, packet2.mission_type);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, VFR_HUD)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::VFR_HUD packet_in{};
    packet_in.airspeed = 17.0;
    packet_in.groundspeed = 45.0;
    packet_in.heading = 18067;
    packet_in.throttle = 18171;
    packet_in.alt = 73.0;
    packet_in.climb = 101.0;

    mavlink::common::msg::VFR_HUD packet1{};
    mavlink::common::msg::VFR_HUD packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.airspeed, packet2.airspeed);
    EXPECT_EQ(packet1.groundspeed, packet2.groundspeed);
    EXPECT_EQ(packet1.heading, packet2.heading);
    EXPECT_EQ(packet1.throttle, packet2.throttle);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.climb, packet2.climb);
}

#ifdef TEST_INTEROP
TEST(common_interop, VFR_HUD)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_vfr_hud_t packet_c {
         17.0, 45.0, 73.0, 101.0, 18067, 18171
    };

    mavlink::common::msg::VFR_HUD packet_in{};
    packet_in.airspeed = 17.0;
    packet_in.groundspeed = 45.0;
    packet_in.heading = 18067;
    packet_in.throttle = 18171;
    packet_in.alt = 73.0;
    packet_in.climb = 101.0;

    mavlink::common::msg::VFR_HUD packet2{};

    mavlink_msg_vfr_hud_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.airspeed, packet2.airspeed);
    EXPECT_EQ(packet_in.groundspeed, packet2.groundspeed);
    EXPECT_EQ(packet_in.heading, packet2.heading);
    EXPECT_EQ(packet_in.throttle, packet2.throttle);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.climb, packet2.climb);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, COMMAND_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::COMMAND_INT packet_in{};
    packet_in.target_system = 223;
    packet_in.target_component = 34;
    packet_in.frame = 101;
    packet_in.command = 18691;
    packet_in.current = 168;
    packet_in.autocontinue = 235;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.x = 963498296;
    packet_in.y = 963498504;
    packet_in.z = 185.0;

    mavlink::common::msg::COMMAND_INT packet1{};
    mavlink::common::msg::COMMAND_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.frame, packet2.frame);
    EXPECT_EQ(packet1.command, packet2.command);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.autocontinue, packet2.autocontinue);
    EXPECT_EQ(packet1.param1, packet2.param1);
    EXPECT_EQ(packet1.param2, packet2.param2);
    EXPECT_EQ(packet1.param3, packet2.param3);
    EXPECT_EQ(packet1.param4, packet2.param4);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
}

#ifdef TEST_INTEROP
TEST(common_interop, COMMAND_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_command_int_t packet_c {
         17.0, 45.0, 73.0, 101.0, 963498296, 963498504, 185.0, 18691, 223, 34, 101, 168, 235
    };

    mavlink::common::msg::COMMAND_INT packet_in{};
    packet_in.target_system = 223;
    packet_in.target_component = 34;
    packet_in.frame = 101;
    packet_in.command = 18691;
    packet_in.current = 168;
    packet_in.autocontinue = 235;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.x = 963498296;
    packet_in.y = 963498504;
    packet_in.z = 185.0;

    mavlink::common::msg::COMMAND_INT packet2{};

    mavlink_msg_command_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.frame, packet2.frame);
    EXPECT_EQ(packet_in.command, packet2.command);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.autocontinue, packet2.autocontinue);
    EXPECT_EQ(packet_in.param1, packet2.param1);
    EXPECT_EQ(packet_in.param2, packet2.param2);
    EXPECT_EQ(packet_in.param3, packet2.param3);
    EXPECT_EQ(packet_in.param4, packet2.param4);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, COMMAND_LONG)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::COMMAND_LONG packet_in{};
    packet_in.target_system = 223;
    packet_in.target_component = 34;
    packet_in.command = 18691;
    packet_in.confirmation = 101;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.param5 = 129.0;
    packet_in.param6 = 157.0;
    packet_in.param7 = 185.0;

    mavlink::common::msg::COMMAND_LONG packet1{};
    mavlink::common::msg::COMMAND_LONG packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.command, packet2.command);
    EXPECT_EQ(packet1.confirmation, packet2.confirmation);
    EXPECT_EQ(packet1.param1, packet2.param1);
    EXPECT_EQ(packet1.param2, packet2.param2);
    EXPECT_EQ(packet1.param3, packet2.param3);
    EXPECT_EQ(packet1.param4, packet2.param4);
    EXPECT_EQ(packet1.param5, packet2.param5);
    EXPECT_EQ(packet1.param6, packet2.param6);
    EXPECT_EQ(packet1.param7, packet2.param7);
}

#ifdef TEST_INTEROP
TEST(common_interop, COMMAND_LONG)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_command_long_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 18691, 223, 34, 101
    };

    mavlink::common::msg::COMMAND_LONG packet_in{};
    packet_in.target_system = 223;
    packet_in.target_component = 34;
    packet_in.command = 18691;
    packet_in.confirmation = 101;
    packet_in.param1 = 17.0;
    packet_in.param2 = 45.0;
    packet_in.param3 = 73.0;
    packet_in.param4 = 101.0;
    packet_in.param5 = 129.0;
    packet_in.param6 = 157.0;
    packet_in.param7 = 185.0;

    mavlink::common::msg::COMMAND_LONG packet2{};

    mavlink_msg_command_long_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.command, packet2.command);
    EXPECT_EQ(packet_in.confirmation, packet2.confirmation);
    EXPECT_EQ(packet_in.param1, packet2.param1);
    EXPECT_EQ(packet_in.param2, packet2.param2);
    EXPECT_EQ(packet_in.param3, packet2.param3);
    EXPECT_EQ(packet_in.param4, packet2.param4);
    EXPECT_EQ(packet_in.param5, packet2.param5);
    EXPECT_EQ(packet_in.param6, packet2.param6);
    EXPECT_EQ(packet_in.param7, packet2.param7);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, COMMAND_ACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::COMMAND_ACK packet_in{};
    packet_in.command = 17235;
    packet_in.result = 139;

    mavlink::common::msg::COMMAND_ACK packet1{};
    mavlink::common::msg::COMMAND_ACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.command, packet2.command);
    EXPECT_EQ(packet1.result, packet2.result);
}

#ifdef TEST_INTEROP
TEST(common_interop, COMMAND_ACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_command_ack_t packet_c {
         17235, 139
    };

    mavlink::common::msg::COMMAND_ACK packet_in{};
    packet_in.command = 17235;
    packet_in.result = 139;

    mavlink::common::msg::COMMAND_ACK packet2{};

    mavlink_msg_command_ack_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.command, packet2.command);
    EXPECT_EQ(packet_in.result, packet2.result);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MANUAL_SETPOINT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MANUAL_SETPOINT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.roll = 45.0;
    packet_in.pitch = 73.0;
    packet_in.yaw = 101.0;
    packet_in.thrust = 129.0;
    packet_in.mode_switch = 65;
    packet_in.manual_override_switch = 132;

    mavlink::common::msg::MANUAL_SETPOINT packet1{};
    mavlink::common::msg::MANUAL_SETPOINT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.thrust, packet2.thrust);
    EXPECT_EQ(packet1.mode_switch, packet2.mode_switch);
    EXPECT_EQ(packet1.manual_override_switch, packet2.manual_override_switch);
}

#ifdef TEST_INTEROP
TEST(common_interop, MANUAL_SETPOINT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_manual_setpoint_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 65, 132
    };

    mavlink::common::msg::MANUAL_SETPOINT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.roll = 45.0;
    packet_in.pitch = 73.0;
    packet_in.yaw = 101.0;
    packet_in.thrust = 129.0;
    packet_in.mode_switch = 65;
    packet_in.manual_override_switch = 132;

    mavlink::common::msg::MANUAL_SETPOINT packet2{};

    mavlink_msg_manual_setpoint_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.thrust, packet2.thrust);
    EXPECT_EQ(packet_in.mode_switch, packet2.mode_switch);
    EXPECT_EQ(packet_in.manual_override_switch, packet2.manual_override_switch);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_ATTITUDE_TARGET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_ATTITUDE_TARGET packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.target_system = 113;
    packet_in.target_component = 180;
    packet_in.type_mask = 247;
    packet_in.q = {{ 45.0, 46.0, 47.0, 48.0 }};
    packet_in.body_roll_rate = 157.0;
    packet_in.body_pitch_rate = 185.0;
    packet_in.body_yaw_rate = 213.0;
    packet_in.thrust = 241.0;

    mavlink::common::msg::SET_ATTITUDE_TARGET packet1{};
    mavlink::common::msg::SET_ATTITUDE_TARGET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.type_mask, packet2.type_mask);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.body_roll_rate, packet2.body_roll_rate);
    EXPECT_EQ(packet1.body_pitch_rate, packet2.body_pitch_rate);
    EXPECT_EQ(packet1.body_yaw_rate, packet2.body_yaw_rate);
    EXPECT_EQ(packet1.thrust, packet2.thrust);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_ATTITUDE_TARGET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_attitude_target_t packet_c {
         963497464, { 45.0, 46.0, 47.0, 48.0 }, 157.0, 185.0, 213.0, 241.0, 113, 180, 247
    };

    mavlink::common::msg::SET_ATTITUDE_TARGET packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.target_system = 113;
    packet_in.target_component = 180;
    packet_in.type_mask = 247;
    packet_in.q = {{ 45.0, 46.0, 47.0, 48.0 }};
    packet_in.body_roll_rate = 157.0;
    packet_in.body_pitch_rate = 185.0;
    packet_in.body_yaw_rate = 213.0;
    packet_in.thrust = 241.0;

    mavlink::common::msg::SET_ATTITUDE_TARGET packet2{};

    mavlink_msg_set_attitude_target_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.type_mask, packet2.type_mask);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.body_roll_rate, packet2.body_roll_rate);
    EXPECT_EQ(packet_in.body_pitch_rate, packet2.body_pitch_rate);
    EXPECT_EQ(packet_in.body_yaw_rate, packet2.body_yaw_rate);
    EXPECT_EQ(packet_in.thrust, packet2.thrust);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ATTITUDE_TARGET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ATTITUDE_TARGET packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.type_mask = 113;
    packet_in.q = {{ 45.0, 46.0, 47.0, 48.0 }};
    packet_in.body_roll_rate = 157.0;
    packet_in.body_pitch_rate = 185.0;
    packet_in.body_yaw_rate = 213.0;
    packet_in.thrust = 241.0;

    mavlink::common::msg::ATTITUDE_TARGET packet1{};
    mavlink::common::msg::ATTITUDE_TARGET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.type_mask, packet2.type_mask);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.body_roll_rate, packet2.body_roll_rate);
    EXPECT_EQ(packet1.body_pitch_rate, packet2.body_pitch_rate);
    EXPECT_EQ(packet1.body_yaw_rate, packet2.body_yaw_rate);
    EXPECT_EQ(packet1.thrust, packet2.thrust);
}

#ifdef TEST_INTEROP
TEST(common_interop, ATTITUDE_TARGET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_attitude_target_t packet_c {
         963497464, { 45.0, 46.0, 47.0, 48.0 }, 157.0, 185.0, 213.0, 241.0, 113
    };

    mavlink::common::msg::ATTITUDE_TARGET packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.type_mask = 113;
    packet_in.q = {{ 45.0, 46.0, 47.0, 48.0 }};
    packet_in.body_roll_rate = 157.0;
    packet_in.body_pitch_rate = 185.0;
    packet_in.body_yaw_rate = 213.0;
    packet_in.thrust = 241.0;

    mavlink::common::msg::ATTITUDE_TARGET packet2{};

    mavlink_msg_attitude_target_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.type_mask, packet2.type_mask);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.body_roll_rate, packet2.body_roll_rate);
    EXPECT_EQ(packet_in.body_pitch_rate, packet2.body_pitch_rate);
    EXPECT_EQ(packet_in.body_yaw_rate, packet2.body_yaw_rate);
    EXPECT_EQ(packet_in.thrust, packet2.thrust);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_POSITION_TARGET_LOCAL_NED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_POSITION_TARGET_LOCAL_NED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.target_system = 27;
    packet_in.target_component = 94;
    packet_in.coordinate_frame = 161;
    packet_in.type_mask = 19731;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::SET_POSITION_TARGET_LOCAL_NED packet1{};
    mavlink::common::msg::SET_POSITION_TARGET_LOCAL_NED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet1.type_mask, packet2.type_mask);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.afx, packet2.afx);
    EXPECT_EQ(packet1.afy, packet2.afy);
    EXPECT_EQ(packet1.afz, packet2.afz);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.yaw_rate, packet2.yaw_rate);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_POSITION_TARGET_LOCAL_NED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_position_target_local_ned_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 19731, 27, 94, 161
    };

    mavlink::common::msg::SET_POSITION_TARGET_LOCAL_NED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.target_system = 27;
    packet_in.target_component = 94;
    packet_in.coordinate_frame = 161;
    packet_in.type_mask = 19731;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::SET_POSITION_TARGET_LOCAL_NED packet2{};

    mavlink_msg_set_position_target_local_ned_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet_in.type_mask, packet2.type_mask);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.afx, packet2.afx);
    EXPECT_EQ(packet_in.afy, packet2.afy);
    EXPECT_EQ(packet_in.afz, packet2.afz);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.yaw_rate, packet2.yaw_rate);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, POSITION_TARGET_LOCAL_NED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::POSITION_TARGET_LOCAL_NED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.coordinate_frame = 27;
    packet_in.type_mask = 19731;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::POSITION_TARGET_LOCAL_NED packet1{};
    mavlink::common::msg::POSITION_TARGET_LOCAL_NED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet1.type_mask, packet2.type_mask);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.afx, packet2.afx);
    EXPECT_EQ(packet1.afy, packet2.afy);
    EXPECT_EQ(packet1.afz, packet2.afz);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.yaw_rate, packet2.yaw_rate);
}

#ifdef TEST_INTEROP
TEST(common_interop, POSITION_TARGET_LOCAL_NED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_position_target_local_ned_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 19731, 27
    };

    mavlink::common::msg::POSITION_TARGET_LOCAL_NED packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.coordinate_frame = 27;
    packet_in.type_mask = 19731;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::POSITION_TARGET_LOCAL_NED packet2{};

    mavlink_msg_position_target_local_ned_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet_in.type_mask, packet2.type_mask);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.afx, packet2.afx);
    EXPECT_EQ(packet_in.afy, packet2.afy);
    EXPECT_EQ(packet_in.afz, packet2.afz);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.yaw_rate, packet2.yaw_rate);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_POSITION_TARGET_GLOBAL_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_POSITION_TARGET_GLOBAL_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.target_system = 27;
    packet_in.target_component = 94;
    packet_in.coordinate_frame = 161;
    packet_in.type_mask = 19731;
    packet_in.lat_int = 963497672;
    packet_in.lon_int = 963497880;
    packet_in.alt = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::SET_POSITION_TARGET_GLOBAL_INT packet1{};
    mavlink::common::msg::SET_POSITION_TARGET_GLOBAL_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet1.type_mask, packet2.type_mask);
    EXPECT_EQ(packet1.lat_int, packet2.lat_int);
    EXPECT_EQ(packet1.lon_int, packet2.lon_int);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.afx, packet2.afx);
    EXPECT_EQ(packet1.afy, packet2.afy);
    EXPECT_EQ(packet1.afz, packet2.afz);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.yaw_rate, packet2.yaw_rate);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_POSITION_TARGET_GLOBAL_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_position_target_global_int_t packet_c {
         963497464, 963497672, 963497880, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 19731, 27, 94, 161
    };

    mavlink::common::msg::SET_POSITION_TARGET_GLOBAL_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.target_system = 27;
    packet_in.target_component = 94;
    packet_in.coordinate_frame = 161;
    packet_in.type_mask = 19731;
    packet_in.lat_int = 963497672;
    packet_in.lon_int = 963497880;
    packet_in.alt = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::SET_POSITION_TARGET_GLOBAL_INT packet2{};

    mavlink_msg_set_position_target_global_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet_in.type_mask, packet2.type_mask);
    EXPECT_EQ(packet_in.lat_int, packet2.lat_int);
    EXPECT_EQ(packet_in.lon_int, packet2.lon_int);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.afx, packet2.afx);
    EXPECT_EQ(packet_in.afy, packet2.afy);
    EXPECT_EQ(packet_in.afz, packet2.afz);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.yaw_rate, packet2.yaw_rate);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, POSITION_TARGET_GLOBAL_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::POSITION_TARGET_GLOBAL_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.coordinate_frame = 27;
    packet_in.type_mask = 19731;
    packet_in.lat_int = 963497672;
    packet_in.lon_int = 963497880;
    packet_in.alt = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::POSITION_TARGET_GLOBAL_INT packet1{};
    mavlink::common::msg::POSITION_TARGET_GLOBAL_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet1.type_mask, packet2.type_mask);
    EXPECT_EQ(packet1.lat_int, packet2.lat_int);
    EXPECT_EQ(packet1.lon_int, packet2.lon_int);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.afx, packet2.afx);
    EXPECT_EQ(packet1.afy, packet2.afy);
    EXPECT_EQ(packet1.afz, packet2.afz);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.yaw_rate, packet2.yaw_rate);
}

#ifdef TEST_INTEROP
TEST(common_interop, POSITION_TARGET_GLOBAL_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_position_target_global_int_t packet_c {
         963497464, 963497672, 963497880, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 19731, 27
    };

    mavlink::common::msg::POSITION_TARGET_GLOBAL_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.coordinate_frame = 27;
    packet_in.type_mask = 19731;
    packet_in.lat_int = 963497672;
    packet_in.lon_int = 963497880;
    packet_in.alt = 101.0;
    packet_in.vx = 129.0;
    packet_in.vy = 157.0;
    packet_in.vz = 185.0;
    packet_in.afx = 213.0;
    packet_in.afy = 241.0;
    packet_in.afz = 269.0;
    packet_in.yaw = 297.0;
    packet_in.yaw_rate = 325.0;

    mavlink::common::msg::POSITION_TARGET_GLOBAL_INT packet2{};

    mavlink_msg_position_target_global_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.coordinate_frame, packet2.coordinate_frame);
    EXPECT_EQ(packet_in.type_mask, packet2.type_mask);
    EXPECT_EQ(packet_in.lat_int, packet2.lat_int);
    EXPECT_EQ(packet_in.lon_int, packet2.lon_int);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.afx, packet2.afx);
    EXPECT_EQ(packet_in.afy, packet2.afy);
    EXPECT_EQ(packet_in.afz, packet2.afz);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.yaw_rate, packet2.yaw_rate);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.roll = 129.0;
    packet_in.pitch = 157.0;
    packet_in.yaw = 185.0;

    mavlink::common::msg::LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET packet1{};
    mavlink::common::msg::LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_local_position_ned_system_global_offset_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0
    };

    mavlink::common::msg::LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.x = 45.0;
    packet_in.y = 73.0;
    packet_in.z = 101.0;
    packet_in.roll = 129.0;
    packet_in.pitch = 157.0;
    packet_in.yaw = 185.0;

    mavlink::common::msg::LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET packet2{};

    mavlink_msg_local_position_ned_system_global_offset_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_STATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_STATE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.roll = 73.0;
    packet_in.pitch = 101.0;
    packet_in.yaw = 129.0;
    packet_in.rollspeed = 157.0;
    packet_in.pitchspeed = 185.0;
    packet_in.yawspeed = 213.0;
    packet_in.lat = 963499128;
    packet_in.lon = 963499336;
    packet_in.alt = 963499544;
    packet_in.vx = 19523;
    packet_in.vy = 19627;
    packet_in.vz = 19731;
    packet_in.xacc = 19835;
    packet_in.yacc = 19939;
    packet_in.zacc = 20043;

    mavlink::common::msg::HIL_STATE packet1{};
    mavlink::common::msg::HIL_STATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet1.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet1.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_STATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_state_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 963499128, 963499336, 963499544, 19523, 19627, 19731, 19835, 19939, 20043
    };

    mavlink::common::msg::HIL_STATE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.roll = 73.0;
    packet_in.pitch = 101.0;
    packet_in.yaw = 129.0;
    packet_in.rollspeed = 157.0;
    packet_in.pitchspeed = 185.0;
    packet_in.yawspeed = 213.0;
    packet_in.lat = 963499128;
    packet_in.lon = 963499336;
    packet_in.alt = 963499544;
    packet_in.vx = 19523;
    packet_in.vy = 19627;
    packet_in.vz = 19731;
    packet_in.xacc = 19835;
    packet_in.yacc = 19939;
    packet_in.zacc = 20043;

    mavlink::common::msg::HIL_STATE packet2{};

    mavlink_msg_hil_state_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet_in.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet_in.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_CONTROLS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_CONTROLS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.roll_ailerons = 73.0;
    packet_in.pitch_elevator = 101.0;
    packet_in.yaw_rudder = 129.0;
    packet_in.throttle = 157.0;
    packet_in.aux1 = 185.0;
    packet_in.aux2 = 213.0;
    packet_in.aux3 = 241.0;
    packet_in.aux4 = 269.0;
    packet_in.mode = 125;
    packet_in.nav_mode = 192;

    mavlink::common::msg::HIL_CONTROLS packet1{};
    mavlink::common::msg::HIL_CONTROLS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.roll_ailerons, packet2.roll_ailerons);
    EXPECT_EQ(packet1.pitch_elevator, packet2.pitch_elevator);
    EXPECT_EQ(packet1.yaw_rudder, packet2.yaw_rudder);
    EXPECT_EQ(packet1.throttle, packet2.throttle);
    EXPECT_EQ(packet1.aux1, packet2.aux1);
    EXPECT_EQ(packet1.aux2, packet2.aux2);
    EXPECT_EQ(packet1.aux3, packet2.aux3);
    EXPECT_EQ(packet1.aux4, packet2.aux4);
    EXPECT_EQ(packet1.mode, packet2.mode);
    EXPECT_EQ(packet1.nav_mode, packet2.nav_mode);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_CONTROLS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_controls_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 125, 192
    };

    mavlink::common::msg::HIL_CONTROLS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.roll_ailerons = 73.0;
    packet_in.pitch_elevator = 101.0;
    packet_in.yaw_rudder = 129.0;
    packet_in.throttle = 157.0;
    packet_in.aux1 = 185.0;
    packet_in.aux2 = 213.0;
    packet_in.aux3 = 241.0;
    packet_in.aux4 = 269.0;
    packet_in.mode = 125;
    packet_in.nav_mode = 192;

    mavlink::common::msg::HIL_CONTROLS packet2{};

    mavlink_msg_hil_controls_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.roll_ailerons, packet2.roll_ailerons);
    EXPECT_EQ(packet_in.pitch_elevator, packet2.pitch_elevator);
    EXPECT_EQ(packet_in.yaw_rudder, packet2.yaw_rudder);
    EXPECT_EQ(packet_in.throttle, packet2.throttle);
    EXPECT_EQ(packet_in.aux1, packet2.aux1);
    EXPECT_EQ(packet_in.aux2, packet2.aux2);
    EXPECT_EQ(packet_in.aux3, packet2.aux3);
    EXPECT_EQ(packet_in.aux4, packet2.aux4);
    EXPECT_EQ(packet_in.mode, packet2.mode);
    EXPECT_EQ(packet_in.nav_mode, packet2.nav_mode);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_RC_INPUTS_RAW)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_RC_INPUTS_RAW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.chan1_raw = 17651;
    packet_in.chan2_raw = 17755;
    packet_in.chan3_raw = 17859;
    packet_in.chan4_raw = 17963;
    packet_in.chan5_raw = 18067;
    packet_in.chan6_raw = 18171;
    packet_in.chan7_raw = 18275;
    packet_in.chan8_raw = 18379;
    packet_in.chan9_raw = 18483;
    packet_in.chan10_raw = 18587;
    packet_in.chan11_raw = 18691;
    packet_in.chan12_raw = 18795;
    packet_in.rssi = 101;

    mavlink::common::msg::HIL_RC_INPUTS_RAW packet1{};
    mavlink::common::msg::HIL_RC_INPUTS_RAW packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet1.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet1.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet1.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet1.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet1.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet1.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet1.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet1.chan9_raw, packet2.chan9_raw);
    EXPECT_EQ(packet1.chan10_raw, packet2.chan10_raw);
    EXPECT_EQ(packet1.chan11_raw, packet2.chan11_raw);
    EXPECT_EQ(packet1.chan12_raw, packet2.chan12_raw);
    EXPECT_EQ(packet1.rssi, packet2.rssi);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_RC_INPUTS_RAW)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_rc_inputs_raw_t packet_c {
         93372036854775807ULL, 17651, 17755, 17859, 17963, 18067, 18171, 18275, 18379, 18483, 18587, 18691, 18795, 101
    };

    mavlink::common::msg::HIL_RC_INPUTS_RAW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.chan1_raw = 17651;
    packet_in.chan2_raw = 17755;
    packet_in.chan3_raw = 17859;
    packet_in.chan4_raw = 17963;
    packet_in.chan5_raw = 18067;
    packet_in.chan6_raw = 18171;
    packet_in.chan7_raw = 18275;
    packet_in.chan8_raw = 18379;
    packet_in.chan9_raw = 18483;
    packet_in.chan10_raw = 18587;
    packet_in.chan11_raw = 18691;
    packet_in.chan12_raw = 18795;
    packet_in.rssi = 101;

    mavlink::common::msg::HIL_RC_INPUTS_RAW packet2{};

    mavlink_msg_hil_rc_inputs_raw_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.chan1_raw, packet2.chan1_raw);
    EXPECT_EQ(packet_in.chan2_raw, packet2.chan2_raw);
    EXPECT_EQ(packet_in.chan3_raw, packet2.chan3_raw);
    EXPECT_EQ(packet_in.chan4_raw, packet2.chan4_raw);
    EXPECT_EQ(packet_in.chan5_raw, packet2.chan5_raw);
    EXPECT_EQ(packet_in.chan6_raw, packet2.chan6_raw);
    EXPECT_EQ(packet_in.chan7_raw, packet2.chan7_raw);
    EXPECT_EQ(packet_in.chan8_raw, packet2.chan8_raw);
    EXPECT_EQ(packet_in.chan9_raw, packet2.chan9_raw);
    EXPECT_EQ(packet_in.chan10_raw, packet2.chan10_raw);
    EXPECT_EQ(packet_in.chan11_raw, packet2.chan11_raw);
    EXPECT_EQ(packet_in.chan12_raw, packet2.chan12_raw);
    EXPECT_EQ(packet_in.rssi, packet2.rssi);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_ACTUATOR_CONTROLS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_ACTUATOR_CONTROLS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.controls = {{ 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 144.0 }};
    packet_in.mode = 245;
    packet_in.flags = 93372036854776311ULL;

    mavlink::common::msg::HIL_ACTUATOR_CONTROLS packet1{};
    mavlink::common::msg::HIL_ACTUATOR_CONTROLS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.controls, packet2.controls);
    EXPECT_EQ(packet1.mode, packet2.mode);
    EXPECT_EQ(packet1.flags, packet2.flags);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_ACTUATOR_CONTROLS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_actuator_controls_t packet_c {
         93372036854775807ULL, 93372036854776311ULL, { 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 144.0 }, 245
    };

    mavlink::common::msg::HIL_ACTUATOR_CONTROLS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.controls = {{ 129.0, 130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0, 140.0, 141.0, 142.0, 143.0, 144.0 }};
    packet_in.mode = 245;
    packet_in.flags = 93372036854776311ULL;

    mavlink::common::msg::HIL_ACTUATOR_CONTROLS packet2{};

    mavlink_msg_hil_actuator_controls_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.controls, packet2.controls);
    EXPECT_EQ(packet_in.mode, packet2.mode);
    EXPECT_EQ(packet_in.flags, packet2.flags);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, OPTICAL_FLOW)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::OPTICAL_FLOW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_id = 77;
    packet_in.flow_x = 18275;
    packet_in.flow_y = 18379;
    packet_in.flow_comp_m_x = 73.0;
    packet_in.flow_comp_m_y = 101.0;
    packet_in.quality = 144;
    packet_in.ground_distance = 129.0;
    packet_in.flow_rate_x = 199.0;
    packet_in.flow_rate_y = 227.0;

    mavlink::common::msg::OPTICAL_FLOW packet1{};
    mavlink::common::msg::OPTICAL_FLOW packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.sensor_id, packet2.sensor_id);
    EXPECT_EQ(packet1.flow_x, packet2.flow_x);
    EXPECT_EQ(packet1.flow_y, packet2.flow_y);
    EXPECT_EQ(packet1.flow_comp_m_x, packet2.flow_comp_m_x);
    EXPECT_EQ(packet1.flow_comp_m_y, packet2.flow_comp_m_y);
    EXPECT_EQ(packet1.quality, packet2.quality);
    EXPECT_EQ(packet1.ground_distance, packet2.ground_distance);
    EXPECT_EQ(packet1.flow_rate_x, packet2.flow_rate_x);
    EXPECT_EQ(packet1.flow_rate_y, packet2.flow_rate_y);
}

#ifdef TEST_INTEROP
TEST(common_interop, OPTICAL_FLOW)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_optical_flow_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 18275, 18379, 77, 144, 199.0, 227.0
    };

    mavlink::common::msg::OPTICAL_FLOW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_id = 77;
    packet_in.flow_x = 18275;
    packet_in.flow_y = 18379;
    packet_in.flow_comp_m_x = 73.0;
    packet_in.flow_comp_m_y = 101.0;
    packet_in.quality = 144;
    packet_in.ground_distance = 129.0;
    packet_in.flow_rate_x = 199.0;
    packet_in.flow_rate_y = 227.0;

    mavlink::common::msg::OPTICAL_FLOW packet2{};

    mavlink_msg_optical_flow_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.sensor_id, packet2.sensor_id);
    EXPECT_EQ(packet_in.flow_x, packet2.flow_x);
    EXPECT_EQ(packet_in.flow_y, packet2.flow_y);
    EXPECT_EQ(packet_in.flow_comp_m_x, packet2.flow_comp_m_x);
    EXPECT_EQ(packet_in.flow_comp_m_y, packet2.flow_comp_m_y);
    EXPECT_EQ(packet_in.quality, packet2.quality);
    EXPECT_EQ(packet_in.ground_distance, packet2.ground_distance);
    EXPECT_EQ(packet_in.flow_rate_x, packet2.flow_rate_x);
    EXPECT_EQ(packet_in.flow_rate_y, packet2.flow_rate_y);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GLOBAL_VISION_POSITION_ESTIMATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GLOBAL_VISION_POSITION_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.roll = 157.0;
    packet_in.pitch = 185.0;
    packet_in.yaw = 213.0;
    packet_in.covariance = {{ 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }};

    mavlink::common::msg::GLOBAL_VISION_POSITION_ESTIMATE packet1{};
    mavlink::common::msg::GLOBAL_VISION_POSITION_ESTIMATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.usec, packet2.usec);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, GLOBAL_VISION_POSITION_ESTIMATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_global_vision_position_estimate_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, { 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }
    };

    mavlink::common::msg::GLOBAL_VISION_POSITION_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.roll = 157.0;
    packet_in.pitch = 185.0;
    packet_in.yaw = 213.0;
    packet_in.covariance = {{ 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }};

    mavlink::common::msg::GLOBAL_VISION_POSITION_ESTIMATE packet2{};

    mavlink_msg_global_vision_position_estimate_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.usec, packet2.usec);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, VISION_POSITION_ESTIMATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::VISION_POSITION_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.roll = 157.0;
    packet_in.pitch = 185.0;
    packet_in.yaw = 213.0;
    packet_in.covariance = {{ 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }};

    mavlink::common::msg::VISION_POSITION_ESTIMATE packet1{};
    mavlink::common::msg::VISION_POSITION_ESTIMATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.usec, packet2.usec);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, VISION_POSITION_ESTIMATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_vision_position_estimate_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, { 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }
    };

    mavlink::common::msg::VISION_POSITION_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.roll = 157.0;
    packet_in.pitch = 185.0;
    packet_in.yaw = 213.0;
    packet_in.covariance = {{ 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }};

    mavlink::common::msg::VISION_POSITION_ESTIMATE packet2{};

    mavlink_msg_vision_position_estimate_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.usec, packet2.usec);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, VISION_SPEED_ESTIMATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::VISION_SPEED_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.covariance = {{ 157.0, 158.0, 159.0, 160.0, 161.0, 162.0, 163.0, 164.0, 165.0 }};

    mavlink::common::msg::VISION_SPEED_ESTIMATE packet1{};
    mavlink::common::msg::VISION_SPEED_ESTIMATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.usec, packet2.usec);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, VISION_SPEED_ESTIMATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_vision_speed_estimate_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, { 157.0, 158.0, 159.0, 160.0, 161.0, 162.0, 163.0, 164.0, 165.0 }
    };

    mavlink::common::msg::VISION_SPEED_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.covariance = {{ 157.0, 158.0, 159.0, 160.0, 161.0, 162.0, 163.0, 164.0, 165.0 }};

    mavlink::common::msg::VISION_SPEED_ESTIMATE packet2{};

    mavlink_msg_vision_speed_estimate_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.usec, packet2.usec);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, VICON_POSITION_ESTIMATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::VICON_POSITION_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.roll = 157.0;
    packet_in.pitch = 185.0;
    packet_in.yaw = 213.0;
    packet_in.covariance = {{ 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }};

    mavlink::common::msg::VICON_POSITION_ESTIMATE packet1{};
    mavlink::common::msg::VICON_POSITION_ESTIMATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.usec, packet2.usec);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, VICON_POSITION_ESTIMATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_vicon_position_estimate_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, { 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }
    };

    mavlink::common::msg::VICON_POSITION_ESTIMATE packet_in{};
    packet_in.usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.roll = 157.0;
    packet_in.pitch = 185.0;
    packet_in.yaw = 213.0;
    packet_in.covariance = {{ 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0, 250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 256.0, 257.0, 258.0, 259.0, 260.0, 261.0 }};

    mavlink::common::msg::VICON_POSITION_ESTIMATE packet2{};

    mavlink_msg_vicon_position_estimate_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.usec, packet2.usec);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIGHRES_IMU)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIGHRES_IMU packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.xacc = 73.0;
    packet_in.yacc = 101.0;
    packet_in.zacc = 129.0;
    packet_in.xgyro = 157.0;
    packet_in.ygyro = 185.0;
    packet_in.zgyro = 213.0;
    packet_in.xmag = 241.0;
    packet_in.ymag = 269.0;
    packet_in.zmag = 297.0;
    packet_in.abs_pressure = 325.0;
    packet_in.diff_pressure = 353.0;
    packet_in.pressure_alt = 381.0;
    packet_in.temperature = 409.0;
    packet_in.fields_updated = 20355;

    mavlink::common::msg::HIGHRES_IMU packet1{};
    mavlink::common::msg::HIGHRES_IMU packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.xmag, packet2.xmag);
    EXPECT_EQ(packet1.ymag, packet2.ymag);
    EXPECT_EQ(packet1.zmag, packet2.zmag);
    EXPECT_EQ(packet1.abs_pressure, packet2.abs_pressure);
    EXPECT_EQ(packet1.diff_pressure, packet2.diff_pressure);
    EXPECT_EQ(packet1.pressure_alt, packet2.pressure_alt);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.fields_updated, packet2.fields_updated);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIGHRES_IMU)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_highres_imu_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 353.0, 381.0, 409.0, 20355
    };

    mavlink::common::msg::HIGHRES_IMU packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.xacc = 73.0;
    packet_in.yacc = 101.0;
    packet_in.zacc = 129.0;
    packet_in.xgyro = 157.0;
    packet_in.ygyro = 185.0;
    packet_in.zgyro = 213.0;
    packet_in.xmag = 241.0;
    packet_in.ymag = 269.0;
    packet_in.zmag = 297.0;
    packet_in.abs_pressure = 325.0;
    packet_in.diff_pressure = 353.0;
    packet_in.pressure_alt = 381.0;
    packet_in.temperature = 409.0;
    packet_in.fields_updated = 20355;

    mavlink::common::msg::HIGHRES_IMU packet2{};

    mavlink_msg_highres_imu_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.xmag, packet2.xmag);
    EXPECT_EQ(packet_in.ymag, packet2.ymag);
    EXPECT_EQ(packet_in.zmag, packet2.zmag);
    EXPECT_EQ(packet_in.abs_pressure, packet2.abs_pressure);
    EXPECT_EQ(packet_in.diff_pressure, packet2.diff_pressure);
    EXPECT_EQ(packet_in.pressure_alt, packet2.pressure_alt);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.fields_updated, packet2.fields_updated);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, OPTICAL_FLOW_RAD)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::OPTICAL_FLOW_RAD packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_id = 3;
    packet_in.integration_time_us = 963497880;
    packet_in.integrated_x = 101.0;
    packet_in.integrated_y = 129.0;
    packet_in.integrated_xgyro = 157.0;
    packet_in.integrated_ygyro = 185.0;
    packet_in.integrated_zgyro = 213.0;
    packet_in.temperature = 19315;
    packet_in.quality = 70;
    packet_in.time_delta_distance_us = 963499128;
    packet_in.distance = 269.0;

    mavlink::common::msg::OPTICAL_FLOW_RAD packet1{};
    mavlink::common::msg::OPTICAL_FLOW_RAD packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.sensor_id, packet2.sensor_id);
    EXPECT_EQ(packet1.integration_time_us, packet2.integration_time_us);
    EXPECT_EQ(packet1.integrated_x, packet2.integrated_x);
    EXPECT_EQ(packet1.integrated_y, packet2.integrated_y);
    EXPECT_EQ(packet1.integrated_xgyro, packet2.integrated_xgyro);
    EXPECT_EQ(packet1.integrated_ygyro, packet2.integrated_ygyro);
    EXPECT_EQ(packet1.integrated_zgyro, packet2.integrated_zgyro);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.quality, packet2.quality);
    EXPECT_EQ(packet1.time_delta_distance_us, packet2.time_delta_distance_us);
    EXPECT_EQ(packet1.distance, packet2.distance);
}

#ifdef TEST_INTEROP
TEST(common_interop, OPTICAL_FLOW_RAD)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_optical_flow_rad_t packet_c {
         93372036854775807ULL, 963497880, 101.0, 129.0, 157.0, 185.0, 213.0, 963499128, 269.0, 19315, 3, 70
    };

    mavlink::common::msg::OPTICAL_FLOW_RAD packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_id = 3;
    packet_in.integration_time_us = 963497880;
    packet_in.integrated_x = 101.0;
    packet_in.integrated_y = 129.0;
    packet_in.integrated_xgyro = 157.0;
    packet_in.integrated_ygyro = 185.0;
    packet_in.integrated_zgyro = 213.0;
    packet_in.temperature = 19315;
    packet_in.quality = 70;
    packet_in.time_delta_distance_us = 963499128;
    packet_in.distance = 269.0;

    mavlink::common::msg::OPTICAL_FLOW_RAD packet2{};

    mavlink_msg_optical_flow_rad_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.sensor_id, packet2.sensor_id);
    EXPECT_EQ(packet_in.integration_time_us, packet2.integration_time_us);
    EXPECT_EQ(packet_in.integrated_x, packet2.integrated_x);
    EXPECT_EQ(packet_in.integrated_y, packet2.integrated_y);
    EXPECT_EQ(packet_in.integrated_xgyro, packet2.integrated_xgyro);
    EXPECT_EQ(packet_in.integrated_ygyro, packet2.integrated_ygyro);
    EXPECT_EQ(packet_in.integrated_zgyro, packet2.integrated_zgyro);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.quality, packet2.quality);
    EXPECT_EQ(packet_in.time_delta_distance_us, packet2.time_delta_distance_us);
    EXPECT_EQ(packet_in.distance, packet2.distance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_SENSOR)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_SENSOR packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.xacc = 73.0;
    packet_in.yacc = 101.0;
    packet_in.zacc = 129.0;
    packet_in.xgyro = 157.0;
    packet_in.ygyro = 185.0;
    packet_in.zgyro = 213.0;
    packet_in.xmag = 241.0;
    packet_in.ymag = 269.0;
    packet_in.zmag = 297.0;
    packet_in.abs_pressure = 325.0;
    packet_in.diff_pressure = 353.0;
    packet_in.pressure_alt = 381.0;
    packet_in.temperature = 409.0;
    packet_in.fields_updated = 963500584;

    mavlink::common::msg::HIL_SENSOR packet1{};
    mavlink::common::msg::HIL_SENSOR packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.xmag, packet2.xmag);
    EXPECT_EQ(packet1.ymag, packet2.ymag);
    EXPECT_EQ(packet1.zmag, packet2.zmag);
    EXPECT_EQ(packet1.abs_pressure, packet2.abs_pressure);
    EXPECT_EQ(packet1.diff_pressure, packet2.diff_pressure);
    EXPECT_EQ(packet1.pressure_alt, packet2.pressure_alt);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.fields_updated, packet2.fields_updated);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_SENSOR)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_sensor_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 353.0, 381.0, 409.0, 963500584
    };

    mavlink::common::msg::HIL_SENSOR packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.xacc = 73.0;
    packet_in.yacc = 101.0;
    packet_in.zacc = 129.0;
    packet_in.xgyro = 157.0;
    packet_in.ygyro = 185.0;
    packet_in.zgyro = 213.0;
    packet_in.xmag = 241.0;
    packet_in.ymag = 269.0;
    packet_in.zmag = 297.0;
    packet_in.abs_pressure = 325.0;
    packet_in.diff_pressure = 353.0;
    packet_in.pressure_alt = 381.0;
    packet_in.temperature = 409.0;
    packet_in.fields_updated = 963500584;

    mavlink::common::msg::HIL_SENSOR packet2{};

    mavlink_msg_hil_sensor_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.xmag, packet2.xmag);
    EXPECT_EQ(packet_in.ymag, packet2.ymag);
    EXPECT_EQ(packet_in.zmag, packet2.zmag);
    EXPECT_EQ(packet_in.abs_pressure, packet2.abs_pressure);
    EXPECT_EQ(packet_in.diff_pressure, packet2.diff_pressure);
    EXPECT_EQ(packet_in.pressure_alt, packet2.pressure_alt);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.fields_updated, packet2.fields_updated);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SIM_STATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SIM_STATE packet_in{};
    packet_in.q1 = 17.0;
    packet_in.q2 = 45.0;
    packet_in.q3 = 73.0;
    packet_in.q4 = 101.0;
    packet_in.roll = 129.0;
    packet_in.pitch = 157.0;
    packet_in.yaw = 185.0;
    packet_in.xacc = 213.0;
    packet_in.yacc = 241.0;
    packet_in.zacc = 269.0;
    packet_in.xgyro = 297.0;
    packet_in.ygyro = 325.0;
    packet_in.zgyro = 353.0;
    packet_in.lat = 381.0;
    packet_in.lon = 409.0;
    packet_in.alt = 437.0;
    packet_in.std_dev_horz = 465.0;
    packet_in.std_dev_vert = 493.0;
    packet_in.vn = 521.0;
    packet_in.ve = 549.0;
    packet_in.vd = 577.0;

    mavlink::common::msg::SIM_STATE packet1{};
    mavlink::common::msg::SIM_STATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.q1, packet2.q1);
    EXPECT_EQ(packet1.q2, packet2.q2);
    EXPECT_EQ(packet1.q3, packet2.q3);
    EXPECT_EQ(packet1.q4, packet2.q4);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.std_dev_horz, packet2.std_dev_horz);
    EXPECT_EQ(packet1.std_dev_vert, packet2.std_dev_vert);
    EXPECT_EQ(packet1.vn, packet2.vn);
    EXPECT_EQ(packet1.ve, packet2.ve);
    EXPECT_EQ(packet1.vd, packet2.vd);
}

#ifdef TEST_INTEROP
TEST(common_interop, SIM_STATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_sim_state_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 353.0, 381.0, 409.0, 437.0, 465.0, 493.0, 521.0, 549.0, 577.0
    };

    mavlink::common::msg::SIM_STATE packet_in{};
    packet_in.q1 = 17.0;
    packet_in.q2 = 45.0;
    packet_in.q3 = 73.0;
    packet_in.q4 = 101.0;
    packet_in.roll = 129.0;
    packet_in.pitch = 157.0;
    packet_in.yaw = 185.0;
    packet_in.xacc = 213.0;
    packet_in.yacc = 241.0;
    packet_in.zacc = 269.0;
    packet_in.xgyro = 297.0;
    packet_in.ygyro = 325.0;
    packet_in.zgyro = 353.0;
    packet_in.lat = 381.0;
    packet_in.lon = 409.0;
    packet_in.alt = 437.0;
    packet_in.std_dev_horz = 465.0;
    packet_in.std_dev_vert = 493.0;
    packet_in.vn = 521.0;
    packet_in.ve = 549.0;
    packet_in.vd = 577.0;

    mavlink::common::msg::SIM_STATE packet2{};

    mavlink_msg_sim_state_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.q1, packet2.q1);
    EXPECT_EQ(packet_in.q2, packet2.q2);
    EXPECT_EQ(packet_in.q3, packet2.q3);
    EXPECT_EQ(packet_in.q4, packet2.q4);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.std_dev_horz, packet2.std_dev_horz);
    EXPECT_EQ(packet_in.std_dev_vert, packet2.std_dev_vert);
    EXPECT_EQ(packet_in.vn, packet2.vn);
    EXPECT_EQ(packet_in.ve, packet2.ve);
    EXPECT_EQ(packet_in.vd, packet2.vd);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RADIO_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RADIO_STATUS packet_in{};
    packet_in.rssi = 17;
    packet_in.remrssi = 84;
    packet_in.txbuf = 151;
    packet_in.noise = 218;
    packet_in.remnoise = 29;
    packet_in.rxerrors = 17235;
    packet_in.fixed = 17339;

    mavlink::common::msg::RADIO_STATUS packet1{};
    mavlink::common::msg::RADIO_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.rssi, packet2.rssi);
    EXPECT_EQ(packet1.remrssi, packet2.remrssi);
    EXPECT_EQ(packet1.txbuf, packet2.txbuf);
    EXPECT_EQ(packet1.noise, packet2.noise);
    EXPECT_EQ(packet1.remnoise, packet2.remnoise);
    EXPECT_EQ(packet1.rxerrors, packet2.rxerrors);
    EXPECT_EQ(packet1.fixed, packet2.fixed);
}

#ifdef TEST_INTEROP
TEST(common_interop, RADIO_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_radio_status_t packet_c {
         17235, 17339, 17, 84, 151, 218, 29
    };

    mavlink::common::msg::RADIO_STATUS packet_in{};
    packet_in.rssi = 17;
    packet_in.remrssi = 84;
    packet_in.txbuf = 151;
    packet_in.noise = 218;
    packet_in.remnoise = 29;
    packet_in.rxerrors = 17235;
    packet_in.fixed = 17339;

    mavlink::common::msg::RADIO_STATUS packet2{};

    mavlink_msg_radio_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.rssi, packet2.rssi);
    EXPECT_EQ(packet_in.remrssi, packet2.remrssi);
    EXPECT_EQ(packet_in.txbuf, packet2.txbuf);
    EXPECT_EQ(packet_in.noise, packet2.noise);
    EXPECT_EQ(packet_in.remnoise, packet2.remnoise);
    EXPECT_EQ(packet_in.rxerrors, packet2.rxerrors);
    EXPECT_EQ(packet_in.fixed, packet2.fixed);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, FILE_TRANSFER_PROTOCOL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::FILE_TRANSFER_PROTOCOL packet_in{};
    packet_in.target_network = 5;
    packet_in.target_system = 72;
    packet_in.target_component = 139;
    packet_in.payload = {{ 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200 }};

    mavlink::common::msg::FILE_TRANSFER_PROTOCOL packet1{};
    mavlink::common::msg::FILE_TRANSFER_PROTOCOL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_network, packet2.target_network);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.payload, packet2.payload);
}

#ifdef TEST_INTEROP
TEST(common_interop, FILE_TRANSFER_PROTOCOL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_file_transfer_protocol_t packet_c {
         5, 72, 139, { 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200 }
    };

    mavlink::common::msg::FILE_TRANSFER_PROTOCOL packet_in{};
    packet_in.target_network = 5;
    packet_in.target_system = 72;
    packet_in.target_component = 139;
    packet_in.payload = {{ 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200 }};

    mavlink::common::msg::FILE_TRANSFER_PROTOCOL packet2{};

    mavlink_msg_file_transfer_protocol_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_network, packet2.target_network);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.payload, packet2.payload);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, TIMESYNC)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::TIMESYNC packet_in{};
    packet_in.tc1 = 93372036854775807LL;
    packet_in.ts1 = 170LL;

    mavlink::common::msg::TIMESYNC packet1{};
    mavlink::common::msg::TIMESYNC packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.tc1, packet2.tc1);
    EXPECT_EQ(packet1.ts1, packet2.ts1);
}

#ifdef TEST_INTEROP
TEST(common_interop, TIMESYNC)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_timesync_t packet_c {
         93372036854775807LL, 170LL
    };

    mavlink::common::msg::TIMESYNC packet_in{};
    packet_in.tc1 = 93372036854775807LL;
    packet_in.ts1 = 170LL;

    mavlink::common::msg::TIMESYNC packet2{};

    mavlink_msg_timesync_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.tc1, packet2.tc1);
    EXPECT_EQ(packet_in.ts1, packet2.ts1);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CAMERA_TRIGGER)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CAMERA_TRIGGER packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.seq = 963497880;

    mavlink::common::msg::CAMERA_TRIGGER packet1{};
    mavlink::common::msg::CAMERA_TRIGGER packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.seq, packet2.seq);
}

#ifdef TEST_INTEROP
TEST(common_interop, CAMERA_TRIGGER)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_trigger_t packet_c {
         93372036854775807ULL, 963497880
    };

    mavlink::common::msg::CAMERA_TRIGGER packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.seq = 963497880;

    mavlink::common::msg::CAMERA_TRIGGER packet2{};

    mavlink_msg_camera_trigger_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.seq, packet2.seq);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_GPS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_GPS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.fix_type = 235;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.eph = 18275;
    packet_in.epv = 18379;
    packet_in.vel = 18483;
    packet_in.vn = 18587;
    packet_in.ve = 18691;
    packet_in.vd = 18795;
    packet_in.cog = 18899;
    packet_in.satellites_visible = 46;

    mavlink::common::msg::HIL_GPS packet1{};
    mavlink::common::msg::HIL_GPS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.fix_type, packet2.fix_type);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.eph, packet2.eph);
    EXPECT_EQ(packet1.epv, packet2.epv);
    EXPECT_EQ(packet1.vel, packet2.vel);
    EXPECT_EQ(packet1.vn, packet2.vn);
    EXPECT_EQ(packet1.ve, packet2.ve);
    EXPECT_EQ(packet1.vd, packet2.vd);
    EXPECT_EQ(packet1.cog, packet2.cog);
    EXPECT_EQ(packet1.satellites_visible, packet2.satellites_visible);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_GPS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_gps_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 963498296, 18275, 18379, 18483, 18587, 18691, 18795, 18899, 235, 46
    };

    mavlink::common::msg::HIL_GPS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.fix_type = 235;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.eph = 18275;
    packet_in.epv = 18379;
    packet_in.vel = 18483;
    packet_in.vn = 18587;
    packet_in.ve = 18691;
    packet_in.vd = 18795;
    packet_in.cog = 18899;
    packet_in.satellites_visible = 46;

    mavlink::common::msg::HIL_GPS packet2{};

    mavlink_msg_hil_gps_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.fix_type, packet2.fix_type);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.eph, packet2.eph);
    EXPECT_EQ(packet_in.epv, packet2.epv);
    EXPECT_EQ(packet_in.vel, packet2.vel);
    EXPECT_EQ(packet_in.vn, packet2.vn);
    EXPECT_EQ(packet_in.ve, packet2.ve);
    EXPECT_EQ(packet_in.vd, packet2.vd);
    EXPECT_EQ(packet_in.cog, packet2.cog);
    EXPECT_EQ(packet_in.satellites_visible, packet2.satellites_visible);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_OPTICAL_FLOW)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_OPTICAL_FLOW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_id = 3;
    packet_in.integration_time_us = 963497880;
    packet_in.integrated_x = 101.0;
    packet_in.integrated_y = 129.0;
    packet_in.integrated_xgyro = 157.0;
    packet_in.integrated_ygyro = 185.0;
    packet_in.integrated_zgyro = 213.0;
    packet_in.temperature = 19315;
    packet_in.quality = 70;
    packet_in.time_delta_distance_us = 963499128;
    packet_in.distance = 269.0;

    mavlink::common::msg::HIL_OPTICAL_FLOW packet1{};
    mavlink::common::msg::HIL_OPTICAL_FLOW packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.sensor_id, packet2.sensor_id);
    EXPECT_EQ(packet1.integration_time_us, packet2.integration_time_us);
    EXPECT_EQ(packet1.integrated_x, packet2.integrated_x);
    EXPECT_EQ(packet1.integrated_y, packet2.integrated_y);
    EXPECT_EQ(packet1.integrated_xgyro, packet2.integrated_xgyro);
    EXPECT_EQ(packet1.integrated_ygyro, packet2.integrated_ygyro);
    EXPECT_EQ(packet1.integrated_zgyro, packet2.integrated_zgyro);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.quality, packet2.quality);
    EXPECT_EQ(packet1.time_delta_distance_us, packet2.time_delta_distance_us);
    EXPECT_EQ(packet1.distance, packet2.distance);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_OPTICAL_FLOW)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_optical_flow_t packet_c {
         93372036854775807ULL, 963497880, 101.0, 129.0, 157.0, 185.0, 213.0, 963499128, 269.0, 19315, 3, 70
    };

    mavlink::common::msg::HIL_OPTICAL_FLOW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_id = 3;
    packet_in.integration_time_us = 963497880;
    packet_in.integrated_x = 101.0;
    packet_in.integrated_y = 129.0;
    packet_in.integrated_xgyro = 157.0;
    packet_in.integrated_ygyro = 185.0;
    packet_in.integrated_zgyro = 213.0;
    packet_in.temperature = 19315;
    packet_in.quality = 70;
    packet_in.time_delta_distance_us = 963499128;
    packet_in.distance = 269.0;

    mavlink::common::msg::HIL_OPTICAL_FLOW packet2{};

    mavlink_msg_hil_optical_flow_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.sensor_id, packet2.sensor_id);
    EXPECT_EQ(packet_in.integration_time_us, packet2.integration_time_us);
    EXPECT_EQ(packet_in.integrated_x, packet2.integrated_x);
    EXPECT_EQ(packet_in.integrated_y, packet2.integrated_y);
    EXPECT_EQ(packet_in.integrated_xgyro, packet2.integrated_xgyro);
    EXPECT_EQ(packet_in.integrated_ygyro, packet2.integrated_ygyro);
    EXPECT_EQ(packet_in.integrated_zgyro, packet2.integrated_zgyro);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.quality, packet2.quality);
    EXPECT_EQ(packet_in.time_delta_distance_us, packet2.time_delta_distance_us);
    EXPECT_EQ(packet_in.distance, packet2.distance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIL_STATE_QUATERNION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIL_STATE_QUATERNION packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.attitude_quaternion = {{ 73.0, 74.0, 75.0, 76.0 }};
    packet_in.rollspeed = 185.0;
    packet_in.pitchspeed = 213.0;
    packet_in.yawspeed = 241.0;
    packet_in.lat = 963499336;
    packet_in.lon = 963499544;
    packet_in.alt = 963499752;
    packet_in.vx = 19731;
    packet_in.vy = 19835;
    packet_in.vz = 19939;
    packet_in.ind_airspeed = 20043;
    packet_in.true_airspeed = 20147;
    packet_in.xacc = 20251;
    packet_in.yacc = 20355;
    packet_in.zacc = 20459;

    mavlink::common::msg::HIL_STATE_QUATERNION packet1{};
    mavlink::common::msg::HIL_STATE_QUATERNION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.attitude_quaternion, packet2.attitude_quaternion);
    EXPECT_EQ(packet1.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet1.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet1.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.ind_airspeed, packet2.ind_airspeed);
    EXPECT_EQ(packet1.true_airspeed, packet2.true_airspeed);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIL_STATE_QUATERNION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hil_state_quaternion_t packet_c {
         93372036854775807ULL, { 73.0, 74.0, 75.0, 76.0 }, 185.0, 213.0, 241.0, 963499336, 963499544, 963499752, 19731, 19835, 19939, 20043, 20147, 20251, 20355, 20459
    };

    mavlink::common::msg::HIL_STATE_QUATERNION packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.attitude_quaternion = {{ 73.0, 74.0, 75.0, 76.0 }};
    packet_in.rollspeed = 185.0;
    packet_in.pitchspeed = 213.0;
    packet_in.yawspeed = 241.0;
    packet_in.lat = 963499336;
    packet_in.lon = 963499544;
    packet_in.alt = 963499752;
    packet_in.vx = 19731;
    packet_in.vy = 19835;
    packet_in.vz = 19939;
    packet_in.ind_airspeed = 20043;
    packet_in.true_airspeed = 20147;
    packet_in.xacc = 20251;
    packet_in.yacc = 20355;
    packet_in.zacc = 20459;

    mavlink::common::msg::HIL_STATE_QUATERNION packet2{};

    mavlink_msg_hil_state_quaternion_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.attitude_quaternion, packet2.attitude_quaternion);
    EXPECT_EQ(packet_in.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet_in.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet_in.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.ind_airspeed, packet2.ind_airspeed);
    EXPECT_EQ(packet_in.true_airspeed, packet2.true_airspeed);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SCALED_IMU2)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SCALED_IMU2 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.xacc = 17443;
    packet_in.yacc = 17547;
    packet_in.zacc = 17651;
    packet_in.xgyro = 17755;
    packet_in.ygyro = 17859;
    packet_in.zgyro = 17963;
    packet_in.xmag = 18067;
    packet_in.ymag = 18171;
    packet_in.zmag = 18275;

    mavlink::common::msg::SCALED_IMU2 packet1{};
    mavlink::common::msg::SCALED_IMU2 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.xmag, packet2.xmag);
    EXPECT_EQ(packet1.ymag, packet2.ymag);
    EXPECT_EQ(packet1.zmag, packet2.zmag);
}

#ifdef TEST_INTEROP
TEST(common_interop, SCALED_IMU2)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_scaled_imu2_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 18275
    };

    mavlink::common::msg::SCALED_IMU2 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.xacc = 17443;
    packet_in.yacc = 17547;
    packet_in.zacc = 17651;
    packet_in.xgyro = 17755;
    packet_in.ygyro = 17859;
    packet_in.zgyro = 17963;
    packet_in.xmag = 18067;
    packet_in.ymag = 18171;
    packet_in.zmag = 18275;

    mavlink::common::msg::SCALED_IMU2 packet2{};

    mavlink_msg_scaled_imu2_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.xmag, packet2.xmag);
    EXPECT_EQ(packet_in.ymag, packet2.ymag);
    EXPECT_EQ(packet_in.zmag, packet2.zmag);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOG_REQUEST_LIST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOG_REQUEST_LIST packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.start = 17235;
    packet_in.end = 17339;

    mavlink::common::msg::LOG_REQUEST_LIST packet1{};
    mavlink::common::msg::LOG_REQUEST_LIST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.start, packet2.start);
    EXPECT_EQ(packet1.end, packet2.end);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOG_REQUEST_LIST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_log_request_list_t packet_c {
         17235, 17339, 17, 84
    };

    mavlink::common::msg::LOG_REQUEST_LIST packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.start = 17235;
    packet_in.end = 17339;

    mavlink::common::msg::LOG_REQUEST_LIST packet2{};

    mavlink_msg_log_request_list_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.start, packet2.start);
    EXPECT_EQ(packet_in.end, packet2.end);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOG_ENTRY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOG_ENTRY packet_in{};
    packet_in.id = 17651;
    packet_in.num_logs = 17755;
    packet_in.last_log_num = 17859;
    packet_in.time_utc = 963497464;
    packet_in.size = 963497672;

    mavlink::common::msg::LOG_ENTRY packet1{};
    mavlink::common::msg::LOG_ENTRY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.id, packet2.id);
    EXPECT_EQ(packet1.num_logs, packet2.num_logs);
    EXPECT_EQ(packet1.last_log_num, packet2.last_log_num);
    EXPECT_EQ(packet1.time_utc, packet2.time_utc);
    EXPECT_EQ(packet1.size, packet2.size);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOG_ENTRY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_log_entry_t packet_c {
         963497464, 963497672, 17651, 17755, 17859
    };

    mavlink::common::msg::LOG_ENTRY packet_in{};
    packet_in.id = 17651;
    packet_in.num_logs = 17755;
    packet_in.last_log_num = 17859;
    packet_in.time_utc = 963497464;
    packet_in.size = 963497672;

    mavlink::common::msg::LOG_ENTRY packet2{};

    mavlink_msg_log_entry_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.id, packet2.id);
    EXPECT_EQ(packet_in.num_logs, packet2.num_logs);
    EXPECT_EQ(packet_in.last_log_num, packet2.last_log_num);
    EXPECT_EQ(packet_in.time_utc, packet2.time_utc);
    EXPECT_EQ(packet_in.size, packet2.size);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOG_REQUEST_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOG_REQUEST_DATA packet_in{};
    packet_in.target_system = 163;
    packet_in.target_component = 230;
    packet_in.id = 17651;
    packet_in.ofs = 963497464;
    packet_in.count = 963497672;

    mavlink::common::msg::LOG_REQUEST_DATA packet1{};
    mavlink::common::msg::LOG_REQUEST_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.id, packet2.id);
    EXPECT_EQ(packet1.ofs, packet2.ofs);
    EXPECT_EQ(packet1.count, packet2.count);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOG_REQUEST_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_log_request_data_t packet_c {
         963497464, 963497672, 17651, 163, 230
    };

    mavlink::common::msg::LOG_REQUEST_DATA packet_in{};
    packet_in.target_system = 163;
    packet_in.target_component = 230;
    packet_in.id = 17651;
    packet_in.ofs = 963497464;
    packet_in.count = 963497672;

    mavlink::common::msg::LOG_REQUEST_DATA packet2{};

    mavlink_msg_log_request_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.id, packet2.id);
    EXPECT_EQ(packet_in.ofs, packet2.ofs);
    EXPECT_EQ(packet_in.count, packet2.count);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOG_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOG_DATA packet_in{};
    packet_in.id = 17443;
    packet_in.ofs = 963497464;
    packet_in.count = 151;
    packet_in.data = {{ 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 }};

    mavlink::common::msg::LOG_DATA packet1{};
    mavlink::common::msg::LOG_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.id, packet2.id);
    EXPECT_EQ(packet1.ofs, packet2.ofs);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOG_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_log_data_t packet_c {
         963497464, 17443, 151, { 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 }
    };

    mavlink::common::msg::LOG_DATA packet_in{};
    packet_in.id = 17443;
    packet_in.ofs = 963497464;
    packet_in.count = 151;
    packet_in.data = {{ 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 }};

    mavlink::common::msg::LOG_DATA packet2{};

    mavlink_msg_log_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.id, packet2.id);
    EXPECT_EQ(packet_in.ofs, packet2.ofs);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOG_ERASE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOG_ERASE packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::common::msg::LOG_ERASE packet1{};
    mavlink::common::msg::LOG_ERASE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOG_ERASE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_log_erase_t packet_c {
         5, 72
    };

    mavlink::common::msg::LOG_ERASE packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::common::msg::LOG_ERASE packet2{};

    mavlink_msg_log_erase_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOG_REQUEST_END)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOG_REQUEST_END packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::common::msg::LOG_REQUEST_END packet1{};
    mavlink::common::msg::LOG_REQUEST_END packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOG_REQUEST_END)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_log_request_end_t packet_c {
         5, 72
    };

    mavlink::common::msg::LOG_REQUEST_END packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::common::msg::LOG_REQUEST_END packet2{};

    mavlink_msg_log_request_end_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_INJECT_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_INJECT_DATA packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.len = 139;
    packet_in.data = {{ 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 }};

    mavlink::common::msg::GPS_INJECT_DATA packet1{};
    mavlink::common::msg::GPS_INJECT_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.len, packet2.len);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_INJECT_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_inject_data_t packet_c {
         5, 72, 139, { 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 }
    };

    mavlink::common::msg::GPS_INJECT_DATA packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.len = 139;
    packet_in.data = {{ 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 }};

    mavlink::common::msg::GPS_INJECT_DATA packet2{};

    mavlink_msg_gps_inject_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.len, packet2.len);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS2_RAW)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS2_RAW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.fix_type = 101;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.eph = 18483;
    packet_in.epv = 18587;
    packet_in.vel = 18691;
    packet_in.cog = 18795;
    packet_in.satellites_visible = 168;
    packet_in.dgps_numch = 235;
    packet_in.dgps_age = 963498504;

    mavlink::common::msg::GPS2_RAW packet1{};
    mavlink::common::msg::GPS2_RAW packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.fix_type, packet2.fix_type);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.eph, packet2.eph);
    EXPECT_EQ(packet1.epv, packet2.epv);
    EXPECT_EQ(packet1.vel, packet2.vel);
    EXPECT_EQ(packet1.cog, packet2.cog);
    EXPECT_EQ(packet1.satellites_visible, packet2.satellites_visible);
    EXPECT_EQ(packet1.dgps_numch, packet2.dgps_numch);
    EXPECT_EQ(packet1.dgps_age, packet2.dgps_age);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS2_RAW)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps2_raw_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 963498296, 963498504, 18483, 18587, 18691, 18795, 101, 168, 235
    };

    mavlink::common::msg::GPS2_RAW packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.fix_type = 101;
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.alt = 963498296;
    packet_in.eph = 18483;
    packet_in.epv = 18587;
    packet_in.vel = 18691;
    packet_in.cog = 18795;
    packet_in.satellites_visible = 168;
    packet_in.dgps_numch = 235;
    packet_in.dgps_age = 963498504;

    mavlink::common::msg::GPS2_RAW packet2{};

    mavlink_msg_gps2_raw_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.fix_type, packet2.fix_type);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.eph, packet2.eph);
    EXPECT_EQ(packet_in.epv, packet2.epv);
    EXPECT_EQ(packet_in.vel, packet2.vel);
    EXPECT_EQ(packet_in.cog, packet2.cog);
    EXPECT_EQ(packet_in.satellites_visible, packet2.satellites_visible);
    EXPECT_EQ(packet_in.dgps_numch, packet2.dgps_numch);
    EXPECT_EQ(packet_in.dgps_age, packet2.dgps_age);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, POWER_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::POWER_STATUS packet_in{};
    packet_in.Vcc = 17235;
    packet_in.Vservo = 17339;
    packet_in.flags = 17443;

    mavlink::common::msg::POWER_STATUS packet1{};
    mavlink::common::msg::POWER_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.Vcc, packet2.Vcc);
    EXPECT_EQ(packet1.Vservo, packet2.Vservo);
    EXPECT_EQ(packet1.flags, packet2.flags);
}

#ifdef TEST_INTEROP
TEST(common_interop, POWER_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_power_status_t packet_c {
         17235, 17339, 17443
    };

    mavlink::common::msg::POWER_STATUS packet_in{};
    packet_in.Vcc = 17235;
    packet_in.Vservo = 17339;
    packet_in.flags = 17443;

    mavlink::common::msg::POWER_STATUS packet2{};

    mavlink_msg_power_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.Vcc, packet2.Vcc);
    EXPECT_EQ(packet_in.Vservo, packet2.Vservo);
    EXPECT_EQ(packet_in.flags, packet2.flags);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SERIAL_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SERIAL_CONTROL packet_in{};
    packet_in.device = 151;
    packet_in.flags = 218;
    packet_in.timeout = 17443;
    packet_in.baudrate = 963497464;
    packet_in.count = 29;
    packet_in.data = {{ 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165 }};

    mavlink::common::msg::SERIAL_CONTROL packet1{};
    mavlink::common::msg::SERIAL_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.device, packet2.device);
    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.timeout, packet2.timeout);
    EXPECT_EQ(packet1.baudrate, packet2.baudrate);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, SERIAL_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_serial_control_t packet_c {
         963497464, 17443, 151, 218, 29, { 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165 }
    };

    mavlink::common::msg::SERIAL_CONTROL packet_in{};
    packet_in.device = 151;
    packet_in.flags = 218;
    packet_in.timeout = 17443;
    packet_in.baudrate = 963497464;
    packet_in.count = 29;
    packet_in.data = {{ 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165 }};

    mavlink::common::msg::SERIAL_CONTROL packet2{};

    mavlink_msg_serial_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.device, packet2.device);
    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.timeout, packet2.timeout);
    EXPECT_EQ(packet_in.baudrate, packet2.baudrate);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_RTK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_RTK packet_in{};
    packet_in.time_last_baseline_ms = 963497464;
    packet_in.rtk_receiver_id = 223;
    packet_in.wn = 18691;
    packet_in.tow = 963497672;
    packet_in.rtk_health = 34;
    packet_in.rtk_rate = 101;
    packet_in.nsats = 168;
    packet_in.baseline_coords_type = 235;
    packet_in.baseline_a_mm = 963497880;
    packet_in.baseline_b_mm = 963498088;
    packet_in.baseline_c_mm = 963498296;
    packet_in.accuracy = 963498504;
    packet_in.iar_num_hypotheses = 963498712;

    mavlink::common::msg::GPS_RTK packet1{};
    mavlink::common::msg::GPS_RTK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_last_baseline_ms, packet2.time_last_baseline_ms);
    EXPECT_EQ(packet1.rtk_receiver_id, packet2.rtk_receiver_id);
    EXPECT_EQ(packet1.wn, packet2.wn);
    EXPECT_EQ(packet1.tow, packet2.tow);
    EXPECT_EQ(packet1.rtk_health, packet2.rtk_health);
    EXPECT_EQ(packet1.rtk_rate, packet2.rtk_rate);
    EXPECT_EQ(packet1.nsats, packet2.nsats);
    EXPECT_EQ(packet1.baseline_coords_type, packet2.baseline_coords_type);
    EXPECT_EQ(packet1.baseline_a_mm, packet2.baseline_a_mm);
    EXPECT_EQ(packet1.baseline_b_mm, packet2.baseline_b_mm);
    EXPECT_EQ(packet1.baseline_c_mm, packet2.baseline_c_mm);
    EXPECT_EQ(packet1.accuracy, packet2.accuracy);
    EXPECT_EQ(packet1.iar_num_hypotheses, packet2.iar_num_hypotheses);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_RTK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_rtk_t packet_c {
         963497464, 963497672, 963497880, 963498088, 963498296, 963498504, 963498712, 18691, 223, 34, 101, 168, 235
    };

    mavlink::common::msg::GPS_RTK packet_in{};
    packet_in.time_last_baseline_ms = 963497464;
    packet_in.rtk_receiver_id = 223;
    packet_in.wn = 18691;
    packet_in.tow = 963497672;
    packet_in.rtk_health = 34;
    packet_in.rtk_rate = 101;
    packet_in.nsats = 168;
    packet_in.baseline_coords_type = 235;
    packet_in.baseline_a_mm = 963497880;
    packet_in.baseline_b_mm = 963498088;
    packet_in.baseline_c_mm = 963498296;
    packet_in.accuracy = 963498504;
    packet_in.iar_num_hypotheses = 963498712;

    mavlink::common::msg::GPS_RTK packet2{};

    mavlink_msg_gps_rtk_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_last_baseline_ms, packet2.time_last_baseline_ms);
    EXPECT_EQ(packet_in.rtk_receiver_id, packet2.rtk_receiver_id);
    EXPECT_EQ(packet_in.wn, packet2.wn);
    EXPECT_EQ(packet_in.tow, packet2.tow);
    EXPECT_EQ(packet_in.rtk_health, packet2.rtk_health);
    EXPECT_EQ(packet_in.rtk_rate, packet2.rtk_rate);
    EXPECT_EQ(packet_in.nsats, packet2.nsats);
    EXPECT_EQ(packet_in.baseline_coords_type, packet2.baseline_coords_type);
    EXPECT_EQ(packet_in.baseline_a_mm, packet2.baseline_a_mm);
    EXPECT_EQ(packet_in.baseline_b_mm, packet2.baseline_b_mm);
    EXPECT_EQ(packet_in.baseline_c_mm, packet2.baseline_c_mm);
    EXPECT_EQ(packet_in.accuracy, packet2.accuracy);
    EXPECT_EQ(packet_in.iar_num_hypotheses, packet2.iar_num_hypotheses);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS2_RTK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS2_RTK packet_in{};
    packet_in.time_last_baseline_ms = 963497464;
    packet_in.rtk_receiver_id = 223;
    packet_in.wn = 18691;
    packet_in.tow = 963497672;
    packet_in.rtk_health = 34;
    packet_in.rtk_rate = 101;
    packet_in.nsats = 168;
    packet_in.baseline_coords_type = 235;
    packet_in.baseline_a_mm = 963497880;
    packet_in.baseline_b_mm = 963498088;
    packet_in.baseline_c_mm = 963498296;
    packet_in.accuracy = 963498504;
    packet_in.iar_num_hypotheses = 963498712;

    mavlink::common::msg::GPS2_RTK packet1{};
    mavlink::common::msg::GPS2_RTK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_last_baseline_ms, packet2.time_last_baseline_ms);
    EXPECT_EQ(packet1.rtk_receiver_id, packet2.rtk_receiver_id);
    EXPECT_EQ(packet1.wn, packet2.wn);
    EXPECT_EQ(packet1.tow, packet2.tow);
    EXPECT_EQ(packet1.rtk_health, packet2.rtk_health);
    EXPECT_EQ(packet1.rtk_rate, packet2.rtk_rate);
    EXPECT_EQ(packet1.nsats, packet2.nsats);
    EXPECT_EQ(packet1.baseline_coords_type, packet2.baseline_coords_type);
    EXPECT_EQ(packet1.baseline_a_mm, packet2.baseline_a_mm);
    EXPECT_EQ(packet1.baseline_b_mm, packet2.baseline_b_mm);
    EXPECT_EQ(packet1.baseline_c_mm, packet2.baseline_c_mm);
    EXPECT_EQ(packet1.accuracy, packet2.accuracy);
    EXPECT_EQ(packet1.iar_num_hypotheses, packet2.iar_num_hypotheses);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS2_RTK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps2_rtk_t packet_c {
         963497464, 963497672, 963497880, 963498088, 963498296, 963498504, 963498712, 18691, 223, 34, 101, 168, 235
    };

    mavlink::common::msg::GPS2_RTK packet_in{};
    packet_in.time_last_baseline_ms = 963497464;
    packet_in.rtk_receiver_id = 223;
    packet_in.wn = 18691;
    packet_in.tow = 963497672;
    packet_in.rtk_health = 34;
    packet_in.rtk_rate = 101;
    packet_in.nsats = 168;
    packet_in.baseline_coords_type = 235;
    packet_in.baseline_a_mm = 963497880;
    packet_in.baseline_b_mm = 963498088;
    packet_in.baseline_c_mm = 963498296;
    packet_in.accuracy = 963498504;
    packet_in.iar_num_hypotheses = 963498712;

    mavlink::common::msg::GPS2_RTK packet2{};

    mavlink_msg_gps2_rtk_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_last_baseline_ms, packet2.time_last_baseline_ms);
    EXPECT_EQ(packet_in.rtk_receiver_id, packet2.rtk_receiver_id);
    EXPECT_EQ(packet_in.wn, packet2.wn);
    EXPECT_EQ(packet_in.tow, packet2.tow);
    EXPECT_EQ(packet_in.rtk_health, packet2.rtk_health);
    EXPECT_EQ(packet_in.rtk_rate, packet2.rtk_rate);
    EXPECT_EQ(packet_in.nsats, packet2.nsats);
    EXPECT_EQ(packet_in.baseline_coords_type, packet2.baseline_coords_type);
    EXPECT_EQ(packet_in.baseline_a_mm, packet2.baseline_a_mm);
    EXPECT_EQ(packet_in.baseline_b_mm, packet2.baseline_b_mm);
    EXPECT_EQ(packet_in.baseline_c_mm, packet2.baseline_c_mm);
    EXPECT_EQ(packet_in.accuracy, packet2.accuracy);
    EXPECT_EQ(packet_in.iar_num_hypotheses, packet2.iar_num_hypotheses);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SCALED_IMU3)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SCALED_IMU3 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.xacc = 17443;
    packet_in.yacc = 17547;
    packet_in.zacc = 17651;
    packet_in.xgyro = 17755;
    packet_in.ygyro = 17859;
    packet_in.zgyro = 17963;
    packet_in.xmag = 18067;
    packet_in.ymag = 18171;
    packet_in.zmag = 18275;

    mavlink::common::msg::SCALED_IMU3 packet1{};
    mavlink::common::msg::SCALED_IMU3 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.xacc, packet2.xacc);
    EXPECT_EQ(packet1.yacc, packet2.yacc);
    EXPECT_EQ(packet1.zacc, packet2.zacc);
    EXPECT_EQ(packet1.xgyro, packet2.xgyro);
    EXPECT_EQ(packet1.ygyro, packet2.ygyro);
    EXPECT_EQ(packet1.zgyro, packet2.zgyro);
    EXPECT_EQ(packet1.xmag, packet2.xmag);
    EXPECT_EQ(packet1.ymag, packet2.ymag);
    EXPECT_EQ(packet1.zmag, packet2.zmag);
}

#ifdef TEST_INTEROP
TEST(common_interop, SCALED_IMU3)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_scaled_imu3_t packet_c {
         963497464, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 18171, 18275
    };

    mavlink::common::msg::SCALED_IMU3 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.xacc = 17443;
    packet_in.yacc = 17547;
    packet_in.zacc = 17651;
    packet_in.xgyro = 17755;
    packet_in.ygyro = 17859;
    packet_in.zgyro = 17963;
    packet_in.xmag = 18067;
    packet_in.ymag = 18171;
    packet_in.zmag = 18275;

    mavlink::common::msg::SCALED_IMU3 packet2{};

    mavlink_msg_scaled_imu3_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.xacc, packet2.xacc);
    EXPECT_EQ(packet_in.yacc, packet2.yacc);
    EXPECT_EQ(packet_in.zacc, packet2.zacc);
    EXPECT_EQ(packet_in.xgyro, packet2.xgyro);
    EXPECT_EQ(packet_in.ygyro, packet2.ygyro);
    EXPECT_EQ(packet_in.zgyro, packet2.zgyro);
    EXPECT_EQ(packet_in.xmag, packet2.xmag);
    EXPECT_EQ(packet_in.ymag, packet2.ymag);
    EXPECT_EQ(packet_in.zmag, packet2.zmag);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, DATA_TRANSMISSION_HANDSHAKE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::DATA_TRANSMISSION_HANDSHAKE packet_in{};
    packet_in.type = 163;
    packet_in.size = 963497464;
    packet_in.width = 17443;
    packet_in.height = 17547;
    packet_in.packets = 17651;
    packet_in.payload = 230;
    packet_in.jpg_quality = 41;

    mavlink::common::msg::DATA_TRANSMISSION_HANDSHAKE packet1{};
    mavlink::common::msg::DATA_TRANSMISSION_HANDSHAKE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.size, packet2.size);
    EXPECT_EQ(packet1.width, packet2.width);
    EXPECT_EQ(packet1.height, packet2.height);
    EXPECT_EQ(packet1.packets, packet2.packets);
    EXPECT_EQ(packet1.payload, packet2.payload);
    EXPECT_EQ(packet1.jpg_quality, packet2.jpg_quality);
}

#ifdef TEST_INTEROP
TEST(common_interop, DATA_TRANSMISSION_HANDSHAKE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_data_transmission_handshake_t packet_c {
         963497464, 17443, 17547, 17651, 163, 230, 41
    };

    mavlink::common::msg::DATA_TRANSMISSION_HANDSHAKE packet_in{};
    packet_in.type = 163;
    packet_in.size = 963497464;
    packet_in.width = 17443;
    packet_in.height = 17547;
    packet_in.packets = 17651;
    packet_in.payload = 230;
    packet_in.jpg_quality = 41;

    mavlink::common::msg::DATA_TRANSMISSION_HANDSHAKE packet2{};

    mavlink_msg_data_transmission_handshake_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.size, packet2.size);
    EXPECT_EQ(packet_in.width, packet2.width);
    EXPECT_EQ(packet_in.height, packet2.height);
    EXPECT_EQ(packet_in.packets, packet2.packets);
    EXPECT_EQ(packet_in.payload, packet2.payload);
    EXPECT_EQ(packet_in.jpg_quality, packet2.jpg_quality);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ENCAPSULATED_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ENCAPSULATED_DATA packet_in{};
    packet_in.seqnr = 17235;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135 }};

    mavlink::common::msg::ENCAPSULATED_DATA packet1{};
    mavlink::common::msg::ENCAPSULATED_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.seqnr, packet2.seqnr);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, ENCAPSULATED_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_encapsulated_data_t packet_c {
         17235, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135 }
    };

    mavlink::common::msg::ENCAPSULATED_DATA packet_in{};
    packet_in.seqnr = 17235;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135 }};

    mavlink::common::msg::ENCAPSULATED_DATA packet2{};

    mavlink_msg_encapsulated_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.seqnr, packet2.seqnr);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, DISTANCE_SENSOR)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::DISTANCE_SENSOR packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.min_distance = 17443;
    packet_in.max_distance = 17547;
    packet_in.current_distance = 17651;
    packet_in.type = 163;
    packet_in.id = 230;
    packet_in.orientation = 41;
    packet_in.covariance = 108;

    mavlink::common::msg::DISTANCE_SENSOR packet1{};
    mavlink::common::msg::DISTANCE_SENSOR packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.min_distance, packet2.min_distance);
    EXPECT_EQ(packet1.max_distance, packet2.max_distance);
    EXPECT_EQ(packet1.current_distance, packet2.current_distance);
    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.id, packet2.id);
    EXPECT_EQ(packet1.orientation, packet2.orientation);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, DISTANCE_SENSOR)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_distance_sensor_t packet_c {
         963497464, 17443, 17547, 17651, 163, 230, 41, 108
    };

    mavlink::common::msg::DISTANCE_SENSOR packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.min_distance = 17443;
    packet_in.max_distance = 17547;
    packet_in.current_distance = 17651;
    packet_in.type = 163;
    packet_in.id = 230;
    packet_in.orientation = 41;
    packet_in.covariance = 108;

    mavlink::common::msg::DISTANCE_SENSOR packet2{};

    mavlink_msg_distance_sensor_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.min_distance, packet2.min_distance);
    EXPECT_EQ(packet_in.max_distance, packet2.max_distance);
    EXPECT_EQ(packet_in.current_distance, packet2.current_distance);
    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.id, packet2.id);
    EXPECT_EQ(packet_in.orientation, packet2.orientation);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, TERRAIN_REQUEST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::TERRAIN_REQUEST packet_in{};
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.grid_spacing = 18067;
    packet_in.mask = 93372036854775807ULL;

    mavlink::common::msg::TERRAIN_REQUEST packet1{};
    mavlink::common::msg::TERRAIN_REQUEST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.grid_spacing, packet2.grid_spacing);
    EXPECT_EQ(packet1.mask, packet2.mask);
}

#ifdef TEST_INTEROP
TEST(common_interop, TERRAIN_REQUEST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_terrain_request_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 18067
    };

    mavlink::common::msg::TERRAIN_REQUEST packet_in{};
    packet_in.lat = 963497880;
    packet_in.lon = 963498088;
    packet_in.grid_spacing = 18067;
    packet_in.mask = 93372036854775807ULL;

    mavlink::common::msg::TERRAIN_REQUEST packet2{};

    mavlink_msg_terrain_request_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.grid_spacing, packet2.grid_spacing);
    EXPECT_EQ(packet_in.mask, packet2.mask);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, TERRAIN_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::TERRAIN_DATA packet_in{};
    packet_in.lat = 963497464;
    packet_in.lon = 963497672;
    packet_in.grid_spacing = 17651;
    packet_in.gridbit = 3;
    packet_in.data = {{ 17755, 17756, 17757, 17758, 17759, 17760, 17761, 17762, 17763, 17764, 17765, 17766, 17767, 17768, 17769, 17770 }};

    mavlink::common::msg::TERRAIN_DATA packet1{};
    mavlink::common::msg::TERRAIN_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.grid_spacing, packet2.grid_spacing);
    EXPECT_EQ(packet1.gridbit, packet2.gridbit);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, TERRAIN_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_terrain_data_t packet_c {
         963497464, 963497672, 17651, { 17755, 17756, 17757, 17758, 17759, 17760, 17761, 17762, 17763, 17764, 17765, 17766, 17767, 17768, 17769, 17770 }, 3
    };

    mavlink::common::msg::TERRAIN_DATA packet_in{};
    packet_in.lat = 963497464;
    packet_in.lon = 963497672;
    packet_in.grid_spacing = 17651;
    packet_in.gridbit = 3;
    packet_in.data = {{ 17755, 17756, 17757, 17758, 17759, 17760, 17761, 17762, 17763, 17764, 17765, 17766, 17767, 17768, 17769, 17770 }};

    mavlink::common::msg::TERRAIN_DATA packet2{};

    mavlink_msg_terrain_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.grid_spacing, packet2.grid_spacing);
    EXPECT_EQ(packet_in.gridbit, packet2.gridbit);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, TERRAIN_CHECK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::TERRAIN_CHECK packet_in{};
    packet_in.lat = 963497464;
    packet_in.lon = 963497672;

    mavlink::common::msg::TERRAIN_CHECK packet1{};
    mavlink::common::msg::TERRAIN_CHECK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
}

#ifdef TEST_INTEROP
TEST(common_interop, TERRAIN_CHECK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_terrain_check_t packet_c {
         963497464, 963497672
    };

    mavlink::common::msg::TERRAIN_CHECK packet_in{};
    packet_in.lat = 963497464;
    packet_in.lon = 963497672;

    mavlink::common::msg::TERRAIN_CHECK packet2{};

    mavlink_msg_terrain_check_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, TERRAIN_REPORT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::TERRAIN_REPORT packet_in{};
    packet_in.lat = 963497464;
    packet_in.lon = 963497672;
    packet_in.spacing = 18067;
    packet_in.terrain_height = 73.0;
    packet_in.current_height = 101.0;
    packet_in.pending = 18171;
    packet_in.loaded = 18275;

    mavlink::common::msg::TERRAIN_REPORT packet1{};
    mavlink::common::msg::TERRAIN_REPORT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.spacing, packet2.spacing);
    EXPECT_EQ(packet1.terrain_height, packet2.terrain_height);
    EXPECT_EQ(packet1.current_height, packet2.current_height);
    EXPECT_EQ(packet1.pending, packet2.pending);
    EXPECT_EQ(packet1.loaded, packet2.loaded);
}

#ifdef TEST_INTEROP
TEST(common_interop, TERRAIN_REPORT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_terrain_report_t packet_c {
         963497464, 963497672, 73.0, 101.0, 18067, 18171, 18275
    };

    mavlink::common::msg::TERRAIN_REPORT packet_in{};
    packet_in.lat = 963497464;
    packet_in.lon = 963497672;
    packet_in.spacing = 18067;
    packet_in.terrain_height = 73.0;
    packet_in.current_height = 101.0;
    packet_in.pending = 18171;
    packet_in.loaded = 18275;

    mavlink::common::msg::TERRAIN_REPORT packet2{};

    mavlink_msg_terrain_report_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.spacing, packet2.spacing);
    EXPECT_EQ(packet_in.terrain_height, packet2.terrain_height);
    EXPECT_EQ(packet_in.current_height, packet2.current_height);
    EXPECT_EQ(packet_in.pending, packet2.pending);
    EXPECT_EQ(packet_in.loaded, packet2.loaded);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SCALED_PRESSURE2)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SCALED_PRESSURE2 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.press_abs = 45.0;
    packet_in.press_diff = 73.0;
    packet_in.temperature = 17859;

    mavlink::common::msg::SCALED_PRESSURE2 packet1{};
    mavlink::common::msg::SCALED_PRESSURE2 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.press_abs, packet2.press_abs);
    EXPECT_EQ(packet1.press_diff, packet2.press_diff);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
}

#ifdef TEST_INTEROP
TEST(common_interop, SCALED_PRESSURE2)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_scaled_pressure2_t packet_c {
         963497464, 45.0, 73.0, 17859
    };

    mavlink::common::msg::SCALED_PRESSURE2 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.press_abs = 45.0;
    packet_in.press_diff = 73.0;
    packet_in.temperature = 17859;

    mavlink::common::msg::SCALED_PRESSURE2 packet2{};

    mavlink_msg_scaled_pressure2_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.press_abs, packet2.press_abs);
    EXPECT_EQ(packet_in.press_diff, packet2.press_diff);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ATT_POS_MOCAP)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ATT_POS_MOCAP packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.q = {{ 73.0, 74.0, 75.0, 76.0 }};
    packet_in.x = 185.0;
    packet_in.y = 213.0;
    packet_in.z = 241.0;
    packet_in.covariance = {{ 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 288.0, 289.0 }};

    mavlink::common::msg::ATT_POS_MOCAP packet1{};
    mavlink::common::msg::ATT_POS_MOCAP packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.covariance, packet2.covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, ATT_POS_MOCAP)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_att_pos_mocap_t packet_c {
         93372036854775807ULL, { 73.0, 74.0, 75.0, 76.0 }, 185.0, 213.0, 241.0, { 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 288.0, 289.0 }
    };

    mavlink::common::msg::ATT_POS_MOCAP packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.q = {{ 73.0, 74.0, 75.0, 76.0 }};
    packet_in.x = 185.0;
    packet_in.y = 213.0;
    packet_in.z = 241.0;
    packet_in.covariance = {{ 269.0, 270.0, 271.0, 272.0, 273.0, 274.0, 275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 281.0, 282.0, 283.0, 284.0, 285.0, 286.0, 287.0, 288.0, 289.0 }};

    mavlink::common::msg::ATT_POS_MOCAP packet2{};

    mavlink_msg_att_pos_mocap_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.covariance, packet2.covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_ACTUATOR_CONTROL_TARGET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_ACTUATOR_CONTROL_TARGET packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.group_mlx = 125;
    packet_in.target_system = 192;
    packet_in.target_component = 3;
    packet_in.controls = {{ 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0 }};

    mavlink::common::msg::SET_ACTUATOR_CONTROL_TARGET packet1{};
    mavlink::common::msg::SET_ACTUATOR_CONTROL_TARGET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.group_mlx, packet2.group_mlx);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.controls, packet2.controls);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_ACTUATOR_CONTROL_TARGET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_actuator_control_target_t packet_c {
         93372036854775807ULL, { 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0 }, 125, 192, 3
    };

    mavlink::common::msg::SET_ACTUATOR_CONTROL_TARGET packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.group_mlx = 125;
    packet_in.target_system = 192;
    packet_in.target_component = 3;
    packet_in.controls = {{ 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0 }};

    mavlink::common::msg::SET_ACTUATOR_CONTROL_TARGET packet2{};

    mavlink_msg_set_actuator_control_target_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.group_mlx, packet2.group_mlx);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.controls, packet2.controls);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ACTUATOR_CONTROL_TARGET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ACTUATOR_CONTROL_TARGET packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.group_mlx = 125;
    packet_in.controls = {{ 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0 }};

    mavlink::common::msg::ACTUATOR_CONTROL_TARGET packet1{};
    mavlink::common::msg::ACTUATOR_CONTROL_TARGET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.group_mlx, packet2.group_mlx);
    EXPECT_EQ(packet1.controls, packet2.controls);
}

#ifdef TEST_INTEROP
TEST(common_interop, ACTUATOR_CONTROL_TARGET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_actuator_control_target_t packet_c {
         93372036854775807ULL, { 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0 }, 125
    };

    mavlink::common::msg::ACTUATOR_CONTROL_TARGET packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.group_mlx = 125;
    packet_in.controls = {{ 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0, 80.0 }};

    mavlink::common::msg::ACTUATOR_CONTROL_TARGET packet2{};

    mavlink_msg_actuator_control_target_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.group_mlx, packet2.group_mlx);
    EXPECT_EQ(packet_in.controls, packet2.controls);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ALTITUDE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ALTITUDE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.altitude_monotonic = 73.0;
    packet_in.altitude_amsl = 101.0;
    packet_in.altitude_local = 129.0;
    packet_in.altitude_relative = 157.0;
    packet_in.altitude_terrain = 185.0;
    packet_in.bottom_clearance = 213.0;

    mavlink::common::msg::ALTITUDE packet1{};
    mavlink::common::msg::ALTITUDE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.altitude_monotonic, packet2.altitude_monotonic);
    EXPECT_EQ(packet1.altitude_amsl, packet2.altitude_amsl);
    EXPECT_EQ(packet1.altitude_local, packet2.altitude_local);
    EXPECT_EQ(packet1.altitude_relative, packet2.altitude_relative);
    EXPECT_EQ(packet1.altitude_terrain, packet2.altitude_terrain);
    EXPECT_EQ(packet1.bottom_clearance, packet2.bottom_clearance);
}

#ifdef TEST_INTEROP
TEST(common_interop, ALTITUDE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_altitude_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0
    };

    mavlink::common::msg::ALTITUDE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.altitude_monotonic = 73.0;
    packet_in.altitude_amsl = 101.0;
    packet_in.altitude_local = 129.0;
    packet_in.altitude_relative = 157.0;
    packet_in.altitude_terrain = 185.0;
    packet_in.bottom_clearance = 213.0;

    mavlink::common::msg::ALTITUDE packet2{};

    mavlink_msg_altitude_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.altitude_monotonic, packet2.altitude_monotonic);
    EXPECT_EQ(packet_in.altitude_amsl, packet2.altitude_amsl);
    EXPECT_EQ(packet_in.altitude_local, packet2.altitude_local);
    EXPECT_EQ(packet_in.altitude_relative, packet2.altitude_relative);
    EXPECT_EQ(packet_in.altitude_terrain, packet2.altitude_terrain);
    EXPECT_EQ(packet_in.bottom_clearance, packet2.bottom_clearance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, RESOURCE_REQUEST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::RESOURCE_REQUEST packet_in{};
    packet_in.request_id = 5;
    packet_in.uri_type = 72;
    packet_in.uri = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2 }};
    packet_in.transfer_type = 243;
    packet_in.storage = {{ 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173 }};

    mavlink::common::msg::RESOURCE_REQUEST packet1{};
    mavlink::common::msg::RESOURCE_REQUEST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.uri_type, packet2.uri_type);
    EXPECT_EQ(packet1.uri, packet2.uri);
    EXPECT_EQ(packet1.transfer_type, packet2.transfer_type);
    EXPECT_EQ(packet1.storage, packet2.storage);
}

#ifdef TEST_INTEROP
TEST(common_interop, RESOURCE_REQUEST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_resource_request_t packet_c {
         5, 72, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2 }, 243, { 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173 }
    };

    mavlink::common::msg::RESOURCE_REQUEST packet_in{};
    packet_in.request_id = 5;
    packet_in.uri_type = 72;
    packet_in.uri = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2 }};
    packet_in.transfer_type = 243;
    packet_in.storage = {{ 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173 }};

    mavlink::common::msg::RESOURCE_REQUEST packet2{};

    mavlink_msg_resource_request_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.uri_type, packet2.uri_type);
    EXPECT_EQ(packet_in.uri, packet2.uri);
    EXPECT_EQ(packet_in.transfer_type, packet2.transfer_type);
    EXPECT_EQ(packet_in.storage, packet2.storage);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SCALED_PRESSURE3)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SCALED_PRESSURE3 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.press_abs = 45.0;
    packet_in.press_diff = 73.0;
    packet_in.temperature = 17859;

    mavlink::common::msg::SCALED_PRESSURE3 packet1{};
    mavlink::common::msg::SCALED_PRESSURE3 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.press_abs, packet2.press_abs);
    EXPECT_EQ(packet1.press_diff, packet2.press_diff);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
}

#ifdef TEST_INTEROP
TEST(common_interop, SCALED_PRESSURE3)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_scaled_pressure3_t packet_c {
         963497464, 45.0, 73.0, 17859
    };

    mavlink::common::msg::SCALED_PRESSURE3 packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.press_abs = 45.0;
    packet_in.press_diff = 73.0;
    packet_in.temperature = 17859;

    mavlink::common::msg::SCALED_PRESSURE3 packet2{};

    mavlink_msg_scaled_pressure3_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.press_abs, packet2.press_abs);
    EXPECT_EQ(packet_in.press_diff, packet2.press_diff);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, FOLLOW_TARGET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::FOLLOW_TARGET packet_in{};
    packet_in.timestamp = 93372036854775807ULL;
    packet_in.est_capabilities = 25;
    packet_in.lat = 963498296;
    packet_in.lon = 963498504;
    packet_in.alt = 185.0;
    packet_in.vel = {{ 213.0, 214.0, 215.0 }};
    packet_in.acc = {{ 297.0, 298.0, 299.0 }};
    packet_in.attitude_q = {{ 381.0, 382.0, 383.0, 384.0 }};
    packet_in.rates = {{ 493.0, 494.0, 495.0 }};
    packet_in.position_cov = {{ 577.0, 578.0, 579.0 }};
    packet_in.custom_state = 93372036854776311ULL;

    mavlink::common::msg::FOLLOW_TARGET packet1{};
    mavlink::common::msg::FOLLOW_TARGET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.timestamp, packet2.timestamp);
    EXPECT_EQ(packet1.est_capabilities, packet2.est_capabilities);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.vel, packet2.vel);
    EXPECT_EQ(packet1.acc, packet2.acc);
    EXPECT_EQ(packet1.attitude_q, packet2.attitude_q);
    EXPECT_EQ(packet1.rates, packet2.rates);
    EXPECT_EQ(packet1.position_cov, packet2.position_cov);
    EXPECT_EQ(packet1.custom_state, packet2.custom_state);
}

#ifdef TEST_INTEROP
TEST(common_interop, FOLLOW_TARGET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_follow_target_t packet_c {
         93372036854775807ULL, 93372036854776311ULL, 963498296, 963498504, 185.0, { 213.0, 214.0, 215.0 }, { 297.0, 298.0, 299.0 }, { 381.0, 382.0, 383.0, 384.0 }, { 493.0, 494.0, 495.0 }, { 577.0, 578.0, 579.0 }, 25
    };

    mavlink::common::msg::FOLLOW_TARGET packet_in{};
    packet_in.timestamp = 93372036854775807ULL;
    packet_in.est_capabilities = 25;
    packet_in.lat = 963498296;
    packet_in.lon = 963498504;
    packet_in.alt = 185.0;
    packet_in.vel = {{ 213.0, 214.0, 215.0 }};
    packet_in.acc = {{ 297.0, 298.0, 299.0 }};
    packet_in.attitude_q = {{ 381.0, 382.0, 383.0, 384.0 }};
    packet_in.rates = {{ 493.0, 494.0, 495.0 }};
    packet_in.position_cov = {{ 577.0, 578.0, 579.0 }};
    packet_in.custom_state = 93372036854776311ULL;

    mavlink::common::msg::FOLLOW_TARGET packet2{};

    mavlink_msg_follow_target_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.timestamp, packet2.timestamp);
    EXPECT_EQ(packet_in.est_capabilities, packet2.est_capabilities);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.vel, packet2.vel);
    EXPECT_EQ(packet_in.acc, packet2.acc);
    EXPECT_EQ(packet_in.attitude_q, packet2.attitude_q);
    EXPECT_EQ(packet_in.rates, packet2.rates);
    EXPECT_EQ(packet_in.position_cov, packet2.position_cov);
    EXPECT_EQ(packet_in.custom_state, packet2.custom_state);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CONTROL_SYSTEM_STATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CONTROL_SYSTEM_STATE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.x_acc = 73.0;
    packet_in.y_acc = 101.0;
    packet_in.z_acc = 129.0;
    packet_in.x_vel = 157.0;
    packet_in.y_vel = 185.0;
    packet_in.z_vel = 213.0;
    packet_in.x_pos = 241.0;
    packet_in.y_pos = 269.0;
    packet_in.z_pos = 297.0;
    packet_in.airspeed = 325.0;
    packet_in.vel_variance = {{ 353.0, 354.0, 355.0 }};
    packet_in.pos_variance = {{ 437.0, 438.0, 439.0 }};
    packet_in.q = {{ 521.0, 522.0, 523.0, 524.0 }};
    packet_in.roll_rate = 633.0;
    packet_in.pitch_rate = 661.0;
    packet_in.yaw_rate = 689.0;

    mavlink::common::msg::CONTROL_SYSTEM_STATE packet1{};
    mavlink::common::msg::CONTROL_SYSTEM_STATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.x_acc, packet2.x_acc);
    EXPECT_EQ(packet1.y_acc, packet2.y_acc);
    EXPECT_EQ(packet1.z_acc, packet2.z_acc);
    EXPECT_EQ(packet1.x_vel, packet2.x_vel);
    EXPECT_EQ(packet1.y_vel, packet2.y_vel);
    EXPECT_EQ(packet1.z_vel, packet2.z_vel);
    EXPECT_EQ(packet1.x_pos, packet2.x_pos);
    EXPECT_EQ(packet1.y_pos, packet2.y_pos);
    EXPECT_EQ(packet1.z_pos, packet2.z_pos);
    EXPECT_EQ(packet1.airspeed, packet2.airspeed);
    EXPECT_EQ(packet1.vel_variance, packet2.vel_variance);
    EXPECT_EQ(packet1.pos_variance, packet2.pos_variance);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.roll_rate, packet2.roll_rate);
    EXPECT_EQ(packet1.pitch_rate, packet2.pitch_rate);
    EXPECT_EQ(packet1.yaw_rate, packet2.yaw_rate);
}

#ifdef TEST_INTEROP
TEST(common_interop, CONTROL_SYSTEM_STATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_control_system_state_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, { 353.0, 354.0, 355.0 }, { 437.0, 438.0, 439.0 }, { 521.0, 522.0, 523.0, 524.0 }, 633.0, 661.0, 689.0
    };

    mavlink::common::msg::CONTROL_SYSTEM_STATE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.x_acc = 73.0;
    packet_in.y_acc = 101.0;
    packet_in.z_acc = 129.0;
    packet_in.x_vel = 157.0;
    packet_in.y_vel = 185.0;
    packet_in.z_vel = 213.0;
    packet_in.x_pos = 241.0;
    packet_in.y_pos = 269.0;
    packet_in.z_pos = 297.0;
    packet_in.airspeed = 325.0;
    packet_in.vel_variance = {{ 353.0, 354.0, 355.0 }};
    packet_in.pos_variance = {{ 437.0, 438.0, 439.0 }};
    packet_in.q = {{ 521.0, 522.0, 523.0, 524.0 }};
    packet_in.roll_rate = 633.0;
    packet_in.pitch_rate = 661.0;
    packet_in.yaw_rate = 689.0;

    mavlink::common::msg::CONTROL_SYSTEM_STATE packet2{};

    mavlink_msg_control_system_state_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.x_acc, packet2.x_acc);
    EXPECT_EQ(packet_in.y_acc, packet2.y_acc);
    EXPECT_EQ(packet_in.z_acc, packet2.z_acc);
    EXPECT_EQ(packet_in.x_vel, packet2.x_vel);
    EXPECT_EQ(packet_in.y_vel, packet2.y_vel);
    EXPECT_EQ(packet_in.z_vel, packet2.z_vel);
    EXPECT_EQ(packet_in.x_pos, packet2.x_pos);
    EXPECT_EQ(packet_in.y_pos, packet2.y_pos);
    EXPECT_EQ(packet_in.z_pos, packet2.z_pos);
    EXPECT_EQ(packet_in.airspeed, packet2.airspeed);
    EXPECT_EQ(packet_in.vel_variance, packet2.vel_variance);
    EXPECT_EQ(packet_in.pos_variance, packet2.pos_variance);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.roll_rate, packet2.roll_rate);
    EXPECT_EQ(packet_in.pitch_rate, packet2.pitch_rate);
    EXPECT_EQ(packet_in.yaw_rate, packet2.yaw_rate);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, BATTERY_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::BATTERY_STATUS packet_in{};
    packet_in.id = 101;
    packet_in.battery_function = 168;
    packet_in.type = 235;
    packet_in.temperature = 17651;
    packet_in.voltages = {{ 17755, 17756, 17757, 17758, 17759, 17760, 17761, 17762, 17763, 17764 }};
    packet_in.current_battery = 18795;
    packet_in.current_consumed = 963497464;
    packet_in.energy_consumed = 963497672;
    packet_in.battery_remaining = 46;
    packet_in.time_remaining = 963499336;
    packet_in.charge_state = 125;

    mavlink::common::msg::BATTERY_STATUS packet1{};
    mavlink::common::msg::BATTERY_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.id, packet2.id);
    EXPECT_EQ(packet1.battery_function, packet2.battery_function);
    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.voltages, packet2.voltages);
    EXPECT_EQ(packet1.current_battery, packet2.current_battery);
    EXPECT_EQ(packet1.current_consumed, packet2.current_consumed);
    EXPECT_EQ(packet1.energy_consumed, packet2.energy_consumed);
    EXPECT_EQ(packet1.battery_remaining, packet2.battery_remaining);
    EXPECT_EQ(packet1.time_remaining, packet2.time_remaining);
    EXPECT_EQ(packet1.charge_state, packet2.charge_state);
}

#ifdef TEST_INTEROP
TEST(common_interop, BATTERY_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_battery_status_t packet_c {
         963497464, 963497672, 17651, { 17755, 17756, 17757, 17758, 17759, 17760, 17761, 17762, 17763, 17764 }, 18795, 101, 168, 235, 46, 963499336, 125
    };

    mavlink::common::msg::BATTERY_STATUS packet_in{};
    packet_in.id = 101;
    packet_in.battery_function = 168;
    packet_in.type = 235;
    packet_in.temperature = 17651;
    packet_in.voltages = {{ 17755, 17756, 17757, 17758, 17759, 17760, 17761, 17762, 17763, 17764 }};
    packet_in.current_battery = 18795;
    packet_in.current_consumed = 963497464;
    packet_in.energy_consumed = 963497672;
    packet_in.battery_remaining = 46;
    packet_in.time_remaining = 963499336;
    packet_in.charge_state = 125;

    mavlink::common::msg::BATTERY_STATUS packet2{};

    mavlink_msg_battery_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.id, packet2.id);
    EXPECT_EQ(packet_in.battery_function, packet2.battery_function);
    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.voltages, packet2.voltages);
    EXPECT_EQ(packet_in.current_battery, packet2.current_battery);
    EXPECT_EQ(packet_in.current_consumed, packet2.current_consumed);
    EXPECT_EQ(packet_in.energy_consumed, packet2.energy_consumed);
    EXPECT_EQ(packet_in.battery_remaining, packet2.battery_remaining);
    EXPECT_EQ(packet_in.time_remaining, packet2.time_remaining);
    EXPECT_EQ(packet_in.charge_state, packet2.charge_state);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, AUTOPILOT_VERSION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::AUTOPILOT_VERSION packet_in{};
    packet_in.capabilities = 93372036854775807ULL;
    packet_in.flight_sw_version = 963498296;
    packet_in.middleware_sw_version = 963498504;
    packet_in.os_sw_version = 963498712;
    packet_in.board_version = 963498920;
    packet_in.flight_custom_version = {{ 113, 114, 115, 116, 117, 118, 119, 120 }};
    packet_in.middleware_custom_version = {{ 137, 138, 139, 140, 141, 142, 143, 144 }};
    packet_in.os_custom_version = {{ 161, 162, 163, 164, 165, 166, 167, 168 }};
    packet_in.vendor_id = 18899;
    packet_in.product_id = 19003;
    packet_in.uid = 93372036854776311ULL;
    packet_in.uid2 = {{ 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202 }};

    mavlink::common::msg::AUTOPILOT_VERSION packet1{};
    mavlink::common::msg::AUTOPILOT_VERSION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.capabilities, packet2.capabilities);
    EXPECT_EQ(packet1.flight_sw_version, packet2.flight_sw_version);
    EXPECT_EQ(packet1.middleware_sw_version, packet2.middleware_sw_version);
    EXPECT_EQ(packet1.os_sw_version, packet2.os_sw_version);
    EXPECT_EQ(packet1.board_version, packet2.board_version);
    EXPECT_EQ(packet1.flight_custom_version, packet2.flight_custom_version);
    EXPECT_EQ(packet1.middleware_custom_version, packet2.middleware_custom_version);
    EXPECT_EQ(packet1.os_custom_version, packet2.os_custom_version);
    EXPECT_EQ(packet1.vendor_id, packet2.vendor_id);
    EXPECT_EQ(packet1.product_id, packet2.product_id);
    EXPECT_EQ(packet1.uid, packet2.uid);
    EXPECT_EQ(packet1.uid2, packet2.uid2);
}

#ifdef TEST_INTEROP
TEST(common_interop, AUTOPILOT_VERSION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_autopilot_version_t packet_c {
         93372036854775807ULL, 93372036854776311ULL, 963498296, 963498504, 963498712, 963498920, 18899, 19003, { 113, 114, 115, 116, 117, 118, 119, 120 }, { 137, 138, 139, 140, 141, 142, 143, 144 }, { 161, 162, 163, 164, 165, 166, 167, 168 }, { 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202 }
    };

    mavlink::common::msg::AUTOPILOT_VERSION packet_in{};
    packet_in.capabilities = 93372036854775807ULL;
    packet_in.flight_sw_version = 963498296;
    packet_in.middleware_sw_version = 963498504;
    packet_in.os_sw_version = 963498712;
    packet_in.board_version = 963498920;
    packet_in.flight_custom_version = {{ 113, 114, 115, 116, 117, 118, 119, 120 }};
    packet_in.middleware_custom_version = {{ 137, 138, 139, 140, 141, 142, 143, 144 }};
    packet_in.os_custom_version = {{ 161, 162, 163, 164, 165, 166, 167, 168 }};
    packet_in.vendor_id = 18899;
    packet_in.product_id = 19003;
    packet_in.uid = 93372036854776311ULL;
    packet_in.uid2 = {{ 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202 }};

    mavlink::common::msg::AUTOPILOT_VERSION packet2{};

    mavlink_msg_autopilot_version_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.capabilities, packet2.capabilities);
    EXPECT_EQ(packet_in.flight_sw_version, packet2.flight_sw_version);
    EXPECT_EQ(packet_in.middleware_sw_version, packet2.middleware_sw_version);
    EXPECT_EQ(packet_in.os_sw_version, packet2.os_sw_version);
    EXPECT_EQ(packet_in.board_version, packet2.board_version);
    EXPECT_EQ(packet_in.flight_custom_version, packet2.flight_custom_version);
    EXPECT_EQ(packet_in.middleware_custom_version, packet2.middleware_custom_version);
    EXPECT_EQ(packet_in.os_custom_version, packet2.os_custom_version);
    EXPECT_EQ(packet_in.vendor_id, packet2.vendor_id);
    EXPECT_EQ(packet_in.product_id, packet2.product_id);
    EXPECT_EQ(packet_in.uid, packet2.uid);
    EXPECT_EQ(packet_in.uid2, packet2.uid2);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LANDING_TARGET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LANDING_TARGET packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.target_num = 89;
    packet_in.frame = 156;
    packet_in.angle_x = 73.0;
    packet_in.angle_y = 101.0;
    packet_in.distance = 129.0;
    packet_in.size_x = 157.0;
    packet_in.size_y = 185.0;
    packet_in.x = 227.0;
    packet_in.y = 255.0;
    packet_in.z = 283.0;
    packet_in.q = {{ 311.0, 312.0, 313.0, 314.0 }};
    packet_in.type = 51;
    packet_in.position_valid = 118;

    mavlink::common::msg::LANDING_TARGET packet1{};
    mavlink::common::msg::LANDING_TARGET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.target_num, packet2.target_num);
    EXPECT_EQ(packet1.frame, packet2.frame);
    EXPECT_EQ(packet1.angle_x, packet2.angle_x);
    EXPECT_EQ(packet1.angle_y, packet2.angle_y);
    EXPECT_EQ(packet1.distance, packet2.distance);
    EXPECT_EQ(packet1.size_x, packet2.size_x);
    EXPECT_EQ(packet1.size_y, packet2.size_y);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.position_valid, packet2.position_valid);
}

#ifdef TEST_INTEROP
TEST(common_interop, LANDING_TARGET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_landing_target_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 89, 156, 227.0, 255.0, 283.0, { 311.0, 312.0, 313.0, 314.0 }, 51, 118
    };

    mavlink::common::msg::LANDING_TARGET packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.target_num = 89;
    packet_in.frame = 156;
    packet_in.angle_x = 73.0;
    packet_in.angle_y = 101.0;
    packet_in.distance = 129.0;
    packet_in.size_x = 157.0;
    packet_in.size_y = 185.0;
    packet_in.x = 227.0;
    packet_in.y = 255.0;
    packet_in.z = 283.0;
    packet_in.q = {{ 311.0, 312.0, 313.0, 314.0 }};
    packet_in.type = 51;
    packet_in.position_valid = 118;

    mavlink::common::msg::LANDING_TARGET packet2{};

    mavlink_msg_landing_target_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.target_num, packet2.target_num);
    EXPECT_EQ(packet_in.frame, packet2.frame);
    EXPECT_EQ(packet_in.angle_x, packet2.angle_x);
    EXPECT_EQ(packet_in.angle_y, packet2.angle_y);
    EXPECT_EQ(packet_in.distance, packet2.distance);
    EXPECT_EQ(packet_in.size_x, packet2.size_x);
    EXPECT_EQ(packet_in.size_y, packet2.size_y);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.position_valid, packet2.position_valid);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ESTIMATOR_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ESTIMATOR_STATUS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.flags = 19315;
    packet_in.vel_ratio = 73.0;
    packet_in.pos_horiz_ratio = 101.0;
    packet_in.pos_vert_ratio = 129.0;
    packet_in.mag_ratio = 157.0;
    packet_in.hagl_ratio = 185.0;
    packet_in.tas_ratio = 213.0;
    packet_in.pos_horiz_accuracy = 241.0;
    packet_in.pos_vert_accuracy = 269.0;

    mavlink::common::msg::ESTIMATOR_STATUS packet1{};
    mavlink::common::msg::ESTIMATOR_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.vel_ratio, packet2.vel_ratio);
    EXPECT_EQ(packet1.pos_horiz_ratio, packet2.pos_horiz_ratio);
    EXPECT_EQ(packet1.pos_vert_ratio, packet2.pos_vert_ratio);
    EXPECT_EQ(packet1.mag_ratio, packet2.mag_ratio);
    EXPECT_EQ(packet1.hagl_ratio, packet2.hagl_ratio);
    EXPECT_EQ(packet1.tas_ratio, packet2.tas_ratio);
    EXPECT_EQ(packet1.pos_horiz_accuracy, packet2.pos_horiz_accuracy);
    EXPECT_EQ(packet1.pos_vert_accuracy, packet2.pos_vert_accuracy);
}

#ifdef TEST_INTEROP
TEST(common_interop, ESTIMATOR_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_estimator_status_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 19315
    };

    mavlink::common::msg::ESTIMATOR_STATUS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.flags = 19315;
    packet_in.vel_ratio = 73.0;
    packet_in.pos_horiz_ratio = 101.0;
    packet_in.pos_vert_ratio = 129.0;
    packet_in.mag_ratio = 157.0;
    packet_in.hagl_ratio = 185.0;
    packet_in.tas_ratio = 213.0;
    packet_in.pos_horiz_accuracy = 241.0;
    packet_in.pos_vert_accuracy = 269.0;

    mavlink::common::msg::ESTIMATOR_STATUS packet2{};

    mavlink_msg_estimator_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.vel_ratio, packet2.vel_ratio);
    EXPECT_EQ(packet_in.pos_horiz_ratio, packet2.pos_horiz_ratio);
    EXPECT_EQ(packet_in.pos_vert_ratio, packet2.pos_vert_ratio);
    EXPECT_EQ(packet_in.mag_ratio, packet2.mag_ratio);
    EXPECT_EQ(packet_in.hagl_ratio, packet2.hagl_ratio);
    EXPECT_EQ(packet_in.tas_ratio, packet2.tas_ratio);
    EXPECT_EQ(packet_in.pos_horiz_accuracy, packet2.pos_horiz_accuracy);
    EXPECT_EQ(packet_in.pos_vert_accuracy, packet2.pos_vert_accuracy);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, WIND_COV)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::WIND_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.wind_x = 73.0;
    packet_in.wind_y = 101.0;
    packet_in.wind_z = 129.0;
    packet_in.var_horiz = 157.0;
    packet_in.var_vert = 185.0;
    packet_in.wind_alt = 213.0;
    packet_in.horiz_accuracy = 241.0;
    packet_in.vert_accuracy = 269.0;

    mavlink::common::msg::WIND_COV packet1{};
    mavlink::common::msg::WIND_COV packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.wind_x, packet2.wind_x);
    EXPECT_EQ(packet1.wind_y, packet2.wind_y);
    EXPECT_EQ(packet1.wind_z, packet2.wind_z);
    EXPECT_EQ(packet1.var_horiz, packet2.var_horiz);
    EXPECT_EQ(packet1.var_vert, packet2.var_vert);
    EXPECT_EQ(packet1.wind_alt, packet2.wind_alt);
    EXPECT_EQ(packet1.horiz_accuracy, packet2.horiz_accuracy);
    EXPECT_EQ(packet1.vert_accuracy, packet2.vert_accuracy);
}

#ifdef TEST_INTEROP
TEST(common_interop, WIND_COV)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_wind_cov_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0
    };

    mavlink::common::msg::WIND_COV packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.wind_x = 73.0;
    packet_in.wind_y = 101.0;
    packet_in.wind_z = 129.0;
    packet_in.var_horiz = 157.0;
    packet_in.var_vert = 185.0;
    packet_in.wind_alt = 213.0;
    packet_in.horiz_accuracy = 241.0;
    packet_in.vert_accuracy = 269.0;

    mavlink::common::msg::WIND_COV packet2{};

    mavlink_msg_wind_cov_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.wind_x, packet2.wind_x);
    EXPECT_EQ(packet_in.wind_y, packet2.wind_y);
    EXPECT_EQ(packet_in.wind_z, packet2.wind_z);
    EXPECT_EQ(packet_in.var_horiz, packet2.var_horiz);
    EXPECT_EQ(packet_in.var_vert, packet2.var_vert);
    EXPECT_EQ(packet_in.wind_alt, packet2.wind_alt);
    EXPECT_EQ(packet_in.horiz_accuracy, packet2.horiz_accuracy);
    EXPECT_EQ(packet_in.vert_accuracy, packet2.vert_accuracy);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_INPUT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_INPUT packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.gps_id = 185;
    packet_in.ignore_flags = 20147;
    packet_in.time_week_ms = 963497880;
    packet_in.time_week = 20251;
    packet_in.fix_type = 252;
    packet_in.lat = 963498088;
    packet_in.lon = 963498296;
    packet_in.alt = 157.0;
    packet_in.hdop = 185.0;
    packet_in.vdop = 213.0;
    packet_in.vn = 241.0;
    packet_in.ve = 269.0;
    packet_in.vd = 297.0;
    packet_in.speed_accuracy = 325.0;
    packet_in.horiz_accuracy = 353.0;
    packet_in.vert_accuracy = 381.0;
    packet_in.satellites_visible = 63;

    mavlink::common::msg::GPS_INPUT packet1{};
    mavlink::common::msg::GPS_INPUT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.gps_id, packet2.gps_id);
    EXPECT_EQ(packet1.ignore_flags, packet2.ignore_flags);
    EXPECT_EQ(packet1.time_week_ms, packet2.time_week_ms);
    EXPECT_EQ(packet1.time_week, packet2.time_week);
    EXPECT_EQ(packet1.fix_type, packet2.fix_type);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.hdop, packet2.hdop);
    EXPECT_EQ(packet1.vdop, packet2.vdop);
    EXPECT_EQ(packet1.vn, packet2.vn);
    EXPECT_EQ(packet1.ve, packet2.ve);
    EXPECT_EQ(packet1.vd, packet2.vd);
    EXPECT_EQ(packet1.speed_accuracy, packet2.speed_accuracy);
    EXPECT_EQ(packet1.horiz_accuracy, packet2.horiz_accuracy);
    EXPECT_EQ(packet1.vert_accuracy, packet2.vert_accuracy);
    EXPECT_EQ(packet1.satellites_visible, packet2.satellites_visible);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_INPUT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_input_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 963498296, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 353.0, 381.0, 20147, 20251, 185, 252, 63
    };

    mavlink::common::msg::GPS_INPUT packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.gps_id = 185;
    packet_in.ignore_flags = 20147;
    packet_in.time_week_ms = 963497880;
    packet_in.time_week = 20251;
    packet_in.fix_type = 252;
    packet_in.lat = 963498088;
    packet_in.lon = 963498296;
    packet_in.alt = 157.0;
    packet_in.hdop = 185.0;
    packet_in.vdop = 213.0;
    packet_in.vn = 241.0;
    packet_in.ve = 269.0;
    packet_in.vd = 297.0;
    packet_in.speed_accuracy = 325.0;
    packet_in.horiz_accuracy = 353.0;
    packet_in.vert_accuracy = 381.0;
    packet_in.satellites_visible = 63;

    mavlink::common::msg::GPS_INPUT packet2{};

    mavlink_msg_gps_input_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.gps_id, packet2.gps_id);
    EXPECT_EQ(packet_in.ignore_flags, packet2.ignore_flags);
    EXPECT_EQ(packet_in.time_week_ms, packet2.time_week_ms);
    EXPECT_EQ(packet_in.time_week, packet2.time_week);
    EXPECT_EQ(packet_in.fix_type, packet2.fix_type);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.hdop, packet2.hdop);
    EXPECT_EQ(packet_in.vdop, packet2.vdop);
    EXPECT_EQ(packet_in.vn, packet2.vn);
    EXPECT_EQ(packet_in.ve, packet2.ve);
    EXPECT_EQ(packet_in.vd, packet2.vd);
    EXPECT_EQ(packet_in.speed_accuracy, packet2.speed_accuracy);
    EXPECT_EQ(packet_in.horiz_accuracy, packet2.horiz_accuracy);
    EXPECT_EQ(packet_in.vert_accuracy, packet2.vert_accuracy);
    EXPECT_EQ(packet_in.satellites_visible, packet2.satellites_visible);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, GPS_RTCM_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::GPS_RTCM_DATA packet_in{};
    packet_in.flags = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62 }};

    mavlink::common::msg::GPS_RTCM_DATA packet1{};
    mavlink::common::msg::GPS_RTCM_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.len, packet2.len);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, GPS_RTCM_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gps_rtcm_data_t packet_c {
         5, 72, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62 }
    };

    mavlink::common::msg::GPS_RTCM_DATA packet_in{};
    packet_in.flags = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62 }};

    mavlink::common::msg::GPS_RTCM_DATA packet2{};

    mavlink_msg_gps_rtcm_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.len, packet2.len);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HIGH_LATENCY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HIGH_LATENCY packet_in{};
    packet_in.base_mode = 211;
    packet_in.custom_mode = 963497464;
    packet_in.landed_state = 22;
    packet_in.roll = 17859;
    packet_in.pitch = 17963;
    packet_in.heading = 18067;
    packet_in.throttle = 89;
    packet_in.heading_sp = 18171;
    packet_in.latitude = 963497672;
    packet_in.longitude = 963497880;
    packet_in.altitude_amsl = 18275;
    packet_in.altitude_sp = 18379;
    packet_in.airspeed = 156;
    packet_in.airspeed_sp = 223;
    packet_in.groundspeed = 34;
    packet_in.climb_rate = 101;
    packet_in.gps_nsat = 168;
    packet_in.gps_fix_type = 235;
    packet_in.battery_remaining = 46;
    packet_in.temperature = 113;
    packet_in.temperature_air = -76;
    packet_in.failsafe = 247;
    packet_in.wp_num = 58;
    packet_in.wp_distance = 18483;

    mavlink::common::msg::HIGH_LATENCY packet1{};
    mavlink::common::msg::HIGH_LATENCY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.base_mode, packet2.base_mode);
    EXPECT_EQ(packet1.custom_mode, packet2.custom_mode);
    EXPECT_EQ(packet1.landed_state, packet2.landed_state);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.heading, packet2.heading);
    EXPECT_EQ(packet1.throttle, packet2.throttle);
    EXPECT_EQ(packet1.heading_sp, packet2.heading_sp);
    EXPECT_EQ(packet1.latitude, packet2.latitude);
    EXPECT_EQ(packet1.longitude, packet2.longitude);
    EXPECT_EQ(packet1.altitude_amsl, packet2.altitude_amsl);
    EXPECT_EQ(packet1.altitude_sp, packet2.altitude_sp);
    EXPECT_EQ(packet1.airspeed, packet2.airspeed);
    EXPECT_EQ(packet1.airspeed_sp, packet2.airspeed_sp);
    EXPECT_EQ(packet1.groundspeed, packet2.groundspeed);
    EXPECT_EQ(packet1.climb_rate, packet2.climb_rate);
    EXPECT_EQ(packet1.gps_nsat, packet2.gps_nsat);
    EXPECT_EQ(packet1.gps_fix_type, packet2.gps_fix_type);
    EXPECT_EQ(packet1.battery_remaining, packet2.battery_remaining);
    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.temperature_air, packet2.temperature_air);
    EXPECT_EQ(packet1.failsafe, packet2.failsafe);
    EXPECT_EQ(packet1.wp_num, packet2.wp_num);
    EXPECT_EQ(packet1.wp_distance, packet2.wp_distance);
}

#ifdef TEST_INTEROP
TEST(common_interop, HIGH_LATENCY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_high_latency_t packet_c {
         963497464, 963497672, 963497880, 17859, 17963, 18067, 18171, 18275, 18379, 18483, 211, 22, 89, 156, 223, 34, 101, 168, 235, 46, 113, -76, 247, 58
    };

    mavlink::common::msg::HIGH_LATENCY packet_in{};
    packet_in.base_mode = 211;
    packet_in.custom_mode = 963497464;
    packet_in.landed_state = 22;
    packet_in.roll = 17859;
    packet_in.pitch = 17963;
    packet_in.heading = 18067;
    packet_in.throttle = 89;
    packet_in.heading_sp = 18171;
    packet_in.latitude = 963497672;
    packet_in.longitude = 963497880;
    packet_in.altitude_amsl = 18275;
    packet_in.altitude_sp = 18379;
    packet_in.airspeed = 156;
    packet_in.airspeed_sp = 223;
    packet_in.groundspeed = 34;
    packet_in.climb_rate = 101;
    packet_in.gps_nsat = 168;
    packet_in.gps_fix_type = 235;
    packet_in.battery_remaining = 46;
    packet_in.temperature = 113;
    packet_in.temperature_air = -76;
    packet_in.failsafe = 247;
    packet_in.wp_num = 58;
    packet_in.wp_distance = 18483;

    mavlink::common::msg::HIGH_LATENCY packet2{};

    mavlink_msg_high_latency_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.base_mode, packet2.base_mode);
    EXPECT_EQ(packet_in.custom_mode, packet2.custom_mode);
    EXPECT_EQ(packet_in.landed_state, packet2.landed_state);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.heading, packet2.heading);
    EXPECT_EQ(packet_in.throttle, packet2.throttle);
    EXPECT_EQ(packet_in.heading_sp, packet2.heading_sp);
    EXPECT_EQ(packet_in.latitude, packet2.latitude);
    EXPECT_EQ(packet_in.longitude, packet2.longitude);
    EXPECT_EQ(packet_in.altitude_amsl, packet2.altitude_amsl);
    EXPECT_EQ(packet_in.altitude_sp, packet2.altitude_sp);
    EXPECT_EQ(packet_in.airspeed, packet2.airspeed);
    EXPECT_EQ(packet_in.airspeed_sp, packet2.airspeed_sp);
    EXPECT_EQ(packet_in.groundspeed, packet2.groundspeed);
    EXPECT_EQ(packet_in.climb_rate, packet2.climb_rate);
    EXPECT_EQ(packet_in.gps_nsat, packet2.gps_nsat);
    EXPECT_EQ(packet_in.gps_fix_type, packet2.gps_fix_type);
    EXPECT_EQ(packet_in.battery_remaining, packet2.battery_remaining);
    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.temperature_air, packet2.temperature_air);
    EXPECT_EQ(packet_in.failsafe, packet2.failsafe);
    EXPECT_EQ(packet_in.wp_num, packet2.wp_num);
    EXPECT_EQ(packet_in.wp_distance, packet2.wp_distance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, VIBRATION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::VIBRATION packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.vibration_x = 73.0;
    packet_in.vibration_y = 101.0;
    packet_in.vibration_z = 129.0;
    packet_in.clipping_0 = 963498504;
    packet_in.clipping_1 = 963498712;
    packet_in.clipping_2 = 963498920;

    mavlink::common::msg::VIBRATION packet1{};
    mavlink::common::msg::VIBRATION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.vibration_x, packet2.vibration_x);
    EXPECT_EQ(packet1.vibration_y, packet2.vibration_y);
    EXPECT_EQ(packet1.vibration_z, packet2.vibration_z);
    EXPECT_EQ(packet1.clipping_0, packet2.clipping_0);
    EXPECT_EQ(packet1.clipping_1, packet2.clipping_1);
    EXPECT_EQ(packet1.clipping_2, packet2.clipping_2);
}

#ifdef TEST_INTEROP
TEST(common_interop, VIBRATION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_vibration_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 963498504, 963498712, 963498920
    };

    mavlink::common::msg::VIBRATION packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.vibration_x = 73.0;
    packet_in.vibration_y = 101.0;
    packet_in.vibration_z = 129.0;
    packet_in.clipping_0 = 963498504;
    packet_in.clipping_1 = 963498712;
    packet_in.clipping_2 = 963498920;

    mavlink::common::msg::VIBRATION packet2{};

    mavlink_msg_vibration_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.vibration_x, packet2.vibration_x);
    EXPECT_EQ(packet_in.vibration_y, packet2.vibration_y);
    EXPECT_EQ(packet_in.vibration_z, packet2.vibration_z);
    EXPECT_EQ(packet_in.clipping_0, packet2.clipping_0);
    EXPECT_EQ(packet_in.clipping_1, packet2.clipping_1);
    EXPECT_EQ(packet_in.clipping_2, packet2.clipping_2);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, HOME_POSITION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::HOME_POSITION packet_in{};
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.x = 101.0;
    packet_in.y = 129.0;
    packet_in.z = 157.0;
    packet_in.q = {{ 185.0, 186.0, 187.0, 188.0 }};
    packet_in.approach_x = 297.0;
    packet_in.approach_y = 325.0;
    packet_in.approach_z = 353.0;
    packet_in.time_usec = 93372036854779083ULL;

    mavlink::common::msg::HOME_POSITION packet1{};
    mavlink::common::msg::HOME_POSITION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.latitude, packet2.latitude);
    EXPECT_EQ(packet1.longitude, packet2.longitude);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.approach_x, packet2.approach_x);
    EXPECT_EQ(packet1.approach_y, packet2.approach_y);
    EXPECT_EQ(packet1.approach_z, packet2.approach_z);
    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
}

#ifdef TEST_INTEROP
TEST(common_interop, HOME_POSITION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_home_position_t packet_c {
         963497464, 963497672, 963497880, 101.0, 129.0, 157.0, { 185.0, 186.0, 187.0, 188.0 }, 297.0, 325.0, 353.0, 93372036854779083ULL
    };

    mavlink::common::msg::HOME_POSITION packet_in{};
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.x = 101.0;
    packet_in.y = 129.0;
    packet_in.z = 157.0;
    packet_in.q = {{ 185.0, 186.0, 187.0, 188.0 }};
    packet_in.approach_x = 297.0;
    packet_in.approach_y = 325.0;
    packet_in.approach_z = 353.0;
    packet_in.time_usec = 93372036854779083ULL;

    mavlink::common::msg::HOME_POSITION packet2{};

    mavlink_msg_home_position_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.latitude, packet2.latitude);
    EXPECT_EQ(packet_in.longitude, packet2.longitude);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.approach_x, packet2.approach_x);
    EXPECT_EQ(packet_in.approach_y, packet2.approach_y);
    EXPECT_EQ(packet_in.approach_z, packet2.approach_z);
    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SET_HOME_POSITION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SET_HOME_POSITION packet_in{};
    packet_in.target_system = 161;
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.x = 101.0;
    packet_in.y = 129.0;
    packet_in.z = 157.0;
    packet_in.q = {{ 185.0, 186.0, 187.0, 188.0 }};
    packet_in.approach_x = 297.0;
    packet_in.approach_y = 325.0;
    packet_in.approach_z = 353.0;
    packet_in.time_usec = 93372036854779146ULL;

    mavlink::common::msg::SET_HOME_POSITION packet1{};
    mavlink::common::msg::SET_HOME_POSITION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.latitude, packet2.latitude);
    EXPECT_EQ(packet1.longitude, packet2.longitude);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.approach_x, packet2.approach_x);
    EXPECT_EQ(packet1.approach_y, packet2.approach_y);
    EXPECT_EQ(packet1.approach_z, packet2.approach_z);
    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
}

#ifdef TEST_INTEROP
TEST(common_interop, SET_HOME_POSITION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_home_position_t packet_c {
         963497464, 963497672, 963497880, 101.0, 129.0, 157.0, { 185.0, 186.0, 187.0, 188.0 }, 297.0, 325.0, 353.0, 161, 93372036854779146ULL
    };

    mavlink::common::msg::SET_HOME_POSITION packet_in{};
    packet_in.target_system = 161;
    packet_in.latitude = 963497464;
    packet_in.longitude = 963497672;
    packet_in.altitude = 963497880;
    packet_in.x = 101.0;
    packet_in.y = 129.0;
    packet_in.z = 157.0;
    packet_in.q = {{ 185.0, 186.0, 187.0, 188.0 }};
    packet_in.approach_x = 297.0;
    packet_in.approach_y = 325.0;
    packet_in.approach_z = 353.0;
    packet_in.time_usec = 93372036854779146ULL;

    mavlink::common::msg::SET_HOME_POSITION packet2{};

    mavlink_msg_set_home_position_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.latitude, packet2.latitude);
    EXPECT_EQ(packet_in.longitude, packet2.longitude);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.approach_x, packet2.approach_x);
    EXPECT_EQ(packet_in.approach_y, packet2.approach_y);
    EXPECT_EQ(packet_in.approach_z, packet2.approach_z);
    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MESSAGE_INTERVAL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MESSAGE_INTERVAL packet_in{};
    packet_in.message_id = 17443;
    packet_in.interval_us = 963497464;

    mavlink::common::msg::MESSAGE_INTERVAL packet1{};
    mavlink::common::msg::MESSAGE_INTERVAL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.message_id, packet2.message_id);
    EXPECT_EQ(packet1.interval_us, packet2.interval_us);
}

#ifdef TEST_INTEROP
TEST(common_interop, MESSAGE_INTERVAL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_message_interval_t packet_c {
         963497464, 17443
    };

    mavlink::common::msg::MESSAGE_INTERVAL packet_in{};
    packet_in.message_id = 17443;
    packet_in.interval_us = 963497464;

    mavlink::common::msg::MESSAGE_INTERVAL packet2{};

    mavlink_msg_message_interval_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.message_id, packet2.message_id);
    EXPECT_EQ(packet_in.interval_us, packet2.interval_us);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, EXTENDED_SYS_STATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::EXTENDED_SYS_STATE packet_in{};
    packet_in.vtol_state = 5;
    packet_in.landed_state = 72;

    mavlink::common::msg::EXTENDED_SYS_STATE packet1{};
    mavlink::common::msg::EXTENDED_SYS_STATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.vtol_state, packet2.vtol_state);
    EXPECT_EQ(packet1.landed_state, packet2.landed_state);
}

#ifdef TEST_INTEROP
TEST(common_interop, EXTENDED_SYS_STATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_extended_sys_state_t packet_c {
         5, 72
    };

    mavlink::common::msg::EXTENDED_SYS_STATE packet_in{};
    packet_in.vtol_state = 5;
    packet_in.landed_state = 72;

    mavlink::common::msg::EXTENDED_SYS_STATE packet2{};

    mavlink_msg_extended_sys_state_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.vtol_state, packet2.vtol_state);
    EXPECT_EQ(packet_in.landed_state, packet2.landed_state);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ADSB_VEHICLE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ADSB_VEHICLE packet_in{};
    packet_in.ICAO_address = 963497464;
    packet_in.lat = 963497672;
    packet_in.lon = 963497880;
    packet_in.altitude_type = 211;
    packet_in.altitude = 963498088;
    packet_in.heading = 18067;
    packet_in.hor_velocity = 18171;
    packet_in.ver_velocity = 18275;
    packet_in.callsign = to_char_array("BCDEFGHI");
    packet_in.emitter_type = 113;
    packet_in.tslc = 180;
    packet_in.flags = 18379;
    packet_in.squawk = 18483;

    mavlink::common::msg::ADSB_VEHICLE packet1{};
    mavlink::common::msg::ADSB_VEHICLE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.ICAO_address, packet2.ICAO_address);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.altitude_type, packet2.altitude_type);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.heading, packet2.heading);
    EXPECT_EQ(packet1.hor_velocity, packet2.hor_velocity);
    EXPECT_EQ(packet1.ver_velocity, packet2.ver_velocity);
    EXPECT_EQ(packet1.callsign, packet2.callsign);
    EXPECT_EQ(packet1.emitter_type, packet2.emitter_type);
    EXPECT_EQ(packet1.tslc, packet2.tslc);
    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.squawk, packet2.squawk);
}

#ifdef TEST_INTEROP
TEST(common_interop, ADSB_VEHICLE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_adsb_vehicle_t packet_c {
         963497464, 963497672, 963497880, 963498088, 18067, 18171, 18275, 18379, 18483, 211, "BCDEFGHI", 113, 180
    };

    mavlink::common::msg::ADSB_VEHICLE packet_in{};
    packet_in.ICAO_address = 963497464;
    packet_in.lat = 963497672;
    packet_in.lon = 963497880;
    packet_in.altitude_type = 211;
    packet_in.altitude = 963498088;
    packet_in.heading = 18067;
    packet_in.hor_velocity = 18171;
    packet_in.ver_velocity = 18275;
    packet_in.callsign = to_char_array("BCDEFGHI");
    packet_in.emitter_type = 113;
    packet_in.tslc = 180;
    packet_in.flags = 18379;
    packet_in.squawk = 18483;

    mavlink::common::msg::ADSB_VEHICLE packet2{};

    mavlink_msg_adsb_vehicle_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.ICAO_address, packet2.ICAO_address);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.altitude_type, packet2.altitude_type);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.heading, packet2.heading);
    EXPECT_EQ(packet_in.hor_velocity, packet2.hor_velocity);
    EXPECT_EQ(packet_in.ver_velocity, packet2.ver_velocity);
    EXPECT_EQ(packet_in.callsign, packet2.callsign);
    EXPECT_EQ(packet_in.emitter_type, packet2.emitter_type);
    EXPECT_EQ(packet_in.tslc, packet2.tslc);
    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.squawk, packet2.squawk);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, COLLISION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::COLLISION packet_in{};
    packet_in.src = 53;
    packet_in.id = 963497464;
    packet_in.action = 120;
    packet_in.threat_level = 187;
    packet_in.time_to_minimum_delta = 45.0;
    packet_in.altitude_minimum_delta = 73.0;
    packet_in.horizontal_minimum_delta = 101.0;

    mavlink::common::msg::COLLISION packet1{};
    mavlink::common::msg::COLLISION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.src, packet2.src);
    EXPECT_EQ(packet1.id, packet2.id);
    EXPECT_EQ(packet1.action, packet2.action);
    EXPECT_EQ(packet1.threat_level, packet2.threat_level);
    EXPECT_EQ(packet1.time_to_minimum_delta, packet2.time_to_minimum_delta);
    EXPECT_EQ(packet1.altitude_minimum_delta, packet2.altitude_minimum_delta);
    EXPECT_EQ(packet1.horizontal_minimum_delta, packet2.horizontal_minimum_delta);
}

#ifdef TEST_INTEROP
TEST(common_interop, COLLISION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_collision_t packet_c {
         963497464, 45.0, 73.0, 101.0, 53, 120, 187
    };

    mavlink::common::msg::COLLISION packet_in{};
    packet_in.src = 53;
    packet_in.id = 963497464;
    packet_in.action = 120;
    packet_in.threat_level = 187;
    packet_in.time_to_minimum_delta = 45.0;
    packet_in.altitude_minimum_delta = 73.0;
    packet_in.horizontal_minimum_delta = 101.0;

    mavlink::common::msg::COLLISION packet2{};

    mavlink_msg_collision_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.src, packet2.src);
    EXPECT_EQ(packet_in.id, packet2.id);
    EXPECT_EQ(packet_in.action, packet2.action);
    EXPECT_EQ(packet_in.threat_level, packet2.threat_level);
    EXPECT_EQ(packet_in.time_to_minimum_delta, packet2.time_to_minimum_delta);
    EXPECT_EQ(packet_in.altitude_minimum_delta, packet2.altitude_minimum_delta);
    EXPECT_EQ(packet_in.horizontal_minimum_delta, packet2.horizontal_minimum_delta);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, V2_EXTENSION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::V2_EXTENSION packet_in{};
    packet_in.target_network = 139;
    packet_in.target_system = 206;
    packet_in.target_component = 17;
    packet_in.message_type = 17235;
    packet_in.payload = {{ 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76 }};

    mavlink::common::msg::V2_EXTENSION packet1{};
    mavlink::common::msg::V2_EXTENSION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_network, packet2.target_network);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.message_type, packet2.message_type);
    EXPECT_EQ(packet1.payload, packet2.payload);
}

#ifdef TEST_INTEROP
TEST(common_interop, V2_EXTENSION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_v2_extension_t packet_c {
         17235, 139, 206, 17, { 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76 }
    };

    mavlink::common::msg::V2_EXTENSION packet_in{};
    packet_in.target_network = 139;
    packet_in.target_system = 206;
    packet_in.target_component = 17;
    packet_in.message_type = 17235;
    packet_in.payload = {{ 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76 }};

    mavlink::common::msg::V2_EXTENSION packet2{};

    mavlink_msg_v2_extension_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_network, packet2.target_network);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.message_type, packet2.message_type);
    EXPECT_EQ(packet_in.payload, packet2.payload);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MEMORY_VECT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MEMORY_VECT packet_in{};
    packet_in.address = 17235;
    packet_in.ver = 139;
    packet_in.type = 206;
    packet_in.value = {{ 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 }};

    mavlink::common::msg::MEMORY_VECT packet1{};
    mavlink::common::msg::MEMORY_VECT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.address, packet2.address);
    EXPECT_EQ(packet1.ver, packet2.ver);
    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(common_interop, MEMORY_VECT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_memory_vect_t packet_c {
         17235, 139, 206, { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 }
    };

    mavlink::common::msg::MEMORY_VECT packet_in{};
    packet_in.address = 17235;
    packet_in.ver = 139;
    packet_in.type = 206;
    packet_in.value = {{ 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 }};

    mavlink::common::msg::MEMORY_VECT packet2{};

    mavlink_msg_memory_vect_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.address, packet2.address);
    EXPECT_EQ(packet_in.ver, packet2.ver);
    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, DEBUG_VECT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::DEBUG_VECT packet_in{};
    packet_in.name = to_char_array("UVWXYZABC");
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;

    mavlink::common::msg::DEBUG_VECT packet1{};
    mavlink::common::msg::DEBUG_VECT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.name, packet2.name);
    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
}

#ifdef TEST_INTEROP
TEST(common_interop, DEBUG_VECT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_debug_vect_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, "UVWXYZABC"
    };

    mavlink::common::msg::DEBUG_VECT packet_in{};
    packet_in.name = to_char_array("UVWXYZABC");
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;

    mavlink::common::msg::DEBUG_VECT packet2{};

    mavlink_msg_debug_vect_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.name, packet2.name);
    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, NAMED_VALUE_FLOAT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::NAMED_VALUE_FLOAT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.name = to_char_array("IJKLMNOPQ");
    packet_in.value = 45.0;

    mavlink::common::msg::NAMED_VALUE_FLOAT packet1{};
    mavlink::common::msg::NAMED_VALUE_FLOAT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.name, packet2.name);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(common_interop, NAMED_VALUE_FLOAT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_named_value_float_t packet_c {
         963497464, 45.0, "IJKLMNOPQ"
    };

    mavlink::common::msg::NAMED_VALUE_FLOAT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.name = to_char_array("IJKLMNOPQ");
    packet_in.value = 45.0;

    mavlink::common::msg::NAMED_VALUE_FLOAT packet2{};

    mavlink_msg_named_value_float_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.name, packet2.name);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, NAMED_VALUE_INT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::NAMED_VALUE_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.name = to_char_array("IJKLMNOPQ");
    packet_in.value = 963497672;

    mavlink::common::msg::NAMED_VALUE_INT packet1{};
    mavlink::common::msg::NAMED_VALUE_INT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.name, packet2.name);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(common_interop, NAMED_VALUE_INT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_named_value_int_t packet_c {
         963497464, 963497672, "IJKLMNOPQ"
    };

    mavlink::common::msg::NAMED_VALUE_INT packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.name = to_char_array("IJKLMNOPQ");
    packet_in.value = 963497672;

    mavlink::common::msg::NAMED_VALUE_INT packet2{};

    mavlink_msg_named_value_int_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.name, packet2.name);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, STATUSTEXT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::STATUSTEXT packet_in{};
    packet_in.severity = 5;
    packet_in.text = to_char_array("BCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX");

    mavlink::common::msg::STATUSTEXT packet1{};
    mavlink::common::msg::STATUSTEXT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.severity, packet2.severity);
    EXPECT_EQ(packet1.text, packet2.text);
}

#ifdef TEST_INTEROP
TEST(common_interop, STATUSTEXT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_statustext_t packet_c {
         5, "BCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX"
    };

    mavlink::common::msg::STATUSTEXT packet_in{};
    packet_in.severity = 5;
    packet_in.text = to_char_array("BCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX");

    mavlink::common::msg::STATUSTEXT packet2{};

    mavlink_msg_statustext_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.severity, packet2.severity);
    EXPECT_EQ(packet_in.text, packet2.text);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, DEBUG)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::DEBUG packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.ind = 29;
    packet_in.value = 45.0;

    mavlink::common::msg::DEBUG packet1{};
    mavlink::common::msg::DEBUG packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.ind, packet2.ind);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(common_interop, DEBUG)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_debug_t packet_c {
         963497464, 45.0, 29
    };

    mavlink::common::msg::DEBUG packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.ind = 29;
    packet_in.value = 45.0;

    mavlink::common::msg::DEBUG packet2{};

    mavlink_msg_debug_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.ind, packet2.ind);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, SETUP_SIGNING)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::SETUP_SIGNING packet_in{};
    packet_in.target_system = 29;
    packet_in.target_component = 96;
    packet_in.secret_key = {{ 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194 }};
    packet_in.initial_timestamp = 93372036854775807ULL;

    mavlink::common::msg::SETUP_SIGNING packet1{};
    mavlink::common::msg::SETUP_SIGNING packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.secret_key, packet2.secret_key);
    EXPECT_EQ(packet1.initial_timestamp, packet2.initial_timestamp);
}

#ifdef TEST_INTEROP
TEST(common_interop, SETUP_SIGNING)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_setup_signing_t packet_c {
         93372036854775807ULL, 29, 96, { 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194 }
    };

    mavlink::common::msg::SETUP_SIGNING packet_in{};
    packet_in.target_system = 29;
    packet_in.target_component = 96;
    packet_in.secret_key = {{ 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194 }};
    packet_in.initial_timestamp = 93372036854775807ULL;

    mavlink::common::msg::SETUP_SIGNING packet2{};

    mavlink_msg_setup_signing_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.secret_key, packet2.secret_key);
    EXPECT_EQ(packet_in.initial_timestamp, packet2.initial_timestamp);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, BUTTON_CHANGE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::BUTTON_CHANGE packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.last_change_ms = 963497672;
    packet_in.state = 29;

    mavlink::common::msg::BUTTON_CHANGE packet1{};
    mavlink::common::msg::BUTTON_CHANGE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.last_change_ms, packet2.last_change_ms);
    EXPECT_EQ(packet1.state, packet2.state);
}

#ifdef TEST_INTEROP
TEST(common_interop, BUTTON_CHANGE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_button_change_t packet_c {
         963497464, 963497672, 29
    };

    mavlink::common::msg::BUTTON_CHANGE packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.last_change_ms = 963497672;
    packet_in.state = 29;

    mavlink::common::msg::BUTTON_CHANGE packet2{};

    mavlink_msg_button_change_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.last_change_ms, packet2.last_change_ms);
    EXPECT_EQ(packet_in.state, packet2.state);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, PLAY_TUNE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::PLAY_TUNE packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.tune = to_char_array("CDEFGHIJKLMNOPQRSTUVWXYZABCDE");
    packet_in.tune2 = to_char_array("GHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVW");

    mavlink::common::msg::PLAY_TUNE packet1{};
    mavlink::common::msg::PLAY_TUNE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.tune, packet2.tune);
    EXPECT_EQ(packet1.tune2, packet2.tune2);
}

#ifdef TEST_INTEROP
TEST(common_interop, PLAY_TUNE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_play_tune_t packet_c {
         5, 72, "CDEFGHIJKLMNOPQRSTUVWXYZABCDE", "GHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVW"
    };

    mavlink::common::msg::PLAY_TUNE packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.tune = to_char_array("CDEFGHIJKLMNOPQRSTUVWXYZABCDE");
    packet_in.tune2 = to_char_array("GHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVW");

    mavlink::common::msg::PLAY_TUNE packet2{};

    mavlink_msg_play_tune_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.tune, packet2.tune);
    EXPECT_EQ(packet_in.tune2, packet2.tune2);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CAMERA_INFORMATION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CAMERA_INFORMATION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.vendor_name = {{ 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254 }};
    packet_in.model_name = {{ 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94 }};
    packet_in.firmware_version = 963497672;
    packet_in.focal_length = 73.0;
    packet_in.sensor_size_h = 101.0;
    packet_in.sensor_size_v = 129.0;
    packet_in.resolution_h = 18483;
    packet_in.resolution_v = 18587;
    packet_in.lens_id = 159;
    packet_in.flags = 963498504;
    packet_in.cam_definition_version = 18691;
    packet_in.cam_definition_uri = to_char_array("RSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");

    mavlink::common::msg::CAMERA_INFORMATION packet1{};
    mavlink::common::msg::CAMERA_INFORMATION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.vendor_name, packet2.vendor_name);
    EXPECT_EQ(packet1.model_name, packet2.model_name);
    EXPECT_EQ(packet1.firmware_version, packet2.firmware_version);
    EXPECT_EQ(packet1.focal_length, packet2.focal_length);
    EXPECT_EQ(packet1.sensor_size_h, packet2.sensor_size_h);
    EXPECT_EQ(packet1.sensor_size_v, packet2.sensor_size_v);
    EXPECT_EQ(packet1.resolution_h, packet2.resolution_h);
    EXPECT_EQ(packet1.resolution_v, packet2.resolution_v);
    EXPECT_EQ(packet1.lens_id, packet2.lens_id);
    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.cam_definition_version, packet2.cam_definition_version);
    EXPECT_EQ(packet1.cam_definition_uri, packet2.cam_definition_uri);
}

#ifdef TEST_INTEROP
TEST(common_interop, CAMERA_INFORMATION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_information_t packet_c {
         963497464, 963497672, 73.0, 101.0, 129.0, 963498504, 18483, 18587, 18691, { 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254 }, { 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94 }, 159, "RSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
    };

    mavlink::common::msg::CAMERA_INFORMATION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.vendor_name = {{ 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254 }};
    packet_in.model_name = {{ 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94 }};
    packet_in.firmware_version = 963497672;
    packet_in.focal_length = 73.0;
    packet_in.sensor_size_h = 101.0;
    packet_in.sensor_size_v = 129.0;
    packet_in.resolution_h = 18483;
    packet_in.resolution_v = 18587;
    packet_in.lens_id = 159;
    packet_in.flags = 963498504;
    packet_in.cam_definition_version = 18691;
    packet_in.cam_definition_uri = to_char_array("RSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");

    mavlink::common::msg::CAMERA_INFORMATION packet2{};

    mavlink_msg_camera_information_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.vendor_name, packet2.vendor_name);
    EXPECT_EQ(packet_in.model_name, packet2.model_name);
    EXPECT_EQ(packet_in.firmware_version, packet2.firmware_version);
    EXPECT_EQ(packet_in.focal_length, packet2.focal_length);
    EXPECT_EQ(packet_in.sensor_size_h, packet2.sensor_size_h);
    EXPECT_EQ(packet_in.sensor_size_v, packet2.sensor_size_v);
    EXPECT_EQ(packet_in.resolution_h, packet2.resolution_h);
    EXPECT_EQ(packet_in.resolution_v, packet2.resolution_v);
    EXPECT_EQ(packet_in.lens_id, packet2.lens_id);
    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.cam_definition_version, packet2.cam_definition_version);
    EXPECT_EQ(packet_in.cam_definition_uri, packet2.cam_definition_uri);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CAMERA_SETTINGS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CAMERA_SETTINGS packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.mode_id = 17;

    mavlink::common::msg::CAMERA_SETTINGS packet1{};
    mavlink::common::msg::CAMERA_SETTINGS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.mode_id, packet2.mode_id);
}

#ifdef TEST_INTEROP
TEST(common_interop, CAMERA_SETTINGS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_settings_t packet_c {
         963497464, 17
    };

    mavlink::common::msg::CAMERA_SETTINGS packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.mode_id = 17;

    mavlink::common::msg::CAMERA_SETTINGS packet2{};

    mavlink_msg_camera_settings_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.mode_id, packet2.mode_id);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, STORAGE_INFORMATION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::STORAGE_INFORMATION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.storage_id = 77;
    packet_in.storage_count = 144;
    packet_in.status = 211;
    packet_in.total_capacity = 45.0;
    packet_in.used_capacity = 73.0;
    packet_in.available_capacity = 101.0;
    packet_in.read_speed = 129.0;
    packet_in.write_speed = 157.0;

    mavlink::common::msg::STORAGE_INFORMATION packet1{};
    mavlink::common::msg::STORAGE_INFORMATION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.storage_id, packet2.storage_id);
    EXPECT_EQ(packet1.storage_count, packet2.storage_count);
    EXPECT_EQ(packet1.status, packet2.status);
    EXPECT_EQ(packet1.total_capacity, packet2.total_capacity);
    EXPECT_EQ(packet1.used_capacity, packet2.used_capacity);
    EXPECT_EQ(packet1.available_capacity, packet2.available_capacity);
    EXPECT_EQ(packet1.read_speed, packet2.read_speed);
    EXPECT_EQ(packet1.write_speed, packet2.write_speed);
}

#ifdef TEST_INTEROP
TEST(common_interop, STORAGE_INFORMATION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_storage_information_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0, 157.0, 77, 144, 211
    };

    mavlink::common::msg::STORAGE_INFORMATION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.storage_id = 77;
    packet_in.storage_count = 144;
    packet_in.status = 211;
    packet_in.total_capacity = 45.0;
    packet_in.used_capacity = 73.0;
    packet_in.available_capacity = 101.0;
    packet_in.read_speed = 129.0;
    packet_in.write_speed = 157.0;

    mavlink::common::msg::STORAGE_INFORMATION packet2{};

    mavlink_msg_storage_information_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.storage_id, packet2.storage_id);
    EXPECT_EQ(packet_in.storage_count, packet2.storage_count);
    EXPECT_EQ(packet_in.status, packet2.status);
    EXPECT_EQ(packet_in.total_capacity, packet2.total_capacity);
    EXPECT_EQ(packet_in.used_capacity, packet2.used_capacity);
    EXPECT_EQ(packet_in.available_capacity, packet2.available_capacity);
    EXPECT_EQ(packet_in.read_speed, packet2.read_speed);
    EXPECT_EQ(packet_in.write_speed, packet2.write_speed);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CAMERA_CAPTURE_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CAMERA_CAPTURE_STATUS packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.image_status = 53;
    packet_in.video_status = 120;
    packet_in.image_interval = 45.0;
    packet_in.recording_time_ms = 963497880;
    packet_in.available_capacity = 101.0;

    mavlink::common::msg::CAMERA_CAPTURE_STATUS packet1{};
    mavlink::common::msg::CAMERA_CAPTURE_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.image_status, packet2.image_status);
    EXPECT_EQ(packet1.video_status, packet2.video_status);
    EXPECT_EQ(packet1.image_interval, packet2.image_interval);
    EXPECT_EQ(packet1.recording_time_ms, packet2.recording_time_ms);
    EXPECT_EQ(packet1.available_capacity, packet2.available_capacity);
}

#ifdef TEST_INTEROP
TEST(common_interop, CAMERA_CAPTURE_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_capture_status_t packet_c {
         963497464, 45.0, 963497880, 101.0, 53, 120
    };

    mavlink::common::msg::CAMERA_CAPTURE_STATUS packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.image_status = 53;
    packet_in.video_status = 120;
    packet_in.image_interval = 45.0;
    packet_in.recording_time_ms = 963497880;
    packet_in.available_capacity = 101.0;

    mavlink::common::msg::CAMERA_CAPTURE_STATUS packet2{};

    mavlink_msg_camera_capture_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.image_status, packet2.image_status);
    EXPECT_EQ(packet_in.video_status, packet2.video_status);
    EXPECT_EQ(packet_in.image_interval, packet2.image_interval);
    EXPECT_EQ(packet_in.recording_time_ms, packet2.recording_time_ms);
    EXPECT_EQ(packet_in.available_capacity, packet2.available_capacity);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, CAMERA_IMAGE_CAPTURED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::CAMERA_IMAGE_CAPTURED packet_in{};
    packet_in.time_boot_ms = 963497880;
    packet_in.time_utc = 93372036854775807ULL;
    packet_in.camera_id = 149;
    packet_in.lat = 963498088;
    packet_in.lon = 963498296;
    packet_in.alt = 963498504;
    packet_in.relative_alt = 963498712;
    packet_in.q = {{ 213.0, 214.0, 215.0, 216.0 }};
    packet_in.image_index = 963499752;
    packet_in.capture_result = -40;
    packet_in.file_url = to_char_array("YZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST");

    mavlink::common::msg::CAMERA_IMAGE_CAPTURED packet1{};
    mavlink::common::msg::CAMERA_IMAGE_CAPTURED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.time_utc, packet2.time_utc);
    EXPECT_EQ(packet1.camera_id, packet2.camera_id);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lon, packet2.lon);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.relative_alt, packet2.relative_alt);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.image_index, packet2.image_index);
    EXPECT_EQ(packet1.capture_result, packet2.capture_result);
    EXPECT_EQ(packet1.file_url, packet2.file_url);
}

#ifdef TEST_INTEROP
TEST(common_interop, CAMERA_IMAGE_CAPTURED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_image_captured_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 963498296, 963498504, 963498712, { 213.0, 214.0, 215.0, 216.0 }, 963499752, 149, -40, "YZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST"
    };

    mavlink::common::msg::CAMERA_IMAGE_CAPTURED packet_in{};
    packet_in.time_boot_ms = 963497880;
    packet_in.time_utc = 93372036854775807ULL;
    packet_in.camera_id = 149;
    packet_in.lat = 963498088;
    packet_in.lon = 963498296;
    packet_in.alt = 963498504;
    packet_in.relative_alt = 963498712;
    packet_in.q = {{ 213.0, 214.0, 215.0, 216.0 }};
    packet_in.image_index = 963499752;
    packet_in.capture_result = -40;
    packet_in.file_url = to_char_array("YZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST");

    mavlink::common::msg::CAMERA_IMAGE_CAPTURED packet2{};

    mavlink_msg_camera_image_captured_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.time_utc, packet2.time_utc);
    EXPECT_EQ(packet_in.camera_id, packet2.camera_id);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lon, packet2.lon);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.relative_alt, packet2.relative_alt);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.image_index, packet2.image_index);
    EXPECT_EQ(packet_in.capture_result, packet2.capture_result);
    EXPECT_EQ(packet_in.file_url, packet2.file_url);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, FLIGHT_INFORMATION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::FLIGHT_INFORMATION packet_in{};
    packet_in.time_boot_ms = 963498712;
    packet_in.arming_time_utc = 93372036854775807ULL;
    packet_in.takeoff_time_utc = 93372036854776311ULL;
    packet_in.flight_uuid = 93372036854776815ULL;

    mavlink::common::msg::FLIGHT_INFORMATION packet1{};
    mavlink::common::msg::FLIGHT_INFORMATION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.arming_time_utc, packet2.arming_time_utc);
    EXPECT_EQ(packet1.takeoff_time_utc, packet2.takeoff_time_utc);
    EXPECT_EQ(packet1.flight_uuid, packet2.flight_uuid);
}

#ifdef TEST_INTEROP
TEST(common_interop, FLIGHT_INFORMATION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_flight_information_t packet_c {
         93372036854775807ULL, 93372036854776311ULL, 93372036854776815ULL, 963498712
    };

    mavlink::common::msg::FLIGHT_INFORMATION packet_in{};
    packet_in.time_boot_ms = 963498712;
    packet_in.arming_time_utc = 93372036854775807ULL;
    packet_in.takeoff_time_utc = 93372036854776311ULL;
    packet_in.flight_uuid = 93372036854776815ULL;

    mavlink::common::msg::FLIGHT_INFORMATION packet2{};

    mavlink_msg_flight_information_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.arming_time_utc, packet2.arming_time_utc);
    EXPECT_EQ(packet_in.takeoff_time_utc, packet2.takeoff_time_utc);
    EXPECT_EQ(packet_in.flight_uuid, packet2.flight_uuid);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, MOUNT_ORIENTATION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::MOUNT_ORIENTATION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.roll = 45.0;
    packet_in.pitch = 73.0;
    packet_in.yaw = 101.0;
    packet_in.yaw_absolute = 129.0;

    mavlink::common::msg::MOUNT_ORIENTATION packet1{};
    mavlink::common::msg::MOUNT_ORIENTATION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.yaw_absolute, packet2.yaw_absolute);
}

#ifdef TEST_INTEROP
TEST(common_interop, MOUNT_ORIENTATION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mount_orientation_t packet_c {
         963497464, 45.0, 73.0, 101.0, 129.0
    };

    mavlink::common::msg::MOUNT_ORIENTATION packet_in{};
    packet_in.time_boot_ms = 963497464;
    packet_in.roll = 45.0;
    packet_in.pitch = 73.0;
    packet_in.yaw = 101.0;
    packet_in.yaw_absolute = 129.0;

    mavlink::common::msg::MOUNT_ORIENTATION packet2{};

    mavlink_msg_mount_orientation_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.yaw_absolute, packet2.yaw_absolute);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOGGING_DATA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOGGING_DATA packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.sequence = 17235;
    packet_in.length = 17;
    packet_in.first_message_offset = 84;
    packet_in.data = {{ 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143 }};

    mavlink::common::msg::LOGGING_DATA packet1{};
    mavlink::common::msg::LOGGING_DATA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.sequence, packet2.sequence);
    EXPECT_EQ(packet1.length, packet2.length);
    EXPECT_EQ(packet1.first_message_offset, packet2.first_message_offset);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOGGING_DATA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_logging_data_t packet_c {
         17235, 139, 206, 17, 84, { 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143 }
    };

    mavlink::common::msg::LOGGING_DATA packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.sequence = 17235;
    packet_in.length = 17;
    packet_in.first_message_offset = 84;
    packet_in.data = {{ 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143 }};

    mavlink::common::msg::LOGGING_DATA packet2{};

    mavlink_msg_logging_data_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.sequence, packet2.sequence);
    EXPECT_EQ(packet_in.length, packet2.length);
    EXPECT_EQ(packet_in.first_message_offset, packet2.first_message_offset);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOGGING_DATA_ACKED)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOGGING_DATA_ACKED packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.sequence = 17235;
    packet_in.length = 17;
    packet_in.first_message_offset = 84;
    packet_in.data = {{ 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143 }};

    mavlink::common::msg::LOGGING_DATA_ACKED packet1{};
    mavlink::common::msg::LOGGING_DATA_ACKED packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.sequence, packet2.sequence);
    EXPECT_EQ(packet1.length, packet2.length);
    EXPECT_EQ(packet1.first_message_offset, packet2.first_message_offset);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOGGING_DATA_ACKED)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_logging_data_acked_t packet_c {
         17235, 139, 206, 17, 84, { 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143 }
    };

    mavlink::common::msg::LOGGING_DATA_ACKED packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.sequence = 17235;
    packet_in.length = 17;
    packet_in.first_message_offset = 84;
    packet_in.data = {{ 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143 }};

    mavlink::common::msg::LOGGING_DATA_ACKED packet2{};

    mavlink_msg_logging_data_acked_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.sequence, packet2.sequence);
    EXPECT_EQ(packet_in.length, packet2.length);
    EXPECT_EQ(packet_in.first_message_offset, packet2.first_message_offset);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, LOGGING_ACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::LOGGING_ACK packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.sequence = 17235;

    mavlink::common::msg::LOGGING_ACK packet1{};
    mavlink::common::msg::LOGGING_ACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.sequence, packet2.sequence);
}

#ifdef TEST_INTEROP
TEST(common_interop, LOGGING_ACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_logging_ack_t packet_c {
         17235, 139, 206
    };

    mavlink::common::msg::LOGGING_ACK packet_in{};
    packet_in.target_system = 139;
    packet_in.target_component = 206;
    packet_in.sequence = 17235;

    mavlink::common::msg::LOGGING_ACK packet2{};

    mavlink_msg_logging_ack_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.sequence, packet2.sequence);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, WIFI_CONFIG_AP)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::WIFI_CONFIG_AP packet_in{};
    packet_in.ssid = to_char_array("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE");
    packet_in.password = to_char_array("GHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ");

    mavlink::common::msg::WIFI_CONFIG_AP packet1{};
    mavlink::common::msg::WIFI_CONFIG_AP packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.ssid, packet2.ssid);
    EXPECT_EQ(packet1.password, packet2.password);
}

#ifdef TEST_INTEROP
TEST(common_interop, WIFI_CONFIG_AP)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_wifi_config_ap_t packet_c {
         "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE", "GHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ"
    };

    mavlink::common::msg::WIFI_CONFIG_AP packet_in{};
    packet_in.ssid = to_char_array("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE");
    packet_in.password = to_char_array("GHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ");

    mavlink::common::msg::WIFI_CONFIG_AP packet2{};

    mavlink_msg_wifi_config_ap_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.ssid, packet2.ssid);
    EXPECT_EQ(packet_in.password, packet2.password);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, UAVCAN_NODE_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::UAVCAN_NODE_STATUS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.uptime_sec = 963497880;
    packet_in.health = 175;
    packet_in.mode = 242;
    packet_in.sub_mode = 53;
    packet_in.vendor_specific_status_code = 17859;

    mavlink::common::msg::UAVCAN_NODE_STATUS packet1{};
    mavlink::common::msg::UAVCAN_NODE_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.uptime_sec, packet2.uptime_sec);
    EXPECT_EQ(packet1.health, packet2.health);
    EXPECT_EQ(packet1.mode, packet2.mode);
    EXPECT_EQ(packet1.sub_mode, packet2.sub_mode);
    EXPECT_EQ(packet1.vendor_specific_status_code, packet2.vendor_specific_status_code);
}

#ifdef TEST_INTEROP
TEST(common_interop, UAVCAN_NODE_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_uavcan_node_status_t packet_c {
         93372036854775807ULL, 963497880, 17859, 175, 242, 53
    };

    mavlink::common::msg::UAVCAN_NODE_STATUS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.uptime_sec = 963497880;
    packet_in.health = 175;
    packet_in.mode = 242;
    packet_in.sub_mode = 53;
    packet_in.vendor_specific_status_code = 17859;

    mavlink::common::msg::UAVCAN_NODE_STATUS packet2{};

    mavlink_msg_uavcan_node_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.uptime_sec, packet2.uptime_sec);
    EXPECT_EQ(packet_in.health, packet2.health);
    EXPECT_EQ(packet_in.mode, packet2.mode);
    EXPECT_EQ(packet_in.sub_mode, packet2.sub_mode);
    EXPECT_EQ(packet_in.vendor_specific_status_code, packet2.vendor_specific_status_code);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, UAVCAN_NODE_INFO)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::UAVCAN_NODE_INFO packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.uptime_sec = 963497880;
    packet_in.name = to_char_array("QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ");
    packet_in.hw_version_major = 37;
    packet_in.hw_version_minor = 104;
    packet_in.hw_unique_id = {{ 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186 }};
    packet_in.sw_version_major = 219;
    packet_in.sw_version_minor = 30;
    packet_in.sw_vcs_commit = 963498088;

    mavlink::common::msg::UAVCAN_NODE_INFO packet1{};
    mavlink::common::msg::UAVCAN_NODE_INFO packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.uptime_sec, packet2.uptime_sec);
    EXPECT_EQ(packet1.name, packet2.name);
    EXPECT_EQ(packet1.hw_version_major, packet2.hw_version_major);
    EXPECT_EQ(packet1.hw_version_minor, packet2.hw_version_minor);
    EXPECT_EQ(packet1.hw_unique_id, packet2.hw_unique_id);
    EXPECT_EQ(packet1.sw_version_major, packet2.sw_version_major);
    EXPECT_EQ(packet1.sw_version_minor, packet2.sw_version_minor);
    EXPECT_EQ(packet1.sw_vcs_commit, packet2.sw_vcs_commit);
}

#ifdef TEST_INTEROP
TEST(common_interop, UAVCAN_NODE_INFO)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_uavcan_node_info_t packet_c {
         93372036854775807ULL, 963497880, 963498088, "QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ", 37, 104, { 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186 }, 219, 30
    };

    mavlink::common::msg::UAVCAN_NODE_INFO packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.uptime_sec = 963497880;
    packet_in.name = to_char_array("QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ");
    packet_in.hw_version_major = 37;
    packet_in.hw_version_minor = 104;
    packet_in.hw_unique_id = {{ 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186 }};
    packet_in.sw_version_major = 219;
    packet_in.sw_version_minor = 30;
    packet_in.sw_vcs_commit = 963498088;

    mavlink::common::msg::UAVCAN_NODE_INFO packet2{};

    mavlink_msg_uavcan_node_info_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.uptime_sec, packet2.uptime_sec);
    EXPECT_EQ(packet_in.name, packet2.name);
    EXPECT_EQ(packet_in.hw_version_major, packet2.hw_version_major);
    EXPECT_EQ(packet_in.hw_version_minor, packet2.hw_version_minor);
    EXPECT_EQ(packet_in.hw_unique_id, packet2.hw_unique_id);
    EXPECT_EQ(packet_in.sw_version_major, packet2.sw_version_major);
    EXPECT_EQ(packet_in.sw_version_minor, packet2.sw_version_minor);
    EXPECT_EQ(packet_in.sw_vcs_commit, packet2.sw_vcs_commit);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, OBSTACLE_DISTANCE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::OBSTACLE_DISTANCE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_type = 217;
    packet_in.distances = {{ 17651, 17652, 17653, 17654, 17655, 17656, 17657, 17658, 17659, 17660, 17661, 17662, 17663, 17664, 17665, 17666, 17667, 17668, 17669, 17670, 17671, 17672, 17673, 17674, 17675, 17676, 17677, 17678, 17679, 17680, 17681, 17682, 17683, 17684, 17685, 17686, 17687, 17688, 17689, 17690, 17691, 17692, 17693, 17694, 17695, 17696, 17697, 17698, 17699, 17700, 17701, 17702, 17703, 17704, 17705, 17706, 17707, 17708, 17709, 17710, 17711, 17712, 17713, 17714, 17715, 17716, 17717, 17718, 17719, 17720, 17721, 17722 }};
    packet_in.increment = 28;
    packet_in.min_distance = 25139;
    packet_in.max_distance = 25243;

    mavlink::common::msg::OBSTACLE_DISTANCE packet1{};
    mavlink::common::msg::OBSTACLE_DISTANCE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.sensor_type, packet2.sensor_type);
    EXPECT_EQ(packet1.distances, packet2.distances);
    EXPECT_EQ(packet1.increment, packet2.increment);
    EXPECT_EQ(packet1.min_distance, packet2.min_distance);
    EXPECT_EQ(packet1.max_distance, packet2.max_distance);
}

#ifdef TEST_INTEROP
TEST(common_interop, OBSTACLE_DISTANCE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_obstacle_distance_t packet_c {
         93372036854775807ULL, { 17651, 17652, 17653, 17654, 17655, 17656, 17657, 17658, 17659, 17660, 17661, 17662, 17663, 17664, 17665, 17666, 17667, 17668, 17669, 17670, 17671, 17672, 17673, 17674, 17675, 17676, 17677, 17678, 17679, 17680, 17681, 17682, 17683, 17684, 17685, 17686, 17687, 17688, 17689, 17690, 17691, 17692, 17693, 17694, 17695, 17696, 17697, 17698, 17699, 17700, 17701, 17702, 17703, 17704, 17705, 17706, 17707, 17708, 17709, 17710, 17711, 17712, 17713, 17714, 17715, 17716, 17717, 17718, 17719, 17720, 17721, 17722 }, 25139, 25243, 217, 28
    };

    mavlink::common::msg::OBSTACLE_DISTANCE packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.sensor_type = 217;
    packet_in.distances = {{ 17651, 17652, 17653, 17654, 17655, 17656, 17657, 17658, 17659, 17660, 17661, 17662, 17663, 17664, 17665, 17666, 17667, 17668, 17669, 17670, 17671, 17672, 17673, 17674, 17675, 17676, 17677, 17678, 17679, 17680, 17681, 17682, 17683, 17684, 17685, 17686, 17687, 17688, 17689, 17690, 17691, 17692, 17693, 17694, 17695, 17696, 17697, 17698, 17699, 17700, 17701, 17702, 17703, 17704, 17705, 17706, 17707, 17708, 17709, 17710, 17711, 17712, 17713, 17714, 17715, 17716, 17717, 17718, 17719, 17720, 17721, 17722 }};
    packet_in.increment = 28;
    packet_in.min_distance = 25139;
    packet_in.max_distance = 25243;

    mavlink::common::msg::OBSTACLE_DISTANCE packet2{};

    mavlink_msg_obstacle_distance_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.sensor_type, packet2.sensor_type);
    EXPECT_EQ(packet_in.distances, packet2.distances);
    EXPECT_EQ(packet_in.increment, packet2.increment);
    EXPECT_EQ(packet_in.min_distance, packet2.min_distance);
    EXPECT_EQ(packet_in.max_distance, packet2.max_distance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(common, ODOMETRY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::common::msg::ODOMETRY packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.frame_id = 177;
    packet_in.child_frame_id = 244;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.q = {{ 157.0, 158.0, 159.0, 160.0 }};
    packet_in.vx = 269.0;
    packet_in.vy = 297.0;
    packet_in.vz = 325.0;
    packet_in.rollspeed = 353.0;
    packet_in.pitchspeed = 381.0;
    packet_in.yawspeed = 409.0;
    packet_in.pose_covariance = {{ 437.0, 438.0, 439.0, 440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0, 448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0, 456.0, 457.0 }};
    packet_in.twist_covariance = {{ 1025.0, 1026.0, 1027.0, 1028.0, 1029.0, 1030.0, 1031.0, 1032.0, 1033.0, 1034.0, 1035.0, 1036.0, 1037.0, 1038.0, 1039.0, 1040.0, 1041.0, 1042.0, 1043.0, 1044.0, 1045.0 }};

    mavlink::common::msg::ODOMETRY packet1{};
    mavlink::common::msg::ODOMETRY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.frame_id, packet2.frame_id);
    EXPECT_EQ(packet1.child_frame_id, packet2.child_frame_id);
    EXPECT_EQ(packet1.x, packet2.x);
    EXPECT_EQ(packet1.y, packet2.y);
    EXPECT_EQ(packet1.z, packet2.z);
    EXPECT_EQ(packet1.q, packet2.q);
    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet1.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet1.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet1.pose_covariance, packet2.pose_covariance);
    EXPECT_EQ(packet1.twist_covariance, packet2.twist_covariance);
}

#ifdef TEST_INTEROP
TEST(common_interop, ODOMETRY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_odometry_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, { 157.0, 158.0, 159.0, 160.0 }, 269.0, 297.0, 325.0, 353.0, 381.0, 409.0, { 437.0, 438.0, 439.0, 440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0, 448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0, 456.0, 457.0 }, { 1025.0, 1026.0, 1027.0, 1028.0, 1029.0, 1030.0, 1031.0, 1032.0, 1033.0, 1034.0, 1035.0, 1036.0, 1037.0, 1038.0, 1039.0, 1040.0, 1041.0, 1042.0, 1043.0, 1044.0, 1045.0 }, 177, 244
    };

    mavlink::common::msg::ODOMETRY packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.frame_id = 177;
    packet_in.child_frame_id = 244;
    packet_in.x = 73.0;
    packet_in.y = 101.0;
    packet_in.z = 129.0;
    packet_in.q = {{ 157.0, 158.0, 159.0, 160.0 }};
    packet_in.vx = 269.0;
    packet_in.vy = 297.0;
    packet_in.vz = 325.0;
    packet_in.rollspeed = 353.0;
    packet_in.pitchspeed = 381.0;
    packet_in.yawspeed = 409.0;
    packet_in.pose_covariance = {{ 437.0, 438.0, 439.0, 440.0, 441.0, 442.0, 443.0, 444.0, 445.0, 446.0, 447.0, 448.0, 449.0, 450.0, 451.0, 452.0, 453.0, 454.0, 455.0, 456.0, 457.0 }};
    packet_in.twist_covariance = {{ 1025.0, 1026.0, 1027.0, 1028.0, 1029.0, 1030.0, 1031.0, 1032.0, 1033.0, 1034.0, 1035.0, 1036.0, 1037.0, 1038.0, 1039.0, 1040.0, 1041.0, 1042.0, 1043.0, 1044.0, 1045.0 }};

    mavlink::common::msg::ODOMETRY packet2{};

    mavlink_msg_odometry_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.frame_id, packet2.frame_id);
    EXPECT_EQ(packet_in.child_frame_id, packet2.child_frame_id);
    EXPECT_EQ(packet_in.x, packet2.x);
    EXPECT_EQ(packet_in.y, packet2.y);
    EXPECT_EQ(packet_in.z, packet2.z);
    EXPECT_EQ(packet_in.q, packet2.q);
    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.rollspeed, packet2.rollspeed);
    EXPECT_EQ(packet_in.pitchspeed, packet2.pitchspeed);
    EXPECT_EQ(packet_in.yawspeed, packet2.yawspeed);
    EXPECT_EQ(packet_in.pose_covariance, packet2.pose_covariance);
    EXPECT_EQ(packet_in.twist_covariance, packet2.twist_covariance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif
