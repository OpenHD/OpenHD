//
// Created by consti10 on 13.07.22.
//

#include "wifi_command_helper.h"

#include <iostream>
#include <sstream>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "wifi_channel.h"

static std::shared_ptr<spdlog::logger> get_logger() {
  return openhd::log::create_or_get("w_helper");
}

bool wifi::commandhelper::rfkill_unblock_all() {
  get_logger()->info("rfkill_unblock_all");
  std::vector<std::string> args{"unblock", "all"};
  bool success = OHDUtil::run_command("rfkill", args);
  return success;
}

bool wifi::commandhelper::ip_link_set_card_state(const std::string &device,
                                                 bool up) {
  get_logger()->info("ip_link_set_card_state {} up {}", device, up);
  std::vector<std::string> args{"link", "set", "dev", device,
                                up ? "up" : "down"};
  bool success = OHDUtil::run_command("ip", args);
  return success;
}

bool wifi::commandhelper::iw_enable_monitor_mode(const std::string &device) {
  get_logger()->info("iw_enable_monitor_mode {}", device);
  std::vector<std::string> args{"dev", device, "set", "monitor", "otherbss"};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

// use_ht40_plus: Only in 40Mhz mode
static std::string channel_width_as_iw_string(uint32_t channel_width,
                                              bool use_ht40_plus = true) {
  if (channel_width == 5) {
    return "5MHz";
  } else if (channel_width == 10) {
    return "10Mhz";
  } else if (channel_width == 20) {
    return "HT20";
  } else if (channel_width == 40) {
    return use_ht40_plus ? "HT40+" : "HT40-";
  }
  get_logger()->info("Invalid channel width {}, assuming HT20", channel_width);
  return "HT20";
}

bool wifi::commandhelper::iw_set_frequency_and_channel_width(
    const std::string &device, uint32_t freq_mhz, uint32_t channel_width) {
  const std::string iw_channel_width =
      channel_width_as_iw_string(channel_width);
  return iw_set_frequency_and_channel_width2(device, freq_mhz,
                                             iw_channel_width);
}
bool wifi::commandhelper::iw_set_frequency_and_channel_width2(
    const std::string &device, uint32_t freq_mhz, const std::string &ht_mode,
    bool dummy) {
  get_logger()->info("{}iw_set_frequency_and_channel_width2 {} {}Mhz {}",
                     dummy ? "DUMMY! " : "", device, freq_mhz, ht_mode);
  std::vector<std::string> args{
      "dev", device, "set", "freq", std::to_string(freq_mhz), ht_mode};
  const auto ret = OHDUtil::run_command("iw", args);
  if (ret != 0) {
    get_logger()->warn("iw {}Mhz@{} not supported {}", freq_mhz, ht_mode, ret);
    std::cout << std::flush;
    return false;
  }
  return true;
}

bool wifi::commandhelper::iw_set_tx_power(const std::string &device,
                                          uint32_t tx_power_mBm) {
  get_logger()->info("iw_set_tx_power {} {} mBm", device, tx_power_mBm);
  std::vector<std::string> args{
      "dev", device, "set", "txpower", "fixed", std::to_string(tx_power_mBm)};
  const auto ret = OHDUtil::run_command("iw", args);
  if (ret != 0) {
    get_logger()->warn("iw_set_tx_power failed {}", ret);
    return false;
  }
  return true;
}

bool wifi::commandhelper::iw_set_rate_mcs(const std::string &device,
                                          uint32_t mcs_index, bool is_2g) {
  get_logger()->info("iw_set_rate_mcs {} {} mBm", device, mcs_index);
  std::vector<std::string> args{"dev",
                                device,
                                "set",
                                "bitrates",
                                is_2g ? "ht-mcs-2.4" : "ht-mcs-5",
                                std::to_string(mcs_index)};
  const auto ret = OHDUtil::run_command("iw", args);
  if (ret != 0) {
    get_logger()->warn("iw_set_rate_mcs failed {}", ret);
    return false;
  }
  return true;
}

bool wifi::commandhelper::nmcli_set_device_managed_status(
    const std::string &device, bool managed) {
  get_logger()->info("nmcli_set_device_managed_status {} managed:{}", device,
                     managed);
  std::vector<std::string> arguments{"device", "set", device, "managed"};
  if (managed) {
    arguments.emplace_back("yes");
  } else {
    arguments.emplace_back("no");
  }
  bool success = OHDUtil::run_command("nmcli", arguments);
  return success;
}

static std::string float_without_trailing_zeroes(const float value) {
  std::stringstream ss;
  ss << std::noshowpoint << value;
  return ss.str();
}

// Example supported frequency: 5500 MHz [100] (20.0 dBm) (no IR, radar
// detection) Example not supported freq : 5885 MHz [177] (disabled)
static bool iw_info_line_contains_freq_and_is_supported(
    const std::string &line, const uint32_t freq_mhz) {
  return OHDUtil::contains(line, fmt::format("{} MHz", freq_mhz)) &&
         !OHDUtil::contains(line, "disabled");
}
static bool iw_info_supports_frequency(const std::vector<std::string> &lines,
                                       const uint32_t freq_mhz) {
  for (const auto &line : lines) {
    if (iw_info_line_contains_freq_and_is_supported(line, freq_mhz))
      return true;
  }
  return false;
}

std::vector<uint32_t> wifi::commandhelper::iw_get_supported_frequencies(
    const int phy_index, const std::vector<uint32_t> &frequencies_mhz_to_try) {
  const std::string command = fmt::format("iw phy phy{} info", phy_index);
  const auto res_op = OHDUtil::run_command_out(command);
  if (!res_op.has_value()) {
    openhd::log::get_default()->warn("get_supported_channels for phy{} failed",
                                     phy_index);
    // If this fails, we assume we can do all channels - to not limit the valid
    // inputs by mistake
    return frequencies_mhz_to_try;
  }
  const auto &res = res_op.value();
  // We need to look for lines that have the given frequency and NOT a disabled
  // at the end
  const auto lines = OHDUtil::split_string_by_newline(res, false);
  std::vector<uint32_t> supported_channels{};
  // NOTE: n^2 run time complexity, but we only do this once on startup
  for (const auto &freq_mhz : frequencies_mhz_to_try) {
    if (iw_info_supports_frequency(lines, freq_mhz)) {
      // openhd::log::get_default()->debug("has [{}]",s_freq_ghz);
      supported_channels.push_back(freq_mhz);
    } else {
      // openhd::log::get_default()->debug("doesn't have [{}]",s_freq_ghz);
    }
  }
  return supported_channels;
}

wifi::commandhelper::SupportedFrequencyBand
wifi::commandhelper::iw_get_supported_frequency_bands(
    const std::string &device) {
  wifi::commandhelper::SupportedFrequencyBand ret{false, false};
  const std::string command = "iwlist " + device + " frequency";
  const auto res_op = OHDUtil::run_command_out(command);
  if (!res_op.has_value()) {
    openhd::log::get_default()->warn(
        "iw_get_supported_frequency_bands for {} failed", device);
    return {true, true};
  }
  const auto &res = res_op.value();
  if (res.find("5.") != std::string::npos) {
    ret.supports_any_5G = true;
  }
  if (res.find("2.") != std::string::npos) {
    ret.supports_any_2G = true;
  }
  return ret;
}

bool wifi::commandhelper::iw_supports_monitor_mode(int phy_index) {
  const std::string command =
      "iw phy phy" + std::to_string(phy_index) + " info";
  const auto res_opt = OHDUtil::run_command_out(command);
  if (!res_opt.has_value()) {
    openhd::log::get_default()->warn(
        "iw_supports_monitor_mode for phy{} failed,assuming can do monitor "
        "mode",
        phy_index);
    return true;
  }
  return OHDUtil::contains(res_opt.value(), "* monitor");
}

static constexpr auto OPENHD_DRIVER_RTL8812AU_CHANNEL_OVERRIDE =
    "/sys/module/88XXau_ohd/parameters/openhd_override_channel";
static constexpr auto OPENHD_DRIVER_RTL8812AU_TX_POWER_INDEX_OVERRIDE =
    "/sys/module/88XXau_ohd/parameters/openhd_override_tx_power_index";
static constexpr auto OPENHD_DRIVER_RTL88xxBU_CHANNEL_OVERRIDE =
    "/sys/module/88x2bu_ohd/parameters/openhd_override_channel";
static constexpr auto OPENHD_DRIVER_RTL88xxBU_TX_POWER_MW_OVERRIDE =
    "/sys/module/88x2bu_ohd/parameters/openhd_override_tx_power_mbm";

bool wifi::commandhelper::openhd_driver_set_frequency_and_channel_width(
    int type, const std::string &device, uint32_t freq_mhz,
    uint32_t channel_width) {
  const auto channel_opt = openhd::channel_from_frequency(freq_mhz);
  if (!channel_opt.has_value()) {
    openhd::log::get_default()->warn("Cannot find channel {}Mhz", freq_mhz);
  }
  const auto channel =
      channel_opt.value_or(openhd::channel_from_frequency(5180).value());
  const std::string rtl8812au_channel = fmt::format("{}", channel.channel);
  openhd::log::get_default()->debug(
      "openhd_driver_set_frequency_and_channel_width wanted:{}@{}Mhz, using "
      "channel override:{}",
      freq_mhz, channel_width, rtl8812au_channel);
  const std::string CHANNEL_OVERRIDE_FILENAME =
      type == 0 ? OPENHD_DRIVER_RTL8812AU_CHANNEL_OVERRIDE
                : OPENHD_DRIVER_RTL88xxBU_CHANNEL_OVERRIDE;
  if (!OHDFilesystemUtil::exists(CHANNEL_OVERRIDE_FILENAME)) {
    openhd::log::get_default()->error(
        "YOU ARE USING THE WRONG DRIVER; CHANNEL WON'T WORK");
    // hope this works
    wifi::commandhelper::iw_set_frequency_and_channel_width(device, freq_mhz,
                                                            channel_width);
    return true;
  }
  // /etc/modprobe.d
  // options 88XXau_ohd openhd_override_channel=165
  // openhd_override_channel_width=1 rmmod 88XXau_ohd
  OHDFilesystemUtil::write_file(CHANNEL_OVERRIDE_FILENAME, rtl8812au_channel);
  // Override stuff is set, now we just change to a channel that is always okay
  // in crda such that the method is called - ! the actually applied channel
  // will be the overridden one !
  const bool use_40mhz = channel_width == 40;
  const bool use_ht40_plus = channel.in_40Mhz_ht40_plus;  // only in 40Mhz mode
  int dummy_frequency = -1;
  if (channel.space == openhd::WifiSpace::G2_4) {
    dummy_frequency = use_40mhz ? (use_ht40_plus ? 2412 : 2432) : 2412;
  } else {
    dummy_frequency = use_40mhz ? (use_ht40_plus ? 5180 : 5200) : 5180;
  }
  const std::string bw_mode =
      channel_width == 20 ? "HT20" : (use_ht40_plus ? "HT40+" : "HT40-");
  wifi::commandhelper::iw_set_frequency_and_channel_width2(
      device, dummy_frequency, bw_mode, true);
  return true;
}

bool wifi::commandhelper::openhd_driver_set_tx_power(const std::string &device,
                                                     uint32_t tx_power_mBm) {
  if (!OHDFilesystemUtil::exists(
          OPENHD_DRIVER_RTL88xxBU_TX_POWER_MW_OVERRIDE)) {
    openhd::log::get_default()->error(
        "YOU ARE USING THE WRONG DRIVER; TX POWER WON'T WORK");
    // hope this works
    wifi::commandhelper::iw_set_tx_power(device, tx_power_mBm);
    return true;
  }
  OHDFilesystemUtil::write_file(OPENHD_DRIVER_RTL88xxBU_TX_POWER_MW_OVERRIDE,
                                fmt::format("{}", tx_power_mBm));
  // initiate change
  wifi::commandhelper::iw_set_tx_power(device,
                                       13);  // 20mW ~ 13mBm, should always work
  return true;
}

void wifi::commandhelper::openhd_driver_set_tx_power_index_override(
    const std::string &device, uint32_t tpi) {
  if (!OHDFilesystemUtil::exists(OPENHD_DRIVER_RTL8812AU_CHANNEL_OVERRIDE)) {
    openhd::log::get_default()->error(
        "YOU ARE USING THE WRONG DRIVER; TPI POWER WON'T WORK");
    return;
  }
  OHDFilesystemUtil::write_file(OPENHD_DRIVER_RTL8812AU_TX_POWER_INDEX_OVERRIDE,
                                fmt::format("{}", tpi));
  // initiate change
  wifi::commandhelper::iw_set_tx_power(device,
                                       13);  // 20mW ~ 13mBm, should always work
}

void wifi::commandhelper::cleanup_openhd_driver_overrides() {
  if (OHDFilesystemUtil::exists(OPENHD_DRIVER_RTL8812AU_CHANNEL_OVERRIDE)) {
    OHDFilesystemUtil::write_file(OPENHD_DRIVER_RTL8812AU_CHANNEL_OVERRIDE,
                                  "0");
  }
  if (OHDFilesystemUtil::exists(
          OPENHD_DRIVER_RTL8812AU_TX_POWER_INDEX_OVERRIDE)) {
    OHDFilesystemUtil::write_file(
        OPENHD_DRIVER_RTL8812AU_TX_POWER_INDEX_OVERRIDE, "0");
  }
  if (OHDFilesystemUtil::exists(OPENHD_DRIVER_RTL88xxBU_CHANNEL_OVERRIDE)) {
    OHDFilesystemUtil::write_file(OPENHD_DRIVER_RTL88xxBU_CHANNEL_OVERRIDE,
                                  "0");
  }
  if (OHDFilesystemUtil::exists(OPENHD_DRIVER_RTL88xxBU_TX_POWER_MW_OVERRIDE)) {
    OHDFilesystemUtil::write_file(OPENHD_DRIVER_RTL88xxBU_TX_POWER_MW_OVERRIDE,
                                  "0");
  }
}
