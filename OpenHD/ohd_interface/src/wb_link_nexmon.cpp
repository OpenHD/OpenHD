#include "wb_link.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <thread>

#include "nexmon_scout.h"
#include "openhd_global_constants.hpp"
#include "openhd_util.h"
#ifdef OHD_ENABLE_DEVOURER
#include "chanmig/ChannelScore.h"
#include "chanmig/ScanPlan.h"
#endif

namespace {
std::shared_ptr<WBTxRx> scout_receiver(bool ground,
                                     const std::optional<wb::KeyPairTxRx>& keys) {
  if (!keys) throw std::runtime_error("No keypair for authenticated scout RX");
  WBTxRx::Options options;
  options.secure_keypair = keys;
  options.use_gnd_identifier = ground;
  options.enable_auto_switch_tx_card = false;
  options.receive_thread_max_realtime = false;
  auto rx = std::make_shared<WBTxRx>(
      std::vector<wifibroadcast::WifiCard>{{openhd::NexmonScout::monitor_interface, 0}},
      options, std::make_shared<RadiotapHeaderTxHolder>());
  rx->set_passive_mode(true);
  return rx;
}
}

void WBLink::perform_nexmon_scan(
    const openhd::LinkActionHandler::ScanChannelsParam& params) {
  auto& actions = openhd::LinkActionHandler::instance();
  auto link_stats = actions.get_link_stats();
  link_stats.gnd_operating_mode.operating_mode = 1;
  actions.update_link_stats(link_stats);
  openhd::LinkActionHandler::ScanChannelsProgress progress{};
  progress.channel_width_mhz = 20;
  try {
    if (!m_profile.is_ground()) throw std::runtime_error("Air discovery requires Ground mode");
    if (params.channel_widths_mask & actions.scan_channel_width_bit(10))
      throw std::runtime_error("Internal Pi radio cannot scan 10 MHz waveforms");
    openhd::NexmonScout scout;
    std::atomic<int> reported_frequency{0}, reported_width{0};
    auto rx = scout_receiver(true, m_scout_keypair);
    // This callback only sees authenticated payloads, including their session
    // key validation, through the same decoder as the primary video receiver.
    auto handler = std::make_shared<WBTxRx::StreamRxHandler>(
        openhd::MANAGEMENT_RADIO_PORT_AIR_TX,
        [&](uint64_t, int, const uint8_t* data, int length) {
          if (length != 6 || data[0] != 0) return;
          uint32_t frequency = 0;
          std::memcpy(&frequency, data+1, 4);
          if (data[5] != 20 && data[5] != 40) return;
          reported_width = data[5];
          reported_frequency = static_cast<int>(frequency);
        }, nullptr);
    rx->rx_register_stream_handler(handler);
    const auto channels = openhd::wb::get_scan_channels_frequencies(
        m_broadcast_cards.at(0), params.channels_to_scan);
    for (size_t i = 0; i < channels.size(); ++i) {
      const auto frequency = channels[i].frequency;
      progress.channel_mhz = frequency;
      progress.progress = OHDUtil::calculate_progress_perc(i, channels.size());
      actions.add_scan_channels_progress(progress);
      if (!scout.tune(frequency)) continue;
      // Start a fresh receive loop after tuning so old buffered packets cannot
      // announce an air unit on a different candidate frequency.
      reported_frequency = 0;
      reported_width = 0;
      rx->start_receiving();
      std::this_thread::sleep_for(std::chrono::seconds(2));
      rx->stop_receiving();
      if (reported_frequency == frequency && reported_width > 0) {
        progress.success = true;
        progress.channel_width_mhz = reported_width;
        break;
      }
    }
    rx.reset();
    if (!scout.restore()) throw std::runtime_error("Internal Wi-Fi restoration failed");
    if (progress.success) {
      // Only the successful, authenticated discovery changes the video radio.
      if (!apply_frequency_and_channel_width(progress.channel_mhz,
              progress.channel_width_mhz, progress.channel_width_mhz))
        throw std::runtime_error("Cannot apply the discovered air channel");
      m_settings->unsafe_get_settings().wb_frequency = progress.channel_mhz;
      m_settings->unsafe_get_settings().wb_gnd_rx_channel_width = progress.channel_width_mhz;
      m_settings->persist();
      m_gnd_curr_rx_channel_width = progress.channel_width_mhz;
    }
  } catch (const std::exception& error) {
    progress.success = false;
    m_console->warn("Internal Wi-Fi scan failed: {}", error.what());
  }
  if (!progress.success) progress.channel_mhz = 0;
  progress.progress = 100;
  actions.add_scan_channels_progress(progress);
}

