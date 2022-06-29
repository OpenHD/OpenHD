//
// Created by consti10 on 26.06.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_MSETTINGSCOMPONENT_H_
#define OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_MSETTINGSCOMPONENT_H_

#include <string>
#include <variant>
#include <utility>
#include <functional>

namespace openhd{

using SettingsVariant= std::variant<bool,int,float,std::string>;

struct Setting{
  const std::string id;
  SettingsVariant value;
};

struct mavlink_message_t{
  int lol=3;
};

using SettingChangedListener=std::function<void(const Setting& setting)>;

using SendMavlinkMessageCallback=std::function<void(const mavlink_message_t& msg)>;

class MSettingsComponent {
 public:
  MSettingsComponent(int sys_id,int comp_id,SendMavlinkMessageCallback cb):
  _sys_id(sys_id),_comp_id(comp_id),_send_mavlink_message(cb){

  }

  bool process_mavlink_message(const mavlink_message_t& msg){
    return false;
  }

  void provide_param(Setting setting,SettingChangedListener changed_listener){
  }
 private:
  std::vector<int> settings;
};

namespace ohdtest{

using SettingChangedListener=std::function<void(const Setting& setting)>;

template<class T>
struct Setting{
  const std::string id;
  T value;
};

class XUserSettingsManager{};
  public:
  private:

}

}

#endif  // OPENHD_OPENHD_OHD_COMMON_MAVLINK_SETTINGS_MSETTINGSCOMPONENT_H_
