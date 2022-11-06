//
// Created by consti10 on 18.07.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_

#include <vector>

// Helper for validating user-selectable settings
namespace openhd{

// Wifi channel and the corresponding frequency, in mHz.
// "standard" : listed under wikipedia or not.
struct WifiChannel{
  uint32_t frequency;
  uint32_t channel;
  bool is_standard;
};


// These are not valid 2.4G wifi channel(s) but some cards aparently can do them, too
// From https://github.com/OpenHD/linux/blob/092115ae6a980feaa09722690891d99da3afb55c/drivers/net/wireless/ath/ath9k/common-init.c#L39
// NOTE: channel and frequency seem to be off by one
static std::vector<WifiChannel> get_channels_below_standard_2G_wifi(){
  return std::vector<WifiChannel>{
      WifiChannel{2312, 34,false}, /* Channel XX */
      WifiChannel{2317, 35,false}, /* Channel XX */
      WifiChannel{2322, 36,false}, /* Channel XX */
      WifiChannel{2327, 37,false}, /* Channel XX */
      WifiChannel{2332, 38,false}, /* Channel XX */
      WifiChannel{2337, 39,false}, /* Channel XX */
      WifiChannel{2342, 40,false}, /* Channel XX */
      WifiChannel{2347, 41,false}, /* Channel XX */
      WifiChannel{2352, 42,false}, /* Channel XX */
      WifiChannel{2357, 43,false}, /* Channel XX */
      WifiChannel{2362, 44,false}, /* Channel XX */
      WifiChannel{2367, 45,false}, /* Channel XX */
      WifiChannel{2372, 46,false}, /* Channel XX */
      WifiChannel{2377, 47,false}, /* Channel XX */
      WifiChannel{2382, 48,false}, /* Channel XX */
      WifiChannel{2387, 49,false}, /* Channel XX */
      WifiChannel{2392, 50,false}, /* Channel XX */
      WifiChannel{2397, 51,false}, /* Channel XX */
      WifiChannel{2402, 52,false}, /* Channel XX */
      WifiChannel{2407, 53,false}, /* Channel XX */
  };
}

// https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)
static std::vector<WifiChannel> get_channels_2G_standard() {
  return std::vector<WifiChannel>{
	  WifiChannel{2412,1,true},
	  WifiChannel{2417,2,true},
	  WifiChannel{2422,3,true},
	  WifiChannel{2427,4,true},
	  WifiChannel{2432,5,true},
	  WifiChannel{2437,6,true},
	  WifiChannel{2442,7,true},
	  WifiChannel{2447,8,true},
	  WifiChannel{2452,9,true},
	  WifiChannel{2457,10,true},
	  WifiChannel{2462,11,true},
          // Temporary disabled - they won't work unil we patch this shit in the kernel
	  WifiChannel{2467,12,true},
	  WifiChannel{2472,13,true},
	  // until here it is consistent (5Mhz increments)
	  // this one is neither allowed in EU nor USA
	  // (only japan under 11b)
	  //WifiChannel{2484,14},
  };
};

// These are not valid 2.4G wifi channel(s) but some cards apparently can do them, too
// From https://github.com/OpenHD/linux/blob/092115ae6a980feaa09722690891d99da3afb55c/drivers/net/wireless/ath/ath9k/common-init.c#L39
// NOTE: channel and frequency seem to be off by one
static std::vector<WifiChannel> get_channels_above_standard_2G_wifi(){
  return std::vector<WifiChannel>{
      WifiChannel{2478, 15,false}, /* Channel XX */
      WifiChannel{2482, 16,false}, /* Channel XX */
      WifiChannel{2484, 17,false}, /* Channel 14 */
      WifiChannel{2487, 18,false}, /* Channel XX */
      WifiChannel{2489, 19,false}, /* Channel XX */
      WifiChannel{2492, 20,false}, /* Channel XX */
      WifiChannel{2494, 21,false}, /* Channel XX */
      WifiChannel{2497, 22,false}, /* Channel XX */
      WifiChannel{2499, 23,false}, /* Channel XX */
      WifiChannel{2512, 24,false}, /* Channel XX */
      WifiChannel{2532, 25,false}, /* Channel XX */
      WifiChannel{2572, 26,false}, /* Channel XX */
      WifiChannel{2592, 27,false}, /* Channel XX */
      WifiChannel{2612, 28,false}, /* Channel XX */
      WifiChannel{2632, 29,false}, /* Channel XX */
      WifiChannel{2652, 30,false}, /* Channel XX */
      WifiChannel{2672, 31,false}, /* Channel XX */
      WifiChannel{2692, 32,false}, /* Channel XX */
      WifiChannel{2712, 33,false}, /* Channel XX */
      WifiChannel{2732, 34,false}, /* Channel XX */
  };
}

static std::vector<WifiChannel> get_channels_2G(const bool include_nonstandard_channels){
  auto ret= get_channels_2G_standard();
  if(include_nonstandard_channels){
    auto below=get_channels_below_standard_2G_wifi();
    ret.insert(ret.end(),below.begin(),below.end());
    auto above=get_channels_above_standard_2G_wifi();
    ret.insert(ret.end(),above.begin(),above.end());
  }
  return ret;
}

// These are not valid 2.4G wifi channel(s) but some cards aparently can do them, too
// From https://github.com/OpenHD/linux/blob/092115ae6a980feaa09722690891d99da3afb55c/drivers/net/wireless/ath/ath9k/common-init.c#L39
// NOTE: channel and frequency seem to be off by one
static std::vector<WifiChannel> get_channels_5G_below(){
  return std::vector<WifiChannel>{
      WifiChannel{4920, 54,false}, /* Channel XX */
      WifiChannel{4940, 55,false}, /* Channel XX */
      WifiChannel{4960, 56,false}, /* Channel XX */
      WifiChannel{4980, 57,false}, /* Channel XX */
  };
}

// https://en.wikipedia.org/wiki/List_of_WLAN_channels#5_GHz_(802.11a/h/j/n/ac/ax)
// These are what iw list lists for rtl8812au
static std::vector<WifiChannel> get_channels_5G_rtl8812au() {
  return std::vector<WifiChannel>{
	  //TODO {5170,34},
	  {5180,36,true},
	  {5200,40,true},
	  {5220,44,true},
	  {5240,48,true},
	  {5260,52,true},
	  {5280,56,true},
	  {5300,60,true},
	  {5320,64,true},
	  {5500,100,true},
	  {5520,104,true},
	  {5540,108,true},
	  {5560,112,true},
	  {5580,116,true},
	  {5600,120,true},
	  {5620,124,true},
	  {5640,128,true},
	  {5660,132,true},
	  {5680,136,true},
	  {5700,140,true},
	  {5720,144,true},
	  {5745,149,true},
	  {5765,153,true},
	  {5785,157,true},
	  {5805,161,true},
	  {5825,165,true},
	  {5845,169,true},
	  {5865,173,true},
	  {5885,177,true},
  };
};

static std::vector<WifiChannel> get_channels_5G(const bool include_nonstandard_channels){
  auto ret= get_channels_5G_rtl8812au();
  if(include_nonstandard_channels){
    auto below=get_channels_5G_below();
    ret.insert(ret.end(),below.begin(),below.end());
  }
  return ret;
}

static bool is_valid_frequency_2G(int frequency,bool include_nonstandard_channels=false){
  const auto supported=get_channels_2G(include_nonstandard_channels);
  for(const auto& value:supported){
	if(value.frequency==frequency)return true;
  }
  return false;
}
static bool is_valid_frequency_5G(int frequency,bool include_nonstandard_channels=false){
  const auto supported=get_channels_5G(include_nonstandard_channels);
  for(const auto& value:supported){
	if(value.frequency==frequency)return true;
  }
  return false;
}

// returns true if the frequency is a 2.4G frequency, otherwise it returns false
// AND also guarantees the frequency set is a valid 5G frequency (aka Wi-Fi cards can have a 2G or 5G frequency set,
// but should never have a frequency that is neither 2G nor 5G
static bool is_2G_and_assert(uint32_t frequency){
  if(openhd::is_valid_frequency_2G(frequency,true))return true;
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
  return static_cast<uint32_t>(milli_dbm);
}
// However, this is weird:
// https://linux.die.net/man/8/iwconfig
// the power in dBm is P = 30 + 10.log(W)
// log10(x/1)==log(x) / log(10) = ~2.3


}

#endif //OPENHD_OPENHD_OHD_INTERFACE_INC_VALIDATE_SETTINGS_HELPER_H_
