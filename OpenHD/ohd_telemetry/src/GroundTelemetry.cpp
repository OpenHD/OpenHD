//
// Created by consti10 on 13.04.22.
//

#include "GroundTelemetry.h"

#include <iostream>
#include <chrono>

#include "mav_helper.h"
#include "openhd-temporary-air-or-ground.h"
#include "openhd_util_time.hpp"

GroundTelemetry::GroundTelemetry(OHDPlatform platform,
                                 std::shared_ptr<openhd::ActionHandler> opt_action_handler):
 _platform(platform),MavlinkSystem(OHD_SYS_ID_GROUND) {
  m_console = openhd::log::create_or_get("ground_tele");
  assert(m_console);
  m_groundTelemetrySettings=std::make_unique<openhd::telemetry::ground::SettingsHolder>();
  m_primary_localhost_gcs =
      std::make_unique<UDPEndpoint2>("GroundStationUDP",OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN,
                                     "127.0.0.1","127.0.0.1");
  m_primary_localhost_gcs->registerCallback([this](std::vector<MavlinkMessage> messages) {
    on_messages_ground_station_clients(messages);
  });
  m_ohd_main_component =std::make_shared<OHDMainComponent>(_platform,_sys_id,false,opt_action_handler);
  components.push_back(m_ohd_main_component);
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
  if(m_groundTelemetrySettings->get_settings().enable_rc_over_joystick){
    //m_joystick_reader=std::make_unique<JoystickReader>();
    m_rc_joystick_sender=std::make_unique<RcJoystickSender>([this](const MavlinkMessage &msg){
          send_messages_air_unit({msg});
      // temporary / hacky: Send the messages to QOpenHD, such that we can display it in the UI
          send_messages_ground_station_clients({msg});
    },m_groundTelemetrySettings->get_settings().rc_over_joystick_update_rate_hz,JoystickReader::get_default_channel_mapping());
    const auto parsed=JoystickReader::convert_string_to_channel_mapping(m_groundTelemetrySettings->get_settings().rc_channel_mapping);
    if(parsed==std::nullopt){
      m_console->warn("Not a valid channel mapping,using default");
    }else{
      m_rc_joystick_sender->update_channel_maping(parsed.value());
    }
    m_console->info("Joystick enabled");
  }else{
    m_console->info("Joystick disabled");
  }
#else
  m_console->info("No Joystick support");
#endif
  //
  // NOTE: We don't call set ready yet, since we have to wait until other modules have provided
  // all their parameters.
  generic_mavlink_param_provider=std::make_shared<XMavlinkParamProvider>(_sys_id,MAV_COMP_ID_ONBOARD_COMPUTER);
  generic_mavlink_param_provider->add_params(get_all_settings());
  components.push_back(generic_mavlink_param_provider);
  m_console->debug("Created GroundTelemetry");
}

GroundTelemetry::~GroundTelemetry() {
  // first, stop all the endpoints that have their own threads
  m_wb_endpoint = nullptr;
  m_primary_localhost_gcs = nullptr;
}

void GroundTelemetry::on_messages_air_unit(const std::vector<MavlinkMessage>& messages) {
  // All messages we get from the Air pi (they might come from the AirPi itself or the FC connected to the air pi)
  // get forwarded straight to all the client(s) connected to the ground station.
  send_messages_ground_station_clients(messages);
  // Note: No OpenHD component ever talks to another OpenHD component or the FC, so we do not
  // need to do anything else here.
}

void GroundTelemetry::on_messages_ground_station_clients(const std::vector<MavlinkMessage>& messages) {
  // All messages from the ground station(s) are forwarded to the air unit, unless they have a target sys id
  // of the ohd ground unit itself
  auto [generic,local_only]=split_into_generic_and_local_only(messages,OHD_SYS_ID_GROUND);
  send_messages_air_unit(generic);
  // OpenHD components running on the ground station don't need to talk to the air unit.
  // This is not exactly following the mavlink routing standard, but saves a lot of bandwidth.
  std::lock_guard<std::mutex> guard(components_lock);
  for(auto& component:components){
    const auto responses=component->process_mavlink_messages(messages);
    // for now, send to the ground station clients only
    send_messages_ground_station_clients(responses);
  }
}

void GroundTelemetry::send_messages_ground_station_clients(const std::vector<MavlinkMessage>& messages) {
  if (m_primary_localhost_gcs) {
    m_primary_localhost_gcs->sendMessages(messages);
  }
  std::lock_guard<std::mutex> guard(m_other_udp_ground_stations_lock);
  for (auto const& [key, val] : m_other_udp_ground_stations){
    val->sendMessages(messages);
  }
}

