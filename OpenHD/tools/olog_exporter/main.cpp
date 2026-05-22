#include <chrono>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include "mavlink/v2.0/openhd/mavlink.h"
}

namespace {

constexpr uint8_t kOpenHdGroundSysId = 100;
constexpr uint8_t kOpenHdAirSysId = 101;
constexpr uint8_t kFlightControllerSysId = 1;
constexpr uint8_t kOpenHdComponentId = 190;
constexpr uint8_t kFlightControllerComponentId = 1;
constexpr double kPi = 3.14159265358979323846;

struct Options {
  std::string output = "openhd_sample.oLog";
  std::string debug_output;
  int seconds = 5;
  int rate_hz = 5;
};

struct DroneSample {
  int elapsed_ms{};
  float roll_rad{};
  float pitch_rad{};
  float yaw_rad{};
  float airspeed_mps{};
  float groundspeed_mps{};
  float altitude_m{};
  float climb_mps{};
  int heading_deg{};
  int throttle_percent{};
  int32_t lat_e7{};
  int32_t lon_e7{};
  int satellites{};
  int battery_mv{};
  int current_ca{};
  int battery_remaining{};
  float home_distance_m{};
};

void print_usage(const char* program) {
  std::cerr << "Usage: " << program
            << " [--output <file.oLog>] [--debug-output <file.debug.jsonl>]"
            << " [--seconds <n>] [--rate-hz <n>]\n";
}

bool parse_int(const char* value, int& target) {
  char* end = nullptr;
  const long parsed = std::strtol(value, &end, 10);
  if (end == value || *end != '\0') {
    return false;
  }
  target = static_cast<int>(parsed);
  return true;
}

Options parse_args(int argc, char** argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if ((arg == "--output" || arg == "-o") && i + 1 < argc) {
      options.output = argv[++i];
    } else if (arg == "--debug-output" && i + 1 < argc) {
      options.debug_output = argv[++i];
    } else if (arg == "--seconds" && i + 1 < argc) {
      if (!parse_int(argv[++i], options.seconds)) {
        throw std::runtime_error("Invalid --seconds value");
      }
    } else if (arg == "--rate-hz" && i + 1 < argc) {
      if (!parse_int(argv[++i], options.rate_hz)) {
        throw std::runtime_error("Invalid --rate-hz value");
      }
    } else if (arg == "--help" || arg == "-h") {
      print_usage(argv[0]);
      std::exit(0);
    } else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }

  if (options.seconds < 1) {
    options.seconds = 1;
  }
  if (options.rate_hz < 1) {
    options.rate_hz = 1;
  }
  if (options.debug_output.empty()) {
    options.debug_output = options.output + ".debug.jsonl";
  }
  return options;
}

std::string iso_utc(std::chrono::system_clock::time_point tp) {
  const auto time = std::chrono::system_clock::to_time_t(tp);
  std::tm tm{};
  gmtime_r(&time, &tm);
  const auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) %
      std::chrono::seconds(1);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
  oss << '.' << std::setw(3) << std::setfill('0') << ms.count() << 'Z';
  return oss.str();
}

void write_debug_line(std::ofstream& debug, const std::string& json) {
  debug << json << '\n';
  debug.flush();
}

void write_message(std::ofstream& stream, const mavlink_message_t& message,
                   int& packet_index) {
  uint8_t buffer[MAVLINK_MAX_PACKET_LEN]{};
  const uint16_t length = mavlink_msg_to_send_buffer(buffer, &message);
  stream.write(reinterpret_cast<const char*>(buffer), length);
  ++packet_index;
}

void write_openhd_heartbeat(std::ofstream& stream, uint8_t sysid,
                            int& packet_index) {
  mavlink_message_t msg{};
  mavlink_msg_heartbeat_pack(sysid, kOpenHdComponentId, &msg, MAV_TYPE_GENERIC,
                             MAV_AUTOPILOT_INVALID, 0, 0,
                             MAV_STATE_ACTIVE);
  write_message(stream, msg, packet_index);
}

void write_fc_heartbeat(std::ofstream& stream, int& packet_index) {
  mavlink_message_t msg{};
  mavlink_msg_heartbeat_pack(kFlightControllerSysId, kFlightControllerComponentId,
                             &msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC,
                             MAV_MODE_FLAG_SAFETY_ARMED, 0, MAV_STATE_ACTIVE);
  write_message(stream, msg, packet_index);
}

