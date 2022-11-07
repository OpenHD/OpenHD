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
uint16_t rcData[16];

static int16_t parsetoMultiWii(Sint16 value) {
  return (int16_t)(((((double)value)+32768.0)/65.536)+1000);
}

static void readAxis(SDL_Event *event) {
  auto myevent = (SDL_Event)*event;
  if ( myevent.jaxis.axis == ROLL_AXIS)
    rcData[0]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == PITCH_AXIS)
    rcData[1]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == THROTTLE_AXIS)
    rcData[2]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == YAW_AXIS)
    rcData[3]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis ==  AUX1_AXIS)
    rcData[4]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == AUX2_AXIS)
    rcData[5]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == AUX3_AXIS)
    rcData[6]=parsetoMultiWii(myevent.jaxis.value);

  if ( myevent.jaxis.axis == AUX4_AXIS)
    rcData[7]=parsetoMultiWii(myevent.jaxis.value);
}

static bool check_if_joystick_is_connected_via_fd(){
    return access(JOY_DEV, F_OK);
}

JoystickReader::JoystickReader(NEW_JOYSTICK_DATA_CB cb):m_cb(cb) {
  m_console = openhd::loggers::create_or_get("joystick_reader");
  assert(m_console);
  m_console->set_level(spd::level::debug);
  m_console->debug("JoystickReader::JoystickReader");
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
    //std::cout<<"Read joystick\n";
    read_events_until_empty();
    std::this_thread::sleep_for(std::chrono::milliseconds (1));
    /*if(!check_if_joystick_is_connected_via_fd()){
      // When the joystick is re-connected, SDL won't resume working again.
      std::cerr<<"Joystick not connected, restarting\n";
      break;
    }*/
  }
  SDL_Quit();
  // either joystick disconnected or somethings wrong.
}

// Reads all queued SDL events until there are none remaining
// We are only interested in the Joystick events
int JoystickReader::read_events_until_empty() {
  SDL_Event event;
  bool any_new_data=false;
  while (SDL_PollEvent (&event)) {
    switch (event.type) {
      case SDL_JOYAXISMOTION:
        m_console->debug("Joystick {}, Axis {} moved to {}", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
        readAxis(&event);
        any_new_data= true;
        //return 2;
        break;
      case SDL_JOYBUTTONDOWN:
        m_console->debug("Button down");
        if (event.jbutton.button < SWITCH_COUNT) { // newer Taranis software can send 24 buttons - we use 16
          rcData[8] |= 1 << event.jbutton.button;
        }
        any_new_data= true;
        //return 5;
        break;
      case SDL_JOYBUTTONUP:
        m_console->debug("Button up");
        if (event.jbutton.button < SWITCH_COUNT) {
          rcData[8] &= ~(1 << event.jbutton.button);
        }
        any_new_data= true;
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
  if(any_new_data){
    std::lock_guard<std::mutex> guard(m_curr_values_mutex);
    for(int i=0;i<m_curr_values.values.size();i++){
      m_curr_values.values[i]=rcData[i];
    }
    m_curr_values.last_update=std::chrono::steady_clock::now();
    m_curr_values.considered_connected= true;
  }
  return 0;
}

JoystickReader::CurrChannelValues JoystickReader::get_current_state() {
  std::lock_guard<std::mutex> guard(m_curr_values_mutex);
  return m_curr_values;
}

/*std::optional<std::array<uint16_t, 16>>
JoystickReader::get_new_data_if_available() {
  return std::optional<std::array<uint16_t, 16>>();
}*/