void GroundTelemetry::send_messages_air_unit(const std::vector<MavlinkMessage>& messages) {
  // transmit via wb / the abstract link we use for sending message(s) to the air unit
  if (m_wb_endpoint) {
    m_wb_endpoint->sendMessages(messages);
  }
}

void GroundTelemetry::loop_infinite(bool& terminate,const bool enableExtendedLogging) {
  const auto log_intervall=std::chrono::seconds(5);
  const auto loop_intervall=std::chrono::milliseconds(500);
  auto last_log=std::chrono::steady_clock::now();
  while (!terminate) {
    const auto loopBegin=std::chrono::steady_clock::now();
    if(std::chrono::steady_clock::now()-last_log>=log_intervall) {
      last_log = std::chrono::steady_clock::now();
      //m_console->debug("GroundTelemetry::loopInfinite()");
      // for debugging, check if any of the endpoints is not alive
      if (enableExtendedLogging && m_wb_endpoint) {
        m_console->debug(m_wb_endpoint->createInfo());
      }
      if (enableExtendedLogging && m_primary_localhost_gcs) {
        m_console->debug(m_primary_localhost_gcs->createInfo());
      }
    }
    // send messages to the ground station in regular intervals, includes heartbeat.
    // everything else is handled by the callbacks and their threads
    {
      std::lock_guard<std::mutex> guard(components_lock);
      for(auto& component:components){
        assert(component);
        const auto messages=component->generate_mavlink_messages();
        send_messages_ground_station_clients(messages);
        for(const auto& msg:messages){
          // r.n no ground unit component needs to talk to the air unit directly.
          // but we send heartbeats to the air pi anyways, just to keep the link active.
          if(msg.m.msgid==MAVLINK_MSG_ID_HEARTBEAT && msg.m.compid==MAV_COMP_ID_ONBOARD_COMPUTER){
            // but we send heartbeats to the air pi anyways, just to keep the link active.
            //m_console->debug("Heartbeat sent to air unit");
            send_messages_air_unit({msg});
          }
        }
      }
    }
    const auto loopDelta=std::chrono::steady_clock::now()-loopBegin;
    if(loopDelta>loop_intervall){
      // We can't keep up with the wanted loop interval
      // We can't keep up with the wanted loop interval
      m_console->debug("Warning GroundTelemetry cannot keep up with the wanted loop interval. Took {}",
                       openhd::util::time::R(loopDelta));
    }else{
      const auto sleepTime=loop_intervall-loopDelta;
      // send out in X second intervals
      std::this_thread::sleep_for(loop_intervall);
    }
  }
}

std::string GroundTelemetry::create_debug() const {
  std::stringstream ss;
  //ss<<"GT:\n";
  if (m_wb_endpoint) {
    ss<< m_wb_endpoint->createInfo();
  }
  if (m_primary_localhost_gcs) {
    ss<< m_primary_localhost_gcs->createInfo();
  }
  return ss.str();
}

void GroundTelemetry::add_settings_generic(const std::vector<openhd::Setting>& settings) {
  std::lock_guard<std::mutex> guard(components_lock);
  generic_mavlink_param_provider->add_params(settings);
  m_console->debug("Added parameter component");
}


void GroundTelemetry::settings_generic_ready() {
  generic_mavlink_param_provider->set_ready();
}

void GroundTelemetry::add_external_ground_station_ip(const openhd::ExternalDevice& ext_device) {
  m_console->debug("add_external_ground_station_ip {}",ext_device.to_string());
  std::lock_guard<std::mutex> guard(m_other_udp_ground_stations_lock);
  const std::string identifier=ext_device.create_identifier();
  const auto port_offset= m_other_udp_ground_stations.size()+1;
  auto tmp=std::make_shared<UDPEndpoint2>("GroundStationUDPX",OHD_GROUND_CLIENT_UDP_PORT_OUT, OHD_GROUND_CLIENT_UDP_PORT_IN+port_offset,
                                            ext_device.external_device_ip,ext_device.local_network_ip);
  tmp->registerCallback([this](std::vector<MavlinkMessage> messages){
    for(auto msg:messages){
      // Now this is weird, but somehow we get a lot of junk from QGroundControll on android ??!!
      // QGroundControll defaults to 255
      // QOpenHD defaults to 225;
      const bool is_from_ground_controll=msg.m.sysid==255 || msg.m.sysid==225;
      if(!is_from_ground_controll){
        // This can't really be a message from a ground controll application
        //m_console->debug("Dropping message");
        return;
      }
      //debugMavlinkMessage(mavlinkMessage.m, "GroundTelemetry::external GCS message");
      on_messages_ground_station_clients({msg});
    }
  });
  m_other_udp_ground_stations[identifier]=tmp;
}

