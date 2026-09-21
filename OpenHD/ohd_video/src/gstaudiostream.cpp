/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "gstaudiostream.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <utility>

#include "config_paths.h"
#include "gst_appsink_helper.h"
#include "gst_debug_helper.h"
#include "gst_helper.hpp"
#include "ohd_video_air_generic_settings.h"

AirCameraGenericSettings g_airCameraGenericSettings;

namespace {

#ifndef OPENHD_FALLBACK_AUDIO_FILE
#define OPENHD_FALLBACK_AUDIO_FILE "/usr/local/share/openhd/audio/example.mp3"
#endif

std::string escape_gst_string(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (const char c : value) {
    if (c == '\\' || c == '"') escaped.push_back('\\');
    escaped.push_back(c);
  }
  return escaped;
}

}  // namespace

GstAudioStream::GstAudioStream(std::string device_token, int mic_gain_percent)
    : m_device_token(std::move(device_token)),
      m_mic_gain_percent(mic_gain_percent) {
  OHDGstHelper::initGstreamerOrThrow();
  m_console = openhd::log::create_or_get("audio");
}

GstAudioStream::~GstAudioStream() { stop_looping(); }

void GstAudioStream::set_link_cb(openhd::ON_AUDIO_TX_DATA_PACKET cb) {
  m_cb = std::move(cb);
}

std::vector<GstAudioStream::DeviceInfo>
GstAudioStream::discover_capture_devices() {
  OHDGstHelper::initGstreamerOrThrow();
  std::vector<DeviceInfo> result;
  GstDeviceMonitor* monitor = gst_device_monitor_new();
  if (!monitor) return result;
  gst_device_monitor_add_filter(monitor, "Audio/Source", nullptr);
  if (!gst_device_monitor_start(monitor)) {
    gst_object_unref(monitor);
    return result;
  }
  GList* devices = gst_device_monitor_get_devices(monitor);
  for (GList* item = devices; item; item = item->next) {
    auto* device = GST_DEVICE(item->data);
    GstElement* element = gst_device_create_element(device, nullptr);
    if (!element) continue;
    GstElementFactory* factory = gst_element_get_factory(element);
    const char* factory_name =
        factory ? gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory))
                : nullptr;
    gchar* device_name = nullptr;
    if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "device")) {
      g_object_get(element, "device", &device_name, nullptr);
    }
    if (factory_name) {
      DeviceInfo info;
      info.token = factory_name;
      if (device_name && device_name[0] != '\0') {
        info.token += ":" + std::string(device_name);
      }
      const char* display_name = gst_device_get_display_name(device);
      info.display_name = display_name ? display_name : info.token;
      const auto duplicate =
          std::find_if(result.begin(), result.end(), [&](const DeviceInfo& d) {
            return d.token == info.token;
          });
      if (duplicate == result.end()) result.emplace_back(std::move(info));
    }
    g_free(device_name);
    gst_object_unref(element);
  }
  g_list_free_full(devices, gst_object_unref);
  gst_device_monitor_stop(monitor);
  gst_object_unref(monitor);
  return result;
}

void GstAudioStream::set_mic_gain_percent(int gain_percent) {
  m_mic_gain_percent = std::clamp(gain_percent, 0, 200);
}

void GstAudioStream::start_looping() {
  {
    std::lock_guard<std::mutex> lock(m_loop_mutex);
    m_loop_exited = false;
  }
  m_keep_looping = true;
  m_loop_thread =
      std::make_unique<std::thread>(&GstAudioStream::loop_infinite, this);
}

void GstAudioStream::stop_looping() {
  static constexpr auto kJoinTimeout = std::chrono::seconds(5);
  m_keep_looping = false;
  if (m_loop_thread) {
    m_console->debug("Waiting for loop thread to terminate");
    bool exited = false;
    {
      std::unique_lock<std::mutex> lock(m_loop_mutex);
      exited = m_loop_cv.wait_for(lock, kJoinTimeout,
                                  [this]() { return m_loop_exited.load(); });
    }
    if (!exited) {
      m_console->error(
          "Loop thread did not exit within {}ms; exiting before destroying "
          "an object still used by that thread",
          std::chrono::duration_cast<std::chrono::milliseconds>(kJoinTimeout)
              .count());
      std::_Exit(EXIT_FAILURE);
    }
    m_loop_thread->join();
    m_loop_thread = nullptr;
  }
}

