//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_

#include <vector>

#include "openhd_spdlog_include.h"
#include "wifi_channel.h"

// Helper for validating user-selectable settings

namespace openhd {

static bool is_valid_frequency_2G(uint32_t frequency) {
  const auto supported = openhd::get_channels_2G();
  for (const auto& value : supported) {
    if (value.frequency == frequency) return true;
  }
  return false;
}

static bool is_valid_frequency_5G(uint32_t frequency) {
  const auto supported = openhd::get_channels_5G();
  for (const auto& value : supported) {
    if (value.frequency == frequency) return true;
  }
  return false;
}

static bool is_valid_channel_width(uint32_t channel_width) {
  return channel_width == 20 || channel_width == 40;
}

static bool is_valid_mcs_index(uint32_t mcs_index) {
  return mcs_index >= 0 && mcs_index <= 31;
}

// Internally, OpenHD uses milli watt (mW)
// No wifi card will ever do 30W, but some cards increase their tx power a bit
// more when you set a higher value (I think)
static bool is_valid_tx_power_milli_watt(int tx_power_mw) {
  return tx_power_mw >= 10 && tx_power_mw <= 30 * 1000;
}

// NOTE: 0 means variable fec, video codec has to be set in this case
static bool is_valid_fec_block_length(int block_length) {
  return block_length >= 0 && block_length < 100;
}
// max 100% fec (2x the amount of data), this is already too much
// 21.10: Using more than 2x for FEC can be usefully for testing
static bool is_valid_fec_percentage(int fec_perc) {
  bool valid = fec_perc > 0 && fec_perc <= 400;
  if (!valid) {
    openhd::log::warning_log("Invalid fec percentage");
  }
  return valid;
}

// https://www.rapidtables.com/convert/power/dBm_to_mW.html
// P(mW) = 1mW ⋅ 10(P(dBm)/ 10)
static float milli_dbm_to_milli_watt(float milli_dbm) {
  double exponent = milli_dbm / 1000.0 / 10.0;
  auto ret = std::pow(10.0, exponent);
  return static_cast<float>(ret);
}

// P(dBm) = 10 ⋅ log10( P(mW) / 1mW)
static uint32_t milli_watt_to_milli_dbm(uint32_t milli_watt) {
  const double tmp = std::log10(static_cast<double>(milli_watt) / 1.0);
  const double milli_dbm = tmp * 10 * 100;
  // return static_cast<uint32_t>(milli_dbm);
  return std::lround(milli_dbm);
}
// However, this is weird:
// https://linux.die.net/man/8/iwconfig
// the power in dBm is P = 30 + 10.log(W)
// log10(x/1)==log(x) / log(10) = ~2.3

static uint32_t milli_watt_to_mBm(uint32_t milli_watt) {
  const double tmp = std::log10(static_cast<double>(milli_watt) / 1.0);
  const double milli_dbm = tmp * 10 * 100;
  // return static_cast<uint32_t>(milli_dbm);
  return std::lround(milli_dbm);
}

}  // namespace openhd

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
