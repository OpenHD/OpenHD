//
// Created by consti10 on 25.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include "openhd_spdlog.h"

// Code that depends on gstreamer and falls in the "helper for debugging"
// category
namespace openhd {

// Helpfull links:
// https://gstreamer.freedesktop.org/documentation/additional/design/states.html?gi-language=c

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
  auto res = gst_element_set_state(element, state);
  openhd::log::get_default()->debug("State changed to {} result {}",
                                    gst_element_state_get_name(state),
                                    gst_state_change_return_to_string(res));
}

// From
// https://gstreamer.freedesktop.org/documentation/application-development/advanced/pipeline-manipulation.html?gi-language=c
// and https://github.com/GStreamer/gst-docs/blob/master/examples/bus_example.c
static gboolean my_bus_callback(GstBus *bus, GstMessage *message,
                                gpointer user_data) {
  openhd::log::get_default()->debug("Got gst message [{}]",
                                    GST_MESSAGE_TYPE_NAME(message));
  switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR:
      openhd::log::get_default()->debug("we received an error!");
      // g_main_loop_quit (loop);
      break;
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
