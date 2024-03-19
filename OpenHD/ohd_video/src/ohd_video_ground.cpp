//
// Created by consti10 on 04.01.23.
//

#include "ohd_video_ground.h"

#include <utility>

#include "openhd_config.h"
#include "openhd_util.h"

OHDVideoGround::OHDVideoGround(std::shared_ptr<OHDLink> link_handle)
    : m_link_handle(std::move(link_handle)) {
  m_console = openhd::log::create_or_get("v_gnd");
  m_primary_video_forwarder = std::make_unique<openhd::UDPMultiForwarder>();
  m_secondary_video_forwarder = std::make_unique<openhd::UDPMultiForwarder>();
  m_audio_forwarder = std::make_unique<openhd::UDPMultiForwarder>();
  // We always forward video to localhost::5600 (primary) and 5601 (secondary)
  // for the default Ground control application (e.g. QOpenHD) to pick up
  addForwarder("127.0.0.1");
  // See the description in the .config file for more info
  if (openhd::load_config().NW_FORWARD_TO_LOCALHOST_58XX || true) {
    m_console->debug("Forwarding video to 5800/5801 localhost is enabled");
    // Adding forwarder for WebRTC
    m_primary_video_forwarder->addForwarder("127.0.0.1", 5800);
    m_secondary_video_forwarder->addForwarder("127.0.0.1", 5801);
  }
  if (m_link_handle) {
    m_link_handle->register_on_receive_video_data_cb(
        [this](int stream_index, const uint8_t* data, int data_len) {
          on_video_data(stream_index, data, data_len);
        });
    m_link_handle->m_audio_data_rx_cb = [this](const uint8_t* data,
                                               int data_len) {
      on_audio_data(data, data_len);
    };
  } else {
    m_console->warn("No link handle, no video forwarding");
  }
  // On the ground, forward video to all connected external devices via UDP
  // (In addition to localhost)
  // However, if a tcp client on localhost connects, we forward to the udp ports
  // higher than 5600 on localhost - see the method below
  openhd::ExternalDeviceManager::instance().register_listener(
      [this](openhd::ExternalDevice external_device, bool connected) {
        start_stop_forwarding_external_device(external_device, connected);
      });
}

OHDVideoGround::~OHDVideoGround() {
  if (m_link_handle) {
    m_link_handle->register_on_receive_video_data_cb(nullptr);
    m_link_handle->m_audio_data_rx_cb = nullptr;
  }
}

void OHDVideoGround::addForwarder(const std::string& client_addr) {
  m_primary_video_forwarder->addForwarder(client_addr, 5600);
  m_secondary_video_forwarder->addForwarder(client_addr, 5601);
  m_audio_forwarder->addForwarder(client_addr, 5610);
}

void OHDVideoGround::removeForwarder(const std::string& client_addr) {
  m_primary_video_forwarder->removeForwarder(client_addr, 5600);
  m_secondary_video_forwarder->removeForwarder(client_addr, 5601);
}

void OHDVideoGround::on_video_data(int stream_index, const uint8_t* data,
                                   int data_len) {
  // openhd::log::get_default()->debug("on_video_data {}",stream_index);
  if (stream_index == 0) {
    m_primary_video_forwarder->forwardPacketViaUDP(data, data_len);
  } else if (stream_index == 1) {
    m_secondary_video_forwarder->forwardPacketViaUDP(data, data_len);
  } else {
    openhd::log::get_default()->debug("Invalid stream index {}", stream_index);
  }
}

static bool ip_is_host_self(const std::string& ip) {
  if (OHDUtil::str_equal(ip, "127.0.0.1")) {
    // always self
    return true;
  }
  const auto hostname_ips = OHDUtil::run_command_out("hostname -I", false);
  if (hostname_ips.has_value() && OHDUtil::contains(hostname_ips.value(), ip)) {
    return true;
  }
  return false;
}

void OHDVideoGround::start_stop_forwarding_external_device(
    openhd::ExternalDevice external_device, bool connected) {
  if (external_device.discovered_by_mavlink_tcp_server) {
    const bool is_host_self =
        ip_is_host_self(external_device.external_device_ip);
    if (is_host_self) {
      m_console->debug("Not forwarding video to {}, since self",
                       external_device.external_device_ip);
      return;
    }
  }
  if (connected) {
    addForwarder(external_device.external_device_ip);
  } else {
    removeForwarder(external_device.external_device_ip);
  }
}

void OHDVideoGround::on_audio_data(const uint8_t* data, int data_len) {
  m_audio_forwarder->forwardPacketViaUDP(data, data_len);
}
