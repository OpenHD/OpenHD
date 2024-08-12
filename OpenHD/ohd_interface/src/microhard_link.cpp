// refactored/rewritten by Raphael
// based on consti10's work
//

#include "microhard_link.h"

#include <Poco/Exception.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "openhd_temporary_air_or_ground.h"

// Constants
static constexpr auto MICROHARD_AIR_IP = "192.168.168.11";
static constexpr auto MICROHARD_GND_IP = "192.168.168.12";
static constexpr int MICROHARD_UDP_PORT_VIDEO_AIR_TX = 5910;
static constexpr int MICROHARD_UDP_PORT_TELEMETRY_AIR_TX = 5920;
static const std::string DEFAULT_DEVICE_IP_GND = "192.168.168.122";
static const std::string DEFAULT_DEVICE_IP_AIR = "192.168.168.153";
const std::string telnet_cmd = "telnet 192.168.168.1";
const std::string username = "admin\n";
const std::string password = "qwertz1\n";
const std::string command = "AT+MWRSSI\n";
const std::string command2 = "AT+MWTXPOWER\n";
const std::string command3 = "AT+MWBAND\n";
const std::string command4 = "AT+MWFREQ2400\n";
const std::string command5 = "AT+MWVRATE\n";
const std::string command6 = "AT+MWNOISEFLOOR\n";
const std::string command7 = "AT+MWSNR\n";



// Helper function to retrieve IP addresses starting with a specific prefix
std::vector<std::string> get_ip_addresses(const std::string& prefix) {
  std::vector<std::string> ip_addresses;
  int sockfd;
  struct ifreq ifr;
  struct ifconf ifc;
  char buf[4096];
  struct ifreq* it;
  struct ifreq* end;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("socket");
    openhd::log::get_default()->warn(
        "Failed to create socket for IP retrieval.");
    return ip_addresses;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
    perror("ioctl");
    openhd::log::get_default()->warn(
        "ioctl failed while getting interface configuration.");
    close(sockfd);
    return ip_addresses;
  }

  it = ifc.ifc_req;
  end = (struct ifreq*)(buf + ifc.ifc_len);

  for (; it != end; ++it) {
    strncpy(ifr.ifr_name, it->ifr_name, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
      perror("ioctl");
      openhd::log::get_default()->warn(
          "ioctl failed while getting IP address for interface {}",
          ifr.ifr_name);
      continue;
    }
    auto* addr = (struct sockaddr_in*)&ifr.ifr_addr;
    std::string ip = inet_ntoa(addr->sin_addr);
    if (ip.find(prefix) == 0) {
      ip_addresses.push_back(ip);
      openhd::log::get_default()->warn("Found IP address: {}", ip);
    }
  }

  close(sockfd);

  return ip_addresses;
}

void communicate_with_device(const std::string& ip,
                             const std::string& command) {
  openhd::log::get_default()->warn(
      "Starting communication with device at IP: {}", ip);

  try {
    Poco::Net::SocketAddress address(ip, 23);
    Poco::Net::StreamSocket socket(address);
    Poco::Net::SocketStream stream(socket);

    // Login to the device
    std::this_thread::sleep_for(
        std::chrono::seconds(1));  // Wait for a second to process username
    openhd::log::get_default()->debug("Sending username: {}", username);
    stream << username << std::flush;
    std::this_thread::sleep_for(
        std::chrono::seconds(1));  // Wait for a second to process username

    openhd::log::get_default()->debug("Sending password: {}", password);
    stream << password << std::flush;
    std::this_thread::sleep_for(
        std::chrono::seconds(3));  // Wait for a second to process password

    while (
        true) {  // Infinite loop for sending commands and receiving responses
      // Send the command to the device
      stream << command << std::flush;

      // Read the response from the device
      std::string response;
      std::string line;
      while (std::getline(stream, line)) {
        response += line + "\n";
        // Break out of the loop if the end of the response is reached
        if (line.find("OK") != std::string::npos) {
          break;
        }
      }

      // Extract and log the RSSI value
      std::regex rssi_regex(R"(([-\d]+) dBm)");
      std::smatch match;
      if (std::regex_search(response, match, rssi_regex)) {
        std::string rssi_value_str = match[1].str();
        int rssi_value = std::stoi(rssi_value_str);
        openhd::log::get_default()->warn("Extracted RSSI value: {} dBm",
                                         rssi_value);

        // some_other_function(rssi_value);

      } else {
        openhd::log::get_default()->warn("RSSI value not found in response");
      }
    }

  } catch (const Poco::Exception& e) {
    openhd::log::get_default()->warn("POCO Exception: {}", e.displayText());
  } catch (const std::exception& e) {
    openhd::log::get_default()->warn("Standard Exception: {}", e.what());
  }
}

