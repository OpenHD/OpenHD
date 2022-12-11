//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_

#include <vector>

#include "wifi_channel.h"

// Helper for validating user-selectable settings

static bool is_valid_frequency_2G(uint32_t frequency,bool include_nonstandard_channels=false){
  const auto supported=openhd::get_channels_2G(include_nonstandard_channels);
  for(const auto& value:supported){
    if(value.frequency==frequency)return true;
  }
  return false;
}
static bool is_valid_frequency_5G(uint32_t frequency,bool include_nonstandard_channels=false){
  const auto supported=get_channels_5G(include_nonstandard_channels);
  for(const auto& value:supported){
    if(value.frequency==frequency)return true;
  }
  return false;
}

static bool is_valid_channel_width(uint32_t channel_width){
  return channel_width==20 || channel_width==40;
}

static bool is_valid_mcs_index(uint32_t mcs_index){
  return mcs_index>=1 && mcs_index<=7;
}

// Internally, OpenHD uses milli watt (mW)
// No wifi card will ever do 30W, but some cards increase their tx power a bit more
// when you set a higher value (I think)
static bool is_valid_tx_power_milli_watt(int tx_power_mw){
  return tx_power_mw>=10 && tx_power_mw<= 30*1000;
}

// NOTE: 0 means variable fec, video codec has to be set in this case
static bool is_valid_fec_block_length(int block_length){
  return block_length>=0 && block_length <100;
}
// max 100% fec (2x the amount of data), this is already too much
// 21.10: Using more than 2x for FEC can be usefully for testing
static bool is_valid_fec_percentage(int fec_perc){
  return fec_perc>0 && fec_perc<=400;
}

// https://www.rapidtables.com/convert/power/dBm_to_mW.html
// P(mW) = 1mW ⋅ 10(P(dBm)/ 10)
static float milli_dbm_to_milli_watt(float milli_dbm){
  double exponent=milli_dbm / 1000.0 / 10.0;
  auto ret= std::pow(10.0,exponent);
  return static_cast<float>(ret);
}

// P(dBm) = 10 ⋅ log10( P(mW) / 1mW)
static uint32_t milli_watt_to_milli_dbm(uint32_t milli_watt){
  const double tmp=std::log10(static_cast<double>(milli_watt)/1.0);
  const double milli_dbm=tmp*10*100;
  //return static_cast<uint32_t>(milli_dbm);
  return std::lround(milli_dbm);
}
// However, this is weird:
// https://linux.die.net/man/8/iwconfig
// the power in dBm is P = 30 + 10.log(W)
// log10(x/1)==log(x) / log(10) = ~2.3


static uint32_t milli_watt_to_mBm(uint32_t milli_watt){
  const double tmp=std::log10(static_cast<double>(milli_watt)/1.0);
  const double milli_dbm=tmp*10*100;
  //return static_cast<uint32_t>(milli_dbm);
  return std::lround(milli_dbm);
}

// TODO improve me -R.n I am really conservative here
// Returns the maximum theoretical bitrate in bits per second for the given configuration
/*static uint32_t get_max_rate_kbits_for_configuration(bool is_2g,uint32_t mcs_index,uint32_t channel_width){
  if(is_2g){
    // 2G is always 20Mhz channel width
    switch (mcs_index) {
      case 1:
        break;
    }
  }
  return 0;
}*/

// TODO improve me - R.n complete bollocks
static uint32_t get_max_rate_kbits(uint32_t mcs_index){
  switch (mcs_index) {
    case 1:
      return 5500;
    case 2:
      return 11000;
    case 3:
      return 12000;
    case 4:
      return 19500;
    case 5:
      return 24000;
    case 6:
      return 36000;
    default:
      break;
  }
  return 5500;
}



}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
