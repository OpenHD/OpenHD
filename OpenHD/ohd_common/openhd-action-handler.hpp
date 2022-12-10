//
// Created by consti10 on 09.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_

#include <functional>
#include <mutex>
#include <utility>

#include "openhd-link-statistics.hpp"

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
    uint32_t recommended_encoder_bitrate_kbits;
  };
  typedef std::function<void(LinkBitrateInformation link_bitrate_info)> ACTION_REQUEST_BITRATE_CHANGE;
  static std::string link_bitrate_info_to_string(const LinkBitrateInformation& lb){
    std::stringstream ss;
    ss<<"recommended_encoder_bitrate:"<<lb.recommended_encoder_bitrate_kbits<<" kBit/s";
    return ss.str();
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
      cb(link_bitrate_info);
    }
  }
 public:
  // Link statistics - for that the wb link (ohd_interface) needs to talk to ohd_telemetry
  // register callback that is called in regular intervals with link statistics
  void action_wb_link_statistics_register(const openhd::link_statistics::STATS_CALLBACK& stats_callback){
    if(stats_callback== nullptr){
      m_link_statistics_callback= nullptr;
      return;
    }
    m_link_statistics_callback =std::make_shared<openhd::link_statistics::STATS_CALLBACK>(stats_callback);
  }
  void action_wb_link_statistcs_handle(openhd::link_statistics::StatsAirGround all_stats){
    auto tmp=m_link_statistics_callback;
    if(tmp){
      auto & cb=*tmp;
      cb(all_stats);
    }
  }
 public:
  struct ScanChannelsParam{
    bool scan_2G_channels;
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
  // Cleanup, set all lambdas that handle things to 0
  void disable_all_callables(){
    action_wb_link_statistics_register(nullptr);
    action_request_bitrate_change_register(nullptr);
    action_wb_link_statistics_register(nullptr);
  }
 private:
  // By using shared_ptr to wrap the stored the cb we are semi thread-safe
  std::shared_ptr<ACTION_REQUEST_BITRATE_CHANGE> m_action_request_bitrate_change =nullptr;
  std::shared_ptr<openhd::link_statistics::STATS_CALLBACK> m_link_statistics_callback=nullptr;
  std::shared_ptr<SCAN_CHANNELS_CB> m_scan_channels_cb=nullptr;
};

}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
