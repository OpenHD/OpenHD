#include "gstreamerstream.h"

#include <gst/gst.h>
#include <unistd.h>

#include <iostream>
#include <regex>
#include <utility>
#include <vector>

#include "air_recording_helper.hpp"
#include "ffmpeg_videosamples.hpp"
#include "gst_appsink_helper.h"
#include "gst_debug_helper.h"
#include "gst_helper.hpp"
#include "gst_recording_demuxer.h"
#include "openhd_util.h"
#include "rtp_eof_helper.h"

GStreamerStream::GStreamerStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,
                                 openhd::ON_ENCODE_FRAME_CB out_cb,std::shared_ptr<openhd::ActionHandler> opt_action_handler)
    //: CameraStream(platform, camera_holder, video_udp_port) {
    : CameraStream(platform,std::move(camera_holder),std::move(out_cb)),
      m_opt_action_handler(std::move(opt_action_handler))
{
  m_console=openhd::log::create_or_get("v_gststream");
  assert(m_console);
  m_console->debug("GStreamerStream::GStreamerStream()");
  // Since the dummy camera is SW, we generally cannot do more than 640x480@30 anyways.
  // (640x48@30 might already be too much on embedded devices).
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  if (camera.type == CameraType::DUMMY_SW && (setting.streamed_video_format.width > 640 ||
      setting.streamed_video_format.height > 480 || setting.streamed_video_format.framerate > 30)) {
    m_console->warn("Warning- Dummy camera is done in sw, high resolution/framerate might not work");
  }
  m_camera_holder->register_listener([this](){
    // right now, every time the settings for this camera change, we just re-start the whole stream.
    // That is not ideal, since some cameras support changing for example the bitrate or white balance during operation.
    // But wiring that up is not that easy.
    // We call request_restart() to make sure to not perform heavy operation(s) on the mavlink settings callback, since we need to send the
    // acknowledging response in time. Also, gstreamer and camera(s) are sometimes buggy, so in the worst case gstreamer can become unresponsive
    // and block on the restart operation(s) which would be fatal for telemetry.
    this->request_restart();
  });
  assert(setting.streamed_video_format.isValid());
  OHDGstHelper::initGstreamerOrThrow();
  //m_gst_video_recorder=std::make_unique<GstVideoRecorder>();
  m_console->debug("GStreamerStream::GStreamerStream done");
}

GStreamerStream::~GStreamerStream() {
  terminate_looping();
}

void GStreamerStream::start_looping() {
  m_keep_looping=true;
  m_loop_thread=std::make_unique<std::thread>(&GStreamerStream::loop_infinite, this);
}
void GStreamerStream::terminate_looping() {
  m_keep_looping=false;
  if(m_loop_thread){
    m_console->debug("Wating for loop thread to terminate");
    m_loop_thread->join();
    m_loop_thread=nullptr;
  }
}

void GStreamerStream::setup() {
  m_console->debug("GStreamerStream::setup() begin");
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  // atomic & called in regular intervals if variable bitrate is enabled.
  m_curr_dynamic_bitrate_kbits=setting.h26x_bitrate_kbits;
  if(!setting.enable_streaming){
    // When streaming is disabled, we just don't create the pipeline. We fully restart on all changes anyways.
    m_console->info("Streaming disabled");
    return;
  }
  m_pipeline_content.str("");
  m_pipeline_content.clear();
  m_bitrate_ctrl_element= std::nullopt;
  switch (camera.type) {
    case CameraType::RPI_CSI_MMAL: {
      setup_raspberrypi_mmal_csi();
      break;
    }
    case CameraType::RPI_CSI_LIBCAMERA: {
      setup_raspberrypi_libcamera();
      break;
    }
    case CameraType::JETSON_CSI: {
      setup_jetson_csi();
      break;
    }
    case CameraType::UVC: {
      setup_usb_uvc();
      break;
    }
    case CameraType::UVC_H264: {
      setup_usb_uvch264();
      break;
    }
    case CameraType::IP: {
      setup_ip_camera();
      break;
    }
    case CameraType::ALLWINNER_CSI: {
      setup_allwinner_csi();
      break;
    }
    case CameraType::DUMMY_SW: {
      setup_sw_dummy_camera();
      break;
    }
    case CameraType::RPI_CSI_VEYE_V4l2:{
      setup_raspberrypi_veye_v4l2();
      break;
    }
    case CameraType::ROCKCHIP_CSI:
      setup_rockchip_csi();     
      break;
    case CameraType::ROCKCHIP_HDMI: {
      setup_rockchip_hdmi();
      break;
    }
    case CameraType::CUSTOM_UNMANAGED_CAMERA:{
      setup_custom_unmanaged_camera();
    }break;
    case CameraType::UNKNOWN: {
      m_console->warn( "Unknown camera type");
      return;
    }
  }
  // quick check,here the pipeline should end with a "! ";
  if(!OHDUtil::endsWith(m_pipeline_content.str(),"! ")){
    m_console->warn("Probably ill-formatted pipeline: [{}]",m_pipeline_content.str());
  }
  const bool ADD_RECORDING_TO_PIPELINE=
      setting.air_recording==AIR_RECORDING_ON ||
      (setting.air_recording==AIR_RECORDING_AUTO_ARM_DISARM && m_armed_enable_air_recording);
  // for safety we only add the tee command at the right place if recording is enabled.
  if(ADD_RECORDING_TO_PIPELINE){
    m_console->info("Air recording active");
    m_pipeline_content <<"tee name=t ! ";
  }
  // After we've written the parts for the different camera implementation(s) we just need to append the rtp part and the udp out
  // add rtp part
  // Optimization: For high bitrate(s) we use slightly bigger rtp packet size
  /*int rtp_fragment_size=1024;
  if(setting.h26x_bitrate_kbits>10*1000){
    rtp_fragment_size=1440;
  }*/
  const int rtp_fragment_size=1440;
  m_console->debug("Using {} for rtp fragmentation",rtp_fragment_size);
  m_pipeline_content << OHDGstHelper::create_parse_and_rtp_packetize(
      setting.streamed_video_format.videoCodec,rtp_fragment_size);
  // forward data via udp localhost or using appsink and data callback
  //m_pipeline_content << OHDGstHelper::createOutputUdpLocalhost(m_video_udp_port);
  m_pipeline_content << OHDGstHelper::createOutputAppSink();
  if(ADD_RECORDING_TO_PIPELINE){
    const auto recording_filename=openhd::video::create_unused_recording_filename(
        OHDGstHelper::file_suffix_for_video_codec(setting.streamed_video_format.videoCodec));
    m_console->debug("Using [{}] for recording",recording_filename);
    m_pipeline_content <<OHDGstHelper::createRecordingForVideoCodec(setting.streamed_video_format.videoCodec,recording_filename);
    m_opt_curr_recording_filename=recording_filename;
  }else{
    m_opt_curr_recording_filename=std::nullopt;
  }
  m_console->debug("Starting pipeline:[{}]",m_pipeline_content.str());
  // Protect against unwanted use - stop and free the pipeline first
  assert(m_gst_pipeline == nullptr);
  // Now start the (as a string) built pipeline
  GError *error = nullptr;
  m_gst_pipeline = gst_parse_launch(m_pipeline_content.str().c_str(), &error);
  m_console->debug("GStreamerStream::setup() end");
  m_stream_creation_time=std::chrono::steady_clock::now();
  if (error) {
    m_console->error( "Failed to create pipeline: {}",error->message);
    return;
  }
  m_bitrate_ctrl_element=get_dynamic_bitrate_control_element_in_pipeline(m_gst_pipeline,camera.type);
  // we pull data out of the gst pipeline as cpu memory buffer(s) using the gstreamer "appsink" element
  m_app_sink_element=gst_bin_get_by_name(GST_BIN(m_gst_pipeline), "out_appsink");
  assert(m_app_sink_element);
  if(m_opt_action_handler){
    const auto index=m_camera_holder->get_camera().index;
    const auto cam_type= camera_type_to_int(m_camera_holder->get_camera().type);
    auto cam_info=openhd::ActionHandler::CamInfo{true,
     (uint8_t)index,cam_type,0,ADD_RECORDING_TO_PIPELINE, (uint8_t)video_codec_to_int(setting.streamed_video_format.videoCodec),(uint16_t)setting.h26x_bitrate_kbits,
                                           (uint8_t)setting.h26x_keyframe_interval,(uint16_t )setting.streamed_video_format.width,
                                           (uint16_t )setting.streamed_video_format.height,(uint16_t )setting.streamed_video_format.framerate};
    m_opt_action_handler->set_cam_info(index,cam_info);
    //m_console->debug("Cam encoding format: {}",(int)cam_info.encoding_format);
  }
}

void GStreamerStream::setup_raspberrypi_mmal_csi() {
  m_console->debug("Setting up Raspberry Pi CSI camera");
  // similar to jetson, for now we assume there is only one CSI camera connected.
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createRpicamsrcStream(-1, setting,m_camera_holder->requires_half_bitrate_workaround());
}

void GStreamerStream::setup_raspberrypi_veye_v4l2() {
  m_console->debug("setup_raspberrypi_veye_v4l2");
  // similar to jetson, for now we assume there is only one CSI camera connected.
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::create_veye_vl2_stream(setting,m_camera_holder->get_camera().bus);
}

void GStreamerStream::setup_raspberrypi_libcamera() {
  m_console->debug("Setting up Raspberry Pi libcamera camera");
  // similar to jetson, for now we assume there is only one CSI camera
  // connected.
  const auto& setting = m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createLibcamerasrcStream(
      m_camera_holder->get_camera().name, setting);
}

void GStreamerStream::setup_jetson_csi() {
  m_console->debug("Setting up Jetson CSI camera");
  // Well, i fixed the bug in the detection, with v4l2_open.
  // But still, /dev/video1 can be camera index 0 on jetson.
  // Therefore, for now, we just default to no camera index rn and let nvarguscamerasrc figure out the camera index.
  // This will work as long as there is no more than 1 CSI camera.
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createJetsonStream(-1,setting);
}

void GStreamerStream::setup_rockchip_hdmi() {
  m_console->debug("Setting up Rockchip HDMI");
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createRockchipHDMIStream(false, setting.h26x_bitrate_kbits, setting.streamed_video_format, setting.recordingFormat, setting.h26x_keyframe_interval);
}

void GStreamerStream::setup_rockchip_csi() {
  m_console->debug("Setting up Rockchip CSI");
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createRockchipCSIStream(false, setting.h26x_bitrate_kbits, setting.camera_rotation_degree, setting.streamed_video_format, setting.recordingFormat, setting.h26x_keyframe_interval);
}

void GStreamerStream::setup_allwinner_csi() {
  m_console->debug("Setting up Allwinner CSI camera");
  const auto& setting=m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createAllwinnerStream(0,setting.h26x_bitrate_kbits, setting.streamed_video_format, setting.h26x_keyframe_interval);
}

void GStreamerStream::setup_usb_uvc() {
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  m_console->debug("Setting up usb UVC camera Name:{}",camera.name);
  // Upcoming change - we do not do any non-raw formats anymore, since usb cameras with an integrated
  // encoder just don't have a way to reliably set params on the encoder
  if(!setting.force_sw_encode){
    // First we try and start a hw encoded path, (but hw encode by the camera encoder, not a local hw encoder)
    // where v4l2src directly provides encoded video buffers
    // (unless force sw encode is explicitly requested by the user)
    const auto opt_endpoint_for_codec= get_endpoint_supporting_codec(camera.v4l2_endpoints,setting.streamed_video_format.videoCodec);
    if(opt_endpoint_for_codec.has_value()){
      m_console->debug("Selected non-raw endpoint");
      m_pipeline_content << OHDGstHelper::createV4l2SrcAlreadyEncodedStream(opt_endpoint_for_codec.value().v4l2_device_node, setting);
      return;
    }
  }
  // If we land here, we need to do SW encoding, the v4l2src can only do raw video formats like YUV
  const auto opt_raw_endpoint= get_endpoint_supporting_raw(camera.v4l2_endpoints);
  if(opt_raw_endpoint.has_value()){
    m_console->debug("Selected RAW endpoint");
    m_pipeline_content << OHDGstHelper::createV4l2SrcRawAndSwEncodeStream(opt_raw_endpoint.value().v4l2_device_node,setting);
    return;
  }
  // If we land here, we couldn't create a stream for this camera.
  m_console->error("Setup USB UVC failed");
}

void GStreamerStream::setup_usb_uvch264() {
  m_console->debug("Setting up UVC H264 camera");
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  const auto endpoint = camera.v4l2_endpoints.front();
  // this one is always h264
  m_pipeline_content << OHDGstHelper::createUVCH264Stream(endpoint.v4l2_device_node,setting);
}

void GStreamerStream::setup_ip_camera() {
  m_console->debug("Setting up IP camera");
  const auto& setting= m_camera_holder->get_settings();
  if (setting.ip_cam_url.empty()) {
    //setting.url = "rtsp://192.168.0.10:554/user=admin&password=&channel=1&stream=0.sdp";
  }
  m_pipeline_content << OHDGstHelper::create_ip_cam_stream_with_depacketize_and_parse(
      setting.ip_cam_url,setting.streamed_video_format.videoCodec);
}

void GStreamerStream::setup_sw_dummy_camera() {
  m_console->debug("Setting up SW dummy camera");
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createDummyStream(setting);
}

void GStreamerStream::setup_custom_unmanaged_camera() {
  m_console->debug("Setting up custom unmanaged camera");
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::create_input_custom_udp_rtp_port(setting);
}

void GStreamerStream::stop_cleanup_restart() {
  const auto before=std::chrono::steady_clock::now();
  if(m_opt_action_handler){
      // Restarting status
      m_opt_action_handler->set_cam_info_status(m_camera_holder->get_camera().index,CAM_STATUS_RESTARTING);
  }
  stop();
  cleanup_pipe();
  setup();
  start();
  const auto elapsed=std::chrono::steady_clock::now()-before;
  m_console->debug("stop_cleanup_restart took {}",OHDUtil::time_readable(elapsed));
}


void GStreamerStream::start() {
  m_console->debug("GStreamerStream::start()");
  if(!m_gst_pipeline){
    m_console->warn("gst_pipeline==null");
    return;
  }
  openhd::register_message_cb(m_gst_pipeline);
  const auto ret=gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
  m_console->debug("State change ret:{}",openhd::gst_state_change_return_to_string(ret));
  // assume the cam is streaming okay - we log a message otherwise
  if(m_opt_action_handler){
     m_opt_action_handler->set_cam_info_status(m_camera_holder->get_camera().index,CAM_STATUS_STREAMING);
  }
  bool succesfully_streaming=false;
  m_console->debug(openhd::gst_element_get_current_state_as_string(m_gst_pipeline,&succesfully_streaming));
  if(!succesfully_streaming){
      // Not successfully in changing state, we'l restart the next time it is to check streaming state - but most likely
      // the cam doesn't support the set resolution
      m_console->warn("Camera {} error - unsupported resolution ?",m_camera_holder->get_camera().index);
  }
}

void GStreamerStream::stop() {
  m_console->debug("GStreamerStream::stop()");
  if(!m_gst_pipeline){
    m_console->debug("gst_pipeline==null");
    return;
  }
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline, GST_STATE_PAUSED);
  m_console->debug(openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
}

void GStreamerStream::cleanup_pipe() {
  m_console->debug("GStreamerStream::cleanup_pipe() begin");
  // Jan 22: Confirmed this hangs quite a lot of pipeline(s) - removed for that reason
  /*m_console->debug("send EOS begin");
  // according to @Alex W we need a EOS signal here to properly shut down the pipeline
  if(!gst_element_send_event (m_gst_pipeline, gst_event_new_eos())){
    m_console->info("error gst_element_send_event eos"); // No idea what that means
  }else{
    m_console->info("success gst_element_send_event eos");
  }*/
  // TODO do we need to wait until the pipeline is actually in state NULL ?
  openhd::gst_element_set_set_state_and_log_result(m_gst_pipeline, GST_STATE_NULL);
  gst_object_unref (m_gst_pipeline);
  m_gst_pipeline =nullptr;
  if(m_opt_curr_recording_filename){
    // make file read / writeable by everybody
    OHDFilesystemUtil::make_file_read_write_everyone(m_opt_curr_recording_filename.value());
    // we do not want empty files - this can happen rarely in case the file is created, but no video data is actually written to it
    // actually, looks like it is possible the file might be empty until the gst pipeline is actually terminating -
    // annoying gst crap ! why is there no way to properly terminate !
    // better leave the file there
    /*if(OHDFilesystemUtil::get_file_size_bytes(m_opt_curr_recording_filename.value())==0){
      m_console->warn("Ground recording {} is empty",m_opt_curr_recording_filename.value());
      OHDFilesystemUtil::remove_if_existing(m_opt_curr_recording_filename.value());
    }*/
    m_opt_curr_recording_filename=std::nullopt;
  }
  // start demuxing of (all) .mkv files unless the FC is currently armed ( we are in flight)
  // this will of course also de-mux the new ground recording (if there is any)
  //if(m_opt_action_handler && !m_opt_action_handler->arm_state.is_currently_armed()){
    //GstRecordingDemuxer::instance().demux_all_remaining_mkv_files_async();
  //}
  m_console->debug("GStreamerStream::cleanup_pipe() end");
}

void GStreamerStream::request_restart() {
  m_request_restart=true;
}

void GStreamerStream::handle_change_bitrate_request(openhd::ActionHandler::LinkBitrateInformation lb) {
  //m_console->debug("handle_change_bitrate_request prev: {} new:{}",
  //                 kbits_per_second_to_string(m_curr_dynamic_bitrate_kbits),
  //                 kbits_per_second_to_string(lb.recommended_encoder_bitrate_kbits));
  // We do some safety checks first - the link might recommend too much / too little
  auto bitrate_for_encoder_kbits =lb.recommended_encoder_bitrate_kbits;
  // No encoder I've seen can do <2MBit/s, at least the ones we use
  static constexpr auto MIN_BITRATE_KBITS=2*1000;
  if(bitrate_for_encoder_kbits <MIN_BITRATE_KBITS){
    //m_console->debug("Cam cannot do <{}", kbits_per_second_to_string(MIN_BITRATE_KBITS));
    bitrate_for_encoder_kbits =MIN_BITRATE_KBITS;
  }
  const auto cam_type=m_camera_holder->get_camera().type;
  if(cam_type==CameraType::RPI_CSI_MMAL || cam_type==CameraType::RPI_CSI_LIBCAMERA || cam_type==CameraType::RPI_CSI_VEYE_V4l2){
    // upper-bound - hard coded for now, since pi cannot do more than 20MBit/s
    static constexpr auto max_bitrate_kbits=19999;
    if(bitrate_for_encoder_kbits>max_bitrate_kbits){
      //m_console->debug("Cam cannot do more than {}", kbits_per_second_to_string(max_bitrate_kbits));
      bitrate_for_encoder_kbits =max_bitrate_kbits;
    }
  }
  // The gst thread is responsible for changing the bitrate - it will be applied (as long as the cam is not bugged or the OS is overloaded)
  // after a max delay of 40ms
  m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits=bitrate_for_encoder_kbits;
  m_curr_dynamic_bitrate_kbits=bitrate_for_encoder_kbits;
}

void GStreamerStream::on_new_rtp_fragmented_frame(std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments) {
  //m_console->debug("Got frame with {} fragments",frame_fragments.size());
  if(m_output_cb){
      const auto stream_index=m_camera_holder->get_camera().index;
      auto frame=openhd::FragmentedVideoFrame{
              std::move(frame_fragments),
              std::chrono::steady_clock::now(),
              m_camera_holder->get_settings().enable_ultra_secure_encryption};
      m_output_cb(stream_index,frame);
  }else{
      m_console->debug("No output cb");
  }
}

void GStreamerStream::on_new_rtp_frame_fragment(std::shared_ptr<std::vector<uint8_t>> fragment,uint64_t dts) {
  m_frame_fragments.push_back(fragment);
  const auto curr_video_codec=m_camera_holder->get_settings().streamed_video_format.videoCodec;
  bool is_last_fragment_of_frame=false;
  if(curr_video_codec==VideoCodec::H264){
    if(openhd::rtp_eof_helper::h264_end_block(fragment->data(),fragment->size())){
      is_last_fragment_of_frame= true;
    }
  }else if(curr_video_codec==VideoCodec::H265){
    if(openhd::rtp_eof_helper::h265_end_block(fragment->data(),fragment->size())){
      is_last_fragment_of_frame= true;
    }
  }else{
    // Not supported yet, forward them in chuncks of 20 (NOTE: This workaround is not ideal, since it creates ~1 frame of latency).
    is_last_fragment_of_frame=m_frame_fragments.size()>=20;
  }
  if(m_frame_fragments.size()>1000){
    // Most likely something wrong with the "find end of frame" workaround
    m_console->debug("No end of frame found after 1000 fragments");
    is_last_fragment_of_frame= true;
  }
  if(is_last_fragment_of_frame){
    on_new_rtp_fragmented_frame(m_frame_fragments);
    m_frame_fragments.resize(0);
  }
  /*if(m_gst_video_recorder){
    m_gst_video_recorder->enqueue_rtp_fragment(fragment);
  }*/
}

void GStreamerStream::handle_update_arming_state(bool armed) {
  m_console->debug("handle_update_arming_state: {}",armed);
  const auto settings=m_camera_holder->get_settings();
  if(settings.air_recording==AIR_RECORDING_AUTO_ARM_DISARM){
    if(armed){
      m_armed_enable_air_recording= true;
      m_console->warn("Starting air recording");
    }else{
      m_armed_enable_air_recording= false;
    }
    // restart pipeline such that recording is started / stopped
    request_restart();
  }
}

void GStreamerStream::loop_infinite() {
  while (m_keep_looping){
    try{
      // The user can disable streaming for a camera, in which case a restart is requested and after that we land here
      // (and do nothing)
      if(!m_camera_holder->get_settings().enable_streaming){
        m_console->debug("streaming disabled");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue ;
      }
      // First, we start the stream
      m_console->debug("starting");
      if(m_opt_action_handler){
        m_opt_action_handler->set_cam_info_status(m_camera_holder->get_camera().index,CAM_STATUS_RESTARTING);
      }
      setup();
      start();
      // Check if we were able to successfully start the camera stream.
      bool succesfully_streaming=false;
      m_console->debug(openhd::gst_element_get_current_state_as_string(m_gst_pipeline,&succesfully_streaming));
      if(!succesfully_streaming){
        m_console->warn("Camera {} error - unsupported resolution ?",m_camera_holder->get_camera().index);
        cleanup_pipe();
        // Sleep a bit and hope it works next time
        std::this_thread::sleep_for(std::chrono::seconds(5));
        continue ;
      }
      if(m_opt_action_handler){
        m_opt_action_handler->set_cam_info_status(m_camera_holder->get_camera().index,CAM_STATUS_STREAMING);
      }
      // Bitrate is the only value we (NEED) to support changing without a restart
      int currently_applied_bitrate=m_camera_holder->get_settings().h26x_bitrate_kbits;
      // Now we should have a running pipeline and are able to pull samples from it
      // We use a timeout of 40ms to not unnecessarily wake up the thread on up to 30fps (33ms) but also
      // quickly respond to restart requests or bitrate change(s)
      const uint64_t timeout_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(40)).count();
      // We constantly monitor the camera output - if we do not get a frame from gst for more than X seconds,
      // we restart the camera in the hopes of fixing things
      std::chrono::steady_clock::time_point m_last_camera_frame=std::chrono::steady_clock::now();
      while (true){
        if(!m_keep_looping) break ;
        // ANNOYING BUGGED CAMERAS FIX
        if(std::chrono::steady_clock::now()-m_last_camera_frame>std::chrono::seconds(5)){
          m_console->warn("Restarting camera due to no frame after 5 seconds");
          m_request_restart=true;
        }
        // Bitrate change (by WB link)
        { // 2) Check if we need to set a new bitrate
          if(currently_applied_bitrate != m_curr_dynamic_bitrate_kbits){
            const int new_bitrate=m_curr_dynamic_bitrate_kbits;
            m_console->debug("Got new bitrate {} for camera {}",new_bitrate,m_camera_holder->get_camera().index);
            if(m_bitrate_ctrl_element!=std::nullopt){
              // apply the new bitrate
              // Don't forget, the rpi csi hdmi needs the 'half bitrate' hack
              auto hacked_bitrate_kbits=new_bitrate;
              if(m_camera_holder->requires_half_bitrate_workaround()){
                m_console->debug("applying hack - reduce bitrate by 2 to get actual correct bitrate");
                hacked_bitrate_kbits =  hacked_bitrate_kbits / 2;
              }
              auto bitrate_ctrl_element=m_bitrate_ctrl_element.value();
              if(change_bitrate(bitrate_ctrl_element,hacked_bitrate_kbits)){
                currently_applied_bitrate=new_bitrate;
              }else{
                m_console->warn("Cannot apply bitrate though code assumes itl work");
              }
            }else{
              // Sad, but if the camera doesn't support changing the bitrate without a restart, we need to restart
              m_console->info("Bitrate change requires restart (Not good)");
              m_request_restart= true;
            }
          }
          if(m_opt_action_handler){
            m_opt_action_handler->set_cam_info_bitrate(m_camera_holder->get_camera().index,m_curr_dynamic_bitrate_kbits);
          }
        }
        // 1) Check if we require a full restart
        bool tmp_true= true;
        if(m_request_restart.compare_exchange_strong(tmp_true, false)){
          // Something that requires a whole restart of the pipeline happened
          m_console->debug("Restart requested, restarting");
          break ;
        }
        // try get a new frame from gst
        GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(m_app_sink_element),timeout_ns);
        if (sample) {
          // If we got a new sample, forward it to the link
          GstBuffer* buffer = gst_sample_get_buffer(sample);
          if (buffer) {
            auto buff_copy=openhd::gst_copy_buffer(buffer);
            on_new_rtp_frame_fragment(std::move(buff_copy),buffer->dts);
            m_last_camera_frame=std::chrono::steady_clock::now();
          }
          gst_sample_unref(sample);
        }
      }
      // If we land here, we need to clean up the pipe and (re) start
      stop();
      cleanup_pipe();
      m_frame_fragments.resize(0);
    } catch (std::exception &ex) {
      std::cerr << "GStreamerStream::Error: " << ex.what() << std::endl;
    } catch (...) {
      std::cerr << "GStreamerStream::Unknown exception occurred" << std::endl;
    }
  }
}

