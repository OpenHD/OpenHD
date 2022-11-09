//
// Created by consti10 on 22.08.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#include "JoystickReader.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <unistd.h>
#include <sstream>

static constexpr auto JOYSTICK_N=0;
static constexpr auto JOY_DEV="/sys/class/input/js0";

static SDL_Joystick *js;

static bool check_if_joystick_is_connected_via_fd(){
    return access(JOY_DEV, F_OK);
}

JoystickReader::JoystickReader(CHAN_MAP chan_map) {
  m_console = openhd::loggers::create_or_get("joystick_reader");
  assert(m_console);
  m_console->set_level(spd::level::debug);
  m_console->debug("JoystickReader::JoystickReader");
  reset_curr_values();
  m_chan_map=chan_map;
  if(!validate_channel_mapping(m_chan_map)){
    m_console->warn("Not a valid channel mapping, using default");
    m_chan_map=get_default_channel_mapping();
  }
  m_read_joystick_thread=std::make_unique<std::thread>([this] {
    loop();
  });
}

JoystickReader::~JoystickReader() {
  m_console->debug("JoystickReader::~JoystickReader()");
  terminate= true;
  m_read_joystick_thread->join();
  m_read_joystick_thread= nullptr;
}

void JoystickReader::loop() {
  while (!terminate){
    connect_once_and_read_until_error();
    // Error / no joystick found, try again later
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

void JoystickReader::connect_once_and_read_until_error() {
  /*if(!check_if_joystick_is_connected_via_fd()){
    // don't bother to try opening via SDL if there is no proper joystick FD
    std::cerr<<"Joystick FD does not exist\n";
    return;
  }*/
  if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) != 0){
    m_console->warn("SDL_INIT Error: {}",SDL_GetError());
    return;
  }
  const auto n_joysticks=SDL_NumJoysticks();
  if(n_joysticks<1){
    m_console->warn("No joysticks, num:{}",n_joysticks);
    SDL_Quit();
    return;
  }
  js = SDL_JoystickOpen(JOYSTICK_N);
  if (js == nullptr){
    m_console->warn("Couldn't open desired Joystick: {}",SDL_GetError());
    SDL_Quit();
    return;
  }
  auto name_pointer=SDL_JoystickName(js);
  std::string name="unknown";
  if(name_pointer!= nullptr){
    name=std::string(name_pointer);
  }
  std::stringstream ss;
  ss<<"Found joystick:\n";
  ss<<"Name:"<<name<<"\n";
  ss<<"N Axis:"<<SDL_JoystickNumAxes(js)<<"\n";
  ss<<"Trackballs::"<<SDL_JoystickNumBalls(js)<<"\n";
  ss<<"Buttons:"<<SDL_JoystickNumButtons(js)<<"\n";
  ss<<"Hats:"<<SDL_JoystickNumHats(js)<<"\n";
  m_console->info(ss.str());
  // Populate the data once by querying everything (after that, we just get the events from SDL)
  {
    // make a copy
    auto copy=m_curr_values;
    for(int i=0;i< SDL_JoystickNumAxes(js);i++){
      const auto curr= SDL_JoystickGetAxis(js,i);
      write_matching_axis(copy.values,i,curr);
    }
    for(int i=0;i< SDL_JoystickNumButtons(js);i++){
      const auto curr= SDL_JoystickGetButton(js,i);
      write_matching_button(copy.values,i,curr==0);
    }
    // write out the results
    std::lock_guard<std::mutex> guard(m_curr_values_mutex);
    m_curr_values=copy;
    m_curr_values.considered_connected= true;
    m_curr_values.last_update=std::chrono::steady_clock::now();
    m_curr_values.joystick_name=name;
  }
  bool disconnected=false;
  while (!disconnected && !terminate){
    wait_for_events(200);
    const int curr_num_joysticks=SDL_NumJoysticks();
    if(curr_num_joysticks<1){
      // This one seems to work just find
      m_console->warn("Joystick disconnected, SDL_NumJoysticks:{}",curr_num_joysticks);
      disconnected= true;
    }
    if(!SDL_JoystickGetAttached(js)){
      m_console->warn("Joystick disconnected, SDL_JoystickGetAttached() reports false");
      disconnected= true;
    }
    /*if(!check_if_joystick_is_connected_via_fd()){
      // When the joystick is re-connected, SDL won't resume working again.
      std::cerr<<"Joystick not connected, restarting\n";
      break;
    }*/
  }
  m_console->info("Joystick disconnected");
  reset_curr_values();
  SDL_Quit();
  // either joystick disconnected or something else went wrong.
}

