//
// Created by consti10 on 09.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_

#include <functional>
#include <mutex>
#include <utility>

// This way we can forward action(s) received from the mavlink telemetry module
// To other OpenHD modules.
// For example, action restart wb streams is obviously handled from ohd_interface but
// received via ohd_telemetry. Since we can't have any dependencies between them,
// we use this common file in both.
// This is similar how settings are handled.
namespace openhd{

class ActionHandler{
 public:
  ActionHandler()=default;
  // delete copy and move constructor
  ActionHandler(const ActionHandler&)=delete;
  ActionHandler(const ActionHandler&&)=delete;
  // for all the actions we have xxx_set (set the callback)
  // and xxx_handle (handle the callback if set).
  void action_restart_wb_streams_set(std::function<void()> action_restart_wb_streams){
    std::lock_guard<std::mutex> lock(_mutex);
    _action_restart_wb_streams=std::move(action_restart_wb_streams);
  }
  void action_restart_wb_streams_handle(){
    std::lock_guard<std::mutex> lock(_mutex);
    if(_action_restart_wb_streams){
      _action_restart_wb_streams();
    }
  }
  // The video codec set is one of the few changes we need to propagate from video to the rf wifibroadcast link
  void action_set_video_codec_set(std::function<void(int value)> action_set_video_codec){
    std::lock_guard<std::mutex> lock(_mutex);
    m_action_set_video_codec=std::move(action_set_video_codec);
  }
  void action_set_video_codec_handle(int value){
    std::lock_guard<std::mutex> lock(_mutex);
    if(m_action_set_video_codec){
      m_action_set_video_codec(value);
    }
  }
 private:
  std::mutex _mutex;
  std::function<void()> _action_restart_wb_streams=nullptr;
  std::function<void(int value)> m_action_set_video_codec=nullptr;
};

}

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_ACTION_HANDLER_HPP_
