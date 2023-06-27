#include "gstreamerstream.h"

#include <gst/gst.h>
#include <unistd.h>

#include <regex>
#include <vector>

#include "air_recording_helper.hpp"
#include "ffmpeg_videosamples.hpp"
#include "gst_helper.hpp"

#include "gst_appsink_helper.h"
#include "gst_debug_helper.h"
#include "rtp_eof_helper.h"
#include "gst_recording_demuxer.h"

#include "openhd_util_time.hpp"

GStreamerStream::GStreamerStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder,
                                 std::shared_ptr<OHDLink> i_transmit_video,std::shared_ptr<openhd::ActionHandler> opt_action_handler)
    //: CameraStream(platform, camera_holder, video_udp_port) {
    : CameraStream(platform,camera_holder,i_transmit_video),
      m_opt_action_handler(opt_action_handler)
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
    // We call restart_async() to make sure to not perform heavy operation(s) on the mavlink settings callback, since we need to send the
    // acknowledging response in time. Also, gstreamer and camera(s) are sometimes buggy, so in the worst case gstreamer can become unresponsive
    // and block on the restart operation(s) which would be fatal for telemetry.
    this->restart_async();
  });
  assert(setting.streamed_video_format.isValid());
  OHDGstHelper::initGstreamerOrThrow();
  //m_gst_video_recorder=std::make_unique<GstVideoRecorder>();
  // Register a callback such that we get notified when the FC is armed / disarmed
  if(m_opt_action_handler){
    auto cb=[this](bool armed){
      this->update_arming_state(armed);
    };
    m_opt_action_handler->m_action_record_video_when_armed=std::make_shared<openhd::ActionHandler::ACTION_RECORD_VIDEO_WHEN_ARMED>(cb);
  }
  m_console->debug("GStreamerStream::GStreamerStream done");
}

GStreamerStream::~GStreamerStream() {
  if(m_opt_action_handler){
    m_opt_action_handler->m_action_record_video_when_armed= nullptr;
  }
  // they are safe to call, regardless if we are already in cleaned up state or not
  GStreamerStream::stop();
  GStreamerStream::cleanup_pipe();
}

void GStreamerStream::setup() {
  m_console->debug("GStreamerStream::setup() begin");
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  if(m_opt_action_handler){
    m_opt_action_handler->dirty_set_bitrate_of_camera(m_camera_holder->get_camera().index,setting.h26x_bitrate_kbits);
  }
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
      break;
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
  m_pipeline_content << OHDGstHelper::create_parse_and_rtp_packetize(
      setting.streamed_video_format.videoCodec);
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
  m_pull_samples_run= true;
  m_pull_samples_thread=std::make_unique<std::thread>(&GStreamerStream::loop_pull_samples, this);
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
  m_pipeline_content << OHDGstHelper::createRockchipCSIStream(false, setting.h26x_bitrate_kbits, setting.streamed_video_format, setting.recordingFormat, setting.h26x_keyframe_interval);
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
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  if (setting.ip_cam_url.empty()) {
    //setting.url = "rtsp://192.168.0.10:554/user=admin&password=&channel=1&stream=0.sdp";
  }
  m_pipeline_content << OHDGstHelper::create_ip_cam_stream_with_depacketize_and_parse(
      setting.ip_cam_url,setting.streamed_video_format.videoCodec);
}

void GStreamerStream::setup_sw_dummy_camera() {
  m_console->debug("Setting up SW dummy camera");
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::createDummyStream(setting);
}

void GStreamerStream::setup_custom_unmanaged_camera() {
  m_console->debug("Setting up custom unmanaged camera");
  const auto& camera= m_camera_holder->get_camera();
  const auto& setting= m_camera_holder->get_settings();
  m_pipeline_content << OHDGstHelper::create_input_custom_udp_rtp_port(setting);
}

void GStreamerStream::stop_cleanup_restart() {
  const auto before=std::chrono::steady_clock::now();
  stop();
  cleanup_pipe();
  setup();
  start();
  const auto elapsed=std::chrono::steady_clock::now()-before;
  m_console->debug("stop_cleanup_restart took {}",openhd::util::time::R(elapsed));
}

std::string GStreamerStream::createDebug(){
  std::unique_lock<std::mutex> lock(m_pipeline_mutex, std::try_to_lock);
  if(!lock.owns_lock()){
    // We can just discard statistics data during a re-start
    return "GStreamerStream::No debug during restart\n";
  }
  if(!m_camera_holder->get_settings().enable_streaming){
    return fmt::format("GStreamerStream for camera {} disabled",m_camera_holder->get_camera().to_short_string());
  }
  std::stringstream ss;
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(m_gst_pipeline, &state, &pending, 1000000000);
  ss << "GStreamerStream for camera:"<< m_camera_holder->get_camera().to_short_string()<<" State:"<< returnValue << "." << state << "." << pending << ".";
  return ss.str();
}

