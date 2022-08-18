//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_

#include <vector>

// Helper for validating user-selectable settings
namespace openhd{

// Wifi channel and the corresponding frequency, in mHz
struct WifiChannel{
  const uint32_t frequency;
  const uint32_t channel;
};

static std::vector<WifiChannel> get_channels_2G() {
  return std::vector<WifiChannel>{
	  WifiChannel{2412,1},
	  WifiChannel{2417,2},
	  WifiChannel{2422,3},
	  WifiChannel{2427,4},
	  WifiChannel{2432,5},
	  WifiChannel{2437,6},
	  WifiChannel{2442,7},
	  WifiChannel{2447,8},
	  WifiChannel{2452,9},
	  WifiChannel{2457,10},
	  WifiChannel{2462,11},
	  WifiChannel{2467,12},
	  WifiChannel{2472,13},
	  // until here it is consistent (5Mhz increments)
	  // this one is neither allowed in EU nor USA
	  // (only japan under 11b)
	  WifiChannel{2484,14},
  };
};


// https://en.wikipedia.org/wiki/List_of_WLAN_channels#5_GHz_(802.11a/h/j/n/ac/ax)
// These are what iw list lists for rtl8812au
static std::vector<WifiChannel> get_channels_5G_rtl8812au() {
  return std::vector<WifiChannel>{
	  //TODO {5170,34},
	  {5180,36},
	  {5200,40},
	  {5220,44},
	  {5240,48},
	  {5260,52},
	  {5280,56},
	  {5300,60},
	  {5320,64},
	  {5500,100},
	  {5520,104},
	  {5540,108},
	  {5560,112},
	  {5580,116},
	  {5600,120},
	  {5620,124},
	  {5640,128},
	  {5660,132},
	  {5680,136},
	  {5700,140},
	  {5720,144},
	  {5745,149},
	  {5765,153},
	  {5785,157},
	  {5805,161},
	  {5825,165},
	  {5845,169},
	  {5865,173},
	  {5885,177},
  };
};

static bool is_valid_frequency_2G(int frequency){
  const auto supported=get_channels_2G();
  for(const auto& value:supported){
	if(value.frequency==frequency)return true;
  }
  return false;
}
static bool is_valid_frequency_5G(int frequency){
  const auto supported=get_channels_5G_rtl8812au();
  for(const auto& value:supported){
	if(value.frequency==frequency)return true;
  }
  return false;
}

// returns true if the frequency is a 2.4G frequency, otherwise it returns false
// AND also guarantees the frequency set is a valid 5G frequency (aka Wi-Fi cards can have a 2G or 5G frequency set,
// but should never have a frequency that is neither 2G nor 5G
static bool is_2G_and_assert(uint32_t frequency){
  if(openhd::is_valid_frequency_2G(frequency))return true;
  assert(openhd::is_valid_frequency_5G(frequency));
  return false;
}

static bool is_valid_channel_width(int channel_width){
  return channel_width==20 || channel_width==40;
}

static bool is_valid_mcs_index(int mcs_index){
  return mcs_index>=1 && mcs_index<=7;
}

// Internally, OpenHD uses milli watt (mW)
static bool is_valid_tx_power_milli_watt(int tx_power_mw){
  return tx_power_mw>=10 && tx_power_mw<= 2000;
}

static bool is_valid_fec_block_length(int block_length){
  return block_length>=1 && block_length <100;
}
// max 100% fec (2x the amount of data), this is already too much
static bool is_valid_fec_percentage(int fec_perc){
  return fec_perc>0 && fec_perc<100;
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
  return static_cast<uint32_t>(milli_dbm);
}
// However, this is weird:
// https://linux.die.net/man/8/iwconfig
// the power in dBm is P = 30 + 10.log(W)
// log10(x/1)==log(x) / log(10) = ~2.3


}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
