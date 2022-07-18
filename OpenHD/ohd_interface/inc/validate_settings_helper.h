//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_

#include <vector>

// Helper for validating user-selectable settings
namespace openhd{

static std::vector<uint32_t> get_frequencies_2G() {
  return std::vector<uint32_t>{
	  2412,
	  2417,
	  2422,
	  2427,
	  2432,
	  2437,
	  2442,
	  2447,
	  2452,
	  2457,
	  2462,
	  2467,
	  2472,
	  2484,
  };
};

static std::vector<uint32_t> get_frequencies_5G() {
  return std::vector<uint32_t>{
	  5180,
	  5200,
	  5220,
	  5240,
	  5260,
	  5280,
	  5300,
	  5320,
	  5340,
	  5360,
	  5380,
	  5400,
	  5420,
	  5440,
	  5460,
	  5480,
	  5500,
	  5520,
	  5540,
	  5560,
	  5580,
	  5600,
	  5620,
	  5640,
	  5660,
	  5680,
	  5700,
	  5720,
	  5745,
	  5765,
	  5785,
	  5805,
	  5825,
	  5845,
	  5865,
	  5885,
	  5905
  };
};


static bool is_valid_frequency_2G(uint32_t frequency){
  const auto supported=get_frequencies_2G();
  for(const auto& value:supported){
	if(value==frequency)return true;
  }
  return false;
}
static bool is_valid_frequency_5G(uint32_t frequency){
  const auto supported=get_frequencies_5G();
  for(const auto& value:supported){
	if(value==frequency)return true;
  }
  return false;
}

static bool is_valid_channel_width(uint32_t channel_width){
  return channel_width==20 || channel_width==40;
}

static bool is_valid_mcs_index(uint32_t mcs_index){
  return mcs_index<=7;
}

// think it is always in milli dbm
static bool validate_tx_power(uint32_t tx_power){
  return tx_power < 4000;
}

}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