void GStreamerStream::start() {
  m_console->debug("GStreamerStream::start()");
  if(!m_gst_pipeline){
    m_console->warn("gst_pipeline==null");
    return;
  }
  openhd::register_message_cb(m_gst_pipeline);
  gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
  m_console->debug(openhd::gst_element_get_current_state_as_string(m_gst_pipeline));
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
  if(m_pull_samples_thread){
    m_console->debug("terminating appsink poll thread begin");
    m_pull_samples_run= false;
    if(m_pull_samples_thread->joinable())m_pull_samples_thread->join();
    m_pull_samples_thread= nullptr;
    m_console->debug("terminating appsink poll thread end");
  }
  if(!m_gst_pipeline){
    m_console->debug("gst_pipeline==null");
    return;
  }
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
    if(OHDFilesystemUtil::get_file_size_bytes(m_opt_curr_recording_filename.value())==0){
      m_console->warn("Ground recording {} is empty",m_opt_curr_recording_filename.value());
      OHDFilesystemUtil::remove_if_existing(m_opt_curr_recording_filename.value());
    }
    // after we have stopped the pipeline, if recording was active, we have a .mkv file -
    // convert it to something more usable in the background, unless the FC is currently armed
    if(m_opt_action_handler && !m_opt_action_handler->is_currently_armed()){
      GstRecordingDemuxer::instance().demux_all_remaining_mkv_files_async();
    }
    m_opt_curr_recording_filename=std::nullopt;
  }
  m_console->debug("GStreamerStream::cleanup_pipe() end");
}

void GStreamerStream::restartIfStopped() {
  std::lock_guard<std::mutex> guard(m_pipeline_mutex);
  if(!m_gst_pipeline){
    m_console->debug("gst_pipeline==null");
    return;
  }
  if(OHDFilesystemUtil::get_remaining_space_in_mb()<MINIMUM_AMOUNT_FREE_SPACE_FOR_AIR_RECORDING_MB){
    if(m_camera_holder->get_settings().air_recording==AIR_RECORDING_ON
        || m_camera_holder->get_settings().air_recording==AIR_RECORDING_AUTO_ARM_DISARM){
      m_console->warn("Disabling recording, not enough free space (<300MB)");
      m_camera_holder->unsafe_get_settings().air_recording=AIR_RECORDING_OFF;
      m_camera_holder->persist();
      stop_cleanup_restart();
    }
  }
  if(m_camera_holder->get_camera().type==CameraType::CUSTOM_UNMANAGED_CAMERA){
    // this pattern doesn't work here
    return;
  }
  const auto elapsed_since_start=std::chrono::steady_clock::now()-m_stream_creation_time;
  if(elapsed_since_start<std::chrono::seconds(5)){
    // give the cam X seconds in the beginning to properly start before restarting
    return;
  }
  GstState state;
  GstState pending;
  auto returnValue = gst_element_get_state(m_gst_pipeline, &state, &pending, 1000000000); // timeout in ns
  if (returnValue == 0) {
    m_console->debug("Panic gstreamer pipeline state is not running, restarting camera stream for camera:{}",m_camera_holder->get_camera().name);
    // We fully restart the whole pipeline, since some issues might not be fixable by just setting paused
    // This will also show up in QOpenHD (log level >= warn), but we are limited by the n of characters in mavlink
    m_console->warn("Restarting camera, check your parameters / connection");
    stop_cleanup_restart();
    m_console->debug("Restarted");
  }
}

// Restart after a new settings value has been applied
void GStreamerStream::restart_after_new_setting() {
  std::lock_guard<std::mutex> guard(m_pipeline_mutex);
  m_console->debug("GStreamerStream::restart_after_new_setting() begin");
  // R.N we need to fully re-set the pipeline if any camera setting has changed
  stop_cleanup_restart();
  m_console->debug("GStreamerStream::restart_after_new_setting() end");
}

