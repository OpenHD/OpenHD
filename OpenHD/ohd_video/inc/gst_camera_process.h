#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include "camerastream.h"

// The parent owns settings and the radio. Camera backend objects live in
// an exec'd copy of OpenHD, connected through a private Unix socketpair.
class CameraProcess : public CameraStream {
 public:
  enum class Backend { GStreamer, Mpp, Libcamera };
  CameraProcess(std::shared_ptr<CameraHolder> camera,
                openhd::ON_ENCODE_FRAME_CB output, bool video_outputs,
                Backend backend = Backend::GStreamer);
  ~CameraProcess();
  void start_looping() override;
  void terminate_looping() override;
  int minimum_link_bitrate_kbits() const override { return m_backend == Backend::Libcamera ? 2000 : CameraStream::minimum_link_bitrate_kbits(); }
  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;
  void handle_update_arming_state(bool armed) override;

 private:
  void run();
  void refresh_settings(bool restart);
  bool m_video_outputs;
  Backend m_backend;
  std::atomic<unsigned> m_keyframe_revision{0};
  std::atomic<bool> m_running{false};
  std::atomic<unsigned> m_revision{0};
  std::thread m_thread;
  std::mutex m_control_mutex;
  std::string m_snapshot;
  bool m_settings_restart = true;
  int m_bitrate = -1;
  bool m_capacity_limit = false;
  int m_qp_min = -1;
  int m_qp_max = -1;
  bool m_armed = false;
  int m_rotation_degree = -1;
  int m_flip = -1;
};

// Must be called before starting the ordinary application's services/threads.
// Returns -1 for a normal invocation, otherwise the worker's exit status.
int run_camera_worker_if_requested(int argc, char* argv[]);
