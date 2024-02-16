//
// Created by consti10 on 19.06.23.
//

#include "gst_recorder.h"

#include <gst/app/gstappsrc.h>

#include <string>

#include "air_recording_helper.hpp"
#include "camera_enums.hpp"
#include "gst_debug_helper.h"
#include "gst_helper.hpp"

static std::string create_recording_pipeline(const VideoCodec videoCodec,
                                             const std::string& out_filename) {
  std::stringstream ss;
  // input - appsrc
  // <<" ! ";
  ss << "appsrc name=m_in_appsrc ";
  // ss<<"is-live=true format=0 do-timestamp=true ";
  //  https://stackoverflow.com/a/60582926
  // ss<<"stream-type=0 format=3 is-live=false ";
  // ss<<"is-live=true ";
  // ss<<"emit-signals=true ";
  // ss<<" size=0 ";
  ss << " do-timestamp=true ";
  ss << OHDGstHelper::gst_create_rtp_caps(videoCodec);
  // ss<<" ! queue ";
  ss << " ! ";
  // first de-packetize rtp
  ss << OHDGstHelper::create_rtp_depacketize_for_codec(videoCodec);
  // parse just to be safe
  ss << OHDGstHelper::create_parse_for_codec(videoCodec);
  if (videoCodec == VideoCodec::H264 || videoCodec == VideoCodec::H265) {
    // ss <<"matroskamux ! filesink location="<<out_filename;
    ss << " filesink location=" << out_filename;
    // ss<< "udpsink host=127.0.0.1 port=5600";
  } else {
    ss << "avimux ! filesink location=" << out_filename;
  }
  // ss<<"decodebin ! autovideosink";
  return ss.str();
}

static void need_data(GstElement* pipeline, guint size,
                      GstVideoRecorder* self) {
  openhd::log::get_default()->debug("need_data");
  self->ready_data = true;
}

static void enough_data(GstElement* pipeline, GstVideoRecorder* self) {
  openhd::log::get_default()->debug("enough_data");
  self->ready_data = false;
}

GstVideoRecorder::GstVideoRecorder() {
  m_console = openhd::log::create_or_get("v_gst_recorder");
  assert(m_console);
  m_console->debug("GstVideoRecorder");
  const auto video_codec = VideoCodec::H264;
  const auto recording_filename =
      openhd::video::create_unused_recording_filename(
          OHDGstHelper::file_suffix_for_video_codec(video_codec));
  m_console->debug("Using [{}] for recording", recording_filename);

  const auto pipeline_str =
      create_recording_pipeline(video_codec, recording_filename);
  m_console->debug("Starting pipeline:[{}]", pipeline_str);
  GError* error = nullptr;
  m_gst_pipeline = gst_parse_launch(pipeline_str.c_str(), &error);
  if (error) {
    m_console->error("Failed to create pipeline: {}", error->message);
    return;
  }
  m_app_src_element =
      gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "m_in_appsrc");
  g_object_set(m_app_src_element, "format", GST_FORMAT_TIME, NULL);

  g_signal_connect(m_app_src_element, "need-data", G_CALLBACK(need_data), this);
  g_signal_connect(m_app_src_element, "enough-data", G_CALLBACK(enough_data),
                   this);

  /*g_object_set (G_OBJECT (m_app_src_element),
               "stream-type", 0,
               "is-live", TRUE,
               "format", GST_FORMAT_TIME,
               "do-timestamp",TRUE,
               NULL);*/
  start();
}

GstVideoRecorder::~GstVideoRecorder() { stop_and_cleanup(); }

void GstVideoRecorder::on_video_data(const uint8_t* data, int data_len) {
  if (!ready_data) return;
  // static GstClockTime timestamp = 0;
  GstBuffer* buffer = gst_buffer_new_and_alloc(data_len);
  const auto copied_data = gst_buffer_fill(buffer, 0, data, data_len);
  assert(copied_data == data_len);
  // GST_BUFFER_PTS (buffer) = timestamp;
  // timestamp++;
  // GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND,
  // 2); timestamp += GST_BUFFER_DURATION (buffer); GST_BUFFER_TIMESTAMP
  // (buffer) = std::chrono::steady_clock::now().time_since_epoch().count();
  // GST_BUFFER_DURATION (buffer) = 100;
  GST_BUFFER_DTS(buffer) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_PTS(buffer) = GST_CLOCK_TIME_NONE;

  GstFlowReturn ret;
  g_signal_emit_by_name(m_app_src_element, "push-buffer", buffer, &ret);
  // ret = gst_app_src_push_buffer(GST_APP_SRC(m_app_src_element), buffer);
  gst_buffer_unref(buffer);
  if (ret != GST_FLOW_OK) {
    m_console->warn("Cannot push buffer");
  } else {
    m_console->debug("Pushed buffer {}", data_len);
  }
  m_console->debug("Curr n buffers: {}", gst_app_src_get_current_level_buffers(
                                             GST_APP_SRC(m_app_src_element)));
  gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
  /*GstBuffer *buffer;
  GstFlowReturn ret;
  GstMapInfo map;

  buffer = gst_buffer_new_and_alloc (data_len);

  gst_buffer_map (buffer, &map, GST_MAP_WRITE);
  memcpy(map.data,data,data_len);
  gst_buffer_unmap (buffer, &map);

  g_signal_emit_by_name (m_app_src_element, "push-buffer", buffer, &ret);

  // Free the buffer now that we are done with it
  gst_buffer_unref (buffer);
  if (ret != GST_FLOW_OK) {
    m_console->warn("Cannot push buffer");
  }else{
    //m_console->debug("Pushed buffer {}",data_len);
  }*/
}

void GstVideoRecorder::enqueue_rtp_fragment(
    std::shared_ptr<std::vector<uint8_t>> fragment) {
  on_video_data(fragment->data(), fragment->size());
}

void GstVideoRecorder::start() {
  gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
  m_console->debug(
      openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  m_console->debug(
      openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
}

void GstVideoRecorder::stop_and_cleanup() {
  gst_element_send_event(m_gst_pipeline, gst_event_new_eos());
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_PAUSED);
  m_console->debug(
      openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_NULL);
  m_console->debug(
      openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
  gst_object_unref(m_gst_pipeline);
}