void communicate_with_device_slow(const std::string& ip,
                             const std::string& command2) {
  openhd::log::get_default()->warn(
      "Starting slower communication with device at IP: {}", ip);

  try {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    Poco::Net::SocketAddress address(ip, 23);
    Poco::Net::StreamSocket socket(address);
    Poco::Net::SocketStream stream(socket);

    // Login to the device
    std::this_thread::sleep_for(
        std::chrono::seconds(1));  // Wait for a second to process username
    openhd::log::get_default()->debug("Sending username: {}", username);
    stream << username << std::flush;
    std::this_thread::sleep_for(
        std::chrono::seconds(1));  // Wait for a second to process username

    openhd::log::get_default()->debug("Sending password: {}", password);
    stream << password << std::flush;
    std::this_thread::sleep_for(
        std::chrono::seconds(3));  // Wait for a second to process password

    while (true) {
      // Infinite loop for sending commands and receiving responses
      // Send the command to the device
      
//COMMAND 1
      stream << command2 << std::flush;

      // Read the response from the device
      std::string response;
      std::string line;
      while (std::getline(stream, line)) {
        response += line + "\n";
        // Break out of the loop if the end of the response is reached
        if (line.find("OK") != std::string::npos) {
          break;
        }
      }

      // Extract and log the value
      std::regex txpower_regex(R"(([-\d]+) dBm)", std::regex::icase);
      std::smatch match;
      if (std::regex_search(response, match, txpower_regex)) {
        std::string rssi_value_str = match[1].str();
        int rssi_value = std::stoi(rssi_value_str);
        openhd::log::get_default()->warn("TX-Power value: {} dBm",
                                         rssi_value);

      } else {
          openhd::log::get_default()->warn("TX-Power not found in response: '{}'", response);
      }
//COMMAND 2
      stream << command3 << std::flush;

      // Read the response2 from the device
      std::string response2;
      std::string line2;
      while (std::getline(stream, line2)) {
        response2 += line2 + "\n";
        // Break out of the loop if the end of the response2 is reached
        if (line2.find("OK") != std::string::npos) {
          break;
        }
      }

      // Extract and log the value
      std::regex bandwidth_regex(R"(\b(\d+)\s*MHz\b)", std::regex::icase);
      std::smatch match2;
      if (std::regex_search(response2, match2, bandwidth_regex)) {
        std::string bandwidth_value_str = match2[1].str();
        int bandwidth_value = std::stoi(bandwidth_value_str);
        openhd::log::get_default()->warn("Bandwith value: {} mhz",
                                         bandwidth_value_str);

      } else {
          openhd::log::get_default()->warn("Bandwith value not found in response2: '{}'", response2);
      }

//COMMAND 3
      stream << command4 << std::flush;

      // Read the response from the device
      std::string response3;
      std::string line3;
      while (std::getline(stream, line3)) {
        response3 += line3 + "\n";
        // Break out of the loop if the end of the response3 is reached
        if (line3.find("OK") != std::string::npos) {
          break;
        }
      }

      // Extract and log the value
    std::regex freq_regex(R"(\b(\d+)\s*MHz\b)", std::regex::icase);
    std::smatch match3;

    if (std::regex_search(response3, match3, freq_regex)) {
        // Extract the last number which is the frequency value
        std::string freq_value_str = match3[1].str();
        int freq_value = std::stoi(freq_value_str);
        openhd::log::get_default()->warn("Frequency: {} MHz", freq_value);

      } else {
          openhd::log::get_default()->warn("Frequency not found in response: '{}'", response3);
      }
//COMMAND 4
      stream << command5 << std::flush;

      // Read the response4 from the device
      std::string response4;
      std::string line4;
      while (std::getline(stream, line4)) {
        response4 += line4 + "\n";
        // Break out of the loop if the end of the response4 is reached
        if (line4.find("OK") != std::string::npos) {
          break;
        }
      }

      std::regex rate_regex(R"(\b(\d+)\b)", std::regex::icase);
    std::smatch match4;

    if (std::regex_search(response4, match4, rate_regex)) {
        // Extract the first number which is the rate value
        std::string rate_value_str = match4[1].str();
        int rate_value = std::stoi(rate_value_str);
        openhd::log::get_default()->warn("Rate Mode: {}", rate_value);

    } else {
        openhd::log::get_default()->warn("Rate value not found in response4: '{}'", response4);
    }
//COMMAND 5
      stream << command6 << std::flush;

      // Read the response5 from the device
      std::string response5;
      std::string line5;
      while (std::getline(stream, line5)) {
        response5 += line5 + "\n";
        // Break out of the loop if the end of the response5 is reached
        if (line5.find("OK") != std::string::npos) {
          break;
        }
      }

      // Extract and log the value
      std::regex noise_regex(R"((-?\d+)\s*dBm\b)", std::regex::icase);
      std::smatch match5;
      if (std::regex_search(response5, match5, noise_regex)) {
        std::string noise_value_str = match5[1].str();
        int noise_value = std::stoi(noise_value_str);
        openhd::log::get_default()->warn("NoiseFloor value: {} mhz",
                                         noise_value_str);

      } else {
          openhd::log::get_default()->warn("NoiseFloor value not found in response5: '{}'", response5);
      }
//COMMAND 6
      stream << command7 << std::flush;

      // Read the response6 from the device
      std::string response6;
      std::string line6;
      while (std::getline(stream, line6)) {
        response6 += line6 + "\n";
        // Break out of the loop if the end of the response6 is reached
        if (line6.find("OK") != std::string::npos) {
          break;
        }
      }

      // Extract and log the value
      std::regex snr_regex(R"(\b(\d+)\s*MHz\b)", std::regex::icase);
      std::smatch match6;
      if (std::regex_search(response6, match6, snr_regex)) {
        std::string snr_value_str = match6[1].str();
        int snr_value = std::stoi(snr_value_str);
        openhd::log::get_default()->warn("SNR value: {} mhz",
                                         snr_value_str);

      } else {
          openhd::log::get_default()->warn("SNR value not found in response6: '{}'", response6);
      }
          std::this_thread::sleep_for(std::chrono::seconds(3));

    }

  } catch (const Poco::Exception& e) {
    openhd::log::get_default()->warn("POCO Exception: {}", e.displayText());
  } catch (const std::exception& e) {
    openhd::log::get_default()->warn("Standard Exception: {}", e.what());
  }
}


