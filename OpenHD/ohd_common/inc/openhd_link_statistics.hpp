//
// Created by consti10 on 13.07.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_

#include <cstring>
#include <optional>
#include <sstream>
#include <string>

// NOTE: While annoying, we do not want mavlink as a direct dependency inside
// ohd_common / ohd_interface, So we double-declare the mavlink message
// struct(s) here.

namespace openhd::link_statistics {

struct Xmavlink_openhd_stats_monitor_mode_wifi_link_t {
  int32_t curr_tx_bps;               /*<  tx bits per second*/
  int32_t curr_rx_bps;               /*<  rx bits per second*/
  uint32_t count_tx_inj_error_hint;  /*<  count_tx_inj_error_hint*/
  uint32_t count_tx_dropped_packets; /*<  count_tx_dropped_packets*/
  int32_t dummy2;                    /*<  for future use*/
  int16_t curr_tx_pps;               /*<  tx packets per second*/
  int16_t curr_rx_pps;               /*<  rx packets per second*/
  int16_t curr_rx_big_gaps_counter;  /*<  complicated but important stat*/
  uint16_t
      curr_tx_channel_mhz; /*<  curr tx channel used when injecting packets*/
  uint16_t
      curr_rate_kbits; /*<  curr link rate, in kbit/s, might be slightly lower
                          than default for MCS when TX errors are detected*/
  int16_t dummy1;      /*<  for future use*/
  int8_t curr_rx_packet_loss_perc; /*<  curr_rx_packet_loss*/
  uint8_t curr_tx_channel_w_mhz; /*<  curr tx channel width used when injecting
                                    packets (20mhz/40Mhz)*/
  uint8_t
      curr_tx_mcs_index; /*<  curr tx mcs index used when injecting packets*/
  uint8_t curr_n_rate_adjustments; /*<  If the TX cannot keep up (at a given
                                      mcs), openhd reduces the bitrate in
                                      1MBit/s increments*/
  uint8_t bitfield;                /*<  bitfield, see openhd for usage.*/
  uint8_t pollution_perc; /*<  pollution [0..100],aka percentage of foreign vs
                             openhd packets on the current channel.*/
  int8_t dummy0;          /*<  for future use*/
};

struct Xmavlink_openhd_stats_telemetry_t {
  int32_t curr_tx_bps;              /*<  tx bits per second*/
  int32_t curr_rx_bps;              /*<  rx bits per second*/
  int32_t dummy2;                   /*<  for future use*/
  int16_t curr_tx_pps;              /*<  tx packets per second*/
  int16_t curr_rx_pps;              /*<  rx packets per second*/
  int16_t curr_rx_packet_loss_perc; /*<  curr_rx_packet_loss_perc*/
  int16_t dummy1;                   /*<  for future use*/
  int8_t dummy0;                    /*<  for future use*/
  [[nodiscard]] std::string to_string() const {
    std::stringstream ss;
    ss << "StatsTelemetry{";
    ss << "curr_tx_pps:" << curr_tx_pps << ",curr_rx_pps:" << curr_rx_pps
       << ",curr_tx_bps:" << curr_tx_bps << ", curr_rx_bps:" << curr_rx_bps;
    ss << ", curr_rx_packet_loss_perc" << curr_rx_packet_loss_perc;
    ss << "}";
    return ss.str();
  }
};

struct Xmavlink_openhd_stats_wb_video_air_t {
  int32_t curr_measured_encoder_bitrate; /*<  curr_measured_encoder_bitrate*/
  int32_t curr_injected_bitrate;    /*<  curr_injected_bitrate (+FEC overhead)*/
  int32_t curr_injected_pps;        /*<  curr_injected_pps*/
  int32_t curr_dropped_frames;      /*<  curr_dropped_frames*/
  int32_t dummy2;                   /*<  for future use*/
  int16_t curr_recommended_bitrate; /*<  curr_recommended_bitrate*/
  int16_t curr_fec_percentage;      /*<  curr_fec_percentage*/
  int16_t dummy1;                   /*<  for future use*/
  uint8_t link_index;               /*<  link_index*/
  int8_t dummy0;                    /*<  for future use*/
};
struct Xmavlink_openhd_stats_wb_video_air_fec_performance_t {
  uint32_t curr_fec_encode_time_avg_us; /*<  curr_fec_encode_time_avg_us*/
  uint32_t curr_fec_encode_time_min_us; /*<  curr_fec_encode_time_min_us*/
  uint32_t curr_fec_encode_time_max_us; /*<  curr_fec_encode_time_max_us*/
  int32_t dummy2;                       /*<  for future use*/
  uint16_t curr_fec_block_size_avg;     /*<  curr_fec_block_size_avg*/
  uint16_t curr_fec_block_size_min;     /*<  curr_fec_block_size_min*/
  uint16_t curr_fec_block_size_max;     /*<  curr_fec_block_size_max*/
  uint16_t curr_tx_delay_avg_us;        /*<  none*/
  uint16_t curr_tx_delay_min_us;        /*<  none*/
  uint16_t curr_tx_delay_max_us;        /*<  none*/
  int16_t dummy1;                       /*<  for future use*/
  uint8_t link_index;                   /*<  link_index*/
  int8_t dummy0;                        /*<  for future use*/
};

struct Xmavlink_openhd_stats_wb_video_ground_t {
  int32_t curr_incoming_bitrate;      /*<  todo*/
  uint32_t count_blocks_total;        /*<  count_blocks_total*/
  uint32_t count_blocks_lost;         /*<  count_blocks_lost*/
  uint32_t count_blocks_recovered;    /*<  count_blocks_recovered*/
  uint32_t count_fragments_recovered; /*<  count_fragments_recovered*/
  int32_t dummy2;                     /*<  for future use*/
  int16_t dummy1;                     /*<  for future use*/
  uint8_t link_index;                 /*<  link_index*/
  int8_t dummy0;                      /*<  for future use*/
};
struct Xmavlink_openhd_stats_wb_video_ground_fec_performance_t {
  uint32_t curr_fec_decode_time_avg_us; /*<  todo*/
  uint32_t curr_fec_decode_time_min_us; /*<  todo*/
  uint32_t curr_fec_decode_time_max_us; /*<  todo*/
  int32_t dummy2;                       /*<  for future use*/
  int16_t dummy1;                       /*<  for future use*/
  uint8_t link_index;                   /*<  link_index*/
  int8_t dummy0;                        /*<  for future use*/
};
struct Xmavlink_openhd_stats_monitor_mode_wifi_card_t {
  bool NON_MAVLINK_CARD_ACTIVE =
      false;                 // Optimization, only send for active card(s).
  uint32_t count_p_received; /*<  All received (incoming) packets*/
  uint32_t count_p_injected; /*<  All injected (outgoing) packets*/
  int32_t dummy2;            /*<  for future use*/
  int16_t tx_power_current;  /*<  either in override index units or mW*/
  int16_t tx_power_armed;    /*<  either in override index units or mW*/
  int16_t tx_power_disarmed; /*<  either in override index units or mW*/
  int16_t dummy1;            /*<  for future use*/
  uint8_t card_index; /*<  Ground might have multiple card(s) for diversity.*/
  uint8_t card_type;  /*<  See openhd card_type enum*/
  // extra
  uint8_t card_sub_type;
  uint8_t tx_active; /*<  On ground stations with multiple card(s), only one
                        card is selected for TX at a time.*/
  int8_t rx_rssi; /*<  rx rssi in dBm of this card - depending on the hw, might
                     be the max of all antennas or different.*/
  int8_t rx_rssi_1;                  /*<  rx rssi in dBm for antenna 1*/
  int8_t rx_rssi_2;                  /*<  rx rssi in dBm for antenna 2*/
  int8_t rx_noise_adapter;           /*<  RX noise in dBm of adapter*/
  int8_t rx_noise_antenna1;          /*<  RX noise in dBm of antenna1*/
  int8_t rx_noise_antenna2;          /*<  RX noise in dBm of antenna2*/
  int8_t rx_signal_quality_adapter;  /*<  Signal quality [0..100] of card*/
  int8_t rx_signal_quality_antenna1; /*<  Signal quality [0..100] of antenna1*/
  int8_t rx_signal_quality_antenna2; /*<  Signal quality [0..100] of antenna2*/
  int8_t curr_rx_packet_loss_perc;   /*<  rx packet loss (for this card)*/
  uint8_t curr_status; /*<  set to 1 if something's wrong with the card*/
  int8_t dummy0;       /*<  for future use*/
};
struct Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t {
  int32_t dummy1;             /*<  future use*/
  int32_t dummy2;             /*<  future use*/
  uint16_t extra_channel_mhz; /*<  channel that is currently scanned / analyzed,
                                 0 otherwise*/
  uint16_t dummy0;            /*<  future use*/
  uint8_t operating_mode;     /*<  0=normal,1=currently scanning, 2=currently
                                 analyzing*/
  uint8_t
      extra_channel_width_mhz; /*<  samel like extra channel, for bandwidth*/
  uint8_t progress; /*<  progress [0..100], when scanning / analyzing is
                       active,undefined otherwise.*/
  uint8_t success;  /*<  Set to 1 if channel scan succeeded.*/
  int8_t tx_passive_mode_is_enabled; /*<  Act as a passive listener (NO TX
                                        EVER), 0=disabled, 1=enabled.*/
};

// Stats per connected card
using StatsAllCards =
    std::array<Xmavlink_openhd_stats_monitor_mode_wifi_card_t, 4>;

struct StatsAirGround {
  bool is_air = false;
  bool ready = false;
  // air and ground
  Xmavlink_openhd_stats_monitor_mode_wifi_link_t monitor_mode_link;
  Xmavlink_openhd_stats_telemetry_t telemetry;
  StatsAllCards cards;
  // for air
  std::vector<Xmavlink_openhd_stats_wb_video_air_t> stats_wb_video_air;
  Xmavlink_openhd_stats_wb_video_air_fec_performance_t air_fec_performance;
  // for ground
  std::vector<Xmavlink_openhd_stats_wb_video_ground_t> stats_wb_video_ground;
  Xmavlink_openhd_stats_wb_video_ground_fec_performance_t gnd_fec_performance;
  Xmavlink_openhd_wifbroadcast_gnd_operating_mode_t gnd_operating_mode;
};

typedef std::function<void(StatsAirGround all_stats)> STATS_CALLBACK;

// Bit field for boolean only value(s)
struct MonitorModeLinkBitfield {
  unsigned int stbc : 1;
  unsigned int lpdc : 1;
  unsigned int short_guard : 1;
  unsigned int curr_rx_last_packet_status_good : 1;
  unsigned int unused : 4;
} __attribute__((packed));
static_assert(sizeof(MonitorModeLinkBitfield) == 1);
static uint8_t write_monitor_link_bitfield(
    const MonitorModeLinkBitfield& bitfield) {
  uint8_t ret;
  std::memcpy(&ret, (uint8_t*)&bitfield, 1);
  return ret;
}
static MonitorModeLinkBitfield parse_monitor_link_bitfield(uint8_t bitfield) {
  MonitorModeLinkBitfield ret{};
  std::memcpy((uint8_t*)&ret, &bitfield, 1);
  return ret;
}

using VALUE_BITRATE = uint16_t;
using VALUE_DELAY = uint16_t;
// User can change: BW, MCS, FREQUENCY, RESILIENCY_MODE
struct LinkStatusAir {
  uint8_t mcs_index;
  uint8_t bw_mhz;
  uint16_t frequency;
  uint8_t throttle;
  uint8_t resiliency_mode;
  // Video bitrate (before FEC, ...)
  VALUE_BITRATE video_bitrate;
  VALUE_DELAY cam0_tx_delay;
  uint8_t cam0_dropped_frames;
  VALUE_DELAY cam1_tx_delay;
  uint8_t cam1_dropped_frames;
  uint8_t rx_loss_percent;
};
struct LinkStatusGround {};

}  // namespace openhd::link_statistics
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_LINK_STATISTICS_HPP_