void GstAudioStream::loop_infinite() {
  while (m_keep_looping) {
    try {
      stream_once();
    } catch (std::exception& ex) {
      std::cerr << "GStreamerStream::Error: " << ex.what() << std::endl;
    } catch (...) {
      std::cerr << "GStreamerStream::Unknown exception occurred" << std::endl;
    }
    if (m_keep_looping) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  {
    std::lock_guard<std::mutex> lock(m_loop_mutex);
    m_loop_exited = true;
  }
  m_loop_cv.notify_all();
}

// Quite dirty, but hey ...
static std::optional<std::string> rpi_detect_alsasrc_device() {
  const auto opt_arecord_list_output = OHDUtil::run_command_out("arecord -l");
  if (!opt_arecord_list_output.has_value()) {
    return std::nullopt;
  }
  const auto& arecord_list_output = opt_arecord_list_output.value();
  std::smatch match;
  static const std::regex capture_device_pattern(
      R"(card\s+([0-9]+):.*device\s+([0-9]+):)");
  if (std::regex_search(arecord_list_output, match,
                        capture_device_pattern)) {
    const auto device = "hw:" + match[1].str() + "," + match[2].str();
    openhd::log::get_default()->debug("Found ALSA capture device {}", device);
    return device;
  }
  return std::nullopt;
}

// 2.0 pipeline tx:
// gst-launch-1.0 alsasrc device=plughw:1,0 name=mic provide-clock=true
// do-timestamp=true buffer-time=20000 ! alawenc ! rtppcmapay max-ptime=20000000
// ! udpsink host=127.0.0.1 port=5051 |
//
// pipeline rx:
// gst-launch-1.0 udpsrc port=5051 caps="application/x-rtp, media=(string)audio,
// clock-rate=(int)8000, encoding-name=(string)PCMA" ! rtppcmadepay !
// audio/x-alaw, rate=8000, channels=1 ! alawdec ! alsasink device=hw:0
std::string GstAudioStream::create_pipeline() {
  std::stringstream ss;
  m_current_pipeline_uses_fallback_file = false;
  const bool fallback_file_exists =
      OHDFilesystemUtil::exists(OPENHD_FALLBACK_AUDIO_FILE);
  const auto append_fallback_file = [&]() {
    m_current_pipeline_uses_fallback_file = true;
    m_console->info("Streaming fallback audio file {}",
                    OPENHD_FALLBACK_AUDIO_FILE);
    ss << "filesrc location=\""
       << escape_gst_string(OPENHD_FALLBACK_AUDIO_FILE)
       << "\" ! decodebin ! ";
  };
  auto opt_manual_audio_source = OHDFilesystemUtil::opt_read_file(
      std::string(getConfigBasePath()) + "audio_source.txt", false);
  // audiotestsrc always works, but obviously is not a mic ;)
  if (OHDFilesystemUtil::exists(std::string(getConfigBasePath()) +
                                "test_audio.txt") ||
      openhd_enable_audio_test) {
    ss << "audiotestsrc"
       << " ! ";
  } else if (opt_manual_audio_source.has_value()) {
    // File, for development
    ss << opt_manual_audio_source.value() << " ! ";
  } else {
    const auto devices = discover_capture_devices();
    const auto selected = std::find_if(
        devices.begin(), devices.end(), [this](const DeviceInfo& device) {
          return device.token == m_device_token;
        });
    if ((m_force_fallback_audio_file || devices.empty()) &&
        fallback_file_exists) {
      append_fallback_file();
    } else if (selected != devices.end()) {
      const auto separator = selected->token.find(':');
      const auto factory = selected->token.substr(0, separator);
      ss << factory;
      if (separator != std::string::npos) {
        const auto device = selected->token.substr(separator + 1);
        ss << " device=\"" << escape_gst_string(device) << "\"";
      }
      ss << " ! ";
    } else if (OHDPlatform::instance().is_rpi()) {
      if (!m_device_token.empty()) {
        m_console->warn("Configured audio device is unavailable; using default");
      }
      // RPI is weird. autoaudiosrc doesn't work, so verify capture hardware
      // with arecord instead of guessing a fixed card number.
      const auto alsa_device = rpi_detect_alsasrc_device();
      if (alsa_device.has_value()) {
        ss << "alsasrc device=" << alsa_device.value() << " ! ";
      } else if (fallback_file_exists) {
        m_console->info("No ALSA capture device found on RPI");
        append_fallback_file();
      } else {
        m_console->warn(
            "No ALSA capture device found and fallback file {} is missing",
            OPENHD_FALLBACK_AUDIO_FILE);
        ss << "audiotestsrc wave=silence ! ";
      }
    } else {
      if (devices.empty()) {
        m_console->warn(
            "No audio capture device found and fallback file {} is missing; "
            "trying the default audio source",
            OPENHD_FALLBACK_AUDIO_FILE);
      }
      ss << "autoaudiosrc"
         << " ! ";
    }
  }
  /*ss << "autoaudiosrc ! ";
  ss << "audioconvert ! ";
  ss << "rtpL16pay ! ";*/
  ss << "queue ! ";
  // audioconvert might or might not be needed ...
  // alawenc needs S16LE
  ss << "audioconvert ! ";
  ss << "audio/x-raw,format=S16LE,channels=1,rate=8000 ! ";
  ss << "audioresample ! ";  // Might or might not be needed ...
  ss << "volume name=mic_volume volume="
     << (m_mic_gain_percent.load() / 100.0) << " ! ";
  ss << "alawenc ! rtppcmapay max-ptime=20000000 ! ";
  ss << OHDGstHelper::createOutputAppSink();
  return ss.str();
}

void GstAudioStream::stream_once() {
  int complainOnce = 0;
  m_console->debug("GstAudioStream::stream_once");
  auto pipeline = create_pipeline();
  m_console->debug("Pipeline: [{}]", pipeline);
  GError* error = nullptr;
  m_gst_pipeline = gst_parse_launch(pipeline.c_str(), &error);
  m_console->debug("GStreamerStream::setup() end");

  // Check both error and null pipeline
  if (error) {
    m_console->error("Failed to create pipeline: {}", error->message);
    g_error_free(error);
    return;
  }
  if (!m_gst_pipeline) {
    m_console->error("Failed to create pipeline: pipeline is null");
    return;
  }
  m_app_sink_element =
      gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "out_appsink");
  if (!m_app_sink_element) {
    m_console->error("Failed to get appsink element");
    openhd::gst_object_unref_with_timeout(GST_OBJECT(m_gst_pipeline));
    m_gst_pipeline = nullptr;
    return;
  }
  m_volume_element =
      gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "mic_volume");

  const auto ret = openhd::gst_element_set_state_with_timeout(
      m_gst_pipeline, GST_STATE_PLAYING);
  if (ret.has_value()) {
    m_console->debug("State change ret:{}",
                     openhd::gst_state_change_return_to_string(ret.value()));
  }
  if (!ret.has_value() || ret.value() == GST_STATE_CHANGE_FAILURE) {
    m_console->error("Failed to set pipeline to PLAYING state");
    if (!m_current_pipeline_uses_fallback_file &&
        OHDFilesystemUtil::exists(OPENHD_FALLBACK_AUDIO_FILE)) {
      m_console->warn(
          "Audio capture source failed; switching to fallback file {}",
          OPENHD_FALLBACK_AUDIO_FILE);
      m_force_fallback_audio_file = true;
    }
    openhd::gst_object_unref_with_timeout(GST_OBJECT(m_gst_pipeline));
    m_gst_pipeline = nullptr;
    return;
  }
  const uint64_t timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::milliseconds(100))
          .count();
  std::chrono::steady_clock::time_point m_last_audio_packet =
      std::chrono::steady_clock::now();
  int applied_gain_percent = m_mic_gain_percent.load();
  // Streaming - keep running while keep_looping is true
  while (m_keep_looping) {
    // Quickly terminate if openhd wants to terminate
    if (!m_keep_looping) break;

    const int requested_gain_percent = m_mic_gain_percent.load();
    if (m_volume_element && requested_gain_percent != applied_gain_percent) {
      g_object_set(m_volume_element, "volume",
                   requested_gain_percent / 100.0, nullptr);
      applied_gain_percent = requested_gain_percent;
    }

    auto buffer_x = openhd::gst_app_sink_try_pull_sample_and_copy(
        m_app_sink_element, timeout_ns);
    if (buffer_x.has_value()) {
      on_audio_packet(buffer_x->buffer);
      m_last_audio_packet = std::chrono::steady_clock::now();
    } else {
      GstBus* bus = gst_element_get_bus(m_gst_pipeline);
      GstMessage* message = gst_bus_pop_filtered(
          bus, static_cast<GstMessageType>(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
      gst_object_unref(bus);
      if (message) {
        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
          m_console->debug("Audio source reached end; looping it");
          gst_message_unref(message);
          const auto seek_flags = static_cast<GstSeekFlags>(
              GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT);
          if (gst_element_seek_simple(m_gst_pipeline, GST_FORMAT_TIME,
                                      seek_flags, 0)) {
            m_last_audio_packet = std::chrono::steady_clock::now();
            continue;
          }
          m_console->warn("Audio source cannot seek; restarting pipeline");
          break;
        } else {
          GError* bus_error = nullptr;
          gchar* debug_info = nullptr;
          gst_message_parse_error(message, &bus_error, &debug_info);
          m_console->error("Audio pipeline error: {}",
                           bus_error ? bus_error->message : "unknown error");
          g_clear_error(&bus_error);
          g_free(debug_info);
        }
        gst_message_unref(message);
        break;
      }
      // Check if pipeline is dead (no data for 5 seconds)
      if (std::chrono::steady_clock::now() - m_last_audio_packet >
          std::chrono::seconds(5)) {
        m_console->warn("No audio data for 5 seconds, restarting pipeline");
        break;
      }
    }
  }
  // cleanup
  if (m_volume_element) {
    gst_object_unref(m_volume_element);
    m_volume_element = nullptr;
  }
  openhd::unref_appsink_element(m_app_sink_element);
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_NULL);
  openhd::gst_object_unref_with_timeout(GST_OBJECT(m_gst_pipeline));
  m_gst_pipeline = nullptr;
}

void GstAudioStream::on_audio_packet(
    std::shared_ptr<std::vector<uint8_t>> packet) {
  // m_console->debug("Got audio packet {}", packet->size());
  if (m_cb) {
    openhd::AudioPacket audioPacket;
    audioPacket.data = packet;
    m_cb(audioPacket);
  }
}