void MicrohardLink::monitor_gateway_signal_strength(
    const std::string& gateway_ip) {
  if (gateway_ip.empty()) {
    openhd::log::get_default()->warn(
        "Gateway IP is empty. Exiting monitoring.");

    return;
  }

  // Continuously connect, send command, and print output every second
  while (true) {
    openhd::log::get_default()->warn("Getting RSSI from gateway IP: {}",
                                     gateway_ip);
    try {
      std::string command2 = "AT+MWRSSI\n";
      communicate_with_device(gateway_ip, command2);
      openhd::log::get_default()->warn("RSSI data retrieval complete.");
    } catch (const std::exception& e) {
      openhd::log::get_default()->warn(
          "Exception occurred while getting RSSI data: {}", e.what());
    }
    // Wait for 1 second before the next iteration
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}


std::string get_gateway_ip() {
  std::string cmd =
      "ip route show default | awk '/default/ {print $3}' | grep "
      "'^192\\.168\\.168'";
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    openhd::log::get_default()->warn("Failed to run command: {}", cmd.c_str());
    throw std::runtime_error("Failed to run command.");
  }

  char buffer[128];
  std::string result;
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
  }
  pclose(pipe);

  // Trim trailing newline
  if (!result.empty()) {
    result.erase(result.find_last_not_of("\n\r") + 1);
  }

  openhd::log::get_default()->warn("Filtered Gateway IP: {}", result.c_str());

  return result;
}

bool check_ip_alive(const std::string& ip, int port = 23) {
  openhd::log::get_default()->warn("Checking if IP {} is alive on port {}", ip,
                                   port);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    openhd::log::get_default()->warn("Failed to create socket for IP check: {}",
                                     ip);
    return false;
  }

  struct sockaddr_in addr = {AF_INET, htons(port), inet_addr(ip.c_str())};
  bool connected = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0;
  close(sockfd);

  if (connected) {
    openhd::log::get_default()->warn("IP {} is alive", ip);
  } else {
    openhd::log::get_default()->warn("IP {} is not alive", ip);
  }

  return connected;
}

std::string find_device_ip_gnd() {
  auto ip_addresses = get_ip_addresses("192.168.168");
  for (const auto& ip : ip_addresses) {
    if (ip != MICROHARD_AIR_IP && ip != MICROHARD_GND_IP) {
      return ip;
    }
  }
  openhd::log::get_default()->warn(
      "No suitable IP address found for DEVICE_IP_GND. Using default.");

  return DEFAULT_DEVICE_IP_GND;
}

std::string find_device_ip_air() {
  auto ip_addresses = get_ip_addresses("192.168.168");
  for (const auto& ip : ip_addresses) {
    if (ip != MICROHARD_AIR_IP && ip != MICROHARD_GND_IP) {
      return ip;
    }
  }
  openhd::log::get_default()->warn(
      "No suitable IP address found for DEVICE_IP_AIR. Using default.");

  return DEFAULT_DEVICE_IP_AIR;
}

