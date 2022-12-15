//
// Created by consti10 on 13.07.22.
//

#include "wifi_command_helper.h"

#include "openhd-util.hpp"

static std::shared_ptr<spdlog::logger> get_logger(){
  return openhd::log::create_or_get("w_helper");
}

bool wifi::commandhelper::rfkill_unblock_all() {
  get_logger()->info("rfkill_unblock_all");
  std::vector<std::string> args{"unblock","all"};
  bool success=OHDUtil::run_command("rfkill",args);
  return success;
}

bool wifi::commandhelper::ip_link_set_card_state(const std::string &device,bool up) {
  get_logger()->info("ip_link_set_card_state {} up {}",device,up);
  std::vector<std::string> args{"link", "set", "dev",device, up ? "up" : "down"};
  bool success = OHDUtil::run_command("ip", args);
  return success;
}

bool wifi::commandhelper::iw_enable_monitor_mode(const std::string &device) {
  get_logger()->info("iw_enable_monitor_mode {}",device);
  std::vector<std::string> args{"dev", device, "set", "monitor", "otherbss"};
  bool success = OHDUtil::run_command("iw", args);
  return success;
}

static std::string channel_width_as_iw_string(uint32_t channel_width){
  std::string ret="HT20";
  if(channel_width==5){
    ret = "5MHz";
  }else if(channel_width==10){
    return "10Mhz";
  }else if(channel_width==40){
    ret="HT40+";
  }else{
    get_logger()->info("Invalid channel width {}, assuming HT20",channel_width);
  }
  return ret;
}

bool wifi::commandhelper::iw_set_frequency_and_channel_width(const std::string &device, uint32_t freq_mhz,uint32_t channel_width) {
  const std::string iw_channel_width= channel_width_as_iw_string(channel_width);
  get_logger()->info("iw_set_frequency_and_channel_width {} {}Mhz {}",device,freq_mhz,iw_channel_width);
  std::vector<std::string> args{"dev", device, "set", "freq", std::to_string(freq_mhz), iw_channel_width};
  const auto ret = OHDUtil::run_command("iw", args);
  if(ret!=0){
    get_logger()->warn("iw_set_frequency_and_channel_width failed {}",ret);
    return false;
  }
  return true;
}

bool wifi::commandhelper::iw_set_tx_power(const std::string &device,uint32_t tx_power_mBm) {
  get_logger()->info("iw_set_tx_power {} {} mBm",device,tx_power_mBm);
  std::vector<std::string> args{"dev",device, "set", "txpower", "fixed", std::to_string(tx_power_mBm)};
  const auto ret = OHDUtil::run_command("iw", args);
  if(ret!=0){
    get_logger()->warn("iw_set_tx_power failed {}",ret);
    return false;
  }
  return true;
}

bool wifi::commandhelper::nmcli_set_device_managed_status(const std::string &device,bool managed){
  get_logger()->info("nmcli_set_device_managed_status {} managed:{}",device,managed);
  std::vector<std::string> arguments{"device","set",device,"managed"};
  if(managed){
    arguments.emplace_back("no");
  }else{
    arguments.emplace_back("yes");
  }
  bool success = OHDUtil::run_command("nmcli",arguments);
  return success;
}


static std::string float_without_trailing_zeroes(const float value){
  std::stringstream ss;
  ss << std::noshowpoint << value;
  return ss.str();
}

std::vector<uint32_t> wifi::commandhelper::iw_get_supported_frequencies(const std::string& device,const std::vector<uint32_t> &frequencies_mhz_to_try) {
  const std::string command="iwlist "+device+" frequency";
  const auto res_op=OHDUtil::run_command_out(command);
  if(!res_op.has_value()){
    openhd::log::get_default()->warn("get_supported_channels for {} failed",device);
    return frequencies_mhz_to_try;
  }
  const auto& res=res_op.value();
  std::vector<uint32_t> supported_channels{};
  for(const auto& freq_mhz: frequencies_mhz_to_try){
    // annoying - iwlist reports them with decimals in GHz
    const float freq_ghz=static_cast<float>(freq_mhz) / 1000.0f;
    const auto s_freq_ghz= float_without_trailing_zeroes(freq_ghz);
    //openhd::log::get_default()->debug("checking [{}]",s_freq_ghz);
    if(OHDUtil::contains(res,s_freq_ghz)){
      //openhd::log::get_default()->debug("has [{}]",s_freq_ghz);
      supported_channels.push_back(freq_mhz);
    }else{
      //openhd::log::get_default()->debug("doesn't have [{}]",s_freq_ghz);
    }
  }
  return supported_channels;
}

wifi::commandhelper::SupportedFrequencyBand
wifi::commandhelper::iw_get_supported_frequency_bands(const std::string &device) {
  wifi::commandhelper::SupportedFrequencyBand ret{false, false};
  const std::string command="iwlist "+device+" frequency";
  const auto res_op=OHDUtil::run_command_out(command);
  if(!res_op.has_value()){
    openhd::log::get_default()->warn("iw_get_supported_frequency_bands for {} failed",device);
    return {true, true};
  }
  const auto& res=res_op.value();
  if(res.find("5.")!= std::string::npos){
    ret.supports_any_5G= true;
  }
  if(res.find("2.")!= std::string::npos){
    ret.supports_any_2G= true;
  }
  return ret;
}

bool wifi::commandhelper::iw_supports_monitor_mode(int phy_index) {
  const std::string command="iw phy phy"+std::to_string(phy_index)+" info";
  const auto res_opt=OHDUtil::run_command_out(command);
  if(!res_opt.has_value()){
    openhd::log::get_default()->warn("iw_supports_monitor_mode for phy{} failed,assuming can do monitor mode",phy_index);
    return true;
  }
  return OHDUtil::contains(res_opt.value(),"* monitor");
}