void WBLink::perform_nexmon_analyze(int selection) {
  auto& actions = openhd::LinkActionHandler::instance();
  auto link_stats = actions.get_link_stats();
  link_stats.gnd_operating_mode.operating_mode = 2;
  actions.update_link_stats(link_stats);
  openhd::LinkActionHandler::AnalyzeChannelsResult result{};
  try {
    const auto channels = openhd::wb::get_analyze_channels_frequencies(
        m_broadcast_cards.at(0), selection);
    if (channels.empty() || channels.size() > result.channels_mhz.size())
      throw std::runtime_error("Unsupported analysis channel count");
    openhd::NexmonScout scout;
    auto rx = scout_receiver(m_profile.is_ground(), m_scout_keypair);
#ifdef OHD_ENABLE_DEVOURER
    using namespace devourer::chanmig;
    ScanPlanConfig plan;
    for (const auto& channel : channels) {
      ChannelDef def;
      def.band = channel.frequency < 3000 ? 2 : 5;
      def.primary = channel.channel;
      if (validate(def) == DefError::Ok) plan.candidates.push_back(def);
    }
    ChannelDef active;
    const int home = m_settings->get_settings().wb_frequency;
    active.band = home < 3000 ? 2 : 5;
    active.primary = home < 3000 ? (home-2407)/5 : (home-5000)/5;
    RecommendEngine engine(PolicyConfig{}, plan.candidates, plan.plan_hash(), active);
    // Packet-only sensing cannot certify absence of non-Wi-Fi interference.
    // Retain the ranking, but do not qualify automatic migration candidates.
    engine.note_scout_health(false);
#endif
    for (int round = 0; round < 3; ++round) {
      for (size_t i = 0; i < channels.size(); ++i) {
        const auto& channel = channels[i];
          if (scout.tune(channel.frequency)) {
          rx->rx_reset_stats();
          rx->start_receiving();
          const auto start = openhd::util::steady_clock_time_epoch_ms();
          const auto sample = openhd::observe_nexmon(channel.frequency, 1100);
          rx->stop_receiving();
          const auto received = rx->get_rx_stats();
          result.channels_mhz[i] = channel.frequency;
          const int64_t foreign = std::max<int64_t>(0, received.count_p_any-received.count_p_valid);
          result.foreign_packets[i] = static_cast<uint16_t>(std::min<int64_t>(65535,
              result.foreign_packets[i]+foreign));
#ifdef OHD_ENABLE_DEVOURER
          SurveyDwell dwell;
          dwell.def.band = channel.frequency < 3000 ? 2 : 5;
          dwell.def.primary = channel.channel;
          dwell.plan_hash = plan.plan_hash();
          dwell.scout_id = 0x4e45584d;
          dwell.round = round;
          dwell.t_start_ms = start;
          dwell.t_end_ms = openhd::util::steady_clock_time_epoch_ms();
          dwell.observe_ms = dwell.t_end_ms-start;
          dwell.frames = sample.foreign_packets;
          dwell.oth_air_us = sample.decoded_airtime_us;
          dwell.flags = kFlagNhmMissing;
          if (sample.unknown_rate_packets || sample.malformed_packets || received.count_p_valid)
            dwell.flags |= kFlagReadFailed;
          engine.ingest_dwell(dwell, dwell.t_end_ms);
#endif
          m_console->info("Internal Wi-Fi {} MHz: {} foreign packets, {} us decoded airtime, {} unknown rates",
                          channel.frequency, foreign, sample.decoded_airtime_us, sample.unknown_rate_packets);
        }
        result.progress = std::min<int>(99, OHDUtil::calculate_progress_perc(
            round*channels.size()+i+1, 3*channels.size()));
        actions.add_analyze_result(result);
      }
    }
    rx.reset();
    if (!scout.restore()) throw std::runtime_error("Internal Wi-Fi restoration failed");
#ifdef OHD_ENABLE_DEVOURER
    const auto decision = engine.decide(openhd::util::steady_clock_time_epoch_ms());
    int best_frequency = 0;
    double best_occupancy = 2.0;
    for (const auto& score : decision.ranking) {
      m_console->info("Nexmon/Devourer {}: decoded occupancy {:.3f}, rounds {}, observed {}ms (packet-only advisory)",
                      score.def.str(), score.occ_q50, score.rounds, score.observe_ms);
      if (score.rounds >= 3 && score.observe_ms >= 3000 &&
          score.bins_covered == score.bins_total && score.occ_q50 < best_occupancy) {
        best_frequency = score.def.center_mhz();
        best_occupancy = score.occ_q50;
      }
    }
    if (best_frequency)
      m_console->warn("Wi-Fi best: {} MHz (packet-only)", best_frequency);
    else
      m_console->warn("Nexmon: insufficient valid evidence to select a best channel");
#endif
  } catch (const std::exception& error) {
    m_console->warn("Internal Wi-Fi analysis failed: {}", error.what());
  }
  result.progress = 100;
  actions.add_analyze_result(result);
}
