//
// Created by consti10 on 22.08.22.
//

#include "JoystickReader.h"

#include <iostream>
#include <unistd.h>

#define JOYSTICK_N 0
#define JOY_DEV "/sys/class/input/js0"

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
  SDL_Event myevent = (SDL_Event)*event;
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

static int eventloop_joystick (void) {
  SDL_Event event;
  while (SDL_PollEvent (&event)) {
    switch (event.type) {
      case SDL_JOYAXISMOTION:
        //printf ("Joystick %d, Axis %d moved to %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
        readAxis(&event);
        return 2;
        break;
      case SDL_JOYBUTTONDOWN:
        if (event.jbutton.button < SWITCH_COUNT) { // newer Taranis software can send 24 buttons - we use 16
          rcData[8] |= 1 << event.jbutton.button;
        }
        return 5;
        break;
      case SDL_JOYBUTTONUP:
        if (event.jbutton.button < SWITCH_COUNT) {
          rcData[8] &= ~(1 << event.jbutton.button);
        }
        return 4;
        break;
      case SDL_QUIT:
        return 0;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
  return 1;
}

static bool check_if_joystick_is_connected_via_fd(){
    return access(JOY_DEV, F_OK);
}


JoystickReader::JoystickReader(NEW_JOYSTICK_DATA_CB cb):m_cb(cb) {
  std::cout<<"Joystick init\n";
  m_read_joystick_thread=std::make_unique<std::thread>([this] {
    loop();
  });
}

JoystickReader::~JoystickReader() {
  terminate= true;
  m_read_joystick_thread->join();
  m_read_joystick_thread= nullptr;
}

void JoystickReader::loop() {
  while (!terminate){
    connect_once_and_read_until_error();
    // Error / no joystick found, try again later
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if(m_cb){
      m_cb(m_curr_joystick_data);
    }
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
  js = SDL_JoystickOpen (JOYSTICK_N);
  if (js == nullptr){
    printf("Couldn't open desired Joystick: %s\n",SDL_GetError());
    SDL_Quit();
    return;
  }
  printf ("\tName:       %s\n", SDL_JoystickName(JOYSTICK_N));
  printf ("\tAxis:       %i\n", SDL_JoystickNumAxes(js));
  printf ("\tTrackballs: %i\n", SDL_JoystickNumBalls(js));
  printf ("\tButtons:   %i\n",SDL_JoystickNumButtons(js));
  printf ("\tHats: %i\n",SDL_JoystickNumHats(js));
  while (!terminate){
    std::cout<<"Read joystick\n";
    eventloop_joystick();
  }
  // either joystick disconnected or somethings wrong.
}

std::optional<std::array<uint16_t, 16>>
JoystickReader::get_new_data_if_available() {
  return std::optional<std::array<uint16_t, 16>>();
}
