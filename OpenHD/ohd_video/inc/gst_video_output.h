#pragma once

#include <gst/gst.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

namespace openhd {

// An output is a rendition of a camera, not another camera. Each output owns
// its encoder, rate/resolution limits and destination independently of the
// primary radio stream. More outputs can attach to the same capture pad.
struct VideoOutputProfile {
  std::string name;
  std::string host;
  int port = 0;
  int width = 854;
  int height = 480;
  int fps = 15;
  int bitrate_kbit = 1000;
  bool prefer_hardware = false;
};

class GstVideoOutput {
 public:
  explicit GstVideoOutput(VideoOutputProfile profile);
  ~GstVideoOutput();
  GstVideoOutput(const GstVideoOutput&) = delete;
  GstVideoOutput& operator=(const GstVideoOutput&) = delete;

  // Prefer the encoder's raw input: capture is shared without decoding the
  // radio output. Integrated camera/encoder sources use an encoded fallback.
  bool attach(GstElement* camera_pipeline, bool input_h265, bool rtp_input, bool prefer_raw = true);
  bool uses_raw_input() const { return m_raw; }
  static std::string pipeline(const VideoOutputProfile& profile, bool raw,
                              bool h265, bool rtp, bool hardware);

 private:
  struct Sample { GstBuffer* buffer; GstCaps* caps; };
  static GstPadProbeReturn probe(GstPad*, GstPadProbeInfo*, gpointer);
  void enqueue(GstPad* pad, GstBuffer* buffer);
  void run();
  VideoOutputProfile m_profile;
  GstPad* m_pad = nullptr;
  gulong m_probe = 0;
  bool m_raw = false;
  bool m_h265 = false;
  bool m_rtp = false;
  std::mutex m_mutex;
  std::condition_variable m_changed;
  std::deque<Sample> m_queue;
  bool m_stopping = false;
  std::thread m_worker;
};
}  // namespace openhd