void JoystickReader::wait_for_events(const int timeout_ms) {
  auto current=m_curr_values.values;
  int n_polled_events=0;
  SDL_Event event;
  bool any_new_data=false;
  // wait for at least one event with a timeut
  if(!SDL_WaitEventTimeout(&event,timeout_ms)){
    //m_console->debug("Got no event after 100ms");
    return;
  }
  // process this event
  auto ret= process_event(&event,current);
  if(ret==2 || ret==5 || ret==4){
    any_new_data= true;
    n_polled_events++;
  }
  // and then get as many more events as we can get (we already spun up the cpu anyways)
  while (SDL_PollEvent (&event)) {
    ret= process_event(&event,current);
    if(ret==2 || ret==5 || ret==4){
      any_new_data= true;
      n_polled_events++;
    }
  }
  //m_console->debug("N polled events:{}",n_polled_events);
  if(any_new_data){
    std::lock_guard<std::mutex> guard(m_curr_values_mutex);
    for(int i=0;i<m_curr_values.values.size();i++){
      m_curr_values.values[i]=current[i];
    }
    m_curr_values.last_update=std::chrono::steady_clock::now();
    m_curr_values.considered_connected= true;
  }
}


int JoystickReader::process_event(void *event1,std::array<uint16_t,N_CHANNELS>& current) {
  auto* event=(SDL_Event*)event1;
  int ret=0;
  switch (event->type) {
    case SDL_JOYAXISMOTION:
      m_console->debug("Joystick {}, Axis {} moved to {}", event->jaxis.which, event->jaxis.axis, event->jaxis.value);
      write_matching_axis(current, event->jaxis.axis,event->jaxis.value);
      ret= 2;
      break;
    case SDL_JOYBUTTONDOWN:
      m_console->debug("Button down");
      write_matching_button(current,event->jbutton.button, false);
      ret=5;
      break;
    case SDL_JOYBUTTONUP:
      m_console->debug("Button up");
      write_matching_button(current,event->jbutton.button, true);
      ret=4;
      break;
    case SDL_QUIT:
      m_console->debug("Got SDL_QUIT");
      ret=0;
      break;
    default:
      m_console->debug("Got Unknown SDL event type");
      ret=0;
      break;
  }
  return ret;
}

JoystickReader::CurrChannelValues JoystickReader::get_current_state() {
  std::lock_guard<std::mutex> guard(m_curr_values_mutex);
  return m_curr_values;
}

void JoystickReader::reset_curr_values() {
  std::lock_guard<std::mutex> guard(m_curr_values_mutex);
  m_curr_values.considered_connected=false;
  for(auto& el:m_curr_values.values){
    el=DEFAULT_RC_CHANNELS_VALUE;
  }
  m_curr_values.joystick_name="unknown";
}

std::string JoystickReader::curr_state_to_string(
    const JoystickReader::CurrChannelValues& curr_channel_values) {
  std::stringstream ss;
  ss<<"Connected:"<<(curr_channel_values.considered_connected ? "Y":"N")<<"\n";
  ss<<"Name:"<<curr_channel_values.joystick_name<<"\n";
  ss<<"Values:[";
  for(int i=0;i<curr_channel_values.values.size();i++){
    ss<<(int)curr_channel_values.values[i];
    if(i==curr_channel_values.values.size()-1){
      ss<<"]\n";
    }else{
      ss<<",";
    }
  }
  const auto delay_since_last_update=std::chrono::steady_clock::now()-curr_channel_values.last_update;
  ss<<"Delay since last update:"<<std::chrono::duration_cast<std::chrono::milliseconds>(delay_since_last_update).count()<<"ms";
  return ss.str();
}

uint16_t JoystickReader::parsetoMultiWii(int16_t value) {
  return (int16_t)(((((double)value)+32768.0)/65.536)+1000);
}

void JoystickReader::write_matching_axis(std::array<uint16_t, JoystickReader::N_CHANNELS>& rc_data,const uint8_t axis_index, const Sint16 value) {
  const auto index_opt= get_mapped_axis(axis_index);
  if(index_opt==std::nullopt){
    return;
  }
  const auto index=index_opt.value();
  if(index>=N_CHANNELS_RESERVED_FOR_AXES){
    m_console->warn("only {} channels reserved for axis, wanted {}",N_CHANNELS_RESERVED_FOR_AXES,index);
    return;
  }
  rc_data[index]=parsetoMultiWii(value);
}

void JoystickReader::write_matching_button(std::array<uint16_t, 18>& rc_data,const Uint8 button, bool up) {
  // The mavlink rc channels override message has more than enough "channels" anyways.
  //However, we could optimize here putting multiple buttons (aka bool) into one channel
  const int channel_index=JoystickReader::N_CHANNELS_RESERVED_FOR_AXES+button;
  if(channel_index<rc_data.size()){
    rc_data[channel_index] = up ? JoystickReader::VALUE_BUTTON_UP : JoystickReader::VALUE_BUTTON_DOWN;
  }
}

std::optional<JoystickReader::CHAN_MAP>
JoystickReader::convert_string_to_channel_mapping(const std::string& input) {
  auto split_into_substrings=OHDUtil::split_into_substrings(input,',');
  if(split_into_substrings.size()!=N_CHANNELS_RESERVED_FOR_AXES){
    openhd::loggers::get_default()->warn("Channel mapping wrong n channels:{}",split_into_substrings.size());
    return std::nullopt;
  }
  CHAN_MAP parsed_as_int{};
  for(int i=0;i<N_CHANNELS_RESERVED_FOR_AXES;i++){
    const auto as_int=OHDUtil::string_to_int(split_into_substrings[i]);
    if(!as_int.has_value())return std::nullopt;
    parsed_as_int[i]=as_int.value();
  }
  if(!validate_channel_mapping(parsed_as_int))return std::nullopt;
  return parsed_as_int;
}

bool JoystickReader::validate_channel_mapping(const CHAN_MAP& chan_map) {
  for(const auto& el:chan_map){ // NOLINT(readability-use-anyofallof)
    if(el<0 || el>N_CHANNELS_RESERVED_FOR_AXES-1){
      openhd::loggers::get_default()->warn("Channel mapping not a valid value{}",el);
      return false;
    }
  }
  return true;
}

JoystickReader::CHAN_MAP
JoystickReader::get_default_channel_mapping() {
  JoystickReader::CHAN_MAP ret{};
  for(int i=0;i<N_CHANNELS_RESERVED_FOR_AXES;i++){
    ret[i]=i;
  }
  return ret;
}

void JoystickReader::update_channel_maping(
    const JoystickReader::CHAN_MAP& new_chan_map) {
  std::lock_guard<std::mutex> guard(m_chan_map_mutex);
  if(!validate_channel_mapping(new_chan_map)){
    return;
  }
  m_chan_map=new_chan_map;
}

std::optional<int> JoystickReader::get_mapped_axis(int axis_index) {
  std::lock_guard<std::mutex> guard(m_chan_map_mutex);
  if(axis_index>=m_chan_map.size()){
    m_console->warn("Axis {} not mapped",axis_index);
    return std::nullopt;
  }
  return m_chan_map[axis_index];
}

#endif //OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND