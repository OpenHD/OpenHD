//
// Created by consti10 on 27.02.24.
//

#include "gstaudiostream.h"

#include <iostream>
#include <utility>

#include "gst_appsink_helper.h"
#include "gst_debug_helper.h"
#include "gst_helper.hpp"

GstAudioStream::GstAudioStream() {
  OHDGstHelper::initGstreamerOrThrow();
  m_console = openhd::log::create_or_get("audio");
}

GstAudioStream::~GstAudioStream() { stop_looping(); }

void GstAudioStream::set_link_cb(openhd::ON_AUDIO_TX_DATA_PACKET cb) {
  m_cb = std::move(cb);
}

void GstAudioStream::start_looping() {
  m_keep_looping = true;
  m_loop_thread =
      std::make_unique<std::thread>(&GstAudioStream::loop_infinite, this);
}

void GstAudioStream::stop_looping() {
  m_keep_looping = false;
  if (m_loop_thread) {
    m_console->debug("Waiting for loop thread to terminate");
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
}

// Quite dirty, but hey ...
static std::string rpi_detect_alsasrc_device() {
  static constexpr auto DEFAULT_ALSASRC_DEVICE = "hw:2,0";
  const auto opt_arecord_list_output = OHDUtil::run_command_out("arecord -l");
  if (!opt_arecord_list_output.has_value()) {
    return DEFAULT_ALSASRC_DEVICE;
  }
  const auto& arecord_list_output = opt_arecord_list_output.value();
  if (OHDUtil::contains(arecord_list_output, "card 3: ")) {
    openhd::log::get_default()->debug("Found audio card 3");
    return "hw:3,0";  // Probably KMS
  }
  if (OHDUtil::contains(arecord_list_output, "card 2: ")) {
    openhd::log::get_default()->debug("Found audio card 2");
    return "hw:2,0";  // Probably FKMS
  }
  if (OHDUtil::contains(arecord_list_output, "card 1:")) {
    openhd::log::get_default()->debug("Found audio card 1");
    return "hw:1,0";
  }
  return DEFAULT_ALSASRC_DEVICE;
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
  auto opt_manual_audio_source =
      OHDFilesystemUtil::opt_read_file("/boot/openhd/audio_source.txt", false);
  // audiotestsrc always works, but obviously is not a mic ;)
  if (OHDFilesystemUtil::exists("/boot/openhd/test_audio.txt") ||
      openhd_enable_audio_test) {
    ss << "audiotestsrc"
       << " ! ";
  } else if (opt_manual_audio_source.has_value()) {
    // File, for development
    ss << opt_manual_audio_source.value() << " ! ";
  } else {
    if (OHDPlatform::instance().is_rpi()) {
      // RPI is weird. autoaudiosrc doesn't work, and
      // the device(s) depend on fkms / kms or are in general weird.
      ss << "alsasrc device=" << rpi_detect_alsasrc_device() << " ! ";
    } else {
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
  ss << "audio/x-raw,format=S16LE ! ";
  ss << "audioresample ! ";  // Might or might not be needed ...
  ss << "alawenc ! rtppcmapay max-ptime=20000000 ! ";
  ss << OHDGstHelper::createOutputAppSink();
  return ss.str();
}

void GstAudioStream::stream_once() {
  m_console->debug("GstAudioStream::stream_once");
  auto pipeline = create_pipeline();
  m_console->debug("Pipeline: [{}]", pipeline);
  GError* error = nullptr;
  m_gst_pipeline = gst_parse_launch(pipeline.c_str(), &error);
  m_console->debug("GStreamerStream::setup() end");
  if (error) {
    m_console->error("Failed to create pipeline: {}", error->message);
    return;
  }
  m_app_sink_element =
      gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "out_appsink");
  assert(m_app_sink_element);
  const auto ret = gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
  m_console->debug("State change ret:{}",
                   openhd::gst_state_change_return_to_string(ret));
  const uint64_t timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::milliseconds(100))
          .count();
  std::chrono::steady_clock::time_point m_last_audio_packet =
      std::chrono::steady_clock::now();
  // Streaming
  while (true) {
    // Quickly terminate if openhd wants to terminate
    if (!m_keep_looping) break;
    // Restart in case no data comes in
    if (std::chrono::steady_clock::now() - m_last_audio_packet >
        std::chrono::seconds(5)) {
      m_console->warn("Restarting audio due to no data for >5 seconds");
      break;
    }
    auto buffer_x = openhd::gst_app_sink_try_pull_sample_and_copy(
        m_app_sink_element, timeout_ns);
    if (buffer_x.has_value()) {
      on_audio_packet(buffer_x->buffer);
      m_last_audio_packet = std::chrono::steady_clock::now();
    }
  }
  // cleanup
  openhd::unref_appsink_element(m_app_sink_element);
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_NULL);
  gst_object_unref(m_gst_pipeline);
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