DroneSample make_drone_sample(int tick, int elapsed_ms) {
  const double seconds = static_cast<double>(elapsed_ms) / 1000.0;
  const double turn = seconds * 0.10;
  const double base_lat = 48.137154;
  const double base_lon = 11.576124;
  const double north_m = 35.0 * std::sin(turn);
  const double east_m = 55.0 * std::cos(turn * 0.8);
  const double lat = base_lat + north_m / 111'111.0;
  const double lon =
      base_lon + east_m / (111'111.0 * std::cos(base_lat * kPi / 180.0));
  const auto heading = static_cast<int>(std::fmod(68.0 + seconds * 4.0, 360.0));
  const float groundspeed = 21.0F + static_cast<float>(std::sin(seconds * 0.36) * 4.0);
  const float altitude = 82.0F + static_cast<float>(std::sin(seconds * 0.20) * 18.0);
  const int battery_mv = 16'350 - tick * 12;

  return DroneSample {
      .elapsed_ms = elapsed_ms,
      .roll_rad = static_cast<float>(std::sin(seconds * 0.70) * 0.35),
      .pitch_rad = static_cast<float>(std::sin(seconds * 0.47 + 0.4) * 0.12),
      .yaw_rad = static_cast<float>(heading) * static_cast<float>(kPi / 180.0),
      .airspeed_mps = groundspeed + 1.8F,
      .groundspeed_mps = groundspeed,
      .altitude_m = altitude,
      .climb_mps = static_cast<float>(std::cos(seconds * 0.20) * 1.1),
      .heading_deg = heading,
      .throttle_percent = 47 + static_cast<int>(std::round(std::sin(seconds * 0.30) * 8.0)),
      .lat_e7 = static_cast<int32_t>(std::llround(lat * 10'000'000.0)),
      .lon_e7 = static_cast<int32_t>(std::llround(lon * 10'000'000.0)),
      .satellites = 14 + static_cast<int>(tick % 4),
      .battery_mv = battery_mv,
      .current_ca = 430 + static_cast<int>(std::round(std::sin(seconds * 0.75) * 110.0)),
      .battery_remaining = std::max(0, 96 - tick / 8),
      .home_distance_m = static_cast<float>(std::sqrt((north_m * north_m) + (east_m * east_m))),
  };
}

void write_drone_messages(std::ofstream& stream, int tick, const DroneSample& sample,
                          int& packet_index) {
  mavlink_message_t msg{};
  const uint32_t time_boot_ms = static_cast<uint32_t>(sample.elapsed_ms);

  mavlink_msg_attitude_pack(
      kFlightControllerSysId, kFlightControllerComponentId, &msg, time_boot_ms,
      sample.roll_rad, sample.pitch_rad, sample.yaw_rad, 0.02F, 0.01F, 0.015F);
  write_message(stream, msg, packet_index);

  mavlink_msg_global_position_int_pack(
      kFlightControllerSysId, kFlightControllerComponentId, &msg, time_boot_ms,
      sample.lat_e7, sample.lon_e7,
      static_cast<int32_t>(std::llround(sample.altitude_m * 1000.0F)),
      static_cast<int32_t>(std::llround((sample.altitude_m - 42.0F) * 1000.0F)),
      static_cast<int16_t>(std::llround(sample.groundspeed_mps * 85.0F)),
      static_cast<int16_t>(std::llround(sample.groundspeed_mps * 52.0F)),
      static_cast<int16_t>(std::llround(-sample.climb_mps * 100.0F)),
      static_cast<uint16_t>(sample.heading_deg * 100));
  write_message(stream, msg, packet_index);

  mavlink_msg_gps_raw_int_pack(
      kFlightControllerSysId, kFlightControllerComponentId, &msg,
      static_cast<uint64_t>(sample.elapsed_ms) * 1000ULL, 3, sample.lat_e7,
      sample.lon_e7, static_cast<int32_t>(std::llround(sample.altitude_m * 1000.0F)),
      85, 120, static_cast<uint16_t>(std::llround(sample.groundspeed_mps * 100.0F)),
      static_cast<uint16_t>(sample.heading_deg * 100), static_cast<uint8_t>(sample.satellites),
      static_cast<int32_t>(std::llround((sample.altitude_m + 47.0F) * 1000.0F)),
      650, 900, 120, 150, 0);
  write_message(stream, msg, packet_index);

  mavlink_msg_vfr_hud_pack(
      kFlightControllerSysId, kFlightControllerComponentId, &msg,
      sample.airspeed_mps, sample.groundspeed_mps,
      static_cast<int16_t>(sample.heading_deg),
      static_cast<uint16_t>(sample.throttle_percent), sample.altitude_m,
      sample.climb_mps);
  write_message(stream, msg, packet_index);

  mavlink_msg_sys_status_pack(
      kFlightControllerSysId, kFlightControllerComponentId, &msg, 0, 0, 0, 420,
      static_cast<uint16_t>(sample.battery_mv),
      static_cast<int16_t>(sample.current_ca),
      static_cast<int8_t>(sample.battery_remaining), 0, 0, 0, 0, 0, 0, 0, 0,
      0);
  write_message(stream, msg, packet_index);

  std::array<uint16_t, 10> cell_voltages{};
  cell_voltages.fill(UINT16_MAX);
  for (std::size_t i = 0; i < 4; ++i) {
    cell_voltages[i] = static_cast<uint16_t>(sample.battery_mv / 4);
  }
  std::array<uint16_t, 4> ext_voltages{};
  ext_voltages.fill(0);
  mavlink_msg_battery_status_pack(
      kFlightControllerSysId, kFlightControllerComponentId, &msg, 0,
      MAV_BATTERY_FUNCTION_ALL, MAV_BATTERY_TYPE_LIPO, 3200,
      cell_voltages.data(), static_cast<int16_t>(sample.current_ca), 150 + tick,
      -1, static_cast<int8_t>(sample.battery_remaining), 900,
      MAV_BATTERY_CHARGE_STATE_OK, ext_voltages.data(), MAV_BATTERY_MODE_UNKNOWN,
      0);
  write_message(stream, msg, packet_index);
}

void write_openhd_version(std::ofstream& stream, uint8_t sysid,
                          int& packet_index) {
  mavlink_message_t msg{};
  mavlink_msg_openhd_version_message_pack(sysid, kOpenHdComponentId, &msg, 2, 6,
                                          0, 3, 0);
  write_message(stream, msg, packet_index);
}

void write_telemetry_stats(std::ofstream& stream, uint8_t sysid, int tick,
                           int& packet_index) {
  mavlink_message_t msg{};
  mavlink_msg_openhd_stats_telemetry_pack(
      sysid, kOpenHdComponentId, &msg, static_cast<int16_t>(110 + tick),
      static_cast<int16_t>(95 + tick), 4'000'000 + tick * 12'000,
      3'700'000 + tick * 10'000, static_cast<int16_t>(tick % 6), 0, 0, 0);
  write_message(stream, msg, packet_index);
}

void write_wifi_card_stats(std::ofstream& stream, uint8_t sysid, int tick,
                           int& packet_index) {
  mavlink_message_t msg{};
  mavlink_msg_openhd_stats_monitor_mode_wifi_card_pack(
      sysid, kOpenHdComponentId, &msg, 0, 1, 1,
      static_cast<int8_t>(-43 - (tick % 5)), static_cast<int8_t>(-42),
      static_cast<int8_t>(-47), static_cast<int8_t>(-93),
      static_cast<int8_t>(-94), static_cast<int8_t>(-92),
      static_cast<int8_t>(78), static_cast<int8_t>(80),
      static_cast<int8_t>(73), 20, 20, 10, 1200 + tick * 3,
      1180 + tick * 2, static_cast<int8_t>(tick % 4), 0,
      static_cast<int8_t>(32), static_cast<int8_t>(29),
      static_cast<int8_t>(46), 0, 0, 0);
  write_message(stream, msg, packet_index);
}

void write_video_air_stats(std::ofstream& stream, int tick, int& packet_index) {
  mavlink_message_t msg{};
  mavlink_msg_openhd_stats_wb_video_air_pack(
      kOpenHdAirSysId, kOpenHdComponentId, &msg, 0, 8500,
      7'900'000 + tick * 20'000, 8'300'000 + tick * 25'000, 820 + tick,
      tick / 3, 20, 0, 0, 0);
  write_message(stream, msg, packet_index);
}

std::string replay_json_line(int tick, int first_packet_index, int packet_count,
                             int elapsed_ms,
                             const std::chrono::system_clock::time_point& start,
                             const DroneSample& drone) {
  const auto timestamp = start + std::chrono::milliseconds(elapsed_ms);
  const int ground_tx_pps = 110 + tick;
  const int ground_rx_pps = 95 + tick;
  const int ground_tx_bps = 4'000'000 + tick * 12'000;
  const int ground_rx_bps = 3'700'000 + tick * 10'000;
  const int packet_loss = tick % 6;
  const int air_bitrate = 7'900'000 + tick * 20'000;
  const int injected_bitrate = 8'300'000 + tick * 25'000;
  const int injected_pps = 820 + tick;
  const int dropped_frames = tick / 3;
  const int rx_rssi = -43 - (tick % 5);
  const int link_quality = 78;
  const int snr = 32;
  const int card_temp = 46;

  std::ostringstream oss;
  oss << "{\"schema\":\"openhd-osd-debug-v1\""
      << ",\"tick\":" << tick
      << ",\"elapsed_ms\":" << elapsed_ms
      << ",\"datetime_utc\":\"" << iso_utc(timestamp) << "\""
      << ",\"packet_first\":" << first_packet_index
      << ",\"packet_count\":" << packet_count
      << ",\"variables\":{"
      << "\"OpenHD Ground.OPENHD_STATS_TELEMETRY.curr_tx_pps\":" << ground_tx_pps
      << ",\"OpenHD Ground.OPENHD_STATS_TELEMETRY.curr_rx_pps\":" << ground_rx_pps
      << ",\"OpenHD Ground.OPENHD_STATS_TELEMETRY.curr_tx_bps\":" << ground_tx_bps
      << ",\"OpenHD Ground.OPENHD_STATS_TELEMETRY.curr_rx_bps\":" << ground_rx_bps
      << ",\"OpenHD Ground.OPENHD_STATS_TELEMETRY.curr_rx_packet_loss_perc\":" << packet_loss
      << ",\"OpenHD Ground.OPENHD_STATS_MONITOR_MODE_WIFI_CARD.rx_rssi\":" << rx_rssi
      << ",\"OpenHD Ground.OPENHD_STATS_MONITOR_MODE_WIFI_CARD.rx_signal_quality_adapter\":" << link_quality
      << ",\"OpenHD Ground.OPENHD_STATS_MONITOR_MODE_WIFI_CARD.rx_snr_antenna1\":" << snr
      << ",\"OpenHD Ground.OPENHD_STATS_MONITOR_MODE_WIFI_CARD.card_temperature\":" << card_temp
      << ",\"OpenHD Air.OPENHD_STATS_WB_VIDEO_AIR.curr_measured_encoder_bitrate\":" << air_bitrate
      << ",\"OpenHD Air.OPENHD_STATS_WB_VIDEO_AIR.curr_injected_bitrate\":" << injected_bitrate
      << ",\"OpenHD Air.OPENHD_STATS_WB_VIDEO_AIR.curr_injected_pps\":" << injected_pps
      << ",\"OpenHD Air.OPENHD_STATS_WB_VIDEO_AIR.curr_dropped_frames\":" << dropped_frames
      << ",\"Drone.ATTITUDE.roll\":" << drone.roll_rad
      << ",\"Drone.ATTITUDE.pitch\":" << drone.pitch_rad
      << ",\"Drone.ATTITUDE.yaw\":" << drone.yaw_rad
      << ",\"Drone.VFR_HUD.airspeed\":" << drone.airspeed_mps
      << ",\"Drone.VFR_HUD.groundspeed\":" << drone.groundspeed_mps
      << ",\"Drone.VFR_HUD.heading\":" << drone.heading_deg
      << ",\"Drone.VFR_HUD.throttle\":" << drone.throttle_percent
      << ",\"Drone.VFR_HUD.alt\":" << drone.altitude_m
      << ",\"Drone.VFR_HUD.climb\":" << drone.climb_mps
      << ",\"Drone.GLOBAL_POSITION_INT.lat\":" << drone.lat_e7
      << ",\"Drone.GLOBAL_POSITION_INT.lon\":" << drone.lon_e7
      << ",\"Drone.GPS_RAW_INT.satellites_visible\":" << drone.satellites
      << ",\"Drone.SYS_STATUS.voltage_battery\":" << drone.battery_mv
      << ",\"Drone.SYS_STATUS.current_battery\":" << drone.current_ca
      << ",\"Drone.SYS_STATUS.battery_remaining\":" << drone.battery_remaining
      << "},\"osd\":{"
      << "\"link_quality\":" << link_quality
      << ",\"rssi_dbm\":" << rx_rssi
      << ",\"snr_db\":" << snr
      << ",\"air_video_bitrate_bps\":" << air_bitrate
      << ",\"injected_bitrate_bps\":" << injected_bitrate
      << ",\"tx_pps\":" << ground_tx_pps
      << ",\"rx_pps\":" << ground_rx_pps
      << ",\"packet_loss_percent\":" << packet_loss
      << ",\"dropped_frames\":" << dropped_frames
      << ",\"card_temperature_c\":" << card_temp
      << "},\"drone\":{"
      << "\"roll_deg\":" << (drone.roll_rad * 180.0F / static_cast<float>(kPi))
      << ",\"pitch_deg\":" << (drone.pitch_rad * 180.0F / static_cast<float>(kPi))
      << ",\"yaw_deg\":" << (drone.yaw_rad * 180.0F / static_cast<float>(kPi))
      << ",\"airspeed_mps\":" << drone.airspeed_mps
      << ",\"groundspeed_mps\":" << drone.groundspeed_mps
      << ",\"altitude_m\":" << drone.altitude_m
      << ",\"climb_mps\":" << drone.climb_mps
      << ",\"heading_deg\":" << drone.heading_deg
      << ",\"throttle_percent\":" << drone.throttle_percent
      << ",\"lat_e7\":" << drone.lat_e7
      << ",\"lon_e7\":" << drone.lon_e7
      << ",\"satellites\":" << drone.satellites
      << ",\"battery_voltage_v\":" << (static_cast<float>(drone.battery_mv) / 1000.0F)
      << ",\"battery_current_a\":" << (static_cast<float>(drone.current_ca) / 100.0F)
      << ",\"battery_remaining_percent\":" << drone.battery_remaining
      << ",\"home_distance_m\":" << drone.home_distance_m
      << "}}";
  return oss.str();
}

void export_olog(const Options& options) {
  const std::filesystem::path output_path(options.output);
  if (output_path.has_parent_path()) {
    std::filesystem::create_directories(output_path.parent_path());
  }

  std::ofstream stream(options.output, std::ios::binary | std::ios::trunc);
  if (!stream.is_open()) {
    throw std::runtime_error("Cannot open output file: " + options.output);
  }
  std::ofstream debug(options.debug_output, std::ios::out | std::ios::trunc);
  if (!debug.is_open()) {
    throw std::runtime_error("Cannot open debug output file: " +
                             options.debug_output);
  }

  const int total_ticks = options.seconds * options.rate_hz;
  const auto sleep_time = std::chrono::milliseconds(1000 / options.rate_hz);
  const auto start_time = std::chrono::system_clock::now();
  int packet_index = 0;

  write_debug_line(debug, "{\"schema\":\"openhd-osd-debug-v1\",\"type\":\"start\","
                          "\"datetime_utc\":\"" +
                              iso_utc(start_time) + "\",\"rate_hz\":" +
                              std::to_string(options.rate_hz) +
                              ",\"raw_log\":\"" + options.output + "\"}");

  write_openhd_heartbeat(stream, kOpenHdGroundSysId, packet_index);
  write_openhd_heartbeat(stream, kOpenHdAirSysId, packet_index);
  write_fc_heartbeat(stream, packet_index);
  write_openhd_version(stream, kOpenHdGroundSysId, packet_index);
  write_openhd_version(stream, kOpenHdAirSysId, packet_index);

  for (int tick = 0; tick < total_ticks; ++tick) {
    const int first_packet_index = packet_index;
    const int elapsed_ms = tick * 1000 / options.rate_hz;
    const auto drone = make_drone_sample(tick, elapsed_ms);
    write_telemetry_stats(stream, kOpenHdGroundSysId, tick, packet_index);
    write_telemetry_stats(stream, kOpenHdAirSysId, tick, packet_index);
    write_wifi_card_stats(stream, kOpenHdGroundSysId, tick, packet_index);
    write_video_air_stats(stream, tick, packet_index);
    write_drone_messages(stream, tick, drone, packet_index);
    write_debug_line(debug, replay_json_line(tick, first_packet_index,
                                             packet_index - first_packet_index,
                                             elapsed_ms, start_time, drone));
    stream.flush();
    std::this_thread::sleep_for(sleep_time);
  }
  write_debug_line(debug, "{\"schema\":\"openhd-osd-debug-v1\",\"type\":\"end\","
                          "\"datetime_utc\":\"" +
                              iso_utc(std::chrono::system_clock::now()) +
                              "\",\"packets\":" +
                              std::to_string(packet_index) + "}");
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const auto options = parse_args(argc, argv);
    export_olog(options);
    std::cout << "Wrote " << options.output << " (" << options.seconds
              << "s, " << options.rate_hz << " Hz)" << std::endl;
    std::cout << "Wrote " << options.debug_output << " (OSD replay/debug)"
              << std::endl;
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "olog_exporter: " << ex.what() << std::endl;
    print_usage(argv[0]);
    return 1;
  }
}
