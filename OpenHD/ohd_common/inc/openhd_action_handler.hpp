//
// Created by consti10 on 09.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_

#include <functional>
#include <mutex>
#include <utility>
#include <map>

#include "openhd_link_statistics.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"

// This class exists to handle the rare case(s) when one openhd module needs to talk to another.
// For example, the wb link (ohd_interface) might request a lower encoder bitrate (ohd_video)
// Since we do not have any code dependencies between them, we have a single shared action handler for that
// which calls the appropriate registered cb (if it has been registered)
namespace openhd{

// In a few places inside openhd we need to react to changes on the FC arming state.
// Here once can register / unregister a cb that is called whenever the arming state changes
class ArmingStateHelper{
public:
    typedef std::function<void(const bool armed)> STATE_CHANGED_CB;
    void register_listener(const std::string& tag,STATE_CHANGED_CB cb){
        assert(m_cbs.find(tag)==m_cbs.end());
        m_cbs[tag]=std::move(cb);
    }
    void unregister_listener(const std::string& tag){
        auto element=m_cbs.find(tag);
        if(element==m_cbs.end()){
            openhd::log::get_default()->warn("Cannot unregister arming listener {}",tag);
            return;
        }
        m_cbs.erase(element);
    }
    // For fetching the arming state in a manner where a deterministic arm / disarm pattern is not needed
    bool is_currently_armed(){
        return m_is_armed;
    }
    void update_arming_state_if_changed(bool armed) {
        if (m_is_armed == armed)return;
        m_is_armed = armed;
        m_console->debug("MAV armed:{}, calling listeners.",OHDUtil::yes_or_no(armed));
        for(auto& element: m_cbs){
            //m_console->debug("Calling {},begin",element.first);
            element.second(armed);
            //m_console->debug("Calling {},end",element.first);
        }
        m_console->debug("Done calling listeners.");
    }
    void disable_all(){
        m_cbs.clear();
    }
private:
    std::atomic_bool m_is_armed=false;
    std::map<std::string,STATE_CHANGED_CB> m_cbs;
    std::shared_ptr<spdlog::logger> m_console=openhd::log::create_or_get("ArmingStateHelper");
};
// In (only one) place right now we need to react to changes on the RC channels the FC reports
class FCRcChannelsHelper{
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
    void action_on_any_rc_channel_register(ACTION_ON_ANY_RC_CHANNEL_CB cb){
        if(cb== nullptr){
            m_action_rc_channel= nullptr;
            return;
        }
        m_action_rc_channel=std::make_shared<ACTION_ON_ANY_RC_CHANNEL_CB>(cb);
    }
private:
    std::shared_ptr<ACTION_ON_ANY_RC_CHANNEL_CB> m_action_rc_channel =nullptr;
};

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
  std::function<bool(ScanChannelsParam)> wb_cmd_scan_channels= nullptr;
public:
  // Cleanup, set all lambdas that handle things to nullptr
  void disable_all_callables(){
    action_request_bitrate_change_register(nullptr);
    fc_rc_channels.action_on_any_rc_channel_register(nullptr);
    arm_state.disable_all();
    wb_cmd_scan_channels= nullptr;
    wb_cmd_analyze_channels= nullptr;
    wb_get_supported_channels= nullptr;
  }
public:
  ArmingStateHelper arm_state;
  FCRcChannelsHelper fc_rc_channels;
 private:
  // By using shared_ptr to wrap the stored the cb we are semi thread-safe
  std::shared_ptr<ACTION_REQUEST_BITRATE_CHANGE> m_action_request_bitrate_change =nullptr;
  std::shared_ptr<openhd::link_statistics::STATS_CALLBACK> m_link_statistics_callback=nullptr;
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
public:
    std::function<std::vector<uint16_t>()> wb_get_supported_channels= nullptr;
    std::function<bool()> wb_cmd_analyze_channels= nullptr;

public:
    std::atomic<int> scan_channels_air_unit_progress=-1;
public:
    struct AnalyzeChannelsResult{
        std::array<uint16_t,30> channels_mhz{0};
        std::array<uint16_t,30> foreign_packets{0};
        int8_t progress;
    };
    void add_analyze_result(AnalyzeChannelsResult scan_result){
        std::lock_guard<std::mutex> guard(m_scan_results_mutex);
        m_scan_results.push_back(scan_result);
    }
    std::vector<AnalyzeChannelsResult> get_analyze_results(){
        std::lock_guard<std::mutex> guard(m_scan_results_mutex);
        auto ret=m_scan_results;
        m_scan_results.clear();
        return ret;
    }
private:
    std::mutex m_scan_results_mutex;
    std::vector<AnalyzeChannelsResult> m_scan_results;
public:
    struct ScanChannelsProgress{
        uint16_t channel_mhz;
        uint8_t progress;
        uint8_t channel_width_mhz;
        bool success;
    };
    void add_scan_channels_progress(ScanChannelsProgress val){
        std::lock_guard<std::mutex> guard(m_scan_channels_progress_mutex);
        m_scan_channels_progress.push_back(val);
    }
    std::vector<ScanChannelsProgress> get_scan_channels_progress(){
        std::lock_guard<std::mutex> guard(m_scan_channels_progress_mutex);
        auto ret=m_scan_channels_progress;
        m_scan_channels_progress.clear();
        return ret;
    }
private:
    std::mutex m_scan_channels_progress_mutex;
    std::vector<ScanChannelsProgress> m_scan_channels_progress;
};

}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
