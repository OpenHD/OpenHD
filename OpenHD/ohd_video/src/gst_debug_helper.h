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

#ifndef OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include <chrono>
#include <future>
#include <optional>
#include <thread>
#include <type_traits>

#include "openhd_action_handler.h"
#include "openhd_spdlog.h"

// Code that depends on gstreamer and falls in the "helper for debugging"
// category
namespace openhd {

// Helpfull links:
// https://gstreamer.freedesktop.org/documentation/additional/design/states.html?gi-language=c

static constexpr auto GST_CALL_TIMEOUT = std::chrono::seconds(5);
static constexpr auto GST_TERMINATE_DELAY = std::chrono::milliseconds(100);

template <typename Func>
static std::optional<std::invoke_result_t<Func>> run_gst_with_timeout_result(
    const char* tag, std::chrono::milliseconds timeout, Func&& func) {
  using Ret = std::invoke_result_t<Func>;
  std::packaged_task<Ret()> task(std::forward<Func>(func));
  auto fut = task.get_future();
  std::thread t(std::move(task));
  if (fut.wait_for(timeout) != std::future_status::ready) {
    openhd::log::get_default()->error(
        "{} timed out after {}ms", tag, timeout.count());
    openhd::TerminateHelper::instance().terminate_after(tag,
                                                        GST_TERMINATE_DELAY);
    t.detach();
    return std::nullopt;
  }
  t.join();
  try {
    return fut.get();
  } catch (const std::exception& ex) {
    openhd::log::get_default()->error("{} threw exception: {}", tag, ex.what());
    openhd::TerminateHelper::instance().terminate_after(tag,
                                                        GST_TERMINATE_DELAY);
    return std::nullopt;
  } catch (...) {
    openhd::log::get_default()->error("{} threw unknown exception", tag);
    openhd::TerminateHelper::instance().terminate_after(tag,
                                                        GST_TERMINATE_DELAY);
    return std::nullopt;
  }
}

template <typename Func>
static bool run_gst_with_timeout_void(const char* tag,
                                      std::chrono::milliseconds timeout,
                                      Func&& func) {
  auto res = run_gst_with_timeout_result(
      tag, timeout, [func = std::forward<Func>(func)]() mutable -> bool {
        func();
        return true;
      });
  return res.has_value();
}

static std::optional<GstStateChangeReturn> gst_element_set_state_with_timeout(
    GstElement* element, GstState state,
    std::chrono::milliseconds timeout = GST_CALL_TIMEOUT) {
  if (element == nullptr) {
    openhd::log::get_default()->error(
        "gst_element_set_state_with_timeout: null element");
    return std::nullopt;
  }
  return run_gst_with_timeout_result(
      "gst_element_set_state", timeout,
      [element, state]() { return gst_element_set_state(element, state); });
}

static bool gst_object_unref_with_timeout(
    GstObject* obj, std::chrono::milliseconds timeout = GST_CALL_TIMEOUT) {
  if (obj == nullptr) {
    return true;
  }
  return run_gst_with_timeout_void("gst_object_unref", timeout,
                                   [obj]() { gst_object_unref(obj); });
}

static std::string gst_state_change_return_to_string(
    const GstStateChangeReturn &gst_state_change_return) {
  return fmt::format(
      "{}", gst_element_state_change_return_get_name(gst_state_change_return));
}

// BLocks up to X seconds, but should never block more than that
static std::string gst_element_get_current_state_as_string(
    GstElement *element, bool *out_is_succesfully_streaming = nullptr) {
  GstState state;
  GstState pending;
  const auto timeout = std::chrono::seconds(3);
  auto returnValue = gst_element_get_state(
      element, &state, &pending,
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
  bool is_successfully_streaming = false;
  if (returnValue == GST_STATE_CHANGE_SUCCESS && state == GST_STATE_PLAYING &&
      pending == GST_STATE_VOID_PENDING) {
    is_successfully_streaming = true;
  }
  if (out_is_succesfully_streaming != nullptr) {
    *out_is_succesfully_streaming = is_successfully_streaming;
  }
  return fmt::format("Gst state: ret:{} state:{} pending:{} ok_streaming:{}",
                     gst_state_change_return_to_string(returnValue),
                     gst_element_state_get_name(state),
                     gst_element_state_get_name(pending),
                     is_successfully_streaming);
}

static void gst_element_set_set_state_and_log_result(GstElement *element,
                                                     GstState state) {
  auto res = gst_element_set_state_with_timeout(element, state);
  if (!res.has_value()) {
    openhd::log::get_default()->error(
        "State change to {} timed out", gst_element_state_get_name(state));
    return;
  }
  openhd::log::get_default()->debug(
      "State changed to {} result {}", gst_element_state_get_name(state),
      gst_state_change_return_to_string(res.value()));
}

// From
// https://gstreamer.freedesktop.org/documentation/application-development/advanced/pipeline-manipulation.html?gi-language=c
// and https://github.com/GStreamer/gst-docs/blob/master/examples/bus_example.c
static gboolean my_bus_callback(GstBus *bus, GstMessage *message,
                                gpointer user_data) {
  openhd::log::get_default()->debug("Got gst message [{}]",
                                    GST_MESSAGE_TYPE_NAME(message));
  switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR: {
      GError* error = nullptr;
      gchar* debug = nullptr;
      gst_message_parse_error(message, &error, &debug);
      const char* source = GST_MESSAGE_SRC(message) != nullptr
                               ? GST_OBJECT_NAME(GST_MESSAGE_SRC(message))
                               : "unknown";
      openhd::log::get_default()->error(
          "GStreamer error from {}: {}", source,
          error != nullptr && error->message != nullptr ? error->message
                                                        : "unknown error");
      if (debug != nullptr && debug[0] != '\0') {
        openhd::log::get_default()->error("GStreamer details: {}", debug);
      }
      if (error != nullptr) g_error_free(error);
      if (debug != nullptr) g_free(debug);
      // g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_EOS:
      openhd::log::get_default()->debug("we reached EOS");
      // g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_APPLICATION: {
      openhd::log::get_default()->debug("Got GST_MESSAGE_APPLICATION");
      if (gst_message_has_name(message, "ExPrerolled")) {
        /* it's our message */
        openhd::log::get_default()->debug("we are all prerolled, do seek");
        /*gst_element_seek (pipeline,
                         1.0, GST_FORMAT_TIME,
                         GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
                         GST_SEEK_TYPE_SET, 2 * GST_SECOND,
                         GST_SEEK_TYPE_SET, 5 * GST_SECOND);

        gst_element_set_state (pipeline, GST_STATE_PLAYING);*/
      }
      break;
    }
    // case GST_MESSAGE_STATE_CHANGED:
    default:
      openhd::log::get_default()->debug("unknown message ");
      break;
  }
  return TRUE;
}

static void register_message_cb(GstElement *pipeline) {
  auto bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  if (bus == nullptr) {
    openhd::log::get_default()->debug("Cannot get bus");
    return;
  }
  gst_bus_add_watch(bus, my_bus_callback, NULL);
  gst_object_unref(bus);
  openhd::log::get_default()->debug("added gst bus watch");
}

// From https://github.com/GStreamer/gst-docs/blob/master/examples/bus_example.c

}  // namespace openhd
#endif  // OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_
