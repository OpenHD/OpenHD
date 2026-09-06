#include "gst_video_output.h"
#include "fleet_video_lease.h"

#include <gst/app/gstappsrc.h>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace openhd {
namespace {
bool has_factory(const char* name) {
  auto* factory = gst_element_factory_find(name);
  if (!factory) return false;
  gst_object_unref(factory);
  return true;
}
GstPad* raw_encoder_pad(GstElement* pipeline) {
  GstPad* result = nullptr;
  auto* iterator = gst_bin_iterate_recurse(GST_BIN(pipeline));
  GValue item = G_VALUE_INIT;
  while (!result) {
    const auto next = gst_iterator_next(iterator, &item);
    if (next == GST_ITERATOR_RESYNC) { gst_iterator_resync(iterator); continue; }
    if (next != GST_ITERATOR_OK) break;
    auto* element = GST_ELEMENT(g_value_get_object(&item));
    auto* factory = gst_element_get_factory(element);
    const char* klass = factory ? gst_element_factory_get_metadata(factory, GST_ELEMENT_METADATA_KLASS) : nullptr;
    if (klass && g_strrstr(klass, "Encoder") && g_strrstr(klass, "Video")) {
      auto* pad = gst_element_get_static_pad(element, "sink");
      if (pad) {
        auto* caps = gst_pad_query_caps(pad, nullptr);
        auto* raw = gst_caps_from_string("video/x-raw");
        if (caps && !gst_caps_is_any(caps) && gst_caps_can_intersect(caps, raw)) result = GST_PAD(gst_object_ref(pad));
        if (caps) gst_caps_unref(caps);
        gst_caps_unref(raw);
        gst_object_unref(pad);
      }
    }
    g_value_reset(&item);
  }
  if (G_VALUE_TYPE(&item)) g_value_unset(&item);
  gst_iterator_free(iterator);
  return result;
}
}  // namespace

GstVideoOutput::GstVideoOutput(VideoOutputProfile profile) : m_profile(std::move(profile)) {
  // Also used by device onboarding to reject firmware that would upload the
  // full-resolution radio stream instead of this independent output.
  g_message("OpenHD multi-output v1: 480p15 H264 RTP/UDP");
  if (m_profile.port < 1024 || m_profile.port > 65535 ||
      m_profile.width < 2 || m_profile.height < 2 ||
      m_profile.width % 2 || m_profile.height % 2 ||
      m_profile.fps < 1 || m_profile.bitrate_kbit < 1 ||
      m_profile.host.empty() || m_profile.host.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.:-") != std::string::npos)
    throw std::invalid_argument("Invalid video output profile");
}

GstVideoOutput::~GstVideoOutput() {
  if (m_pad) {
    if (m_probe) gst_pad_remove_probe(m_pad, m_probe);
    gst_object_unref(m_pad);
  }
  { std::lock_guard<std::mutex> lock(m_mutex); m_stopping = true; }
  m_changed.notify_all();
  if (m_worker.joinable()) m_worker.join();
  if (m_rtp_caps) gst_caps_unref(m_rtp_caps);
  for (auto& sample : m_queue) { gst_buffer_unref(sample.buffer); gst_caps_unref(sample.caps); }
}

std::string GstVideoOutput::pipeline(const VideoOutputProfile& p, bool raw,
                                     bool h265, bool rtp, bool hardware) {
  std::ostringstream out;
  out << "appsrc name=input is-live=true format=time do-timestamp=true block=false max-bytes=4194304 ! ";
  if (!raw) {
    if (rtp) out << (h265 ? "rtph265depay ! " : "rtph264depay ! ");
    out << (h265 ? "h265parse ! " : "h264parse ! ") << "decodebin ! ";
  }
  // Rate reduction happens before scaling/encoding. The output queue never
  // applies back-pressure to camera capture or the primary radio encoder.
  out << "queue max-size-buffers=2 max-size-bytes=0 max-size-time=0 leaky=downstream ! "
      << "videorate drop-only=true ! video/x-raw,framerate=" << p.fps << "/1 ! "
      << "videoscale add-borders=true ! videoconvert ! video/x-raw,format="
      << (hardware ? "NV12" : "I420") << ",width=" << p.width
      << ",height=" << p.height << ",pixel-aspect-ratio=1/1 ! ";
  if (hardware) {
    out << "mpph264enc rc-mode=cbr bps=" << p.bitrate_kbit * 1000
        << " gop=" << p.fps << " ! ";
  } else {
    out << "x264enc tune=zerolatency speed-preset=ultrafast bitrate=" << p.bitrate_kbit
        << " key-int-max=" << p.fps
        // Sliced threading produced green/corrupt frames in FleetControl's
        // browser HLS decoder, despite decoding correctly with libav locally.
        << " bframes=0 threads=2 sliced-threads=false vbv-buf-capacity=300 option-string=nal-hrd=cbr ! ";
  }
  out << "h264parse ! rtph264pay pt=96 config-interval=-1 mtu=1200 ! "
      << "udpsink host=" << p.host << " port=" << p.port << " sync=false async=false";
  return out.str();
}

bool GstVideoOutput::attach(GstElement* camera_pipeline, bool input_h265, bool rtp_input, bool prefer_raw) {
  if (m_worker.joinable() || m_pad || !camera_pipeline) return false;
  m_h265 = input_h265; m_rtp = rtp_input;
  m_pad = prefer_raw ? raw_encoder_pad(camera_pipeline) : nullptr;
  m_raw = m_pad != nullptr;
  if (!m_pad) {
    auto* sink = gst_bin_get_by_name(GST_BIN(camera_pipeline), "out_appsink");
    if (!sink) return false;
    m_pad = gst_element_get_static_pad(sink, "sink");
    gst_object_unref(sink);
  }
  if (!m_pad) return false;
  m_probe = gst_pad_add_probe(m_pad, static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST), probe, this, nullptr);
  m_worker = std::thread([this] { run(); });
  g_message("Video output %s: %dx%d@%d %d kbit/s, %s input, RTP/UDP %s:%d",
            m_profile.name.c_str(), m_profile.width, m_profile.height, m_profile.fps,
            m_profile.bitrate_kbit, m_raw ? "shared raw" : "encoded fallback",
            m_profile.host.c_str(), m_profile.port);
  return true;
}

bool GstVideoOutput::start_rtp_input(bool h265) {
  if (m_worker.joinable()) return false;
  m_h265 = h265; m_rtp = true; m_raw = false;
  m_rtp_caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video",
      "encoding-name", G_TYPE_STRING, h265 ? "H265" : "H264", "clock-rate", G_TYPE_INT, 90000,
      "payload", G_TYPE_INT, 96, nullptr);
  m_worker = std::thread([this] { run(); });
  return true;
}

void GstVideoOutput::push_rtp(const uint8_t* data, size_t size) {
  if (!m_rtp_caps || !data || size < 12 || size > 65535) return;
  auto* buffer = gst_buffer_new_allocate(nullptr, size, nullptr);
  if (!buffer) return;
  gst_buffer_fill(buffer, 0, data, size);
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_queue.size() >= 512) {
    gst_buffer_unref(m_queue.front().buffer); gst_caps_unref(m_queue.front().caps);
    m_queue.pop_front();
  }
  m_queue.push_back({buffer, gst_caps_ref(m_rtp_caps)});
  m_changed.notify_one();
}

GstPadProbeReturn GstVideoOutput::probe(GstPad* pad, GstPadProbeInfo* info, gpointer user) {
  auto* self = static_cast<GstVideoOutput*>(user);
  if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) self->enqueue(pad, GST_PAD_PROBE_INFO_BUFFER(info));
  if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER_LIST) {
    auto* buffers = GST_PAD_PROBE_INFO_BUFFER_LIST(info);
    for (guint i = 0; i < gst_buffer_list_length(buffers); ++i) self->enqueue(pad, gst_buffer_list_get(buffers, i));
  }
  return GST_PAD_PROBE_OK;
}

