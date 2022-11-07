//
// Created by consti10 on 22.08.22.
//

#include "JoystickReader.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <unistd.h>
#include <sstream>

static constexpr auto JOYSTICK_N=0;
static constexpr auto JOY_DEV="/sys/class/input/js0";

int ROLL_AXIS = 0;
int PITCH_AXIS =  1;
int YAW_AXIS = 3;
int THROTTLE_AXIS = 2;
int AUX1_AXIS = 4;
int AUX2_AXIS = 5;
int AUX3_AXIS = 6;
int AUX4_AXIS = 7;
static constexpr int SWITCH_COUNT=6;

static SDL_Joystick *js;

static int16_t parsetoMultiWii(Sint16 value) {
  return (int16_t)(((((double)value)+32768.0)/65.536)+1000);
}

static void write_matching_axis(std::array<uint16_t,16>&rc_data,SDL_Event *event) {
  auto myevent = (SDL_Event)*event;
  if ( myevent.jaxis.axis == ROLL_AXIS)
    rc_data[0]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == PITCH_AXIS)
    rc_data[1]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == THROTTLE_AXIS)
    rc_data[2]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == YAW_AXIS)
    rc_data[3]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis ==  AUX1_AXIS)
    rc_data[4]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == AUX2_AXIS)
    rc_data[5]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == AUX3_AXIS)
    rc_data[6]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == AUX4_AXIS)
    rc_data[7]=parsetoMultiWii(myevent.jaxis.value);
}

static bool check_if_joystick_is_connected_via_fd(){
    return access(JOY_DEV, F_OK);
}

JoystickReader::JoystickReader(NEW_JOYSTICK_DATA_CB cb):m_cb(cb) {
  m_console = openhd::loggers::create_or_get("joystick_reader");
  assert(m_console);
  m_console->set_level(spd::level::debug);
  m_console->debug("JoystickReader::JoystickReader");
  reset_curr_values();
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
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
}

void JoystickReader::connect_once_and_read_until_error() {
  /*if(!check_if_joystick_is_connected_via_fd()){
    // don't bother to try opening via SDL if there is no proper joystick FD
    std::cerr<<"Joystick FD does not exist\n";
    return;
  }*/
  if (SDL_Init (SDL_INIT_JOYSTICK | SDL_INIT_VIDEO) != 0){
    printf ("ERROR: %s\n", SDL_GetError ());
    return;
  }
  const auto n_joysticks=SDL_NumJoysticks();
  if(n_joysticks<1){
    printf("No joysticks, num:%d\n",SDL_NumJoysticks());
    SDL_Quit();
    return;
  }
  js = SDL_JoystickOpen(JOYSTICK_N);
  if (js == nullptr){
    printf("Couldn't open desired Joystick: %s\n",SDL_GetError());
    SDL_Quit();
    return;
  }
  auto name=SDL_JoystickName(js);
  std::stringstream ss;
  if(name!= nullptr){
    ss<<"Name:"<<name<<"\n";
  }else{
    ss<<"Name: nullptr\n";
  }
  ss<<"N Axis:"<<SDL_JoystickNumAxes(js)<<"\n";
  ss<<"Trackballs::"<<SDL_JoystickNumBalls(js)<<"\n";
  ss<<"Buttons:"<<SDL_JoystickNumButtons(js)<<"\n";
  ss<<"Hats:"<<SDL_JoystickNumHats(js)<<"\n";
  m_console->debug(ss.str());
  while (!terminate){
    m_curr_values.considered_connected= true;
    //std::cout<<"Read joystick\n";
    read_events_until_empty();
    std::this_thread::sleep_for(std::chrono::milliseconds (1));
    if(SDL_NumJoysticks()<1){
      m_console->warn("Joystick disconnected");
      terminate= true;
    }
    /*if(!check_if_joystick_is_connected_via_fd()){
      // When the joystick is re-connected, SDL won't resume working again.
      std::cerr<<"Joystick not connected, restarting\n";
      break;
    }*/
  }
  reset_curr_values();
  SDL_Quit();
  // either joystick disconnected or somethings wrong.
}

// Reads all queued SDL events until there are none remaining
// We are only interested in the Joystick events
int JoystickReader::read_events_until_empty() {
  auto current=m_curr_values.values;
  int n_polled_events=0;
  SDL_Event event;
  bool any_new_data=false;
  while (SDL_PollEvent (&event)) {
    switch (event.type) {
      case SDL_JOYAXISMOTION:
        m_console->debug("Joystick {}, Axis {} moved to {}", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
        write_matching_axis(current, &event);
        any_new_data= true;
        n_polled_events++;
        //return 2;
        break;
      case SDL_JOYBUTTONDOWN:
        m_console->debug("Button down");
        if (event.jbutton.button < SWITCH_COUNT) { // newer Taranis software can send 24 buttons - we use 16
          current[8] |= 1 << event.jbutton.button;
        }
        any_new_data= true;
        n_polled_events++;
        //return 5;
        break;
      case SDL_JOYBUTTONUP:
        m_console->debug("Button up");
        if (event.jbutton.button < SWITCH_COUNT) {
          current[8] &= ~(1 << event.jbutton.button);
        }
        any_new_data= true;
        n_polled_events++;
        //return 4;
        break;
      case SDL_QUIT:
        std::cout<<"X1\n";
        return 0;
      default:
        std::cout<<"X2\n";
        return 0;
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
  return 0;
}


int JoystickReader::process_event(void *event1,std::array<uint16_t,16>& current) {
  auto* event=(SDL_Event*)event1;
  switch (event->type) {
    case SDL_JOYAXISMOTION:
      m_console->debug("Joystick {}, Axis {} moved to {}", event->jaxis.which, event->jaxis.axis, event->jaxis.value);
      write_matching_axis(current, event);
      //return 2;
      break;
    case SDL_JOYBUTTONDOWN:
      m_console->debug("Button down");
      if (event->jbutton.button < SWITCH_COUNT) { // newer Taranis software can send 24 buttons - we use 16
        current[8] |= 1 << event->jbutton.button;
      }
      //return 5;
      break;
    case SDL_JOYBUTTONUP:
      m_console->debug("Button up");
      if (event->jbutton.button < SWITCH_COUNT) {
        current[8] &= ~(1 << event->jbutton.button);
      }
      //return 4;
      break;
    case SDL_QUIT:
      std::cout<<"X1\n";
      return 0;
    default:
      std::cout<<"X2\n";
      return 0;
  }
  return 0;
}

JoystickReader::CurrChannelValues JoystickReader::get_current_state() {
  std::lock_guard<std::mutex> guard(m_curr_values_mutex);
  return m_curr_values;
}

void JoystickReader::reset_curr_values() {
  std::lock_guard<std::mutex> guard(m_curr_values_mutex);
  m_curr_values.considered_connected=false;
  for(auto& el:m_curr_values.values){
    el=UINT16_MAX;
  }
}

/*std::optional<std::array<uint16_t, 16>>
JoystickReader::get_new_data_if_available() {
  return std::optional<std::array<uint16_t, 16>>();
}*/
