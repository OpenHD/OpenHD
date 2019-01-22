/** @file
 *	@brief MAVLink comm testsuite protocol generated from ardupilotmega.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <gtest/gtest.h>
#include "ardupilotmega.hpp"

#ifdef TEST_INTEROP
using namespace mavlink;
#undef MAVLINK_HELPER
#include "mavlink.h"
#endif


TEST(ardupilotmega, SENSOR_OFFSETS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::SENSOR_OFFSETS packet_in{};
    packet_in.mag_ofs_x = 19107;
    packet_in.mag_ofs_y = 19211;
    packet_in.mag_ofs_z = 19315;
    packet_in.mag_declination = 17.0;
    packet_in.raw_press = 963497672;
    packet_in.raw_temp = 963497880;
    packet_in.gyro_cal_x = 101.0;
    packet_in.gyro_cal_y = 129.0;
    packet_in.gyro_cal_z = 157.0;
    packet_in.accel_cal_x = 185.0;
    packet_in.accel_cal_y = 213.0;
    packet_in.accel_cal_z = 241.0;

    mavlink::ardupilotmega::msg::SENSOR_OFFSETS packet1{};
    mavlink::ardupilotmega::msg::SENSOR_OFFSETS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.mag_ofs_x, packet2.mag_ofs_x);
    EXPECT_EQ(packet1.mag_ofs_y, packet2.mag_ofs_y);
    EXPECT_EQ(packet1.mag_ofs_z, packet2.mag_ofs_z);
    EXPECT_EQ(packet1.mag_declination, packet2.mag_declination);
    EXPECT_EQ(packet1.raw_press, packet2.raw_press);
    EXPECT_EQ(packet1.raw_temp, packet2.raw_temp);
    EXPECT_EQ(packet1.gyro_cal_x, packet2.gyro_cal_x);
    EXPECT_EQ(packet1.gyro_cal_y, packet2.gyro_cal_y);
    EXPECT_EQ(packet1.gyro_cal_z, packet2.gyro_cal_z);
    EXPECT_EQ(packet1.accel_cal_x, packet2.accel_cal_x);
    EXPECT_EQ(packet1.accel_cal_y, packet2.accel_cal_y);
    EXPECT_EQ(packet1.accel_cal_z, packet2.accel_cal_z);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, SENSOR_OFFSETS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_sensor_offsets_t packet_c {
         17.0, 963497672, 963497880, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 19107, 19211, 19315
    };

    mavlink::ardupilotmega::msg::SENSOR_OFFSETS packet_in{};
    packet_in.mag_ofs_x = 19107;
    packet_in.mag_ofs_y = 19211;
    packet_in.mag_ofs_z = 19315;
    packet_in.mag_declination = 17.0;
    packet_in.raw_press = 963497672;
    packet_in.raw_temp = 963497880;
    packet_in.gyro_cal_x = 101.0;
    packet_in.gyro_cal_y = 129.0;
    packet_in.gyro_cal_z = 157.0;
    packet_in.accel_cal_x = 185.0;
    packet_in.accel_cal_y = 213.0;
    packet_in.accel_cal_z = 241.0;

    mavlink::ardupilotmega::msg::SENSOR_OFFSETS packet2{};

    mavlink_msg_sensor_offsets_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.mag_ofs_x, packet2.mag_ofs_x);
    EXPECT_EQ(packet_in.mag_ofs_y, packet2.mag_ofs_y);
    EXPECT_EQ(packet_in.mag_ofs_z, packet2.mag_ofs_z);
    EXPECT_EQ(packet_in.mag_declination, packet2.mag_declination);
    EXPECT_EQ(packet_in.raw_press, packet2.raw_press);
    EXPECT_EQ(packet_in.raw_temp, packet2.raw_temp);
    EXPECT_EQ(packet_in.gyro_cal_x, packet2.gyro_cal_x);
    EXPECT_EQ(packet_in.gyro_cal_y, packet2.gyro_cal_y);
    EXPECT_EQ(packet_in.gyro_cal_z, packet2.gyro_cal_z);
    EXPECT_EQ(packet_in.accel_cal_x, packet2.accel_cal_x);
    EXPECT_EQ(packet_in.accel_cal_y, packet2.accel_cal_y);
    EXPECT_EQ(packet_in.accel_cal_z, packet2.accel_cal_z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, SET_MAG_OFFSETS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::SET_MAG_OFFSETS packet_in{};
    packet_in.target_system = 151;
    packet_in.target_component = 218;
    packet_in.mag_ofs_x = 17235;
    packet_in.mag_ofs_y = 17339;
    packet_in.mag_ofs_z = 17443;

    mavlink::ardupilotmega::msg::SET_MAG_OFFSETS packet1{};
    mavlink::ardupilotmega::msg::SET_MAG_OFFSETS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.mag_ofs_x, packet2.mag_ofs_x);
    EXPECT_EQ(packet1.mag_ofs_y, packet2.mag_ofs_y);
    EXPECT_EQ(packet1.mag_ofs_z, packet2.mag_ofs_z);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, SET_MAG_OFFSETS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_set_mag_offsets_t packet_c {
         17235, 17339, 17443, 151, 218
    };

    mavlink::ardupilotmega::msg::SET_MAG_OFFSETS packet_in{};
    packet_in.target_system = 151;
    packet_in.target_component = 218;
    packet_in.mag_ofs_x = 17235;
    packet_in.mag_ofs_y = 17339;
    packet_in.mag_ofs_z = 17443;

    mavlink::ardupilotmega::msg::SET_MAG_OFFSETS packet2{};

    mavlink_msg_set_mag_offsets_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.mag_ofs_x, packet2.mag_ofs_x);
    EXPECT_EQ(packet_in.mag_ofs_y, packet2.mag_ofs_y);
    EXPECT_EQ(packet_in.mag_ofs_z, packet2.mag_ofs_z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, MEMINFO)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::MEMINFO packet_in{};
    packet_in.brkval = 17235;
    packet_in.freemem = 17339;
    packet_in.freemem32 = 963497672;

    mavlink::ardupilotmega::msg::MEMINFO packet1{};
    mavlink::ardupilotmega::msg::MEMINFO packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.brkval, packet2.brkval);
    EXPECT_EQ(packet1.freemem, packet2.freemem);
    EXPECT_EQ(packet1.freemem32, packet2.freemem32);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, MEMINFO)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_meminfo_t packet_c {
         17235, 17339, 963497672
    };

    mavlink::ardupilotmega::msg::MEMINFO packet_in{};
    packet_in.brkval = 17235;
    packet_in.freemem = 17339;
    packet_in.freemem32 = 963497672;

    mavlink::ardupilotmega::msg::MEMINFO packet2{};

    mavlink_msg_meminfo_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.brkval, packet2.brkval);
    EXPECT_EQ(packet_in.freemem, packet2.freemem);
    EXPECT_EQ(packet_in.freemem32, packet2.freemem32);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AP_ADC)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AP_ADC packet_in{};
    packet_in.adc1 = 17235;
    packet_in.adc2 = 17339;
    packet_in.adc3 = 17443;
    packet_in.adc4 = 17547;
    packet_in.adc5 = 17651;
    packet_in.adc6 = 17755;

    mavlink::ardupilotmega::msg::AP_ADC packet1{};
    mavlink::ardupilotmega::msg::AP_ADC packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.adc1, packet2.adc1);
    EXPECT_EQ(packet1.adc2, packet2.adc2);
    EXPECT_EQ(packet1.adc3, packet2.adc3);
    EXPECT_EQ(packet1.adc4, packet2.adc4);
    EXPECT_EQ(packet1.adc5, packet2.adc5);
    EXPECT_EQ(packet1.adc6, packet2.adc6);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AP_ADC)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_ap_adc_t packet_c {
         17235, 17339, 17443, 17547, 17651, 17755
    };

    mavlink::ardupilotmega::msg::AP_ADC packet_in{};
    packet_in.adc1 = 17235;
    packet_in.adc2 = 17339;
    packet_in.adc3 = 17443;
    packet_in.adc4 = 17547;
    packet_in.adc5 = 17651;
    packet_in.adc6 = 17755;

    mavlink::ardupilotmega::msg::AP_ADC packet2{};

    mavlink_msg_ap_adc_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.adc1, packet2.adc1);
    EXPECT_EQ(packet_in.adc2, packet2.adc2);
    EXPECT_EQ(packet_in.adc3, packet2.adc3);
    EXPECT_EQ(packet_in.adc4, packet2.adc4);
    EXPECT_EQ(packet_in.adc5, packet2.adc5);
    EXPECT_EQ(packet_in.adc6, packet2.adc6);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DIGICAM_CONFIGURE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DIGICAM_CONFIGURE packet_in{};
    packet_in.target_system = 151;
    packet_in.target_component = 218;
    packet_in.mode = 29;
    packet_in.shutter_speed = 17443;
    packet_in.aperture = 96;
    packet_in.iso = 163;
    packet_in.exposure_type = 230;
    packet_in.command_id = 41;
    packet_in.engine_cut_off = 108;
    packet_in.extra_param = 175;
    packet_in.extra_value = 17.0;

    mavlink::ardupilotmega::msg::DIGICAM_CONFIGURE packet1{};
    mavlink::ardupilotmega::msg::DIGICAM_CONFIGURE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.mode, packet2.mode);
    EXPECT_EQ(packet1.shutter_speed, packet2.shutter_speed);
    EXPECT_EQ(packet1.aperture, packet2.aperture);
    EXPECT_EQ(packet1.iso, packet2.iso);
    EXPECT_EQ(packet1.exposure_type, packet2.exposure_type);
    EXPECT_EQ(packet1.command_id, packet2.command_id);
    EXPECT_EQ(packet1.engine_cut_off, packet2.engine_cut_off);
    EXPECT_EQ(packet1.extra_param, packet2.extra_param);
    EXPECT_EQ(packet1.extra_value, packet2.extra_value);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DIGICAM_CONFIGURE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_digicam_configure_t packet_c {
         17.0, 17443, 151, 218, 29, 96, 163, 230, 41, 108, 175
    };

    mavlink::ardupilotmega::msg::DIGICAM_CONFIGURE packet_in{};
    packet_in.target_system = 151;
    packet_in.target_component = 218;
    packet_in.mode = 29;
    packet_in.shutter_speed = 17443;
    packet_in.aperture = 96;
    packet_in.iso = 163;
    packet_in.exposure_type = 230;
    packet_in.command_id = 41;
    packet_in.engine_cut_off = 108;
    packet_in.extra_param = 175;
    packet_in.extra_value = 17.0;

    mavlink::ardupilotmega::msg::DIGICAM_CONFIGURE packet2{};

    mavlink_msg_digicam_configure_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.mode, packet2.mode);
    EXPECT_EQ(packet_in.shutter_speed, packet2.shutter_speed);
    EXPECT_EQ(packet_in.aperture, packet2.aperture);
    EXPECT_EQ(packet_in.iso, packet2.iso);
    EXPECT_EQ(packet_in.exposure_type, packet2.exposure_type);
    EXPECT_EQ(packet_in.command_id, packet2.command_id);
    EXPECT_EQ(packet_in.engine_cut_off, packet2.engine_cut_off);
    EXPECT_EQ(packet_in.extra_param, packet2.extra_param);
    EXPECT_EQ(packet_in.extra_value, packet2.extra_value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DIGICAM_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DIGICAM_CONTROL packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.session = 151;
    packet_in.zoom_pos = 218;
    packet_in.zoom_step = 29;
    packet_in.focus_lock = 96;
    packet_in.shot = 163;
    packet_in.command_id = 230;
    packet_in.extra_param = 41;
    packet_in.extra_value = 17.0;

    mavlink::ardupilotmega::msg::DIGICAM_CONTROL packet1{};
    mavlink::ardupilotmega::msg::DIGICAM_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.session, packet2.session);
    EXPECT_EQ(packet1.zoom_pos, packet2.zoom_pos);
    EXPECT_EQ(packet1.zoom_step, packet2.zoom_step);
    EXPECT_EQ(packet1.focus_lock, packet2.focus_lock);
    EXPECT_EQ(packet1.shot, packet2.shot);
    EXPECT_EQ(packet1.command_id, packet2.command_id);
    EXPECT_EQ(packet1.extra_param, packet2.extra_param);
    EXPECT_EQ(packet1.extra_value, packet2.extra_value);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DIGICAM_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_digicam_control_t packet_c {
         17.0, 17, 84, 151, 218, 29, 96, 163, 230, 41
    };

    mavlink::ardupilotmega::msg::DIGICAM_CONTROL packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.session = 151;
    packet_in.zoom_pos = 218;
    packet_in.zoom_step = 29;
    packet_in.focus_lock = 96;
    packet_in.shot = 163;
    packet_in.command_id = 230;
    packet_in.extra_param = 41;
    packet_in.extra_value = 17.0;

    mavlink::ardupilotmega::msg::DIGICAM_CONTROL packet2{};

    mavlink_msg_digicam_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.session, packet2.session);
    EXPECT_EQ(packet_in.zoom_pos, packet2.zoom_pos);
    EXPECT_EQ(packet_in.zoom_step, packet2.zoom_step);
    EXPECT_EQ(packet_in.focus_lock, packet2.focus_lock);
    EXPECT_EQ(packet_in.shot, packet2.shot);
    EXPECT_EQ(packet_in.command_id, packet2.command_id);
    EXPECT_EQ(packet_in.extra_param, packet2.extra_param);
    EXPECT_EQ(packet_in.extra_value, packet2.extra_value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, MOUNT_CONFIGURE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::MOUNT_CONFIGURE packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.mount_mode = 139;
    packet_in.stab_roll = 206;
    packet_in.stab_pitch = 17;
    packet_in.stab_yaw = 84;

    mavlink::ardupilotmega::msg::MOUNT_CONFIGURE packet1{};
    mavlink::ardupilotmega::msg::MOUNT_CONFIGURE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.mount_mode, packet2.mount_mode);
    EXPECT_EQ(packet1.stab_roll, packet2.stab_roll);
    EXPECT_EQ(packet1.stab_pitch, packet2.stab_pitch);
    EXPECT_EQ(packet1.stab_yaw, packet2.stab_yaw);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, MOUNT_CONFIGURE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mount_configure_t packet_c {
         5, 72, 139, 206, 17, 84
    };

    mavlink::ardupilotmega::msg::MOUNT_CONFIGURE packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.mount_mode = 139;
    packet_in.stab_roll = 206;
    packet_in.stab_pitch = 17;
    packet_in.stab_yaw = 84;

    mavlink::ardupilotmega::msg::MOUNT_CONFIGURE packet2{};

    mavlink_msg_mount_configure_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.mount_mode, packet2.mount_mode);
    EXPECT_EQ(packet_in.stab_roll, packet2.stab_roll);
    EXPECT_EQ(packet_in.stab_pitch, packet2.stab_pitch);
    EXPECT_EQ(packet_in.stab_yaw, packet2.stab_yaw);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, MOUNT_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::MOUNT_CONTROL packet_in{};
    packet_in.target_system = 41;
    packet_in.target_component = 108;
    packet_in.input_a = 963497464;
    packet_in.input_b = 963497672;
    packet_in.input_c = 963497880;
    packet_in.save_position = 175;

    mavlink::ardupilotmega::msg::MOUNT_CONTROL packet1{};
    mavlink::ardupilotmega::msg::MOUNT_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.input_a, packet2.input_a);
    EXPECT_EQ(packet1.input_b, packet2.input_b);
    EXPECT_EQ(packet1.input_c, packet2.input_c);
    EXPECT_EQ(packet1.save_position, packet2.save_position);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, MOUNT_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mount_control_t packet_c {
         963497464, 963497672, 963497880, 41, 108, 175
    };

    mavlink::ardupilotmega::msg::MOUNT_CONTROL packet_in{};
    packet_in.target_system = 41;
    packet_in.target_component = 108;
    packet_in.input_a = 963497464;
    packet_in.input_b = 963497672;
    packet_in.input_c = 963497880;
    packet_in.save_position = 175;

    mavlink::ardupilotmega::msg::MOUNT_CONTROL packet2{};

    mavlink_msg_mount_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.input_a, packet2.input_a);
    EXPECT_EQ(packet_in.input_b, packet2.input_b);
    EXPECT_EQ(packet_in.input_c, packet2.input_c);
    EXPECT_EQ(packet_in.save_position, packet2.save_position);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, MOUNT_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::MOUNT_STATUS packet_in{};
    packet_in.target_system = 41;
    packet_in.target_component = 108;
    packet_in.pointing_a = 963497464;
    packet_in.pointing_b = 963497672;
    packet_in.pointing_c = 963497880;

    mavlink::ardupilotmega::msg::MOUNT_STATUS packet1{};
    mavlink::ardupilotmega::msg::MOUNT_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.pointing_a, packet2.pointing_a);
    EXPECT_EQ(packet1.pointing_b, packet2.pointing_b);
    EXPECT_EQ(packet1.pointing_c, packet2.pointing_c);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, MOUNT_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mount_status_t packet_c {
         963497464, 963497672, 963497880, 41, 108
    };

    mavlink::ardupilotmega::msg::MOUNT_STATUS packet_in{};
    packet_in.target_system = 41;
    packet_in.target_component = 108;
    packet_in.pointing_a = 963497464;
    packet_in.pointing_b = 963497672;
    packet_in.pointing_c = 963497880;

    mavlink::ardupilotmega::msg::MOUNT_STATUS packet2{};

    mavlink_msg_mount_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.pointing_a, packet2.pointing_a);
    EXPECT_EQ(packet_in.pointing_b, packet2.pointing_b);
    EXPECT_EQ(packet_in.pointing_c, packet2.pointing_c);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, FENCE_POINT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::FENCE_POINT packet_in{};
    packet_in.target_system = 29;
    packet_in.target_component = 96;
    packet_in.idx = 163;
    packet_in.count = 230;
    packet_in.lat = 17.0;
    packet_in.lng = 45.0;

    mavlink::ardupilotmega::msg::FENCE_POINT packet1{};
    mavlink::ardupilotmega::msg::FENCE_POINT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.idx, packet2.idx);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lng, packet2.lng);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, FENCE_POINT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_fence_point_t packet_c {
         17.0, 45.0, 29, 96, 163, 230
    };

    mavlink::ardupilotmega::msg::FENCE_POINT packet_in{};
    packet_in.target_system = 29;
    packet_in.target_component = 96;
    packet_in.idx = 163;
    packet_in.count = 230;
    packet_in.lat = 17.0;
    packet_in.lng = 45.0;

    mavlink::ardupilotmega::msg::FENCE_POINT packet2{};

    mavlink_msg_fence_point_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.idx, packet2.idx);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lng, packet2.lng);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, FENCE_FETCH_POINT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::FENCE_FETCH_POINT packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.idx = 139;

    mavlink::ardupilotmega::msg::FENCE_FETCH_POINT packet1{};
    mavlink::ardupilotmega::msg::FENCE_FETCH_POINT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.idx, packet2.idx);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, FENCE_FETCH_POINT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_fence_fetch_point_t packet_c {
         5, 72, 139
    };

    mavlink::ardupilotmega::msg::FENCE_FETCH_POINT packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.idx = 139;

    mavlink::ardupilotmega::msg::FENCE_FETCH_POINT packet2{};

    mavlink_msg_fence_fetch_point_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.idx, packet2.idx);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, FENCE_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::FENCE_STATUS packet_in{};
    packet_in.breach_status = 151;
    packet_in.breach_count = 17443;
    packet_in.breach_type = 218;
    packet_in.breach_time = 963497464;

    mavlink::ardupilotmega::msg::FENCE_STATUS packet1{};
    mavlink::ardupilotmega::msg::FENCE_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.breach_status, packet2.breach_status);
    EXPECT_EQ(packet1.breach_count, packet2.breach_count);
    EXPECT_EQ(packet1.breach_type, packet2.breach_type);
    EXPECT_EQ(packet1.breach_time, packet2.breach_time);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, FENCE_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_fence_status_t packet_c {
         963497464, 17443, 151, 218
    };

    mavlink::ardupilotmega::msg::FENCE_STATUS packet_in{};
    packet_in.breach_status = 151;
    packet_in.breach_count = 17443;
    packet_in.breach_type = 218;
    packet_in.breach_time = 963497464;

    mavlink::ardupilotmega::msg::FENCE_STATUS packet2{};

    mavlink_msg_fence_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.breach_status, packet2.breach_status);
    EXPECT_EQ(packet_in.breach_count, packet2.breach_count);
    EXPECT_EQ(packet_in.breach_type, packet2.breach_type);
    EXPECT_EQ(packet_in.breach_time, packet2.breach_time);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AHRS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AHRS packet_in{};
    packet_in.omegaIx = 17.0;
    packet_in.omegaIy = 45.0;
    packet_in.omegaIz = 73.0;
    packet_in.accel_weight = 101.0;
    packet_in.renorm_val = 129.0;
    packet_in.error_rp = 157.0;
    packet_in.error_yaw = 185.0;

    mavlink::ardupilotmega::msg::AHRS packet1{};
    mavlink::ardupilotmega::msg::AHRS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.omegaIx, packet2.omegaIx);
    EXPECT_EQ(packet1.omegaIy, packet2.omegaIy);
    EXPECT_EQ(packet1.omegaIz, packet2.omegaIz);
    EXPECT_EQ(packet1.accel_weight, packet2.accel_weight);
    EXPECT_EQ(packet1.renorm_val, packet2.renorm_val);
    EXPECT_EQ(packet1.error_rp, packet2.error_rp);
    EXPECT_EQ(packet1.error_yaw, packet2.error_yaw);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AHRS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_ahrs_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0
    };

    mavlink::ardupilotmega::msg::AHRS packet_in{};
    packet_in.omegaIx = 17.0;
    packet_in.omegaIy = 45.0;
    packet_in.omegaIz = 73.0;
    packet_in.accel_weight = 101.0;
    packet_in.renorm_val = 129.0;
    packet_in.error_rp = 157.0;
    packet_in.error_yaw = 185.0;

    mavlink::ardupilotmega::msg::AHRS packet2{};

    mavlink_msg_ahrs_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.omegaIx, packet2.omegaIx);
    EXPECT_EQ(packet_in.omegaIy, packet2.omegaIy);
    EXPECT_EQ(packet_in.omegaIz, packet2.omegaIz);
    EXPECT_EQ(packet_in.accel_weight, packet2.accel_weight);
    EXPECT_EQ(packet_in.renorm_val, packet2.renorm_val);
    EXPECT_EQ(packet_in.error_rp, packet2.error_rp);
    EXPECT_EQ(packet_in.error_yaw, packet2.error_yaw);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, SIMSTATE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::SIMSTATE packet_in{};
    packet_in.roll = 17.0;
    packet_in.pitch = 45.0;
    packet_in.yaw = 73.0;
    packet_in.xacc = 101.0;
    packet_in.yacc = 129.0;
    packet_in.zacc = 157.0;
    packet_in.xgyro = 185.0;
    packet_in.ygyro = 213.0;
    packet_in.zgyro = 241.0;
    packet_in.lat = 963499336;
    packet_in.lng = 963499544;

    mavlink::ardupilotmega::msg::SIMSTATE packet1{};
    mavlink::ardupilotmega::msg::SIMSTATE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

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
    EXPECT_EQ(packet1.lng, packet2.lng);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, SIMSTATE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_simstate_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 963499336, 963499544
    };

    mavlink::ardupilotmega::msg::SIMSTATE packet_in{};
    packet_in.roll = 17.0;
    packet_in.pitch = 45.0;
    packet_in.yaw = 73.0;
    packet_in.xacc = 101.0;
    packet_in.yacc = 129.0;
    packet_in.zacc = 157.0;
    packet_in.xgyro = 185.0;
    packet_in.ygyro = 213.0;
    packet_in.zgyro = 241.0;
    packet_in.lat = 963499336;
    packet_in.lng = 963499544;

    mavlink::ardupilotmega::msg::SIMSTATE packet2{};

    mavlink_msg_simstate_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

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
    EXPECT_EQ(packet_in.lng, packet2.lng);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, HWSTATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::HWSTATUS packet_in{};
    packet_in.Vcc = 17235;
    packet_in.I2Cerr = 139;

    mavlink::ardupilotmega::msg::HWSTATUS packet1{};
    mavlink::ardupilotmega::msg::HWSTATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.Vcc, packet2.Vcc);
    EXPECT_EQ(packet1.I2Cerr, packet2.I2Cerr);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, HWSTATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_hwstatus_t packet_c {
         17235, 139
    };

    mavlink::ardupilotmega::msg::HWSTATUS packet_in{};
    packet_in.Vcc = 17235;
    packet_in.I2Cerr = 139;

    mavlink::ardupilotmega::msg::HWSTATUS packet2{};

    mavlink_msg_hwstatus_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.Vcc, packet2.Vcc);
    EXPECT_EQ(packet_in.I2Cerr, packet2.I2Cerr);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, RADIO)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::RADIO packet_in{};
    packet_in.rssi = 17;
    packet_in.remrssi = 84;
    packet_in.txbuf = 151;
    packet_in.noise = 218;
    packet_in.remnoise = 29;
    packet_in.rxerrors = 17235;
    packet_in.fixed = 17339;

    mavlink::ardupilotmega::msg::RADIO packet1{};
    mavlink::ardupilotmega::msg::RADIO packet2{};

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
TEST(ardupilotmega_interop, RADIO)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_radio_t packet_c {
         17235, 17339, 17, 84, 151, 218, 29
    };

    mavlink::ardupilotmega::msg::RADIO packet_in{};
    packet_in.rssi = 17;
    packet_in.remrssi = 84;
    packet_in.txbuf = 151;
    packet_in.noise = 218;
    packet_in.remnoise = 29;
    packet_in.rxerrors = 17235;
    packet_in.fixed = 17339;

    mavlink::ardupilotmega::msg::RADIO packet2{};

    mavlink_msg_radio_encode(1, 1, &msg, &packet_c);

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

TEST(ardupilotmega, LIMITS_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::LIMITS_STATUS packet_in{};
    packet_in.limits_state = 187;
    packet_in.last_trigger = 963497464;
    packet_in.last_action = 963497672;
    packet_in.last_recovery = 963497880;
    packet_in.last_clear = 963498088;
    packet_in.breach_count = 18067;
    packet_in.mods_enabled = 254;
    packet_in.mods_required = 65;
    packet_in.mods_triggered = 132;

    mavlink::ardupilotmega::msg::LIMITS_STATUS packet1{};
    mavlink::ardupilotmega::msg::LIMITS_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.limits_state, packet2.limits_state);
    EXPECT_EQ(packet1.last_trigger, packet2.last_trigger);
    EXPECT_EQ(packet1.last_action, packet2.last_action);
    EXPECT_EQ(packet1.last_recovery, packet2.last_recovery);
    EXPECT_EQ(packet1.last_clear, packet2.last_clear);
    EXPECT_EQ(packet1.breach_count, packet2.breach_count);
    EXPECT_EQ(packet1.mods_enabled, packet2.mods_enabled);
    EXPECT_EQ(packet1.mods_required, packet2.mods_required);
    EXPECT_EQ(packet1.mods_triggered, packet2.mods_triggered);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, LIMITS_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_limits_status_t packet_c {
         963497464, 963497672, 963497880, 963498088, 18067, 187, 254, 65, 132
    };

    mavlink::ardupilotmega::msg::LIMITS_STATUS packet_in{};
    packet_in.limits_state = 187;
    packet_in.last_trigger = 963497464;
    packet_in.last_action = 963497672;
    packet_in.last_recovery = 963497880;
    packet_in.last_clear = 963498088;
    packet_in.breach_count = 18067;
    packet_in.mods_enabled = 254;
    packet_in.mods_required = 65;
    packet_in.mods_triggered = 132;

    mavlink::ardupilotmega::msg::LIMITS_STATUS packet2{};

    mavlink_msg_limits_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.limits_state, packet2.limits_state);
    EXPECT_EQ(packet_in.last_trigger, packet2.last_trigger);
    EXPECT_EQ(packet_in.last_action, packet2.last_action);
    EXPECT_EQ(packet_in.last_recovery, packet2.last_recovery);
    EXPECT_EQ(packet_in.last_clear, packet2.last_clear);
    EXPECT_EQ(packet_in.breach_count, packet2.breach_count);
    EXPECT_EQ(packet_in.mods_enabled, packet2.mods_enabled);
    EXPECT_EQ(packet_in.mods_required, packet2.mods_required);
    EXPECT_EQ(packet_in.mods_triggered, packet2.mods_triggered);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, WIND)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::WIND packet_in{};
    packet_in.direction = 17.0;
    packet_in.speed = 45.0;
    packet_in.speed_z = 73.0;

    mavlink::ardupilotmega::msg::WIND packet1{};
    mavlink::ardupilotmega::msg::WIND packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.direction, packet2.direction);
    EXPECT_EQ(packet1.speed, packet2.speed);
    EXPECT_EQ(packet1.speed_z, packet2.speed_z);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, WIND)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_wind_t packet_c {
         17.0, 45.0, 73.0
    };

    mavlink::ardupilotmega::msg::WIND packet_in{};
    packet_in.direction = 17.0;
    packet_in.speed = 45.0;
    packet_in.speed_z = 73.0;

    mavlink::ardupilotmega::msg::WIND packet2{};

    mavlink_msg_wind_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.direction, packet2.direction);
    EXPECT_EQ(packet_in.speed, packet2.speed);
    EXPECT_EQ(packet_in.speed_z, packet2.speed_z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DATA16)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DATA16 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154 }};

    mavlink::ardupilotmega::msg::DATA16 packet1{};
    mavlink::ardupilotmega::msg::DATA16 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.len, packet2.len);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DATA16)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_data16_t packet_c {
         5, 72, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154 }
    };

    mavlink::ardupilotmega::msg::DATA16 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154 }};

    mavlink::ardupilotmega::msg::DATA16 packet2{};

    mavlink_msg_data16_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.len, packet2.len);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DATA32)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DATA32 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170 }};

    mavlink::ardupilotmega::msg::DATA32 packet1{};
    mavlink::ardupilotmega::msg::DATA32 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.len, packet2.len);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DATA32)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_data32_t packet_c {
         5, 72, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170 }
    };

    mavlink::ardupilotmega::msg::DATA32 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170 }};

    mavlink::ardupilotmega::msg::DATA32 packet2{};

    mavlink_msg_data32_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.len, packet2.len);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DATA64)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DATA64 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202 }};

    mavlink::ardupilotmega::msg::DATA64 packet1{};
    mavlink::ardupilotmega::msg::DATA64 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.len, packet2.len);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DATA64)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_data64_t packet_c {
         5, 72, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202 }
    };

    mavlink::ardupilotmega::msg::DATA64 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202 }};

    mavlink::ardupilotmega::msg::DATA64 packet2{};

    mavlink_msg_data64_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.len, packet2.len);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DATA96)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DATA96 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234 }};

    mavlink::ardupilotmega::msg::DATA96 packet1{};
    mavlink::ardupilotmega::msg::DATA96 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.type, packet2.type);
    EXPECT_EQ(packet1.len, packet2.len);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DATA96)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_data96_t packet_c {
         5, 72, { 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234 }
    };

    mavlink::ardupilotmega::msg::DATA96 packet_in{};
    packet_in.type = 5;
    packet_in.len = 72;
    packet_in.data = {{ 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234 }};

    mavlink::ardupilotmega::msg::DATA96 packet2{};

    mavlink_msg_data96_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.type, packet2.type);
    EXPECT_EQ(packet_in.len, packet2.len);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, RANGEFINDER)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::RANGEFINDER packet_in{};
    packet_in.distance = 17.0;
    packet_in.voltage = 45.0;

    mavlink::ardupilotmega::msg::RANGEFINDER packet1{};
    mavlink::ardupilotmega::msg::RANGEFINDER packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.distance, packet2.distance);
    EXPECT_EQ(packet1.voltage, packet2.voltage);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, RANGEFINDER)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rangefinder_t packet_c {
         17.0, 45.0
    };

    mavlink::ardupilotmega::msg::RANGEFINDER packet_in{};
    packet_in.distance = 17.0;
    packet_in.voltage = 45.0;

    mavlink::ardupilotmega::msg::RANGEFINDER packet2{};

    mavlink_msg_rangefinder_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.distance, packet2.distance);
    EXPECT_EQ(packet_in.voltage, packet2.voltage);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AIRSPEED_AUTOCAL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AIRSPEED_AUTOCAL packet_in{};
    packet_in.vx = 17.0;
    packet_in.vy = 45.0;
    packet_in.vz = 73.0;
    packet_in.diff_pressure = 101.0;
    packet_in.EAS2TAS = 129.0;
    packet_in.ratio = 157.0;
    packet_in.state_x = 185.0;
    packet_in.state_y = 213.0;
    packet_in.state_z = 241.0;
    packet_in.Pax = 269.0;
    packet_in.Pby = 297.0;
    packet_in.Pcz = 325.0;

    mavlink::ardupilotmega::msg::AIRSPEED_AUTOCAL packet1{};
    mavlink::ardupilotmega::msg::AIRSPEED_AUTOCAL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.vx, packet2.vx);
    EXPECT_EQ(packet1.vy, packet2.vy);
    EXPECT_EQ(packet1.vz, packet2.vz);
    EXPECT_EQ(packet1.diff_pressure, packet2.diff_pressure);
    EXPECT_EQ(packet1.EAS2TAS, packet2.EAS2TAS);
    EXPECT_EQ(packet1.ratio, packet2.ratio);
    EXPECT_EQ(packet1.state_x, packet2.state_x);
    EXPECT_EQ(packet1.state_y, packet2.state_y);
    EXPECT_EQ(packet1.state_z, packet2.state_z);
    EXPECT_EQ(packet1.Pax, packet2.Pax);
    EXPECT_EQ(packet1.Pby, packet2.Pby);
    EXPECT_EQ(packet1.Pcz, packet2.Pcz);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AIRSPEED_AUTOCAL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_airspeed_autocal_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0
    };

    mavlink::ardupilotmega::msg::AIRSPEED_AUTOCAL packet_in{};
    packet_in.vx = 17.0;
    packet_in.vy = 45.0;
    packet_in.vz = 73.0;
    packet_in.diff_pressure = 101.0;
    packet_in.EAS2TAS = 129.0;
    packet_in.ratio = 157.0;
    packet_in.state_x = 185.0;
    packet_in.state_y = 213.0;
    packet_in.state_z = 241.0;
    packet_in.Pax = 269.0;
    packet_in.Pby = 297.0;
    packet_in.Pcz = 325.0;

    mavlink::ardupilotmega::msg::AIRSPEED_AUTOCAL packet2{};

    mavlink_msg_airspeed_autocal_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.vx, packet2.vx);
    EXPECT_EQ(packet_in.vy, packet2.vy);
    EXPECT_EQ(packet_in.vz, packet2.vz);
    EXPECT_EQ(packet_in.diff_pressure, packet2.diff_pressure);
    EXPECT_EQ(packet_in.EAS2TAS, packet2.EAS2TAS);
    EXPECT_EQ(packet_in.ratio, packet2.ratio);
    EXPECT_EQ(packet_in.state_x, packet2.state_x);
    EXPECT_EQ(packet_in.state_y, packet2.state_y);
    EXPECT_EQ(packet_in.state_z, packet2.state_z);
    EXPECT_EQ(packet_in.Pax, packet2.Pax);
    EXPECT_EQ(packet_in.Pby, packet2.Pby);
    EXPECT_EQ(packet_in.Pcz, packet2.Pcz);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, RALLY_POINT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::RALLY_POINT packet_in{};
    packet_in.target_system = 175;
    packet_in.target_component = 242;
    packet_in.idx = 53;
    packet_in.count = 120;
    packet_in.lat = 963497464;
    packet_in.lng = 963497672;
    packet_in.alt = 17651;
    packet_in.break_alt = 17755;
    packet_in.land_dir = 17859;
    packet_in.flags = 187;

    mavlink::ardupilotmega::msg::RALLY_POINT packet1{};
    mavlink::ardupilotmega::msg::RALLY_POINT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.idx, packet2.idx);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lng, packet2.lng);
    EXPECT_EQ(packet1.alt, packet2.alt);
    EXPECT_EQ(packet1.break_alt, packet2.break_alt);
    EXPECT_EQ(packet1.land_dir, packet2.land_dir);
    EXPECT_EQ(packet1.flags, packet2.flags);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, RALLY_POINT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rally_point_t packet_c {
         963497464, 963497672, 17651, 17755, 17859, 175, 242, 53, 120, 187
    };

    mavlink::ardupilotmega::msg::RALLY_POINT packet_in{};
    packet_in.target_system = 175;
    packet_in.target_component = 242;
    packet_in.idx = 53;
    packet_in.count = 120;
    packet_in.lat = 963497464;
    packet_in.lng = 963497672;
    packet_in.alt = 17651;
    packet_in.break_alt = 17755;
    packet_in.land_dir = 17859;
    packet_in.flags = 187;

    mavlink::ardupilotmega::msg::RALLY_POINT packet2{};

    mavlink_msg_rally_point_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.idx, packet2.idx);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lng, packet2.lng);
    EXPECT_EQ(packet_in.alt, packet2.alt);
    EXPECT_EQ(packet_in.break_alt, packet2.break_alt);
    EXPECT_EQ(packet_in.land_dir, packet2.land_dir);
    EXPECT_EQ(packet_in.flags, packet2.flags);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, RALLY_FETCH_POINT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::RALLY_FETCH_POINT packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.idx = 139;

    mavlink::ardupilotmega::msg::RALLY_FETCH_POINT packet1{};
    mavlink::ardupilotmega::msg::RALLY_FETCH_POINT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.idx, packet2.idx);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, RALLY_FETCH_POINT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rally_fetch_point_t packet_c {
         5, 72, 139
    };

    mavlink::ardupilotmega::msg::RALLY_FETCH_POINT packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.idx = 139;

    mavlink::ardupilotmega::msg::RALLY_FETCH_POINT packet2{};

    mavlink_msg_rally_fetch_point_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.idx, packet2.idx);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, COMPASSMOT_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::COMPASSMOT_STATUS packet_in{};
    packet_in.throttle = 18067;
    packet_in.current = 17.0;
    packet_in.interference = 18171;
    packet_in.CompensationX = 45.0;
    packet_in.CompensationY = 73.0;
    packet_in.CompensationZ = 101.0;

    mavlink::ardupilotmega::msg::COMPASSMOT_STATUS packet1{};
    mavlink::ardupilotmega::msg::COMPASSMOT_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.throttle, packet2.throttle);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.interference, packet2.interference);
    EXPECT_EQ(packet1.CompensationX, packet2.CompensationX);
    EXPECT_EQ(packet1.CompensationY, packet2.CompensationY);
    EXPECT_EQ(packet1.CompensationZ, packet2.CompensationZ);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, COMPASSMOT_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_compassmot_status_t packet_c {
         17.0, 45.0, 73.0, 101.0, 18067, 18171
    };

    mavlink::ardupilotmega::msg::COMPASSMOT_STATUS packet_in{};
    packet_in.throttle = 18067;
    packet_in.current = 17.0;
    packet_in.interference = 18171;
    packet_in.CompensationX = 45.0;
    packet_in.CompensationY = 73.0;
    packet_in.CompensationZ = 101.0;

    mavlink::ardupilotmega::msg::COMPASSMOT_STATUS packet2{};

    mavlink_msg_compassmot_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.throttle, packet2.throttle);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.interference, packet2.interference);
    EXPECT_EQ(packet_in.CompensationX, packet2.CompensationX);
    EXPECT_EQ(packet_in.CompensationY, packet2.CompensationY);
    EXPECT_EQ(packet_in.CompensationZ, packet2.CompensationZ);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AHRS2)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AHRS2 packet_in{};
    packet_in.roll = 17.0;
    packet_in.pitch = 45.0;
    packet_in.yaw = 73.0;
    packet_in.altitude = 101.0;
    packet_in.lat = 963498296;
    packet_in.lng = 963498504;

    mavlink::ardupilotmega::msg::AHRS2 packet1{};
    mavlink::ardupilotmega::msg::AHRS2 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lng, packet2.lng);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AHRS2)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_ahrs2_t packet_c {
         17.0, 45.0, 73.0, 101.0, 963498296, 963498504
    };

    mavlink::ardupilotmega::msg::AHRS2 packet_in{};
    packet_in.roll = 17.0;
    packet_in.pitch = 45.0;
    packet_in.yaw = 73.0;
    packet_in.altitude = 101.0;
    packet_in.lat = 963498296;
    packet_in.lng = 963498504;

    mavlink::ardupilotmega::msg::AHRS2 packet2{};

    mavlink_msg_ahrs2_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lng, packet2.lng);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, CAMERA_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::CAMERA_STATUS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.target_system = 211;
    packet_in.cam_idx = 22;
    packet_in.img_idx = 18483;
    packet_in.event_id = 89;
    packet_in.p1 = 73.0;
    packet_in.p2 = 101.0;
    packet_in.p3 = 129.0;
    packet_in.p4 = 157.0;

    mavlink::ardupilotmega::msg::CAMERA_STATUS packet1{};
    mavlink::ardupilotmega::msg::CAMERA_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.cam_idx, packet2.cam_idx);
    EXPECT_EQ(packet1.img_idx, packet2.img_idx);
    EXPECT_EQ(packet1.event_id, packet2.event_id);
    EXPECT_EQ(packet1.p1, packet2.p1);
    EXPECT_EQ(packet1.p2, packet2.p2);
    EXPECT_EQ(packet1.p3, packet2.p3);
    EXPECT_EQ(packet1.p4, packet2.p4);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, CAMERA_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_status_t packet_c {
         93372036854775807ULL, 73.0, 101.0, 129.0, 157.0, 18483, 211, 22, 89
    };

    mavlink::ardupilotmega::msg::CAMERA_STATUS packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.target_system = 211;
    packet_in.cam_idx = 22;
    packet_in.img_idx = 18483;
    packet_in.event_id = 89;
    packet_in.p1 = 73.0;
    packet_in.p2 = 101.0;
    packet_in.p3 = 129.0;
    packet_in.p4 = 157.0;

    mavlink::ardupilotmega::msg::CAMERA_STATUS packet2{};

    mavlink_msg_camera_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.cam_idx, packet2.cam_idx);
    EXPECT_EQ(packet_in.img_idx, packet2.img_idx);
    EXPECT_EQ(packet_in.event_id, packet2.event_id);
    EXPECT_EQ(packet_in.p1, packet2.p1);
    EXPECT_EQ(packet_in.p2, packet2.p2);
    EXPECT_EQ(packet_in.p3, packet2.p3);
    EXPECT_EQ(packet_in.p4, packet2.p4);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, CAMERA_FEEDBACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::CAMERA_FEEDBACK packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.target_system = 3;
    packet_in.cam_idx = 70;
    packet_in.img_idx = 19315;
    packet_in.lat = 963497880;
    packet_in.lng = 963498088;
    packet_in.alt_msl = 129.0;
    packet_in.alt_rel = 157.0;
    packet_in.roll = 185.0;
    packet_in.pitch = 213.0;
    packet_in.yaw = 241.0;
    packet_in.foc_len = 269.0;
    packet_in.flags = 137;
    packet_in.completed_captures = 19575;

    mavlink::ardupilotmega::msg::CAMERA_FEEDBACK packet1{};
    mavlink::ardupilotmega::msg::CAMERA_FEEDBACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.cam_idx, packet2.cam_idx);
    EXPECT_EQ(packet1.img_idx, packet2.img_idx);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lng, packet2.lng);
    EXPECT_EQ(packet1.alt_msl, packet2.alt_msl);
    EXPECT_EQ(packet1.alt_rel, packet2.alt_rel);
    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.foc_len, packet2.foc_len);
    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.completed_captures, packet2.completed_captures);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, CAMERA_FEEDBACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_camera_feedback_t packet_c {
         93372036854775807ULL, 963497880, 963498088, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 19315, 3, 70, 137, 19575
    };

    mavlink::ardupilotmega::msg::CAMERA_FEEDBACK packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.target_system = 3;
    packet_in.cam_idx = 70;
    packet_in.img_idx = 19315;
    packet_in.lat = 963497880;
    packet_in.lng = 963498088;
    packet_in.alt_msl = 129.0;
    packet_in.alt_rel = 157.0;
    packet_in.roll = 185.0;
    packet_in.pitch = 213.0;
    packet_in.yaw = 241.0;
    packet_in.foc_len = 269.0;
    packet_in.flags = 137;
    packet_in.completed_captures = 19575;

    mavlink::ardupilotmega::msg::CAMERA_FEEDBACK packet2{};

    mavlink_msg_camera_feedback_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.cam_idx, packet2.cam_idx);
    EXPECT_EQ(packet_in.img_idx, packet2.img_idx);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lng, packet2.lng);
    EXPECT_EQ(packet_in.alt_msl, packet2.alt_msl);
    EXPECT_EQ(packet_in.alt_rel, packet2.alt_rel);
    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.foc_len, packet2.foc_len);
    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.completed_captures, packet2.completed_captures);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, BATTERY2)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::BATTERY2 packet_in{};
    packet_in.voltage = 17235;
    packet_in.current_battery = 17339;

    mavlink::ardupilotmega::msg::BATTERY2 packet1{};
    mavlink::ardupilotmega::msg::BATTERY2 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.voltage, packet2.voltage);
    EXPECT_EQ(packet1.current_battery, packet2.current_battery);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, BATTERY2)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_battery2_t packet_c {
         17235, 17339
    };

    mavlink::ardupilotmega::msg::BATTERY2 packet_in{};
    packet_in.voltage = 17235;
    packet_in.current_battery = 17339;

    mavlink::ardupilotmega::msg::BATTERY2 packet2{};

    mavlink_msg_battery2_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.voltage, packet2.voltage);
    EXPECT_EQ(packet_in.current_battery, packet2.current_battery);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AHRS3)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AHRS3 packet_in{};
    packet_in.roll = 17.0;
    packet_in.pitch = 45.0;
    packet_in.yaw = 73.0;
    packet_in.altitude = 101.0;
    packet_in.lat = 963498296;
    packet_in.lng = 963498504;
    packet_in.v1 = 185.0;
    packet_in.v2 = 213.0;
    packet_in.v3 = 241.0;
    packet_in.v4 = 269.0;

    mavlink::ardupilotmega::msg::AHRS3 packet1{};
    mavlink::ardupilotmega::msg::AHRS3 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.roll, packet2.roll);
    EXPECT_EQ(packet1.pitch, packet2.pitch);
    EXPECT_EQ(packet1.yaw, packet2.yaw);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.lat, packet2.lat);
    EXPECT_EQ(packet1.lng, packet2.lng);
    EXPECT_EQ(packet1.v1, packet2.v1);
    EXPECT_EQ(packet1.v2, packet2.v2);
    EXPECT_EQ(packet1.v3, packet2.v3);
    EXPECT_EQ(packet1.v4, packet2.v4);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AHRS3)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_ahrs3_t packet_c {
         17.0, 45.0, 73.0, 101.0, 963498296, 963498504, 185.0, 213.0, 241.0, 269.0
    };

    mavlink::ardupilotmega::msg::AHRS3 packet_in{};
    packet_in.roll = 17.0;
    packet_in.pitch = 45.0;
    packet_in.yaw = 73.0;
    packet_in.altitude = 101.0;
    packet_in.lat = 963498296;
    packet_in.lng = 963498504;
    packet_in.v1 = 185.0;
    packet_in.v2 = 213.0;
    packet_in.v3 = 241.0;
    packet_in.v4 = 269.0;

    mavlink::ardupilotmega::msg::AHRS3 packet2{};

    mavlink_msg_ahrs3_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.roll, packet2.roll);
    EXPECT_EQ(packet_in.pitch, packet2.pitch);
    EXPECT_EQ(packet_in.yaw, packet2.yaw);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.lat, packet2.lat);
    EXPECT_EQ(packet_in.lng, packet2.lng);
    EXPECT_EQ(packet_in.v1, packet2.v1);
    EXPECT_EQ(packet_in.v2, packet2.v2);
    EXPECT_EQ(packet_in.v3, packet2.v3);
    EXPECT_EQ(packet_in.v4, packet2.v4);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AUTOPILOT_VERSION_REQUEST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AUTOPILOT_VERSION_REQUEST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::ardupilotmega::msg::AUTOPILOT_VERSION_REQUEST packet1{};
    mavlink::ardupilotmega::msg::AUTOPILOT_VERSION_REQUEST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AUTOPILOT_VERSION_REQUEST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_autopilot_version_request_t packet_c {
         5, 72
    };

    mavlink::ardupilotmega::msg::AUTOPILOT_VERSION_REQUEST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;

    mavlink::ardupilotmega::msg::AUTOPILOT_VERSION_REQUEST packet2{};

    mavlink_msg_autopilot_version_request_encode(1, 1, &msg, &packet_c);

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

TEST(ardupilotmega, REMOTE_LOG_DATA_BLOCK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::REMOTE_LOG_DATA_BLOCK packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.seqno = 963497464;
    packet_in.data = {{ 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94 }};

    mavlink::ardupilotmega::msg::REMOTE_LOG_DATA_BLOCK packet1{};
    mavlink::ardupilotmega::msg::REMOTE_LOG_DATA_BLOCK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seqno, packet2.seqno);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, REMOTE_LOG_DATA_BLOCK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_remote_log_data_block_t packet_c {
         963497464, 17, 84, { 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94 }
    };

    mavlink::ardupilotmega::msg::REMOTE_LOG_DATA_BLOCK packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.seqno = 963497464;
    packet_in.data = {{ 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94 }};

    mavlink::ardupilotmega::msg::REMOTE_LOG_DATA_BLOCK packet2{};

    mavlink_msg_remote_log_data_block_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seqno, packet2.seqno);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, REMOTE_LOG_BLOCK_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::REMOTE_LOG_BLOCK_STATUS packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.seqno = 963497464;
    packet_in.status = 151;

    mavlink::ardupilotmega::msg::REMOTE_LOG_BLOCK_STATUS packet1{};
    mavlink::ardupilotmega::msg::REMOTE_LOG_BLOCK_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.seqno, packet2.seqno);
    EXPECT_EQ(packet1.status, packet2.status);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, REMOTE_LOG_BLOCK_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_remote_log_block_status_t packet_c {
         963497464, 17, 84, 151
    };

    mavlink::ardupilotmega::msg::REMOTE_LOG_BLOCK_STATUS packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.seqno = 963497464;
    packet_in.status = 151;

    mavlink::ardupilotmega::msg::REMOTE_LOG_BLOCK_STATUS packet2{};

    mavlink_msg_remote_log_block_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.seqno, packet2.seqno);
    EXPECT_EQ(packet_in.status, packet2.status);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, LED_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::LED_CONTROL packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.instance = 139;
    packet_in.pattern = 206;
    packet_in.custom_len = 17;
    packet_in.custom_bytes = {{ 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107 }};

    mavlink::ardupilotmega::msg::LED_CONTROL packet1{};
    mavlink::ardupilotmega::msg::LED_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.instance, packet2.instance);
    EXPECT_EQ(packet1.pattern, packet2.pattern);
    EXPECT_EQ(packet1.custom_len, packet2.custom_len);
    EXPECT_EQ(packet1.custom_bytes, packet2.custom_bytes);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, LED_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_led_control_t packet_c {
         5, 72, 139, 206, 17, { 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107 }
    };

    mavlink::ardupilotmega::msg::LED_CONTROL packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.instance = 139;
    packet_in.pattern = 206;
    packet_in.custom_len = 17;
    packet_in.custom_bytes = {{ 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107 }};

    mavlink::ardupilotmega::msg::LED_CONTROL packet2{};

    mavlink_msg_led_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.instance, packet2.instance);
    EXPECT_EQ(packet_in.pattern, packet2.pattern);
    EXPECT_EQ(packet_in.custom_len, packet2.custom_len);
    EXPECT_EQ(packet_in.custom_bytes, packet2.custom_bytes);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, MAG_CAL_PROGRESS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::MAG_CAL_PROGRESS packet_in{};
    packet_in.compass_id = 41;
    packet_in.cal_mask = 108;
    packet_in.cal_status = 175;
    packet_in.attempt = 242;
    packet_in.completion_pct = 53;
    packet_in.completion_mask = {{ 120, 121, 122, 123, 124, 125, 126, 127, 128, 129 }};
    packet_in.direction_x = 17.0;
    packet_in.direction_y = 45.0;
    packet_in.direction_z = 73.0;

    mavlink::ardupilotmega::msg::MAG_CAL_PROGRESS packet1{};
    mavlink::ardupilotmega::msg::MAG_CAL_PROGRESS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.compass_id, packet2.compass_id);
    EXPECT_EQ(packet1.cal_mask, packet2.cal_mask);
    EXPECT_EQ(packet1.cal_status, packet2.cal_status);
    EXPECT_EQ(packet1.attempt, packet2.attempt);
    EXPECT_EQ(packet1.completion_pct, packet2.completion_pct);
    EXPECT_EQ(packet1.completion_mask, packet2.completion_mask);
    EXPECT_EQ(packet1.direction_x, packet2.direction_x);
    EXPECT_EQ(packet1.direction_y, packet2.direction_y);
    EXPECT_EQ(packet1.direction_z, packet2.direction_z);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, MAG_CAL_PROGRESS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mag_cal_progress_t packet_c {
         17.0, 45.0, 73.0, 41, 108, 175, 242, 53, { 120, 121, 122, 123, 124, 125, 126, 127, 128, 129 }
    };

    mavlink::ardupilotmega::msg::MAG_CAL_PROGRESS packet_in{};
    packet_in.compass_id = 41;
    packet_in.cal_mask = 108;
    packet_in.cal_status = 175;
    packet_in.attempt = 242;
    packet_in.completion_pct = 53;
    packet_in.completion_mask = {{ 120, 121, 122, 123, 124, 125, 126, 127, 128, 129 }};
    packet_in.direction_x = 17.0;
    packet_in.direction_y = 45.0;
    packet_in.direction_z = 73.0;

    mavlink::ardupilotmega::msg::MAG_CAL_PROGRESS packet2{};

    mavlink_msg_mag_cal_progress_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.compass_id, packet2.compass_id);
    EXPECT_EQ(packet_in.cal_mask, packet2.cal_mask);
    EXPECT_EQ(packet_in.cal_status, packet2.cal_status);
    EXPECT_EQ(packet_in.attempt, packet2.attempt);
    EXPECT_EQ(packet_in.completion_pct, packet2.completion_pct);
    EXPECT_EQ(packet_in.completion_mask, packet2.completion_mask);
    EXPECT_EQ(packet_in.direction_x, packet2.direction_x);
    EXPECT_EQ(packet_in.direction_y, packet2.direction_y);
    EXPECT_EQ(packet_in.direction_z, packet2.direction_z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, MAG_CAL_REPORT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::MAG_CAL_REPORT packet_in{};
    packet_in.compass_id = 125;
    packet_in.cal_mask = 192;
    packet_in.cal_status = 3;
    packet_in.autosaved = 70;
    packet_in.fitness = 17.0;
    packet_in.ofs_x = 45.0;
    packet_in.ofs_y = 73.0;
    packet_in.ofs_z = 101.0;
    packet_in.diag_x = 129.0;
    packet_in.diag_y = 157.0;
    packet_in.diag_z = 185.0;
    packet_in.offdiag_x = 213.0;
    packet_in.offdiag_y = 241.0;
    packet_in.offdiag_z = 269.0;
    packet_in.orientation_confidence = 325.0;
    packet_in.old_orientation = 149;
    packet_in.new_orientation = 216;

    mavlink::ardupilotmega::msg::MAG_CAL_REPORT packet1{};
    mavlink::ardupilotmega::msg::MAG_CAL_REPORT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.compass_id, packet2.compass_id);
    EXPECT_EQ(packet1.cal_mask, packet2.cal_mask);
    EXPECT_EQ(packet1.cal_status, packet2.cal_status);
    EXPECT_EQ(packet1.autosaved, packet2.autosaved);
    EXPECT_EQ(packet1.fitness, packet2.fitness);
    EXPECT_EQ(packet1.ofs_x, packet2.ofs_x);
    EXPECT_EQ(packet1.ofs_y, packet2.ofs_y);
    EXPECT_EQ(packet1.ofs_z, packet2.ofs_z);
    EXPECT_EQ(packet1.diag_x, packet2.diag_x);
    EXPECT_EQ(packet1.diag_y, packet2.diag_y);
    EXPECT_EQ(packet1.diag_z, packet2.diag_z);
    EXPECT_EQ(packet1.offdiag_x, packet2.offdiag_x);
    EXPECT_EQ(packet1.offdiag_y, packet2.offdiag_y);
    EXPECT_EQ(packet1.offdiag_z, packet2.offdiag_z);
    EXPECT_EQ(packet1.orientation_confidence, packet2.orientation_confidence);
    EXPECT_EQ(packet1.old_orientation, packet2.old_orientation);
    EXPECT_EQ(packet1.new_orientation, packet2.new_orientation);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, MAG_CAL_REPORT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_mag_cal_report_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 125, 192, 3, 70, 325.0, 149, 216
    };

    mavlink::ardupilotmega::msg::MAG_CAL_REPORT packet_in{};
    packet_in.compass_id = 125;
    packet_in.cal_mask = 192;
    packet_in.cal_status = 3;
    packet_in.autosaved = 70;
    packet_in.fitness = 17.0;
    packet_in.ofs_x = 45.0;
    packet_in.ofs_y = 73.0;
    packet_in.ofs_z = 101.0;
    packet_in.diag_x = 129.0;
    packet_in.diag_y = 157.0;
    packet_in.diag_z = 185.0;
    packet_in.offdiag_x = 213.0;
    packet_in.offdiag_y = 241.0;
    packet_in.offdiag_z = 269.0;
    packet_in.orientation_confidence = 325.0;
    packet_in.old_orientation = 149;
    packet_in.new_orientation = 216;

    mavlink::ardupilotmega::msg::MAG_CAL_REPORT packet2{};

    mavlink_msg_mag_cal_report_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.compass_id, packet2.compass_id);
    EXPECT_EQ(packet_in.cal_mask, packet2.cal_mask);
    EXPECT_EQ(packet_in.cal_status, packet2.cal_status);
    EXPECT_EQ(packet_in.autosaved, packet2.autosaved);
    EXPECT_EQ(packet_in.fitness, packet2.fitness);
    EXPECT_EQ(packet_in.ofs_x, packet2.ofs_x);
    EXPECT_EQ(packet_in.ofs_y, packet2.ofs_y);
    EXPECT_EQ(packet_in.ofs_z, packet2.ofs_z);
    EXPECT_EQ(packet_in.diag_x, packet2.diag_x);
    EXPECT_EQ(packet_in.diag_y, packet2.diag_y);
    EXPECT_EQ(packet_in.diag_z, packet2.diag_z);
    EXPECT_EQ(packet_in.offdiag_x, packet2.offdiag_x);
    EXPECT_EQ(packet_in.offdiag_y, packet2.offdiag_y);
    EXPECT_EQ(packet_in.offdiag_z, packet2.offdiag_z);
    EXPECT_EQ(packet_in.orientation_confidence, packet2.orientation_confidence);
    EXPECT_EQ(packet_in.old_orientation, packet2.old_orientation);
    EXPECT_EQ(packet_in.new_orientation, packet2.new_orientation);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, EKF_STATUS_REPORT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::EKF_STATUS_REPORT packet_in{};
    packet_in.flags = 18275;
    packet_in.velocity_variance = 17.0;
    packet_in.pos_horiz_variance = 45.0;
    packet_in.pos_vert_variance = 73.0;
    packet_in.compass_variance = 101.0;
    packet_in.terrain_alt_variance = 129.0;
    packet_in.airspeed_variance = 171.0;

    mavlink::ardupilotmega::msg::EKF_STATUS_REPORT packet1{};
    mavlink::ardupilotmega::msg::EKF_STATUS_REPORT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.flags, packet2.flags);
    EXPECT_EQ(packet1.velocity_variance, packet2.velocity_variance);
    EXPECT_EQ(packet1.pos_horiz_variance, packet2.pos_horiz_variance);
    EXPECT_EQ(packet1.pos_vert_variance, packet2.pos_vert_variance);
    EXPECT_EQ(packet1.compass_variance, packet2.compass_variance);
    EXPECT_EQ(packet1.terrain_alt_variance, packet2.terrain_alt_variance);
    EXPECT_EQ(packet1.airspeed_variance, packet2.airspeed_variance);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, EKF_STATUS_REPORT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_ekf_status_report_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 18275, 171.0
    };

    mavlink::ardupilotmega::msg::EKF_STATUS_REPORT packet_in{};
    packet_in.flags = 18275;
    packet_in.velocity_variance = 17.0;
    packet_in.pos_horiz_variance = 45.0;
    packet_in.pos_vert_variance = 73.0;
    packet_in.compass_variance = 101.0;
    packet_in.terrain_alt_variance = 129.0;
    packet_in.airspeed_variance = 171.0;

    mavlink::ardupilotmega::msg::EKF_STATUS_REPORT packet2{};

    mavlink_msg_ekf_status_report_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.flags, packet2.flags);
    EXPECT_EQ(packet_in.velocity_variance, packet2.velocity_variance);
    EXPECT_EQ(packet_in.pos_horiz_variance, packet2.pos_horiz_variance);
    EXPECT_EQ(packet_in.pos_vert_variance, packet2.pos_vert_variance);
    EXPECT_EQ(packet_in.compass_variance, packet2.compass_variance);
    EXPECT_EQ(packet_in.terrain_alt_variance, packet2.terrain_alt_variance);
    EXPECT_EQ(packet_in.airspeed_variance, packet2.airspeed_variance);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, PID_TUNING)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::PID_TUNING packet_in{};
    packet_in.axis = 77;
    packet_in.desired = 17.0;
    packet_in.achieved = 45.0;
    packet_in.FF = 73.0;
    packet_in.P = 101.0;
    packet_in.I = 129.0;
    packet_in.D = 157.0;

    mavlink::ardupilotmega::msg::PID_TUNING packet1{};
    mavlink::ardupilotmega::msg::PID_TUNING packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.axis, packet2.axis);
    EXPECT_EQ(packet1.desired, packet2.desired);
    EXPECT_EQ(packet1.achieved, packet2.achieved);
    EXPECT_EQ(packet1.FF, packet2.FF);
    EXPECT_EQ(packet1.P, packet2.P);
    EXPECT_EQ(packet1.I, packet2.I);
    EXPECT_EQ(packet1.D, packet2.D);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, PID_TUNING)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_pid_tuning_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 77
    };

    mavlink::ardupilotmega::msg::PID_TUNING packet_in{};
    packet_in.axis = 77;
    packet_in.desired = 17.0;
    packet_in.achieved = 45.0;
    packet_in.FF = 73.0;
    packet_in.P = 101.0;
    packet_in.I = 129.0;
    packet_in.D = 157.0;

    mavlink::ardupilotmega::msg::PID_TUNING packet2{};

    mavlink_msg_pid_tuning_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.axis, packet2.axis);
    EXPECT_EQ(packet_in.desired, packet2.desired);
    EXPECT_EQ(packet_in.achieved, packet2.achieved);
    EXPECT_EQ(packet_in.FF, packet2.FF);
    EXPECT_EQ(packet_in.P, packet2.P);
    EXPECT_EQ(packet_in.I, packet2.I);
    EXPECT_EQ(packet_in.D, packet2.D);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DEEPSTALL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DEEPSTALL packet_in{};
    packet_in.landing_lat = 963497464;
    packet_in.landing_lon = 963497672;
    packet_in.path_lat = 963497880;
    packet_in.path_lon = 963498088;
    packet_in.arc_entry_lat = 963498296;
    packet_in.arc_entry_lon = 963498504;
    packet_in.altitude = 185.0;
    packet_in.expected_travel_distance = 213.0;
    packet_in.cross_track_error = 241.0;
    packet_in.stage = 113;

    mavlink::ardupilotmega::msg::DEEPSTALL packet1{};
    mavlink::ardupilotmega::msg::DEEPSTALL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.landing_lat, packet2.landing_lat);
    EXPECT_EQ(packet1.landing_lon, packet2.landing_lon);
    EXPECT_EQ(packet1.path_lat, packet2.path_lat);
    EXPECT_EQ(packet1.path_lon, packet2.path_lon);
    EXPECT_EQ(packet1.arc_entry_lat, packet2.arc_entry_lat);
    EXPECT_EQ(packet1.arc_entry_lon, packet2.arc_entry_lon);
    EXPECT_EQ(packet1.altitude, packet2.altitude);
    EXPECT_EQ(packet1.expected_travel_distance, packet2.expected_travel_distance);
    EXPECT_EQ(packet1.cross_track_error, packet2.cross_track_error);
    EXPECT_EQ(packet1.stage, packet2.stage);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DEEPSTALL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_deepstall_t packet_c {
         963497464, 963497672, 963497880, 963498088, 963498296, 963498504, 185.0, 213.0, 241.0, 113
    };

    mavlink::ardupilotmega::msg::DEEPSTALL packet_in{};
    packet_in.landing_lat = 963497464;
    packet_in.landing_lon = 963497672;
    packet_in.path_lat = 963497880;
    packet_in.path_lon = 963498088;
    packet_in.arc_entry_lat = 963498296;
    packet_in.arc_entry_lon = 963498504;
    packet_in.altitude = 185.0;
    packet_in.expected_travel_distance = 213.0;
    packet_in.cross_track_error = 241.0;
    packet_in.stage = 113;

    mavlink::ardupilotmega::msg::DEEPSTALL packet2{};

    mavlink_msg_deepstall_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.landing_lat, packet2.landing_lat);
    EXPECT_EQ(packet_in.landing_lon, packet2.landing_lon);
    EXPECT_EQ(packet_in.path_lat, packet2.path_lat);
    EXPECT_EQ(packet_in.path_lon, packet2.path_lon);
    EXPECT_EQ(packet_in.arc_entry_lat, packet2.arc_entry_lat);
    EXPECT_EQ(packet_in.arc_entry_lon, packet2.arc_entry_lon);
    EXPECT_EQ(packet_in.altitude, packet2.altitude);
    EXPECT_EQ(packet_in.expected_travel_distance, packet2.expected_travel_distance);
    EXPECT_EQ(packet_in.cross_track_error, packet2.cross_track_error);
    EXPECT_EQ(packet_in.stage, packet2.stage);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GIMBAL_REPORT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GIMBAL_REPORT packet_in{};
    packet_in.target_system = 125;
    packet_in.target_component = 192;
    packet_in.delta_time = 17.0;
    packet_in.delta_angle_x = 45.0;
    packet_in.delta_angle_y = 73.0;
    packet_in.delta_angle_z = 101.0;
    packet_in.delta_velocity_x = 129.0;
    packet_in.delta_velocity_y = 157.0;
    packet_in.delta_velocity_z = 185.0;
    packet_in.joint_roll = 213.0;
    packet_in.joint_el = 241.0;
    packet_in.joint_az = 269.0;

    mavlink::ardupilotmega::msg::GIMBAL_REPORT packet1{};
    mavlink::ardupilotmega::msg::GIMBAL_REPORT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.delta_time, packet2.delta_time);
    EXPECT_EQ(packet1.delta_angle_x, packet2.delta_angle_x);
    EXPECT_EQ(packet1.delta_angle_y, packet2.delta_angle_y);
    EXPECT_EQ(packet1.delta_angle_z, packet2.delta_angle_z);
    EXPECT_EQ(packet1.delta_velocity_x, packet2.delta_velocity_x);
    EXPECT_EQ(packet1.delta_velocity_y, packet2.delta_velocity_y);
    EXPECT_EQ(packet1.delta_velocity_z, packet2.delta_velocity_z);
    EXPECT_EQ(packet1.joint_roll, packet2.joint_roll);
    EXPECT_EQ(packet1.joint_el, packet2.joint_el);
    EXPECT_EQ(packet1.joint_az, packet2.joint_az);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GIMBAL_REPORT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gimbal_report_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 125, 192
    };

    mavlink::ardupilotmega::msg::GIMBAL_REPORT packet_in{};
    packet_in.target_system = 125;
    packet_in.target_component = 192;
    packet_in.delta_time = 17.0;
    packet_in.delta_angle_x = 45.0;
    packet_in.delta_angle_y = 73.0;
    packet_in.delta_angle_z = 101.0;
    packet_in.delta_velocity_x = 129.0;
    packet_in.delta_velocity_y = 157.0;
    packet_in.delta_velocity_z = 185.0;
    packet_in.joint_roll = 213.0;
    packet_in.joint_el = 241.0;
    packet_in.joint_az = 269.0;

    mavlink::ardupilotmega::msg::GIMBAL_REPORT packet2{};

    mavlink_msg_gimbal_report_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.delta_time, packet2.delta_time);
    EXPECT_EQ(packet_in.delta_angle_x, packet2.delta_angle_x);
    EXPECT_EQ(packet_in.delta_angle_y, packet2.delta_angle_y);
    EXPECT_EQ(packet_in.delta_angle_z, packet2.delta_angle_z);
    EXPECT_EQ(packet_in.delta_velocity_x, packet2.delta_velocity_x);
    EXPECT_EQ(packet_in.delta_velocity_y, packet2.delta_velocity_y);
    EXPECT_EQ(packet_in.delta_velocity_z, packet2.delta_velocity_z);
    EXPECT_EQ(packet_in.joint_roll, packet2.joint_roll);
    EXPECT_EQ(packet_in.joint_el, packet2.joint_el);
    EXPECT_EQ(packet_in.joint_az, packet2.joint_az);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GIMBAL_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GIMBAL_CONTROL packet_in{};
    packet_in.target_system = 41;
    packet_in.target_component = 108;
    packet_in.demanded_rate_x = 17.0;
    packet_in.demanded_rate_y = 45.0;
    packet_in.demanded_rate_z = 73.0;

    mavlink::ardupilotmega::msg::GIMBAL_CONTROL packet1{};
    mavlink::ardupilotmega::msg::GIMBAL_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.demanded_rate_x, packet2.demanded_rate_x);
    EXPECT_EQ(packet1.demanded_rate_y, packet2.demanded_rate_y);
    EXPECT_EQ(packet1.demanded_rate_z, packet2.demanded_rate_z);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GIMBAL_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gimbal_control_t packet_c {
         17.0, 45.0, 73.0, 41, 108
    };

    mavlink::ardupilotmega::msg::GIMBAL_CONTROL packet_in{};
    packet_in.target_system = 41;
    packet_in.target_component = 108;
    packet_in.demanded_rate_x = 17.0;
    packet_in.demanded_rate_y = 45.0;
    packet_in.demanded_rate_z = 73.0;

    mavlink::ardupilotmega::msg::GIMBAL_CONTROL packet2{};

    mavlink_msg_gimbal_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.demanded_rate_x, packet2.demanded_rate_x);
    EXPECT_EQ(packet_in.demanded_rate_y, packet2.demanded_rate_y);
    EXPECT_EQ(packet_in.demanded_rate_z, packet2.demanded_rate_z);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GIMBAL_TORQUE_CMD_REPORT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GIMBAL_TORQUE_CMD_REPORT packet_in{};
    packet_in.target_system = 151;
    packet_in.target_component = 218;
    packet_in.rl_torque_cmd = 17235;
    packet_in.el_torque_cmd = 17339;
    packet_in.az_torque_cmd = 17443;

    mavlink::ardupilotmega::msg::GIMBAL_TORQUE_CMD_REPORT packet1{};
    mavlink::ardupilotmega::msg::GIMBAL_TORQUE_CMD_REPORT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.rl_torque_cmd, packet2.rl_torque_cmd);
    EXPECT_EQ(packet1.el_torque_cmd, packet2.el_torque_cmd);
    EXPECT_EQ(packet1.az_torque_cmd, packet2.az_torque_cmd);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GIMBAL_TORQUE_CMD_REPORT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gimbal_torque_cmd_report_t packet_c {
         17235, 17339, 17443, 151, 218
    };

    mavlink::ardupilotmega::msg::GIMBAL_TORQUE_CMD_REPORT packet_in{};
    packet_in.target_system = 151;
    packet_in.target_component = 218;
    packet_in.rl_torque_cmd = 17235;
    packet_in.el_torque_cmd = 17339;
    packet_in.az_torque_cmd = 17443;

    mavlink::ardupilotmega::msg::GIMBAL_TORQUE_CMD_REPORT packet2{};

    mavlink_msg_gimbal_torque_cmd_report_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.rl_torque_cmd, packet2.rl_torque_cmd);
    EXPECT_EQ(packet_in.el_torque_cmd, packet2.el_torque_cmd);
    EXPECT_EQ(packet_in.az_torque_cmd, packet2.az_torque_cmd);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GOPRO_HEARTBEAT)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GOPRO_HEARTBEAT packet_in{};
    packet_in.status = 5;
    packet_in.capture_mode = 72;
    packet_in.flags = 139;

    mavlink::ardupilotmega::msg::GOPRO_HEARTBEAT packet1{};
    mavlink::ardupilotmega::msg::GOPRO_HEARTBEAT packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.status, packet2.status);
    EXPECT_EQ(packet1.capture_mode, packet2.capture_mode);
    EXPECT_EQ(packet1.flags, packet2.flags);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GOPRO_HEARTBEAT)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gopro_heartbeat_t packet_c {
         5, 72, 139
    };

    mavlink::ardupilotmega::msg::GOPRO_HEARTBEAT packet_in{};
    packet_in.status = 5;
    packet_in.capture_mode = 72;
    packet_in.flags = 139;

    mavlink::ardupilotmega::msg::GOPRO_HEARTBEAT packet2{};

    mavlink_msg_gopro_heartbeat_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.status, packet2.status);
    EXPECT_EQ(packet_in.capture_mode, packet2.capture_mode);
    EXPECT_EQ(packet_in.flags, packet2.flags);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GOPRO_GET_REQUEST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GOPRO_GET_REQUEST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.cmd_id = 139;

    mavlink::ardupilotmega::msg::GOPRO_GET_REQUEST packet1{};
    mavlink::ardupilotmega::msg::GOPRO_GET_REQUEST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.cmd_id, packet2.cmd_id);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GOPRO_GET_REQUEST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gopro_get_request_t packet_c {
         5, 72, 139
    };

    mavlink::ardupilotmega::msg::GOPRO_GET_REQUEST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.cmd_id = 139;

    mavlink::ardupilotmega::msg::GOPRO_GET_REQUEST packet2{};

    mavlink_msg_gopro_get_request_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.cmd_id, packet2.cmd_id);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GOPRO_GET_RESPONSE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GOPRO_GET_RESPONSE packet_in{};
    packet_in.cmd_id = 5;
    packet_in.status = 72;
    packet_in.value = {{ 139, 140, 141, 142 }};

    mavlink::ardupilotmega::msg::GOPRO_GET_RESPONSE packet1{};
    mavlink::ardupilotmega::msg::GOPRO_GET_RESPONSE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.cmd_id, packet2.cmd_id);
    EXPECT_EQ(packet1.status, packet2.status);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GOPRO_GET_RESPONSE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gopro_get_response_t packet_c {
         5, 72, { 139, 140, 141, 142 }
    };

    mavlink::ardupilotmega::msg::GOPRO_GET_RESPONSE packet_in{};
    packet_in.cmd_id = 5;
    packet_in.status = 72;
    packet_in.value = {{ 139, 140, 141, 142 }};

    mavlink::ardupilotmega::msg::GOPRO_GET_RESPONSE packet2{};

    mavlink_msg_gopro_get_response_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.cmd_id, packet2.cmd_id);
    EXPECT_EQ(packet_in.status, packet2.status);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GOPRO_SET_REQUEST)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GOPRO_SET_REQUEST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.cmd_id = 139;
    packet_in.value = {{ 206, 207, 208, 209 }};

    mavlink::ardupilotmega::msg::GOPRO_SET_REQUEST packet1{};
    mavlink::ardupilotmega::msg::GOPRO_SET_REQUEST packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.cmd_id, packet2.cmd_id);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GOPRO_SET_REQUEST)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gopro_set_request_t packet_c {
         5, 72, 139, { 206, 207, 208, 209 }
    };

    mavlink::ardupilotmega::msg::GOPRO_SET_REQUEST packet_in{};
    packet_in.target_system = 5;
    packet_in.target_component = 72;
    packet_in.cmd_id = 139;
    packet_in.value = {{ 206, 207, 208, 209 }};

    mavlink::ardupilotmega::msg::GOPRO_SET_REQUEST packet2{};

    mavlink_msg_gopro_set_request_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.cmd_id, packet2.cmd_id);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, GOPRO_SET_RESPONSE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::GOPRO_SET_RESPONSE packet_in{};
    packet_in.cmd_id = 5;
    packet_in.status = 72;

    mavlink::ardupilotmega::msg::GOPRO_SET_RESPONSE packet1{};
    mavlink::ardupilotmega::msg::GOPRO_SET_RESPONSE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.cmd_id, packet2.cmd_id);
    EXPECT_EQ(packet1.status, packet2.status);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, GOPRO_SET_RESPONSE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_gopro_set_response_t packet_c {
         5, 72
    };

    mavlink::ardupilotmega::msg::GOPRO_SET_RESPONSE packet_in{};
    packet_in.cmd_id = 5;
    packet_in.status = 72;

    mavlink::ardupilotmega::msg::GOPRO_SET_RESPONSE packet2{};

    mavlink_msg_gopro_set_response_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.cmd_id, packet2.cmd_id);
    EXPECT_EQ(packet_in.status, packet2.status);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, RPM)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::RPM packet_in{};
    packet_in.rpm1 = 17.0;
    packet_in.rpm2 = 45.0;

    mavlink::ardupilotmega::msg::RPM packet1{};
    mavlink::ardupilotmega::msg::RPM packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.rpm1, packet2.rpm1);
    EXPECT_EQ(packet1.rpm2, packet2.rpm2);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, RPM)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_rpm_t packet_c {
         17.0, 45.0
    };

    mavlink::ardupilotmega::msg::RPM packet_in{};
    packet_in.rpm1 = 17.0;
    packet_in.rpm2 = 45.0;

    mavlink::ardupilotmega::msg::RPM packet2{};

    mavlink_msg_rpm_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.rpm1, packet2.rpm1);
    EXPECT_EQ(packet_in.rpm2, packet2.rpm2);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DEVICE_OP_READ)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DEVICE_OP_READ packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.request_id = 963497464;
    packet_in.bustype = 151;
    packet_in.bus = 218;
    packet_in.address = 29;
    packet_in.busname = to_char_array("JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV");
    packet_in.regstart = 216;
    packet_in.count = 27;

    mavlink::ardupilotmega::msg::DEVICE_OP_READ packet1{};
    mavlink::ardupilotmega::msg::DEVICE_OP_READ packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.bustype, packet2.bustype);
    EXPECT_EQ(packet1.bus, packet2.bus);
    EXPECT_EQ(packet1.address, packet2.address);
    EXPECT_EQ(packet1.busname, packet2.busname);
    EXPECT_EQ(packet1.regstart, packet2.regstart);
    EXPECT_EQ(packet1.count, packet2.count);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DEVICE_OP_READ)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_device_op_read_t packet_c {
         963497464, 17, 84, 151, 218, 29, "JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV", 216, 27
    };

    mavlink::ardupilotmega::msg::DEVICE_OP_READ packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.request_id = 963497464;
    packet_in.bustype = 151;
    packet_in.bus = 218;
    packet_in.address = 29;
    packet_in.busname = to_char_array("JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV");
    packet_in.regstart = 216;
    packet_in.count = 27;

    mavlink::ardupilotmega::msg::DEVICE_OP_READ packet2{};

    mavlink_msg_device_op_read_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.bustype, packet2.bustype);
    EXPECT_EQ(packet_in.bus, packet2.bus);
    EXPECT_EQ(packet_in.address, packet2.address);
    EXPECT_EQ(packet_in.busname, packet2.busname);
    EXPECT_EQ(packet_in.regstart, packet2.regstart);
    EXPECT_EQ(packet_in.count, packet2.count);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DEVICE_OP_READ_REPLY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DEVICE_OP_READ_REPLY packet_in{};
    packet_in.request_id = 963497464;
    packet_in.result = 17;
    packet_in.regstart = 84;
    packet_in.count = 151;
    packet_in.data = {{ 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89 }};

    mavlink::ardupilotmega::msg::DEVICE_OP_READ_REPLY packet1{};
    mavlink::ardupilotmega::msg::DEVICE_OP_READ_REPLY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.result, packet2.result);
    EXPECT_EQ(packet1.regstart, packet2.regstart);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DEVICE_OP_READ_REPLY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_device_op_read_reply_t packet_c {
         963497464, 17, 84, 151, { 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89 }
    };

    mavlink::ardupilotmega::msg::DEVICE_OP_READ_REPLY packet_in{};
    packet_in.request_id = 963497464;
    packet_in.result = 17;
    packet_in.regstart = 84;
    packet_in.count = 151;
    packet_in.data = {{ 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89 }};

    mavlink::ardupilotmega::msg::DEVICE_OP_READ_REPLY packet2{};

    mavlink_msg_device_op_read_reply_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.result, packet2.result);
    EXPECT_EQ(packet_in.regstart, packet2.regstart);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DEVICE_OP_WRITE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.request_id = 963497464;
    packet_in.bustype = 151;
    packet_in.bus = 218;
    packet_in.address = 29;
    packet_in.busname = to_char_array("JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV");
    packet_in.regstart = 216;
    packet_in.count = 27;
    packet_in.data = {{ 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221 }};

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE packet1{};
    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.target_system, packet2.target_system);
    EXPECT_EQ(packet1.target_component, packet2.target_component);
    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.bustype, packet2.bustype);
    EXPECT_EQ(packet1.bus, packet2.bus);
    EXPECT_EQ(packet1.address, packet2.address);
    EXPECT_EQ(packet1.busname, packet2.busname);
    EXPECT_EQ(packet1.regstart, packet2.regstart);
    EXPECT_EQ(packet1.count, packet2.count);
    EXPECT_EQ(packet1.data, packet2.data);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DEVICE_OP_WRITE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_device_op_write_t packet_c {
         963497464, 17, 84, 151, 218, 29, "JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV", 216, 27, { 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221 }
    };

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE packet_in{};
    packet_in.target_system = 17;
    packet_in.target_component = 84;
    packet_in.request_id = 963497464;
    packet_in.bustype = 151;
    packet_in.bus = 218;
    packet_in.address = 29;
    packet_in.busname = to_char_array("JKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV");
    packet_in.regstart = 216;
    packet_in.count = 27;
    packet_in.data = {{ 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221 }};

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE packet2{};

    mavlink_msg_device_op_write_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.target_system, packet2.target_system);
    EXPECT_EQ(packet_in.target_component, packet2.target_component);
    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.bustype, packet2.bustype);
    EXPECT_EQ(packet_in.bus, packet2.bus);
    EXPECT_EQ(packet_in.address, packet2.address);
    EXPECT_EQ(packet_in.busname, packet2.busname);
    EXPECT_EQ(packet_in.regstart, packet2.regstart);
    EXPECT_EQ(packet_in.count, packet2.count);
    EXPECT_EQ(packet_in.data, packet2.data);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, DEVICE_OP_WRITE_REPLY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE_REPLY packet_in{};
    packet_in.request_id = 963497464;
    packet_in.result = 17;

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE_REPLY packet1{};
    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE_REPLY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.result, packet2.result);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, DEVICE_OP_WRITE_REPLY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_device_op_write_reply_t packet_c {
         963497464, 17
    };

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE_REPLY packet_in{};
    packet_in.request_id = 963497464;
    packet_in.result = 17;

    mavlink::ardupilotmega::msg::DEVICE_OP_WRITE_REPLY packet2{};

    mavlink_msg_device_op_write_reply_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.result, packet2.result);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, ADAP_TUNING)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::ADAP_TUNING packet_in{};
    packet_in.axis = 149;
    packet_in.desired = 17.0;
    packet_in.achieved = 45.0;
    packet_in.error = 73.0;
    packet_in.theta = 101.0;
    packet_in.omega = 129.0;
    packet_in.sigma = 157.0;
    packet_in.theta_dot = 185.0;
    packet_in.omega_dot = 213.0;
    packet_in.sigma_dot = 241.0;
    packet_in.f = 269.0;
    packet_in.f_dot = 297.0;
    packet_in.u = 325.0;

    mavlink::ardupilotmega::msg::ADAP_TUNING packet1{};
    mavlink::ardupilotmega::msg::ADAP_TUNING packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.axis, packet2.axis);
    EXPECT_EQ(packet1.desired, packet2.desired);
    EXPECT_EQ(packet1.achieved, packet2.achieved);
    EXPECT_EQ(packet1.error, packet2.error);
    EXPECT_EQ(packet1.theta, packet2.theta);
    EXPECT_EQ(packet1.omega, packet2.omega);
    EXPECT_EQ(packet1.sigma, packet2.sigma);
    EXPECT_EQ(packet1.theta_dot, packet2.theta_dot);
    EXPECT_EQ(packet1.omega_dot, packet2.omega_dot);
    EXPECT_EQ(packet1.sigma_dot, packet2.sigma_dot);
    EXPECT_EQ(packet1.f, packet2.f);
    EXPECT_EQ(packet1.f_dot, packet2.f_dot);
    EXPECT_EQ(packet1.u, packet2.u);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, ADAP_TUNING)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_adap_tuning_t packet_c {
         17.0, 45.0, 73.0, 101.0, 129.0, 157.0, 185.0, 213.0, 241.0, 269.0, 297.0, 325.0, 149
    };

    mavlink::ardupilotmega::msg::ADAP_TUNING packet_in{};
    packet_in.axis = 149;
    packet_in.desired = 17.0;
    packet_in.achieved = 45.0;
    packet_in.error = 73.0;
    packet_in.theta = 101.0;
    packet_in.omega = 129.0;
    packet_in.sigma = 157.0;
    packet_in.theta_dot = 185.0;
    packet_in.omega_dot = 213.0;
    packet_in.sigma_dot = 241.0;
    packet_in.f = 269.0;
    packet_in.f_dot = 297.0;
    packet_in.u = 325.0;

    mavlink::ardupilotmega::msg::ADAP_TUNING packet2{};

    mavlink_msg_adap_tuning_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.axis, packet2.axis);
    EXPECT_EQ(packet_in.desired, packet2.desired);
    EXPECT_EQ(packet_in.achieved, packet2.achieved);
    EXPECT_EQ(packet_in.error, packet2.error);
    EXPECT_EQ(packet_in.theta, packet2.theta);
    EXPECT_EQ(packet_in.omega, packet2.omega);
    EXPECT_EQ(packet_in.sigma, packet2.sigma);
    EXPECT_EQ(packet_in.theta_dot, packet2.theta_dot);
    EXPECT_EQ(packet_in.omega_dot, packet2.omega_dot);
    EXPECT_EQ(packet_in.sigma_dot, packet2.sigma_dot);
    EXPECT_EQ(packet_in.f, packet2.f);
    EXPECT_EQ(packet_in.f_dot, packet2.f_dot);
    EXPECT_EQ(packet_in.u, packet2.u);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, VISION_POSITION_DELTA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::VISION_POSITION_DELTA packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.time_delta_usec = 93372036854776311ULL;
    packet_in.angle_delta = {{ 129.0, 130.0, 131.0 }};
    packet_in.position_delta = {{ 213.0, 214.0, 215.0 }};
    packet_in.confidence = 297.0;

    mavlink::ardupilotmega::msg::VISION_POSITION_DELTA packet1{};
    mavlink::ardupilotmega::msg::VISION_POSITION_DELTA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.time_delta_usec, packet2.time_delta_usec);
    EXPECT_EQ(packet1.angle_delta, packet2.angle_delta);
    EXPECT_EQ(packet1.position_delta, packet2.position_delta);
    EXPECT_EQ(packet1.confidence, packet2.confidence);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, VISION_POSITION_DELTA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_vision_position_delta_t packet_c {
         93372036854775807ULL, 93372036854776311ULL, { 129.0, 130.0, 131.0 }, { 213.0, 214.0, 215.0 }, 297.0
    };

    mavlink::ardupilotmega::msg::VISION_POSITION_DELTA packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.time_delta_usec = 93372036854776311ULL;
    packet_in.angle_delta = {{ 129.0, 130.0, 131.0 }};
    packet_in.position_delta = {{ 213.0, 214.0, 215.0 }};
    packet_in.confidence = 297.0;

    mavlink::ardupilotmega::msg::VISION_POSITION_DELTA packet2{};

    mavlink_msg_vision_position_delta_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.time_delta_usec, packet2.time_delta_usec);
    EXPECT_EQ(packet_in.angle_delta, packet2.angle_delta);
    EXPECT_EQ(packet_in.position_delta, packet2.position_delta);
    EXPECT_EQ(packet_in.confidence, packet2.confidence);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, AOA_SSA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::AOA_SSA packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.AOA = 73.0;
    packet_in.SSA = 101.0;

    mavlink::ardupilotmega::msg::AOA_SSA packet1{};
    mavlink::ardupilotmega::msg::AOA_SSA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.time_usec, packet2.time_usec);
    EXPECT_EQ(packet1.AOA, packet2.AOA);
    EXPECT_EQ(packet1.SSA, packet2.SSA);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, AOA_SSA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_aoa_ssa_t packet_c {
         93372036854775807ULL, 73.0, 101.0
    };

    mavlink::ardupilotmega::msg::AOA_SSA packet_in{};
    packet_in.time_usec = 93372036854775807ULL;
    packet_in.AOA = 73.0;
    packet_in.SSA = 101.0;

    mavlink::ardupilotmega::msg::AOA_SSA packet2{};

    mavlink_msg_aoa_ssa_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.time_usec, packet2.time_usec);
    EXPECT_EQ(packet_in.AOA, packet2.AOA);
    EXPECT_EQ(packet_in.SSA, packet2.SSA);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, ESC_TELEMETRY_1_TO_4)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_1_TO_4 packet_in{};
    packet_in.temperature = {{ 125, 126, 127, 128 }};
    packet_in.voltage = {{ 17235, 17236, 17237, 17238 }};
    packet_in.current = {{ 17651, 17652, 17653, 17654 }};
    packet_in.totalcurrent = {{ 18067, 18068, 18069, 18070 }};
    packet_in.rpm = {{ 18483, 18484, 18485, 18486 }};
    packet_in.count = {{ 18899, 18900, 18901, 18902 }};

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_1_TO_4 packet1{};
    mavlink::ardupilotmega::msg::ESC_TELEMETRY_1_TO_4 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.voltage, packet2.voltage);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.totalcurrent, packet2.totalcurrent);
    EXPECT_EQ(packet1.rpm, packet2.rpm);
    EXPECT_EQ(packet1.count, packet2.count);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, ESC_TELEMETRY_1_TO_4)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_esc_telemetry_1_to_4_t packet_c {
         { 17235, 17236, 17237, 17238 }, { 17651, 17652, 17653, 17654 }, { 18067, 18068, 18069, 18070 }, { 18483, 18484, 18485, 18486 }, { 18899, 18900, 18901, 18902 }, { 125, 126, 127, 128 }
    };

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_1_TO_4 packet_in{};
    packet_in.temperature = {{ 125, 126, 127, 128 }};
    packet_in.voltage = {{ 17235, 17236, 17237, 17238 }};
    packet_in.current = {{ 17651, 17652, 17653, 17654 }};
    packet_in.totalcurrent = {{ 18067, 18068, 18069, 18070 }};
    packet_in.rpm = {{ 18483, 18484, 18485, 18486 }};
    packet_in.count = {{ 18899, 18900, 18901, 18902 }};

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_1_TO_4 packet2{};

    mavlink_msg_esc_telemetry_1_to_4_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.voltage, packet2.voltage);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.totalcurrent, packet2.totalcurrent);
    EXPECT_EQ(packet_in.rpm, packet2.rpm);
    EXPECT_EQ(packet_in.count, packet2.count);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, ESC_TELEMETRY_5_TO_8)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_5_TO_8 packet_in{};
    packet_in.temperature = {{ 125, 126, 127, 128 }};
    packet_in.voltage = {{ 17235, 17236, 17237, 17238 }};
    packet_in.current = {{ 17651, 17652, 17653, 17654 }};
    packet_in.totalcurrent = {{ 18067, 18068, 18069, 18070 }};
    packet_in.rpm = {{ 18483, 18484, 18485, 18486 }};
    packet_in.count = {{ 18899, 18900, 18901, 18902 }};

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_5_TO_8 packet1{};
    mavlink::ardupilotmega::msg::ESC_TELEMETRY_5_TO_8 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.voltage, packet2.voltage);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.totalcurrent, packet2.totalcurrent);
    EXPECT_EQ(packet1.rpm, packet2.rpm);
    EXPECT_EQ(packet1.count, packet2.count);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, ESC_TELEMETRY_5_TO_8)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_esc_telemetry_5_to_8_t packet_c {
         { 17235, 17236, 17237, 17238 }, { 17651, 17652, 17653, 17654 }, { 18067, 18068, 18069, 18070 }, { 18483, 18484, 18485, 18486 }, { 18899, 18900, 18901, 18902 }, { 125, 126, 127, 128 }
    };

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_5_TO_8 packet_in{};
    packet_in.temperature = {{ 125, 126, 127, 128 }};
    packet_in.voltage = {{ 17235, 17236, 17237, 17238 }};
    packet_in.current = {{ 17651, 17652, 17653, 17654 }};
    packet_in.totalcurrent = {{ 18067, 18068, 18069, 18070 }};
    packet_in.rpm = {{ 18483, 18484, 18485, 18486 }};
    packet_in.count = {{ 18899, 18900, 18901, 18902 }};

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_5_TO_8 packet2{};

    mavlink_msg_esc_telemetry_5_to_8_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.voltage, packet2.voltage);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.totalcurrent, packet2.totalcurrent);
    EXPECT_EQ(packet_in.rpm, packet2.rpm);
    EXPECT_EQ(packet_in.count, packet2.count);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(ardupilotmega, ESC_TELEMETRY_9_TO_12)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_9_TO_12 packet_in{};
    packet_in.temperature = {{ 125, 126, 127, 128 }};
    packet_in.voltage = {{ 17235, 17236, 17237, 17238 }};
    packet_in.current = {{ 17651, 17652, 17653, 17654 }};
    packet_in.totalcurrent = {{ 18067, 18068, 18069, 18070 }};
    packet_in.rpm = {{ 18483, 18484, 18485, 18486 }};
    packet_in.count = {{ 18899, 18900, 18901, 18902 }};

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_9_TO_12 packet1{};
    mavlink::ardupilotmega::msg::ESC_TELEMETRY_9_TO_12 packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.temperature, packet2.temperature);
    EXPECT_EQ(packet1.voltage, packet2.voltage);
    EXPECT_EQ(packet1.current, packet2.current);
    EXPECT_EQ(packet1.totalcurrent, packet2.totalcurrent);
    EXPECT_EQ(packet1.rpm, packet2.rpm);
    EXPECT_EQ(packet1.count, packet2.count);
}

#ifdef TEST_INTEROP
TEST(ardupilotmega_interop, ESC_TELEMETRY_9_TO_12)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_esc_telemetry_9_to_12_t packet_c {
         { 17235, 17236, 17237, 17238 }, { 17651, 17652, 17653, 17654 }, { 18067, 18068, 18069, 18070 }, { 18483, 18484, 18485, 18486 }, { 18899, 18900, 18901, 18902 }, { 125, 126, 127, 128 }
    };

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_9_TO_12 packet_in{};
    packet_in.temperature = {{ 125, 126, 127, 128 }};
    packet_in.voltage = {{ 17235, 17236, 17237, 17238 }};
    packet_in.current = {{ 17651, 17652, 17653, 17654 }};
    packet_in.totalcurrent = {{ 18067, 18068, 18069, 18070 }};
    packet_in.rpm = {{ 18483, 18484, 18485, 18486 }};
    packet_in.count = {{ 18899, 18900, 18901, 18902 }};

    mavlink::ardupilotmega::msg::ESC_TELEMETRY_9_TO_12 packet2{};

    mavlink_msg_esc_telemetry_9_to_12_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.temperature, packet2.temperature);
    EXPECT_EQ(packet_in.voltage, packet2.voltage);
    EXPECT_EQ(packet_in.current, packet2.current);
    EXPECT_EQ(packet_in.totalcurrent, packet2.totalcurrent);
    EXPECT_EQ(packet_in.rpm, packet2.rpm);
    EXPECT_EQ(packet_in.count, packet2.count);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif
