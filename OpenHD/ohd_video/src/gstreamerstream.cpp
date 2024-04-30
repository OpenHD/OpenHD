#include "gstreamerstream.h"

#include <gst/gst.h>

#include <iostream>
#include <utility>
#include <vector>

#include "air_recording_helper.hpp"
#include "gst_appsink_helper.h"
#include "gst_debug_helper.h"
#include "gst_helper.hpp"
#include "gst_recording_demuxer.h"
#include "nalu/CodecConfigFinder.hpp"
#include "nalu/fragment_helper.h"
#include "nalu/nalu_helper.h"
#include "openhd_rtp.h"
#include "openhd_util.h"
#include "rpi_hdmi_to_csi_v4l2_helper.h"
#include "rtp_eof_helper.h"
#include "x20_cam_helper.h"

GStreamerStream::GStreamerStream(std::shared_ptr<CameraHolder> camera_holder,
                                 openhd::ON_ENCODE_FRAME_CB out_cb)
    //: CameraStream(platform, camera_holder, video_udp_port) {
    : CameraStream(std::move(camera_holder), std::move(out_cb)) {
  m_console = openhd::log::create_or_get(
      fmt::format("cam{}", m_camera_holder->get_camera().index));
  assert(m_console);
  m_console->debug("GStreamerStream::GStreamerStream for cam{}",
                   m_camera_holder->get_camera().cam_type_as_verbose_string());
  if (OHDFilesystemUtil::exists("/boot/openhd/exp_raw.txt")) {
    dirty_use_raw = true;
  }
  m_camera_holder->register_listener([this]() {
    // right now, every time the settings for this camera change, we just
    // re-start the whole stream. That is not ideal, since some cameras support
    // changing for example the bitrate or white balance during operation. But
    // wiring that up is not that easy. We call request_restart() to make sure
    // to not perform heavy operation(s) on the mavlink settings callback, since
    // we need to send the acknowledging response in time. Also, gstreamer and
    // camera(s) are sometimes buggy, so in the worst case gstreamer can become
    // unresponsive and block on the restart operation(s) which would be fatal
    // for telemetry.
    this->request_restart();
  });
  OHDGstHelper::initGstreamerOrThrow();
  if (m_camera_holder->get_camera().is_camera_type_usb_infiray()) {
    openhd::set_infiray_custom_control_zoom_absolute_async(
        m_camera_holder->get_settings()
            .infiray_custom_control_zoom_absolute_colorpalete,
        m_camera_holder->get_camera().usb_v4l2_device_number);
  }
  // m_gst_video_recorder=std::make_unique<GstVideoRecorder>();
  m_console->debug("GStreamerStream::GStreamerStream done");
}

GStreamerStream::~GStreamerStream() { GStreamerStream::terminate_looping(); }

void GStreamerStream::start_looping() {
  m_keep_looping = true;
  m_loop_thread =
      std::make_unique<std::thread>(&GStreamerStream::loop_infinite, this);
}

void GStreamerStream::terminate_looping() {
  m_keep_looping = false;
  if (m_loop_thread) {
    m_console->debug("Wating for loop thread to terminate");
    m_loop_thread->join();
    m_loop_thread = nullptr;
  }
}

std::string GStreamerStream::create_source_encode_pipeline(
    const CameraHolder& cam_holder) {
  const auto& camera = cam_holder.get_camera();
  CameraSettings setting = cam_holder.get_settings();
  const bool RPI_HDMI_TO_CSI_USE_V4l2 =
      OHDFilesystemUtil::exists("/boot/openhd/hdmi_v4l2.txt");
  if (OHDPlatform::instance().is_x20()) {
    openhd::x20::apply_x20_runcam_iq_settings(setting);
  } else if (camera.requires_rpi_mmal_pipeline() && RPI_HDMI_TO_CSI_USE_V4l2) {
    openhd::rpi::hdmi::initialize_resolution(
        setting.streamed_video_format.width,
        setting.streamed_video_format.height,
        setting.streamed_video_format.framerate);
  }
  std::stringstream pipeline;
  if (camera.requires_rpi_mmal_pipeline()) {
    if (RPI_HDMI_TO_CSI_USE_V4l2) {
      pipeline << OHDGstHelper::create_rpi_hdmi_v4l2_stream(setting);
    } else {
      pipeline << OHDGstHelper::createRpicamsrcStream(
          -1, setting, cam_holder.requires_half_bitrate_workaround());
    }
  } else if (camera.requires_rpi_libcamera_pipeline()) {
    pipeline << OHDGstHelper::createLibcamerasrcStream(setting);
  } else if (camera.requires_rpi_veye_pipeline()) {
    // veye  IMX462 needs video0. I think the others too ...
    auto bus = "/dev/video0";
    pipeline << OHDGstHelper::create_veye_vl2_stream(setting, bus);
  } else if (camera.requires_rockchip_mpp_pipeline()) {
    if (camera.camera_type == X_CAM_TYPE_ROCK_HDMI_IN) {
      pipeline << OHDGstHelper::createRockchipHDMIStream(setting);
    } else {
      // TODO: Differences Radxa zero and RK3588
      // RK3566 has camera on /dev/video0, RK3588 has it on /dev/video11
      const int v4l2_filenumber =
          OHDPlatform::instance().platform_type ==
                  X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W
              ? 0
              : 11;
      pipeline << OHDGstHelper::createRockchipCSIStream(v4l2_filenumber,
                                                        setting);
    }
  } else if (camera.requires_x20_cedar_pipeline()) {
    pipeline << OHDGstHelper::createAllwinnerStream(setting);
  } else if (is_usb_camera(camera.camera_type)) {
    const auto v4l2_device_name =
        get_v4l2_device_name_string(camera.usb_v4l2_device_number);
    pipeline << OHDGstHelper::createV4l2SrcRawAndSwEncodeStream(
        v4l2_device_name, setting);
  } else if (camera.camera_type == X_CAM_TYPE_DUMMY_SW) {
    pipeline << OHDGstHelper::createDummyStreamX(setting);
    // pipeline<<OHDGstHelper::createDummyStreamRPI(setting);
  } else if (camera.camera_type == X_CAM_TYPE_EXTERNAL ||
             camera.camera_type == X_CAM_TYPE_EXTERNAL_IP) {
    pipeline << OHDGstHelper::create_input_custom_udp_rtp_port(setting);
  } else if (camera.camera_type == X_CAM_TYPE_DEVELOPMENT_FILESRC) {
    pipeline << OHDGstHelper::create_dummy_filesrc_stream(setting);
  } else if (camera.camera_type == X_CAM_TYPE_NVIDIA_XAVIER_IMX577) {
    pipeline << OHDGstHelper::create_nvidia_xavier_stream(setting);
  } else {
    openhd::log::get_default()->warn("UNKNOWN CAMERA TYPE");
    pipeline << "ERROR";
  }
  return pipeline.str();
}

void GStreamerStream::setup() {
  m_console->debug("GStreamerStream::setup() begin");
  const auto& camera = m_camera_holder->get_camera();
  const auto& setting = m_camera_holder->get_settings();
  std::stringstream pipeline_content;
  m_bitrate_ctrl_element = std::nullopt;
  pipeline_content << create_source_encode_pipeline(*m_camera_holder);
  // quick check,here the pipeline should end with a "! ";
  if (!OHDUtil::endsWith(pipeline_content.str(), "! ")) {
    m_console->warn("Probably ill-formatted pipeline: [{}]",
                    pipeline_content.str());
  }
  const bool ADD_RECORDING_TO_PIPELINE =
      setting.air_recording == AIR_RECORDING_ON ||
      (setting.air_recording == AIR_RECORDING_AUTO_ARM_DISARM &&
       m_armed_enable_air_recording);
  // for safety we only add the tee command at the right place if recording is
  // enabled.
  if (ADD_RECORDING_TO_PIPELINE) {
    m_console->info("Air recording active");
    pipeline_content << "tee name=t ! ";
  }
  // After we've written the parts for the different camera implementation(s) we
  // just need to append the rtp part and the udp out add rtp part
  if (dirty_use_raw) {
    /*pipeline_content <<
    OHDGstHelper::create_queue_and_parse(setting.streamed_video_format.videoCodec);
    pipeline_content <<
    OHDGstHelper::create_caps_nal(setting.streamed_video_format.videoCodec);
    pipeline_content << " queue ! ";*/
    pipeline_content << " queue ! ";
    /*pipeline_content << OHDGstHelper::create_parse_for_codec(
        setting.streamed_video_format.videoCodec);
    pipeline_content << OHDGstHelper::create_caps_nal(
        setting.streamed_video_format.videoCodec, true);*/
    pipeline_content << OHDGstHelper::createOutputAppSink();
    /*pipeline_content << "video/x-h264,stream-format=byte-stream ! ";
    pipeline_content << OHDGstHelper::createOutputAppSink();*/
  } else {
    const int rtp_fragment_size = 1440;
    m_console->debug("Using {} for rtp fragmentation", rtp_fragment_size);
    pipeline_content << OHDGstHelper::create_parse_and_rtp_packetize(
        setting.streamed_video_format.videoCodec, rtp_fragment_size);
    pipeline_content << OHDGstHelper::createOutputAppSink();
  }
  if (ADD_RECORDING_TO_PIPELINE) {
    const auto recording_filename =
        openhd::video::create_unused_recording_filename(
            OHDGstHelper::file_suffix_for_video_codec(
                setting.streamed_video_format.videoCodec));
    m_console->debug("Using [{}] for recording", recording_filename);
    pipeline_content << OHDGstHelper::createRecordingForVideoCodec(
        setting.streamed_video_format.videoCodec, recording_filename);
    m_opt_curr_recording_filename = recording_filename;
  } else {
    m_opt_curr_recording_filename = std::nullopt;
  }
  {
    const auto index = m_camera_holder->get_camera().index;
    const uint8_t cam_type = (uint8_t)m_camera_holder->get_camera().camera_type;
    auto cam_info = openhd::LinkActionHandler::CamInfo{
        true,
        (uint8_t)index,
        cam_type,
        CAM_STATUS_RESTARTING,
        ADD_RECORDING_TO_PIPELINE,
        (uint8_t)video_codec_to_int(setting.streamed_video_format.videoCodec),
        (uint16_t)setting.h26x_bitrate_kbits,
        (uint8_t)setting.h26x_keyframe_interval,
        (uint16_t)setting.streamed_video_format.width,
        (uint16_t)setting.streamed_video_format.height,
        (uint16_t)setting.streamed_video_format.framerate};
    openhd::LinkActionHandler::instance().set_cam_info(index, cam_info);
  }
  m_console->debug("Starting pipeline:[{}]", pipeline_content.str());
  // Protect against unwanted use - stop and free the pipeline first
  assert(m_gst_pipeline == nullptr);
  // Now start the (as a string) built pipeline
  GError* error = nullptr;
  m_gst_pipeline = gst_parse_launch(pipeline_content.str().c_str(), &error);
  m_console->debug("GStreamerStream::setup() end");
  if (error) {
    m_console->error("Failed to create pipeline: {}", error->message);
    return;
  }
  m_bitrate_ctrl_element = get_dynamic_bitrate_control_element_in_pipeline(
      m_gst_pipeline, *m_camera_holder);
  // we pull data out of the gst pipeline as cpu memory buffer(s) using the
  // gstreamer "appsink" element
  m_app_sink_element =
      gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "out_appsink");
  assert(m_app_sink_element);
  // m_console->debug("Cam encoding format: {}",(int)cam_info.encoding_format);
  auto lol_cb =
      [this](
          std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments) {
        x_on_new_rtp_fragmented_frame(frame_fragments);
      };
  m_rtp_helper = std::make_shared<openhd::RTPHelper>(
      setting.streamed_video_format.videoCodec == VideoCodec::H265);
  m_rtp_helper->set_out_cb(lol_cb);
}

void GStreamerStream::start() {
  m_console->debug("GStreamerStream::start()");
  assert(m_gst_pipeline != nullptr);
  openhd::register_message_cb(m_gst_pipeline);
  const auto ret = gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
  m_console->debug("State change ret:{}",
                   openhd::gst_state_change_return_to_string(ret));
}

void GStreamerStream::stop() {
  m_console->debug("GStreamerStream::stop()");
  assert(m_gst_pipeline != nullptr);
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_PAUSED);
  m_console->debug(
      openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
}

void GStreamerStream::cleanup_pipe() {
  m_console->debug("GStreamerStream::cleanup_pipe() begin");
  assert(m_gst_pipeline != nullptr);
  // Drop the reference to the bitrate control element (if it exists)
  if (m_bitrate_ctrl_element.has_value()) {
    unref_bitrate_element(m_bitrate_ctrl_element.value());
  }
  // As well as the appsink (always exists)
  openhd::unref_appsink_element(m_app_sink_element);
  // Jan 22: Confirmed this hangs quite a lot of pipeline(s) - removed for that
  // reason
  /*m_console->debug("send EOS begin");
  // according to @Alex W we need a EOS signal here to properly shut down the
  pipeline if(!gst_element_send_event (m_gst_pipeline, gst_event_new_eos())){
    m_console->info("error gst_element_send_event eos"); // No idea what that
  means }else{ m_console->info("success gst_element_send_event eos");
  }*/
  // TODO do we need to wait until the pipeline is actually in state NULL ?
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline,
                                                   GST_STATE_NULL);
  gst_object_unref(m_gst_pipeline);
  m_gst_pipeline = nullptr;
  if (m_opt_curr_recording_filename) {
    // make file read / writeable by everybody
    OHDFilesystemUtil::make_file_read_write_everyone(
        m_opt_curr_recording_filename.value());
    // we do not want empty files - this can happen rarely in case the file is
    // created, but no video data is actually written to it actually, looks like
    // it is possible the file might be empty until the gst pipeline is actually
    // terminating - annoying gst crap ! why is there no way to properly
    // terminate ! better leave the file there
    /*if(OHDFilesystemUtil::get_file_size_bytes(m_opt_curr_recording_filename.value())==0){
      m_console->warn("Ground recording {} is
    empty",m_opt_curr_recording_filename.value());
      OHDFilesystemUtil::remove_if_existing(m_opt_curr_recording_filename.value());
    }*/
    m_opt_curr_recording_filename = std::nullopt;
  }
  // start demuxing of (all) .mkv files unless the FC is currently armed ( we
  // are in flight) this will of course also de-mux the new ground recording (if
  // there is any)
  // if(m_opt_action_handler &&
  // !m_opt_action_handler->arm_state.is_currently_armed()){
  // GstRecordingDemuxer::instance().demux_all_remaining_mkv_files_async();
  //}
  m_console->debug("GStreamerStream::cleanup_pipe() end");
}

void GStreamerStream::request_restart() { m_request_restart = true; }

void GStreamerStream::handle_change_bitrate_request(
    openhd::LinkActionHandler::LinkBitrateInformation lb) {
  // m_console->debug("handle_change_bitrate_request prev: {} new:{}",
  //                  kbits_per_second_to_string(m_curr_dynamic_bitrate_kbits),
  //                  kbits_per_second_to_string(lb.recommended_encoder_bitrate_kbits));
  //  We do some safety checks first - the link might recommend too much / too
  //  little
  auto bitrate_for_encoder_kbits = lb.recommended_encoder_bitrate_kbits;
  static auto MIN_BITRATE_KBITS = 1 * 1000;
  // RPi cannot do less than 2MBit/s
  if (OHDPlatform::instance().is_rpi()) {
    MIN_BITRATE_KBITS = 2 * 1000;
  }
  if (bitrate_for_encoder_kbits < MIN_BITRATE_KBITS) {
    // m_console->debug("Cam cannot do <{}",
    // kbits_per_second_to_string(MIN_BITRATE_KBITS));
    bitrate_for_encoder_kbits = MIN_BITRATE_KBITS;
  }
  // The gst thread is responsible for changing the bitrate - it will be applied
  // (as long as the cam is not bugged or the OS is overloaded) after a max
  // delay of 40ms
  m_curr_dynamic_bitrate_kbits = bitrate_for_encoder_kbits;
  if (m_camera_holder->get_settings().h26x_bitrate_kbits !=
      bitrate_for_encoder_kbits) {
    m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits =
        bitrate_for_encoder_kbits;
    m_camera_holder->persist(false);
  }
}

void GStreamerStream::handle_update_arming_state(bool armed) {
  m_console->debug("handle_update_arming_state: {}", armed);
  const auto settings = m_camera_holder->get_settings();
  if (settings.air_recording == AIR_RECORDING_AUTO_ARM_DISARM) {
    if (armed) {
      m_armed_enable_air_recording = true;
      m_console->warn("Starting air recording");
    } else {
      m_armed_enable_air_recording = false;
    }
    // restart pipeline such that recording is started / stopped
    request_restart();
  }
}

void GStreamerStream::loop_infinite() {
  while (m_keep_looping) {
    try {
      stream_once();
    } catch (std::exception& ex) {
      std::cerr << "GStreamerStream::Error: " << ex.what() << std::endl;
    } catch (...) {
      std::cerr << "GStreamerStream::Unknown exception occurred" << std::endl;
    }
  }
}

void GStreamerStream::stream_once() {
  // The user can disable streaming for a camera, in which case a restart is
  // requested and after that we land here (and do nothing)
  if (!m_camera_holder->get_settings().enable_streaming) {
    const auto elapsed_log =
        std::chrono::steady_clock::now() - m_last_log_streaming_disabled;
    if (elapsed_log > std::chrono::seconds(5)) {
      m_console->debug("streaming disabled");
      m_last_log_streaming_disabled = std::chrono::steady_clock::now();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return;
  }
  // First, we (try) starting the pipeline using the current settings
  openhd::LinkActionHandler::instance().set_cam_info_status(
      m_camera_holder->get_camera().index, CAM_STATUS_RESTARTING);
  setup();
  if (OHDPlatform::instance().is_x20()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  start();
  // Check if we were able to successfully start the pipeline. If - for example
  // - the camera doesn't exist or the resolution set is not supported by the
  // camera, we won't get further than this.
  bool succesfully_streaming = false;
  m_console->debug(openhd::gst_element_get_current_state_as_string(
      m_gst_pipeline, &succesfully_streaming));
  /*if(m_camera_holder->get_camera().rpi_csi_mmal_is_csi_to_hdmi ||
  m_camera_holder->get_camera().type==CameraType::ALLWINNER_CSI){
    m_console->warn("Not checking gst state after calling play (bugged)");
    succesfully_streaming= true;
  }*/
  succesfully_streaming = true;
  if (!succesfully_streaming) {
    m_console->warn("Cannot start streaming. Valid resolution ?",
                    m_camera_holder->get_camera().index);
    stop();
    cleanup_pipe();
    // Sleep a bit and hope it works next time
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;
  }
  //
  // Here we begin the loop where the camera only
  // 1) Constantly produces data
  // 2) reacts to bitrate change(s) from wb link
  // 3) breaks out of if a restart is requested (changed settings) or no data is
  // generated
  //    for X seconds.
  //
  // Bitrate is the only value we (NEED) to support changing without a restart
  int currently_applied_bitrate =
      m_camera_holder->get_settings().h26x_bitrate_kbits;
  m_curr_dynamic_bitrate_kbits = currently_applied_bitrate;
  // Now we should have a running pipeline and are able to pull samples from it
  // We use a timeout of 40ms to not unnecessarily wake up the thread on up to
  // 30fps (33ms) but also quickly respond to restart requests or bitrate
  // change(s)
  const uint64_t timeout_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::milliseconds(40))
          .count();
  // For 'bugged camera restart' fix
  std::chrono::steady_clock::time_point m_last_camera_frame =
      std::chrono::steady_clock::now();
  m_frame_fragments.resize(0);
  // As soon as we get the first frame, we change the status to streaming
  bool has_first_frame = false;
  // Every X seconds, we check if we are about to run out of space
  std::chrono::steady_clock::time_point
      m_last_air_recording_remaining_space_check =
          std::chrono::steady_clock::now();
  while (true) {
    // Quickly terminate if openhd wants to terminate
    if (!m_keep_looping) break;
    // ANNOYING BUGGED CAMERAS FIX - we restart the pipeline if we don't get a
    // frame from the camera for more than X seconds
    if (std::chrono::steady_clock::now() - m_last_camera_frame >
        std::chrono::seconds(5)) {
      m_console->warn("Restarting camera due to no frame after 5 seconds");
      m_request_restart = true;
    }
    // Check if we need to set a new bitrate
    if (currently_applied_bitrate != m_curr_dynamic_bitrate_kbits) {
      const int new_bitrate = m_curr_dynamic_bitrate_kbits;
      m_console->debug("Bitrate change, old:{} new:{}",
                       currently_applied_bitrate, new_bitrate);
      if (m_bitrate_ctrl_element != std::nullopt) {
        // apply the new bitrate
        // Don't forget, the rpi csi hdmi needs the 'half bitrate' hack
        auto hacked_bitrate_kbits = new_bitrate;
        if (m_camera_holder->requires_half_bitrate_workaround()) {
          m_console->debug(
              "applying hack - reduce bitrate by 2 to get actual correct "
              "bitrate");
          hacked_bitrate_kbits = hacked_bitrate_kbits / 2;
        }
        auto bitrate_ctrl_element = m_bitrate_ctrl_element.value();
        if (change_bitrate(bitrate_ctrl_element, hacked_bitrate_kbits)) {
          currently_applied_bitrate = new_bitrate;
          openhd::LinkActionHandler::instance().set_cam_info_bitrate(
              m_camera_holder->get_camera().index, currently_applied_bitrate);
        } else {
          m_console->warn("Cannot apply bitrate though code assumes itl work");
        }
      } else {
        // Sad, but if the camera doesn't support changing the bitrate without a
        // restart, we need to restart
        m_console->info("Bitrate change requires restart (Not good)");
        m_request_restart = true;
      }
    }
    // Check if we require a full restart
    bool tmp_true = true;
    if (m_request_restart.compare_exchange_strong(tmp_true, false)) {
      // Something that requires a whole restart of the pipeline happened
      m_console->debug("Restart requested, restarting");
      break;
    }
    const auto elapsed_remaining_space =
        std::chrono::steady_clock::now() -
        m_last_air_recording_remaining_space_check;
    if (elapsed_remaining_space > std::chrono::seconds(1)) {
      m_camera_holder->check_remaining_space_air_recording(true);
      m_last_air_recording_remaining_space_check =
          std::chrono::steady_clock::now();
    }
    // try get a new frame fragment from gst
    GstSample* sample = gst_app_sink_try_pull_sample(
        GST_APP_SINK(m_app_sink_element), timeout_ns);
    if (sample) {
      if (!has_first_frame) {
        has_first_frame = true;
        openhd::LinkActionHandler::instance().set_cam_info_status(
            m_camera_holder->get_camera().index, CAM_STATUS_STREAMING);
      }
      GstBuffer* buffer = gst_sample_get_buffer(sample);
      // tmp declaration for give sample back early optimization
      std::shared_ptr<std::vector<uint8_t>> fragment_data = nullptr;
      uint64_t buffer_dts = 0;
      if (buffer && gst_buffer_get_size(buffer) > 0) {
        fragment_data = openhd::gst_copy_buffer(buffer);
        buffer_dts = buffer->dts;
      }
      // Optimization: Give the buffer back to gstreamer as soon as possible.
      // After copying the data from the sample, unref it first, then forward
      // the data via cb
      gst_sample_unref(sample);
      sample = nullptr;
      if (fragment_data && !fragment_data->empty()) {
        // If we got a new sample, aggregate then forward
        if (dirty_use_raw) {
          m_rtp_helper->feed_multiple_nalu(fragment_data->data(),
                                           fragment_data->size());
        } else {
          on_new_rtp_frame_fragment(std::move(fragment_data), buffer_dts);
        }
        m_last_camera_frame = std::chrono::steady_clock::now();
      }
    }
  }
  // If we land here, we need to clean up the pipe and (re) start
  const auto terminate_begin = std::chrono::steady_clock::now();
  stop();
  cleanup_pipe();
  m_frame_fragments.resize(0);
  m_console->debug("Terminating pipeline took {}ms",
                   std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - terminate_begin)
                       .count());
}

void GStreamerStream::on_new_rtp_frame_fragment(
    std::shared_ptr<std::vector<uint8_t>> fragment, uint64_t dts) {
  m_frame_fragments.push_back(fragment);
  const auto curr_video_codec =
      m_camera_holder->get_settings().streamed_video_format.videoCodec;
  openhd::rtp_eof_helper::RTPFragmentInfo info{};
  const bool is_h265 = curr_video_codec == VideoCodec::H265;
  if (is_h265) {
    info = openhd::rtp_eof_helper::h265_more_info(fragment->data(),
                                                  fragment->size());
  } else {
    info = openhd::rtp_eof_helper::h264_more_info(fragment->data(),
                                                  fragment->size());
  }
  if (info.is_fu_start) {
    if (is_idr_frame(info.nal_unit_type, is_h265)) {
      m_last_fu_s_idr = true;
    } else {
      m_last_fu_s_idr = false;
    }
  }
  // m_console->debug("Fragment {} start:{} end:{}
  // type:{}",m_frame_fragments.size(),
  //                  OHDUtil::yes_or_no(info.is_fu_start),
  //                  OHDUtil::yes_or_no(info.is_fu_end),
  //                  x_get_nal_unit_type_as_string(info.nal_unit_type,is_h265));
  bool is_last_fragment_of_frame = info.is_fu_end;
  if (m_frame_fragments.size() > 500) {
    // Most likely something wrong with the "find end of frame" workaround
    m_console->debug("No end of frame found after 1000 fragments");
    is_last_fragment_of_frame = true;
  }
  if (is_last_fragment_of_frame) {
    on_new_rtp_fragmented_frame();
    m_frame_fragments.resize(0);
    m_last_fu_s_idr = false;
  }
}

void GStreamerStream::on_new_rtp_fragmented_frame() {
  // m_console->debug("Got frame with {} fragments",rtp_fragments.size());
  if (m_output_cb) {
    const auto stream_index = m_camera_holder->get_camera().index;
    const bool enable_ultra_secure_encryption =
        m_camera_holder->get_settings().enable_ultra_secure_encryption;
    const bool is_intra_enabled =
        m_camera_holder->get_settings().h26x_intra_refresh_type != -1;
    const bool is_intra_frame = m_last_fu_s_idr;
    auto frame = openhd::FragmentedVideoFrame{m_frame_fragments,
                                              std::chrono::steady_clock::now(),
                                              enable_ultra_secure_encryption,
                                              nullptr,
                                              is_intra_enabled,
                                              is_intra_frame};
    // m_console->debug("{}",frame.to_string());
    m_output_cb(stream_index, frame);
  } else {
    m_console->debug("No output cb");
  }
}

void GStreamerStream::x_on_new_rtp_fragmented_frame(
    std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments) {
  if (m_output_cb) {
    const auto stream_index = m_camera_holder->get_camera().index;
    const bool enable_ultra_secure_encryption =
        m_camera_holder->get_settings().enable_ultra_secure_encryption;
    const bool is_intra_enabled =
        m_camera_holder->get_settings().h26x_intra_refresh_type != -1;
    const bool is_intra_frame = m_last_fu_s_idr;
    auto frame = openhd::FragmentedVideoFrame{frame_fragments,
                                              std::chrono::steady_clock::now(),
                                              enable_ultra_secure_encryption,
                                              nullptr,
                                              is_intra_enabled,
                                              is_intra_frame};
    // m_console->debug("{}",frame.to_string());
    m_output_cb(stream_index, frame);
  } else {
    m_console->debug("No output cb");
  }
}