void GstVideoOutput::enqueue(GstPad* pad, GstBuffer* buffer) {
  auto* caps = gst_pad_get_current_caps(pad);
  if (!caps || !buffer) { if (caps) gst_caps_unref(caps); return; }
  std::lock_guard<std::mutex> lock(m_mutex);
  // Encoded input needs enough packets for a complete access unit. Raw input
  // retains only two capture buffers, regardless of encoder speed.
  const size_t limit = m_raw ? 2 : 512;
  if (m_queue.size() >= limit) {
    gst_buffer_unref(m_queue.front().buffer); gst_caps_unref(m_queue.front().caps);
    m_queue.pop_front();
  }
  m_queue.push_back({gst_buffer_ref(buffer), caps});
  m_changed.notify_one();
}

void GstVideoOutput::run() {
  FleetVideoLease lease(m_profile.host, m_profile.port, m_profile.fleet_controlled);
  GstElement* output = nullptr;
  GstAppSrc* input = nullptr;
  GstBus* bus = nullptr;
  GstCaps* current_caps = nullptr;
  bool hardware = m_profile.prefer_hardware && has_factory("mpph264enc");
  auto retry_at = std::chrono::steady_clock::now();
  auto clear = [&] {
    if (output) gst_element_set_state(output, GST_STATE_NULL);
    if (input) gst_object_unref(input);
    if (bus) gst_object_unref(bus);
    if (output) gst_object_unref(output);
    if (current_caps) gst_caps_unref(current_caps);
    output = nullptr; input = nullptr; bus = nullptr; current_caps = nullptr;
  };
  while (true) {
    Sample sample{};
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_changed.wait_for(lock, std::chrono::milliseconds(250), [&] { return m_stopping || !m_queue.empty(); });
      if (m_stopping) break;
      if (m_queue.empty()) continue;
      sample = m_queue.front(); m_queue.pop_front();
    }
    if (!lease.allowed()) {
      if (output) {
        g_message("Video output %s paused: FleetControl upload permission unavailable", m_profile.name.c_str());
        clear();
      }
      gst_buffer_unref(sample.buffer); gst_caps_unref(sample.caps);
      continue;
    }
    if (bus) {
      if (auto* message = gst_bus_pop_filtered(bus, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS))) {
        GError* error = nullptr; gchar* debug = nullptr;
        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) gst_message_parse_error(message, &error, &debug);
        g_warning("Video output %s failed: %s", m_profile.name.c_str(), error ? error->message : "EOS");
        if (error) g_error_free(error);
        g_free(debug); gst_message_unref(message);
        clear(); hardware = false;
        retry_at = std::chrono::steady_clock::now() + std::chrono::seconds(3);
      }
    }
    if (current_caps && !gst_caps_is_equal(current_caps, sample.caps)) clear();
    if (!output && std::chrono::steady_clock::now() >= retry_at) {
      GError* error = nullptr;
      output = gst_parse_launch(pipeline(m_profile, m_raw, m_h265, m_rtp, hardware).c_str(), &error);
      if (error || !output) {
        g_warning("Video output %s unavailable: %s", m_profile.name.c_str(), error ? error->message : "no pipeline");
        if (error) g_error_free(error);
        clear(); hardware = false;
        retry_at = std::chrono::steady_clock::now() + std::chrono::seconds(5);
      } else {
        input = GST_APP_SRC(gst_bin_get_by_name(GST_BIN(output), "input"));
        bus = gst_element_get_bus(output);
        current_caps = gst_caps_ref(sample.caps);
        gst_app_src_set_caps(input, current_caps);
        if (gst_element_set_state(output, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
          clear(); hardware = false;
          retry_at = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        }
      }
    }
    if (input) {
      guint64 queued = 0;
      g_object_get(input, "current-level-bytes", &queued, nullptr);
      if (queued < 4194304) {
        auto* copy = gst_buffer_copy(sample.buffer);
        GST_BUFFER_PTS(copy) = GST_CLOCK_TIME_NONE;
        GST_BUFFER_DTS(copy) = GST_CLOCK_TIME_NONE;
        gst_app_src_push_buffer(input, copy);
      }
    }
    gst_buffer_unref(sample.buffer); gst_caps_unref(sample.caps);
  }
  clear();
}
}  // namespace openhd
