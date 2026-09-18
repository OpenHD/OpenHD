#ifndef LIBCAMERA_APP_STREAM_H
#define LIBCAMERA_APP_STREAM_H

#include <atomic>
#include <memory>
#include <thread>
#include <mutex>

#include "camerastream.h"
#include "openhd_rtp.h"

class LibcameraAppStream final : public CameraStream {
 public:
  LibcameraAppStream(std::shared_ptr<CameraHolder> camera_holder,
                     openhd::ON_ENCODE_FRAME_CB out_cb);
  ~LibcameraAppStream();

  void start_looping() override;
  void terminate_looping() override;

  void handle_change_bitrate_request(
      openhd::LinkActionHandler::LinkBitrateInformation lb) override;

  void handle_update_arming_state(bool armed) override;

 private:
  void run();

  std::atomic<bool> m_run{false};
  std::atomic<bool> m_thread_running{false};
  std::atomic<bool> m_restart_requested{false};
  std::atomic<bool> m_bitrate_update_requested{false};
  std::atomic<bool> m_armed{false};
  std::atomic<int> m_requested_bitrate_kbits{0};
  std::thread m_thread;

  std::shared_ptr<openhd::RTPHelper> m_rtp;
};

#endif // LIBCAMERA_APP_STREAM_H
