//
// Created by consti10 on 09.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_

#include <functional>
#include <mutex>
#include <utility>

#include "openhd_link_statistics.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"

// This class exists to handle the rare case(s) when one openhd module needs to talk to another.
// For example, the wb link (ohd_interface) might request a lower encoder bitrate (ohd_video)
// Since we do not have any code dependencies between them, we have a single shared action handler for that
// which calls the appropriate registered cb (if it has been registered)
namespace openhd{

class ActionHandler{
 public:
  ActionHandler()=default;
  // delete copy and move constructor
  ActionHandler(const ActionHandler&)=delete;
  ActionHandler(const ActionHandler&&)=delete;
 public:
  // Link bitrate change request
  struct LinkBitrateInformation{
    int recommended_encoder_bitrate_kbits;
  };
  typedef std::function<void(LinkBitrateInformation link_bitrate_info)> ACTION_REQUEST_BITRATE_CHANGE;
  static std::string link_bitrate_info_to_string(const LinkBitrateInformation& lb){
    return fmt::format("[recommended_encoder_bitrate:{}kBit/s}",lb.recommended_encoder_bitrate_kbits);
  }
  // used by ohd_video
  void action_request_bitrate_change_register(const ACTION_REQUEST_BITRATE_CHANGE& cb){
    if(cb== nullptr){
      m_action_request_bitrate_change= nullptr;
      return;
    }
    m_action_request_bitrate_change=std::make_shared<ACTION_REQUEST_BITRATE_CHANGE>(cb);
  }
  // called by ohd_interface / wb
  void action_request_bitrate_change_handle(LinkBitrateInformation link_bitrate_info){
    //openhd::log::get_default()->debug("action_request_bitrate_change_handle {}", link_bitrate_info_to_string(link_bitrate_info));
    auto tmp=m_action_request_bitrate_change;
    if(tmp){
      auto& cb=*tmp;
      // The cb will update setting the global atomic value for cam1 / cam2 accordingly
      cb(link_bitrate_info);
    }
  }
 public:
  // checking both 2G and 5G channels takes really long, but in rare cases might be wanted by the user
  // checking both 20Mhz and 40Mhz (instead of only either of them both) also duplicates the scan time
  struct ScanChannelsParam{
    bool check_2g_channels_if_card_support=false;
    bool check_5g_channels_if_card_supports=false;
    bool check_20Mhz_channel_width_if_card_supports=false;
    bool check_40Mhz_channel_width_if_card_supports=false;
  };
  typedef std::function<void(ScanChannelsParam)> SCAN_CHANNELS_CB;
  void action_wb_link_scan_channels_register(const SCAN_CHANNELS_CB& cb){
    if(cb== nullptr){
      m_scan_channels_cb= nullptr;
      return;
    }
    m_scan_channels_cb=std::make_shared<SCAN_CHANNELS_CB>(cb);
  }
  void action_wb_link_scan_channels_handle(ScanChannelsParam params){
    auto tmp=m_scan_channels_cb;
    if(tmp){
      SCAN_CHANNELS_CB& cb=*tmp;
      cb(params);
    }
  }
  // Cleanup, set all lambdas that handle things to nullptr
  void disable_all_callables(){
    action_request_bitrate_change_register(nullptr);
    action_wb_link_scan_channels_register(nullptr);
    action_on_ony_rc_channel_register(nullptr);
    m_action_disable_wifi_when_armed= nullptr;
  }
  // Allows registering actions when vehicle / FC is armed / disarmed
 public:
  void update_arming_state_if_changed(bool armed){
    if(m_is_armed==armed)return;
    m_is_armed=armed;
    openhd::log::get_default()->debug("MAV armed:{}",OHDUtil::yes_or_no(armed));
    {
      // Needs to be called when armed / disarmed
      auto tmp=m_action_record_video_when_armed;
      if(tmp){
        ACTION_RECORD_VIDEO_WHEN_ARMED cb=*tmp;
        cb(armed);
      }
    }
    {
      // Also needs to be called when armed / disarmed
      auto tmp=m_action_tx_power_when_armed;
      if(tmp){
        ACTION_TX_POWER_WHEN_ARMED cb=*tmp;
        cb(armed);
      }
    }
    // We only need to call this when armed (not disarmed)
    if(m_is_armed){
      auto tmp=m_action_disable_wifi_when_armed;
      if(tmp){
        ACTION_DISABLE_WIFI_WHEN_ARMED cb=*tmp;
        cb();
      }
    }
  }
  // For fetching the arming state in a manner where a deterministic arm / disarm pattern is not needed
  bool is_currently_armed(){
    return m_is_armed;
  }
 private:
  bool m_is_armed=false;
  // Allow registering actions when rc channel goes to a specific value
 public:
  typedef std::function<void(const std::array<int,18>& rc_channels)> ACTION_ON_ANY_RC_CHANNEL_CB;
  // called every time a rc channel value(s) mavlink packet is received from the FC
  // (regardless if there was an actual change on any of the channels or not)
  // Works well on Ardupilot, which broadcasts the proper telem message by default
  void update_rc_channels(const std::array<int,18>& rc_channels){
    auto tmp=m_action_rc_channel;
    if(tmp){
      ACTION_ON_ANY_RC_CHANNEL_CB cb=*tmp;
      cb(rc_channels);
    }
  }
  void action_on_ony_rc_channel_register(ACTION_ON_ANY_RC_CHANNEL_CB cb){
    if(cb== nullptr){
      m_action_rc_channel= nullptr;
      return;
    }
    m_action_rc_channel=std::make_shared<ACTION_ON_ANY_RC_CHANNEL_CB>(cb);
  }
 private:
  std::shared_ptr<ACTION_ON_ANY_RC_CHANNEL_CB> m_action_rc_channel =nullptr;
 public:
  // this is called once the first "armed==true" message from the FC is received - in which case
  // we will automatically disable wifi hotspot, if it is enabled.
  typedef std::function<void()> ACTION_DISABLE_WIFI_WHEN_ARMED;
  std::shared_ptr<ACTION_DISABLE_WIFI_WHEN_ARMED> m_action_disable_wifi_when_armed =nullptr;
  // For automatically stop recording when armed / disarmed
  typedef std::function<void(bool armed)> ACTION_RECORD_VIDEO_WHEN_ARMED;
  std::shared_ptr<ACTION_RECORD_VIDEO_WHEN_ARMED> m_action_record_video_when_armed =nullptr;
  // For increasing / decreasing TX power when armed / disarmed
  typedef std::function<void(bool armed)> ACTION_TX_POWER_WHEN_ARMED;
  std::shared_ptr<ACTION_TX_POWER_WHEN_ARMED> m_action_tx_power_when_armed =nullptr;
 private:
  // By using shared_ptr to wrap the stored the cb we are semi thread-safe
  std::shared_ptr<ACTION_REQUEST_BITRATE_CHANGE> m_action_request_bitrate_change =nullptr;
  std::shared_ptr<openhd::link_statistics::STATS_CALLBACK> m_link_statistics_callback=nullptr;
  std::shared_ptr<SCAN_CHANNELS_CB> m_scan_channels_cb=nullptr;
 public:
  // Camera stats / info that is broadcast in regular intervals
  // Set by the camera streaming implementation - read by OHDMainComponent (mavlink broadcast)
  // Simple read - write pattern (mutex is a bit overkill, but we don't have atomic struct)
  struct CamInfo{
    bool active= false; // Do not send stats for a non-active camera
    uint8_t cam_index=0;
    uint8_t cam_type=0;
    uint8_t cam_status=0;
    uint8_t air_recording_active=0;
    uint8_t encoding_format=0;
    uint16_t encoding_bitrate_kbits=0;
    uint8_t encoding_keyframe_interval=0;
    uint16_t stream_w=0;
    uint16_t stream_h=0;
    uint16_t stream_fps=0;
    uint8_t supports_variable_bitrate=0;
  };
  void set_cam_info(uint8_t cam_index,CamInfo camInfo){
    if(cam_index==0){
      std::lock_guard<std::mutex> lock(m_cam_info_cam1_mutex);
      m_cam_info_cam1=camInfo;
    }else{
      std::lock_guard<std::mutex> lock(m_cam_info_cam2_mutex);
      m_cam_info_cam2=camInfo;
    }
  }
  void set_cam_info_bitrate(uint8_t cam_index,uint16_t bitrate_kbits){
    if(cam_index==0){
      std::lock_guard<std::mutex> lock(m_cam_info_cam1_mutex);
      m_cam_info_cam1.encoding_bitrate_kbits=bitrate_kbits;
    }else{
      std::lock_guard<std::mutex> lock(m_cam_info_cam2_mutex);
      m_cam_info_cam2.encoding_bitrate_kbits=bitrate_kbits;
    }
  }
  void set_cam_info_status(uint8_t cam_index,uint8_t status){
      if(cam_index==0){
          std::lock_guard<std::mutex> lock(m_cam_info_cam1_mutex);
          m_cam_info_cam1.cam_status=status;
      }else{
          std::lock_guard<std::mutex> lock(m_cam_info_cam2_mutex);
          m_cam_info_cam2.cam_status=status;
      }
  }
  CamInfo get_cam_info(int cam_index){
    if(cam_index==0){
      std::lock_guard<std::mutex> lock(m_cam_info_cam1_mutex);
      return m_cam_info_cam1;
    }
    std::lock_guard<std::mutex> lock(m_cam_info_cam2_mutex);
    return m_cam_info_cam2;
  }
 private:
  CamInfo m_cam_info_cam1{};
  CamInfo m_cam_info_cam2{};
  std::mutex m_cam_info_cam1_mutex;
  std::mutex m_cam_info_cam2_mutex;
  // LINK STATISTICS
  // Written by wb_link, published via mavlink by telemetry OHDMainComponent
 private:
  std::mutex m_last_link_stats_mutex;
  openhd::link_statistics::StatsAirGround m_last_link_stats{};
 public:
  void update_link_stats(openhd::link_statistics::StatsAirGround stats){
    std::lock_guard<std::mutex> guard(m_last_link_stats_mutex);
    m_last_link_stats=std::move(stats);
  }
  openhd::link_statistics::StatsAirGround get_link_stats(){
    std::lock_guard<std::mutex> guard(m_last_link_stats_mutex);
    return m_last_link_stats;
  }
};

}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
