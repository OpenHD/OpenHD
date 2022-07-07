//
// Created by consti10 on 15.06.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_MAVLINKPARAMEXT_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_MAVLINKPARAMEXT_H_


#ifdef __cplusplus
extern "C" {
#endif
//NOTE: Make sure to include the openhd mavlink flavour, otherwise the custom messages won't bw parsed.
#include <openhd/mavlink.h>
#ifdef __cplusplus
}
#endif

#include <utility>
#include <variant>
#include <string>
#include <functional>


// Ideally, we'd want to support all the types specified here:
// https://mavlink.io/en/messages/common.html#MAV_PARAM_EXT_TYPE
// But for now, I've only implemented the basic ones.
class Value {
 public:
  void set(std::string& new_value){this->value=new_value;};
  std::string get(){return value;};
 private:
  // right now we only support int,float and string.
  //std::variant<int,float,std::string> _value{};
  std::string value;
};

// A (mavlink ext parameters) parameter could also be called a settings tuple.
// It consists of
// 1) a unique string id,
// 2) a unique int index and
// 3) a value of a generic type (see above)
class Parameter{
 public:
  Parameter(std::string id,int index):m_id(std::move(id)),m_index(index){

                                                              };
  Value get_value(){return m_value;};
  void set_value(Value value){
    m_value=std::move(value);
  }
  // since m_id and m_index are immutable, I just like to make them public
 public:
  // unique id of this parameter, never changes
  const std::string m_id;
  // unique index of this parameter, never changes
  const int m_index;
 private:
  Value m_value;
};


// This class holds settings (where settings are tuple of the form (string_id : generic type).
// One can change the settings this instance holds via mavlink commands, and then react to these settings changes
// via the appropriate callbacks.
// See https://mavlink.io/en/services/parameter_ext.html
// One could also call this a Parameter "server", but I decided to avoid the term server here.
class ParamExtProvider{
  // we respond only to commands with this sys_id as target
  const int sys_id=1;
  // we respond only to commands with this comp_id as target
  const int comp_id=1;

 public:
  // A instance of this callback can be passed to every provide_xxx method to react to changes of this settings value.
  // aka every time the value referenced by id is changed, this callback is called
  typedef std::function<void(const std::string& id,int value)> PARAM_CHANGED_CALLBACK;
  void provide_param_int(std::string id,int value){

  }
 private:
  void onNewMavlinkMessage(const mavlink_message_t msg){
    switch (msg.msgid) {
      case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST:{
        // https://mavlink.io/en/messages/common.html#PARAM_EXT_REQUEST_LIST
        handle_message_param_ext_request_list(msg);
        break;
      }
      case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ: {
        //https://mavlink.io/en/messages/common.html#PARAM_EXT_REQUEST_READ
        handle_message_param_ext_request_read(msg);
        break;
      }
      case MAVLINK_MSG_ID_PARAM_EXT_SET:{
        //https://mavlink.io/en/messages/common.html#PARAM_EXT_SET
        handle_message_param_ext_set(msg);
        break;
      }
      default:
        break;
    }
  }

  // https://mavlink.io/en/messages/common.html#PARAM_EXT_REQUEST_LIST
  void handle_message_param_ext_request_list(const mavlink_message_t& msg){
    mavlink_param_ext_request_list_t payload;
    mavlink_msg_param_ext_request_list_decode(&msg,&payload);
    // we just get all the parameters and send them out

  }
  //https://mavlink.io/en/messages/common.html#PARAM_EXT_REQUEST_READ
  void handle_message_param_ext_request_read(const mavlink_message_t& msg){
    mavlink_param_ext_request_read_t payload;
    mavlink_msg_param_ext_request_read_decode(&msg,&payload);
  }
  //https://mavlink.io/en/messages/common.html#PARAM_EXT_SET
  void handle_message_param_ext_set(const mavlink_message_t& msg){
    mavlink_param_ext_set_t payload;
    mavlink_msg_param_ext_set_decode(&msg,&payload);
  }

  // Send a parameter as a response to a request list or request read command
  static void send_param(const int target_sys_id,const int target_comp_id,const Parameter& param){
    mavlink_param_value_t l;
    mavlink_param_set_t l2;
    mavlink_param_ext_value_t l3;
    mavlink_param_ext_set_t l4;
    mavlink_param_ext_request_list_t x;
    //mavlink_msg_param_set_pack()
  }


};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_INTERNAL_MAVLINKPARAMEXT_H_