void GStreamerStream::restart_async() {
  std::lock_guard<std::mutex> guard(m_async_thread_mutex);
  // If there is already an async operation running, we need to wait for it to complete.
  // If the user was to change parameters to quickly, this would be a problem.
  if(m_async_thread != nullptr){
    if(m_async_thread->joinable()){
      m_console->info("restart_async: waiting for previous operation to finish");
      m_async_thread->join();
      m_console->info("restart_async: previous operation finished");
    }
    m_async_thread =nullptr;
  }
  m_async_thread =std::make_unique<std::thread>(&GStreamerStream::restart_after_new_setting,this);
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
  // upper-bound - hard coded for now, since pi cannot do more than 19MBit/s
  static constexpr auto max_bitrate_kbits=19*1000;
  if(bitrate_for_encoder_kbits >max_bitrate_kbits){
    //m_console->debug("Cam cannot do more than {}", kbits_per_second_to_string(max_bitrate_kbits));
    bitrate_for_encoder_kbits =max_bitrate_kbits;
  }
  if(m_curr_dynamic_bitrate_kbits==bitrate_for_encoder_kbits){
    //m_console->debug("Cam already at {}",m_curr_dynamic_bitrate_kbits);
    return ;
  }
  m_console->debug("Changing bitrate to from {} kBit/s to {} kBit/s",m_camera_holder->get_settings().h26x_bitrate_kbits,bitrate_for_encoder_kbits);
  if(try_dynamically_change_bitrate( bitrate_for_encoder_kbits)){
    m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits=bitrate_for_encoder_kbits;
    // Do not trigger a full restart - we already changed the bitrate dynamically
    m_camera_holder->persist(false);
    m_curr_dynamic_bitrate_kbits= bitrate_for_encoder_kbits;
    if(m_opt_action_handler){
      m_opt_action_handler->dirty_set_bitrate_of_camera(m_camera_holder->get_camera().index,m_curr_dynamic_bitrate_kbits);
    }
  }else{
    const auto cam_type=m_camera_holder->get_camera().type;
    if(cam_type==CameraType::RPI_CSI_LIBCAMERA || cam_type==CameraType::RPI_CSI_VEYE_V4l2){
      m_console->warn("Bitrate change requires restart");
      // These cameras are known to handle a restart quickly, but it still sucks v4l2h264enc does not support changing the bitrate at run time
      m_camera_holder->unsafe_get_settings().h26x_bitrate_kbits=bitrate_for_encoder_kbits;
      // This triggers a restart of the pipeline
      m_camera_holder->persist();
    }else{
      m_console->warn("Camera does not support variable bitrate");
    }
  }
}

bool GStreamerStream::try_dynamically_change_bitrate(int bitrate_kbits) {
  std::lock_guard<std::mutex> guard(m_pipeline_mutex);
  if(m_gst_pipeline== nullptr){
    m_console->debug("cannot change_bitrate, no pipeline");
    return false;
  }
  if(m_bitrate_ctrl_element==std::nullopt){
    m_console->warn("Camera {} does not support changing bitrate dynamically",m_camera_holder->get_camera().name);
    return false;
  }
  auto bitrate_ctrl_element=m_bitrate_ctrl_element.value();
  auto hacked_bitrate_kbits=bitrate_kbits;
  if(m_camera_holder->requires_half_bitrate_workaround()){
    m_console->debug("applying hack - reduce bitrate by 2 to get actual correct bitrate");
    hacked_bitrate_kbits =  hacked_bitrate_kbits / 2;
  }
  return change_bitrate(bitrate_ctrl_element,hacked_bitrate_kbits);
}

void GStreamerStream::on_new_rtp_fragmented_frame(std::vector<std::shared_ptr<std::vector<uint8_t>>> frame_fragments) {
  //m_console->debug("Got frame with {} fragments",frame_fragments.size());
  if(m_link_handle){
    const auto stream_index=m_camera_holder->get_camera().index;
    m_link_handle->transmit_video_data(stream_index,openhd::FragmentedVideoFrame{frame_fragments});
  }else{
    m_console->debug("No transmit interface");
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

void GStreamerStream::loop_pull_samples() {
  assert(m_app_sink_element);
  auto cb=[this](std::shared_ptr<std::vector<uint8_t>> fragment,uint64_t dts){
    on_new_rtp_frame_fragment(fragment,dts);
  };
  openhd::loop_pull_appsink_samples(m_pull_samples_run,m_app_sink_element,cb);
  m_frame_fragments.resize(0);
}

void GStreamerStream::update_arming_state(bool armed) {
  m_console->debug("update_arming_state: {}",armed);
  const auto settings=m_camera_holder->get_settings();
  if(settings.air_recording==AIR_RECORDING_AUTO_ARM_DISARM){
    if(armed){
      m_armed_enable_air_recording= true;
      m_console->warn("Starting air recording");
    }else{
      m_armed_enable_air_recording= false;
    }
    // restart pipeline such that recording is started / stopped
    restart_async();
  }
}