void GroundTelemetry::remove_external_ground_station_ip(const openhd::ExternalDevice& ext_device) {
  m_console->debug("remove_external_ground_station_ip {}",ext_device.to_string());
  std::lock_guard<std::mutex> guard(m_other_udp_ground_stations_lock);
  const std::string identifier=ext_device.create_identifier();
  // shared pointer will clean up for us
  m_other_udp_ground_stations.erase(identifier);
}

std::vector<openhd::Setting> GroundTelemetry::get_all_settings() {
  std::vector<openhd::Setting> ret{};
  auto c_config_boot_as_air=[](std::string,int value){
    return openhd::tmp::handle_telemetry_change(value);
  };
  // and this allows an advanced user to change its air unit to a ground unit
  // only expose this setting if OpenHD uses the file workaround to figure out air or ground.
  if(openhd::tmp::file_air_or_ground_exists()){
    ret.push_back(openhd::Setting{"CONFIG_BOOT_AIR",openhd::IntSetting {0,c_config_boot_as_air}});
  }
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
  auto c_config_enable_joystick=[this](std::string,int value){
    if(!openhd::validate_yes_or_no(value))return false;
    m_groundTelemetrySettings->unsafe_get_settings().enable_rc_over_joystick=value;
    m_groundTelemetrySettings->persist();
    // Enabling requires reboot
    return true;
  };
  ret.push_back(openhd::Setting{"ENABLE_JOY_RC",openhd::IntSetting{static_cast<int>(m_groundTelemetrySettings->get_settings().enable_rc_over_joystick),
                                                                    c_config_enable_joystick}});
  if(m_rc_joystick_sender){
    auto c_rc_over_joystick_update_rate_hz=[this](std::string,int value){
      if(!openhd::telemetry::ground::valid_joystick_update_rate(value))return false;
      m_groundTelemetrySettings->unsafe_get_settings().rc_over_joystick_update_rate_hz=value;
      m_groundTelemetrySettings->persist();
      assert(m_rc_joystick_sender);
      m_rc_joystick_sender->change_update_rate(value);
      return true;
    };
    ret.push_back(openhd::Setting{"RC_UPDATE_HZ",openhd::IntSetting{static_cast<int>(m_groundTelemetrySettings->get_settings().rc_over_joystick_update_rate_hz),
                                                                     c_rc_over_joystick_update_rate_hz}});
    auto c_rc_over_joystick_channel_mapping=[this](std::string,std::string value){
      m_console->debug("Change channel mapping {}",value);
      const auto parsed=JoystickReader::convert_string_to_channel_mapping(value);
      if(parsed==std::nullopt){
        m_console->warn("Not a valid channel mapping");
        return false;
      }
      m_rc_joystick_sender->update_channel_maping(parsed.value());
      m_groundTelemetrySettings->unsafe_get_settings().rc_channel_mapping=value;
      m_groundTelemetrySettings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{"RC_CHAN_MAP",openhd::StringSetting {m_groundTelemetrySettings->get_settings().rc_channel_mapping,
                                                                     c_rc_over_joystick_channel_mapping}});
  }
#endif
  openhd::testing::append_dummy_if_empty(ret);
  return ret;
}

void GroundTelemetry::set_link_handle(std::shared_ptr<OHDLink> link) {
  // only call this once, we do not support changing the link handle at run time
  assert(m_wb_endpoint== nullptr);
  m_wb_endpoint = std::make_unique<WBEndpoint>(link,"wb_tx");
  m_wb_endpoint->registerCallback([this](std::vector<MavlinkMessage> messages) {
    on_messages_air_unit(messages);
  });
}

void GroundTelemetry::set_ext_devices_manager(
    std::shared_ptr<openhd::ExternalDeviceManager> ext_device_manager) {
  assert(m_ext_device_manager== nullptr);// only call this once during lifetime
  m_ext_device_manager=ext_device_manager;
  m_ext_device_manager->register_listener([this](openhd::ExternalDevice external_device,bool connected){
    if(connected){
      add_external_ground_station_ip(external_device);
    }else{
      remove_external_ground_station_ip(external_device);
    }
  });
}
