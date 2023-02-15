//
// Created by consti10 on 21.11.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_BITRATE_CONVERSIONS_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_BITRATE_CONVERSIONS_H_

// NOTE: I am not completely sure, but the more common approach seems to multiply / divide by 1000
// When converting mBit/s to kBit/s or the other way around.
// Therefore, we have the conversions here globally, and it is recommended to use them instead of manually converting
// bit-rates by multiplication / division somewhere in code.

static int kbits_to_bits_per_second(int kbits_per_second){
  return kbits_per_second*1000;
}
static int kbits_to_mbits_per_second(int kbits_per_second){
  return kbits_per_second/1000;
}
static int mbits_to_kbits_per_second(int mbits_per_second){
  return mbits_per_second*1000;
}
static int bits_per_second_to_kbits_per_second(int bits_per_second){
  return bits_per_second/1000;
}

static std::string bits_per_second_to_string(uint64_t bits_per_second){
  const double mBits_per_second=static_cast<double>(bits_per_second)/(1000*1000);
  if(mBits_per_second>1){
    return std::to_string(mBits_per_second)+"mBit/s";
  }
  const double kBits_per_second=static_cast<double>(bits_per_second)/1000;
  return std::to_string(kBits_per_second)+"kBit/s";
}
static std::string kbits_per_second_to_string(uint64_t kbits_per_second){
  return bits_per_second_to_string(kbits_per_second*1000);
}

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_BITRATE_CONVERSIONS_H_
