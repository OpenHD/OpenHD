//
// Created by consti10 on 25.01.23.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "openhd-spdlog.hpp"


// Code that depends on gstreamer and falls in the "helper for debugging" category
namespace openhd{

// Helpfull links: https://gstreamer.freedesktop.org/documentation/additional/design/states.html?gi-language=c

static std::string gst_state_change_return_to_string(GstStateChangeReturn & gst_state_change_return){
  return fmt::format("{}",gst_element_state_change_return_get_name(gst_state_change_return));
}

// BLocks up to 1 second, but should never block more than that
static std::string gst_element_get_current_state_as_string(GstElement * element){
  GstState state;
  GstState pending;
  const auto timeout=std::chrono::seconds(1);
  auto returnValue = gst_element_get_state(element, &state, &pending, std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
  return fmt::format("Gst state: ret:{} state:{} pending:{}",
                                    gst_state_change_return_to_string(returnValue),
                                    gst_element_state_get_name(state),
                                    gst_element_state_get_name(pending));
}

static void gst_element_set_set_state_and_log_result(GstElement *element, GstState state){
  auto res=gst_element_set_state(element,state);
  openhd::log::get_default()->debug("State changed to {} result {}",
                                    gst_element_state_get_name(state),
                                    gst_state_change_return_to_string(res));
}

// From https://gstreamer.freedesktop.org/documentation/application-development/advanced/pipeline-manipulation.html?gi-language=c
static void cb_message (GstBus     *bus,GstMessage *message,gpointer    user_data){
  GstElement *pipeline = GST_ELEMENT (user_data);

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
      openhd::log::get_default()->debug("we received an error!");
      //g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_EOS:
      openhd::log::get_default()->debug("we reached EOS");
      //g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_APPLICATION:
    {
      openhd::log::get_default()->debug("Got GST_MESSAGE_APPLICATION");
      if (gst_message_has_name (message, "ExPrerolled")) {
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
    //case GST_MESSAGE_STATE_CHANGED:
    default:
      openhd::log::get_default()->debug("unknown message ");
      break;
  }
}

static void register_message_cb(GstElement *pipeline){
  auto bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  if(bus== nullptr){
    openhd::log::get_default()->debug("Cannot get bus");
    return;
  }
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message", (GCallback) cb_message,
                   pipeline);
  openhd::log::get_default()->debug("added message debug cb");
}
}
#endif  // OPENHD_OPENHD_OHD_VIDEO_SRC_GST_DEBUG_HELPER_H_