// The assigned IP
static const std::string DEVICE_IP_GND = find_device_ip_gnd();
static const std::string DEVICE_IP_AIR = find_device_ip_air();

void log_ip_addresses() {
  auto ip_addresses = get_ip_addresses("192.168.168");
  if (!ip_addresses.empty()) {
    for (const auto& ip : ip_addresses) {
      openhd::log::get_default()->warn("Found IP address: {}", ip);
      std::string gateway_ip = get_gateway_ip();
      openhd::log::get_default()->warn("Gateway IP for {}: {}", ip, gateway_ip);
    }
  } else {
    openhd::log::get_default()->warn(
        "No IP addresses starting with 192.168.168 found.");
  }
}

std::string get_detected_ip_address() {
  auto ip_addresses = get_ip_addresses("192.168.168");
  if (!ip_addresses.empty()) {
    return ip_addresses.front();
  } else {
    openhd::log::get_default()->warn(
        "No IP addresses starting with 192.168.168 found.");

    return "";  // Return an empty string if no IP found
  }
}

static void wait_for_microhard_module(bool is_air) {
  const std::string microhard_device_ip = get_gateway_ip();

  if (microhard_device_ip.empty()) {
    openhd::log::get_default()->warn(
        "No microhard device IP address detected. Exiting.");
    return;
  }

  while (true) {
    if (check_ip_alive(microhard_device_ip)) {
      openhd::log::get_default()->warn("Microhard module found at {}",
                                        microhard_device_ip);
      break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

MicrohardLink::MicrohardLink(OHDProfile profile) : m_profile(profile) {
  wait_for_microhard_module(m_profile.is_air);

  if (m_profile.is_air) {
    m_video_tx = std::make_unique<openhd::UDPForwarder>(
        DEVICE_IP_GND, MICROHARD_UDP_PORT_VIDEO_AIR_TX);
    auto cb_telemetry_rx = [this](const uint8_t* data, std::size_t data_len) {
      auto shared =
          std::make_shared<std::vector<uint8_t>>(data, data + data_len);
      on_receive_telemetry_data(shared);
    };
    m_telemetry_tx_rx = std::make_unique<openhd::UDPReceiver>(
        DEVICE_IP_AIR, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, cb_telemetry_rx);
  } else {
    auto cb_video_rx = [this](const uint8_t* payload, std::size_t payloadSize) {
      on_receive_video_data(0, payload, payloadSize);
    };
    m_video_rx = std::make_unique<openhd::UDPReceiver>(
        DEVICE_IP_GND, MICROHARD_UDP_PORT_VIDEO_AIR_TX, cb_video_rx);

    auto cb_telemetry_rx = [this](const uint8_t* data, std::size_t data_len) {
      auto shared =
          std::make_shared<std::vector<uint8_t>>(data, data + data_len);
      on_receive_telemetry_data(shared);
    };
    m_telemetry_tx_rx = std::make_unique<openhd::UDPReceiver>(
        DEVICE_IP_GND, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, cb_telemetry_rx);
  }

  if (m_telemetry_tx_rx) {
    m_telemetry_tx_rx->runInBackground();
  }

  if (m_video_rx) {
    m_video_rx->runInBackground();
  }

  // Start monitoring gateway signal strength
  std::thread monitor_thread(monitor_gateway_signal_strength, get_gateway_ip());
  monitor_thread.detach();  // Run in the background

  // Start the second communication thread
  std::thread second_thread(communicate_with_device_slow, get_gateway_ip(), command2);
  second_thread.detach();  // Run in the background
}


void MicrohardLink::transmit_telemetry_data(OHDLink::TelemetryTxPacket packet) {
  const auto destination_ip = m_profile.is_air ? DEVICE_IP_GND : DEVICE_IP_AIR;
  m_telemetry_tx_rx->forwardPacketViaUDP(
      destination_ip, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, packet.data->data(),
      packet.data->size());
}

void MicrohardLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  assert(m_profile.is_air);
  if (stream_index == 0) {
    for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
      m_video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
    }
  }
}

void MicrohardLink::transmit_audio_data(
    const openhd::AudioPacket& audio_packet) {
  openhd::log::get_default()->warn("Transmitting audio data (not implemented)");
}

std::vector<openhd::Setting> MicrohardLink::get_all_settings() {
  using namespace openhd;
  std::vector<Setting> settings;
  auto change_dummy =
      IntSetting{0, [this](std::string, int value) { return true; }};
  settings.push_back(Setting{"MICROHARD_DUMMY0", change_dummy});

  return settings;
}
