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

#ifndef OPENHD_GST_RECORDER_H
#define OPENHD_GST_RECORDER_H

#include <gst/gstelement.h>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "openhd_spdlog.h"

// TODO for some reason, I cannot make fucking appsrc work !
// Dummy until this issue is resolved
class GstVideoRecorder {
 public:
  GstVideoRecorder();
  ~GstVideoRecorder();
  void enqueue_rtp_fragment(std::shared_ptr<std::vector<uint8_t>> fragment);
  void on_video_data(const uint8_t *data, int data_len);
  void start();
  void stop_and_cleanup();
  bool ready_data = false;

 private:
  std::shared_ptr<spdlog::logger> m_console;
  GstElement *m_gst_pipeline = nullptr;
  GstElement *m_app_src_element = nullptr;
};

#endif  // OPENHD_GST_RECORDER_H
