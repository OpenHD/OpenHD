//
// Created by consti10 on 02.05.22.
//

#include <OHDTelemetry.h>
#include <camera_discovery.h>
#include <ohd_interface.h>
#include <ohd_video_air.h>
#include <ohd_video_ground.h>

#include <iostream>
#include <memory>

#include "openhd_global_constants.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_rpi_gpio.hpp"
#include "openhd_spdlog.h"
#include "openhd_temporary_air_or_ground.h"
// For logging the commit hash and more
#include "git.h"

///Regarding AIR / GROUND detection: Previous OpenHD releases would detect weather this system is an air pi
// or ground pi by checking weather it has a connected camera. However, this pattern has 2 problems:
// 1) Some camera(s) require a reconfiguration (e.g. when switching from / to libcamera to gst-rpicamsrc(mmal) or
//    even more complex, when switching from/to the arducam-specific libcamera) and are therefore not detectable
//    at startup
// 2) It makes troubleshooting harder - since in case of the ochin, you cannot even connect a display to the unit
//    and check if it boots as ground or air, or if there are issue(s) with something else (e.g. the wifi card).
///This is why we decided to change this pattern as follows:
// One needs to explicitly tell the OpenHD executable weather to start as air or ground. This can be done in multiple ways
// a) during development, just pass in the required parameter (this overrides any check if a file exists,see below)
// b) For normal users, they can select weather they want to create an air or ground unit in the flashing tool -
//    it'l create a file called air.txt or ground.txt under /boot/openhd/
// c) You can also rename the file under /boot/openhd/ and restart OpenHD during development
///As a result, we have the following behaviour:
// If we run as air - check if we can find a connected camera, if there is none, create the "dummy" camera instead
// If we run as ground - easy, just don't check for connected camera(s)
// There is one more thing that is usefully during development: It is possible that a camera is detectable but the pipeline
// is bugged - for this, you can use the --force-dummy-camera parameter, which also sets OpenHD into air mode and always
// uses the dummy camera, no matter if a camera is detected or not.
// NOTE: If you neither pass in the argument in the command line and no file exists, OpenHD will always boot as ground.

static const char optstr[] = "?:agfbdcxyzwr:q";
static const struct option long_options[] = {
    {"air", no_argument, nullptr, 'a'},
    {"ground", no_argument, nullptr, 'g'},
	//{"config-file",required_argument, nullptr,'f'},
    {"clean-start", no_argument, nullptr, 'c'},
    {"debug-interface", no_argument, nullptr, 'x'}, // just use the long options
    {"debug-telemetry", no_argument, nullptr, 'y'},
    {"debug-video", no_argument, nullptr, 'z'},
    {"no-qt-autostart", no_argument, nullptr, 'w'},
    {"run-time-seconds", required_argument, nullptr, 'r'},
    {"continue-without-wb-card", no_argument, nullptr, 'q'},
    {nullptr, 0, nullptr, 0},
};

struct OHDRunOptions {
  bool run_as_air=false;
  bool reset_all_settings=false;
  bool reset_frequencies=false;
  bool enable_interface_debugging=false;
  bool enable_telemetry_debugging=false;
  bool enable_video_debugging=false;
  bool no_qt_autostart=false;
  int run_time_seconds=-1; //-1= infinite, only usefully for debugging
  // do not wait for a card supporting injection (for development)
  bool continue_without_wb_card=false;
};

static OHDRunOptions parse_run_parameters(int argc, char *argv[]){
  OHDRunOptions ret{};
  int c;
  // If this value gets set, we assume a developer is working on OpenHD and skip the discovery via file(s).
  std::optional<bool> commandline_air=std::nullopt;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 'a':
        if(commandline_air!=std::nullopt){
          // Already set, e.g. --ground is already used
          std::cerr<<"Please use either air or ground as param\n";
          exit(1);
        }
        commandline_air = true;
        break;
      case 'g':
        // Already set, e.g. --air is already used
        if(commandline_air!=std::nullopt){
          std::cerr<<"Please use either air or ground as param\n";
          exit(1);
        }
        commandline_air= false;
        break;
      case 'c':ret.reset_all_settings = true;
        break;
      case 'x':ret.enable_interface_debugging = true;
        break;
      case 'y':ret.enable_telemetry_debugging = true;
        break;
      case 'z':ret.enable_video_debugging = true;
        break;
      case 'w':ret.no_qt_autostart = true;
        break;
      case 'r':
        ret.run_time_seconds= atoi(tmp_optarg);
        break;
      case 'q':
        ret.continue_without_wb_card= true;
        break;
      case '?':
      default:
        std::cout << "Usage: \n" <<
            "--air -a          [Run as air, creates dummy camera if no camera is found] \n" <<
            "--ground -g       [Run as ground, no camera detection] \n"<<
            "--clean-start -c  [Wipe all persistent settings OpenHD has written, can fix any boot issues when switching hw around] \n"<<
            "--debug-interface [enable interface debugging] \n"<<
            "--debug-telemetry [enable telemetry debugging] \n"<<
            "--debug-video     [enable video debugging] \n"<<
            "--no-qt-autostart [disable auto start of QOpenHD on ground] \n"<<
            "--run-time-seconds -r [Manually specify run time (default infinite),for debugging] \n"<<
            "--continue-without-wb-card -q [continue the startup process even though no monitor mode card has been found yet] \n";
        exit(1);
    }
  }
  if(commandline_air==std::nullopt){
    // command line parameters not used, use the file(s) for detection (default for normal OpenHD images)
    // The logs/checks here are just to help developer(s) avoid common misconfigurations
    std::cout<<"Using files to detect air or ground\n";
    const bool file_run_as_ground_exists= openhd::tmp::file_ground_exists();
    const bool file_run_as_air_exists = openhd::tmp::file_air_exists();
    bool error=false;
    if(file_run_as_air_exists && file_run_as_ground_exists){ // both files exist
      std::cerr<<"Both air and ground files exist,unknown what you want - either use the command line param or delete one of them\n";
      std::cerr<<"Assuming ground\n";
      // Just run as ground
      ret.run_as_air= false;
      error= true;
    }
    if(!file_run_as_air_exists && !file_run_as_ground_exists){ // no file exists
      std::cerr<<"No file air or ground exists,unknown what you want - either use the command line param or create a file\n";
      std::cerr<<"Assuming ground\n";
      // Just run as ground
      ret.run_as_air= false;
      error= true;
    }
    if(!error){
      if (!file_run_as_air_exists) {
        ret.run_as_air = false;
      } else {
        ret.run_as_air = true;
      }
    }
  }else{
    // command line parameters used, just validate they are not mis-configured
    assert(commandline_air.has_value());
    ret.run_as_air=commandline_air.value();
  }
  // If this file exists, delete all openhd settings resulting in default value(s)
  static constexpr auto FILE_PATH_RESET="/boot/openhd/reset.txt";
  if(OHDUtil::file_exists_and_delete(FILE_PATH_RESET)){
    ret.reset_all_settings= true;
  }
  // If this file exists, delete all openhd wb link / frequency values, which results in default frequencies
  // and fixes issue(s) when user swap hardware around with the wrong frequencies.
  static constexpr auto FILE_PATH_RESET_FREQUENCY="/boot/openhd/reset_freq.txt";
  if(OHDUtil::file_exists_and_delete(FILE_PATH_RESET_FREQUENCY)){
    ret.reset_frequencies=true;
  }
  return ret;
}

int main(int argc, char *argv[]) {
  // OpenHD needs to be run as root, otherwise we cannot access/ modify the Wi-Fi cards for example
  // (And there are also many other places where we just need to be root).
  OHDUtil::terminate_if_not_root();

  // Create the folder structure for the (per-module-specific) settings if needed
  openhd::generateSettingsDirectoryIfNonExists();

  // Parse the program arguments, also uses the "yes if file exists" pattern for some params
  const OHDRunOptions options=parse_run_parameters(argc,argv);

  // Print all the arguments the OHD main executable is started with
  std::cout << "OpenHD START with " <<"\n"<<
      "air:"<<  OHDUtil::yes_or_no(options.run_as_air)<<"\n"<<
      "reset_all_settings:" << OHDUtil::yes_or_no(options.reset_all_settings) <<"\n"<<
      "reset_frequencies:" << OHDUtil::yes_or_no(options.reset_frequencies) <<"\n"<<
      "debug-interface:"<<OHDUtil::yes_or_no(options.enable_interface_debugging) <<"\n"<<
      "debug-telemetry:"<<OHDUtil::yes_or_no(options.enable_telemetry_debugging) <<"\n"<<
      "debug-video:"<<OHDUtil::yes_or_no(options.enable_video_debugging) <<"\n"<<
      "no-qt-autostart:"<<OHDUtil::yes_or_no(options.no_qt_autostart) <<"\n"<<
      "run_time_seconds:"<<options.run_time_seconds<<"\n"<<
      "continue_without_wb_card:"<<OHDUtil::yes_or_no(options.continue_without_wb_card)<<"\n";
  std::cout<<"Version number:"<<openhd::VERSION_NUMBER_STRING<<"\n";
  std::cout<<"Git info:Branch:"<<git_Branch()<<" SHA:"<<git_CommitSHA1()<<"Dirty:"<<OHDUtil::yes_or_no(git_AnyUncommittedChanges())<<"\n";
  openhd::log::get_default()->debug("Heeeee");
  openhd::debug_config();
  openhd::log::get_default()->debug("Heeeee2");
  OHDInterface::print_internal_fec_optimization_method();

  // This is the console we use inside main, in general different openhd modules/classes have their own loggers
  // with different tags
  std::shared_ptr<spdlog::logger> m_console=openhd::log::create_or_get("main");
  assert(m_console);

  // not guaranteed, but better than nothing, check if openhd is already running (kinda) and print warning if yes.
  openhd::check_currently_running_file_and_write();

  // First discover the platform:
  const auto platform = DPlatform::discover();
  m_console->info("Detected Platform:{}",platform->to_string());

  // Create and link all the OpenHD modules.
  try {
    // This results in fresh default values for all modules (e.g. interface, telemetry, video)
    if(options.reset_all_settings){
      openhd::clean_all_settings();
    }
    // or only the wb_link module
    if(options.reset_frequencies){
      openhd::clean_all_interface_settings();
    }
    // on rpi, we have the gpio input such that users don't have to create the reset frequencies file
    if(platform->platform_type==PlatformType::RaspberryPi){
      // Or input via rpi gpio 26
      openhd::rpi::gpio26_configure();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if(openhd::rpi::gpio26_user_wants_reset_frequencies()){
        openhd::clean_all_interface_settings();
      }
    }

    // Profile no longer depends on n discovered cameras,
    // But if we are air, we have at least one camera, sw if no camera was found
    const auto profile=DProfile::discover(options.run_as_air);
    write_profile_manifest(*profile);

    // we need to start QOpenHD when we are running as ground, or stop / disable it when we are running as air.
    // can be disabled for development purposes.
    if(!options.no_qt_autostart){
      if(!profile->is_air){
        OHDUtil::run_command("systemctl",{"start","qopenhd"});
      }else{
        OHDUtil::run_command("systemctl",{"stop","qopenhd"});
      }
    }
    // Now we need to discover camera(s) if we are on the air
    std::vector<Camera> cameras{};
    if(profile->is_air){
      cameras = OHDVideoAir::discover_cameras(*platform);
    }
    // Now print the actual cameras used by OHD. Of course, this prints nothing on ground (where we have no cameras connected).
    for(const auto& camera:cameras){
      m_console->info(camera.to_long_string());
    }
    // And start the blinker (TODO LED output is really dirty right now).
    auto alive_blinker=std::make_unique<openhd::GreenLedAliveBlinker>(*platform,profile->is_air);

    // create the global action handler that allows openhd modules to communicate with each other
    // e.g. when the rf link in ohd_interface needs to talk to the camera streams to reduce the bitrate
    auto ohd_action_handler=std::make_shared<openhd::ActionHandler>();

    // We start ohd_telemetry as early as possible, since even without a link (transmission) it still picks up local
    // log message(s) and forwards them to any ground station clients (e.g. QOpenHD)
    auto ohdTelemetry = std::make_shared<OHDTelemetry>(*platform,* profile,ohd_action_handler);

    // Then start ohdInterface, which discovers detected wifi cards and more.
    auto ohdInterface = std::make_shared<OHDInterface>(*platform,*profile,ohd_action_handler,options.continue_without_wb_card);

    // Telemetry allows changing all settings (even from other modules)
    ohdTelemetry->add_settings_generic(ohdInterface->get_all_settings());

    // Since telemetry handles the data stream(s) to external devices itself, we need to also react to
    // changes to the external device(s) from ohd_interface
    ohdTelemetry->set_ext_devices_manager(ohdInterface->get_ext_devices_manager());

    // either one is active, depending on air or ground
    std::unique_ptr<OHDVideoAir> ohd_video_air = nullptr;
    std::unique_ptr<OHDVideoGround> ohd_video_ground = nullptr;
    if (profile->is_air) {
      ohd_video_air = std::make_unique<OHDVideoAir>(*platform,cameras,ohd_action_handler,ohdInterface->get_link_handle());
      // Let telemetry handle the settings via mavlink
      auto settings_components= ohd_video_air->get_all_camera_settings();
      for(int i=0;i<settings_components.size();i++){
        ohdTelemetry->add_settings_camera_component(i, settings_components.at(i)->get_all_settings());
      }
      ohdTelemetry->add_settings_generic(ohd_video_air->get_generic_settings());
    }else{
      ohd_video_ground = std::make_unique<OHDVideoGround>(ohdInterface->get_link_handle());
      ohd_video_ground->set_ext_devices_manager(ohdInterface->get_ext_devices_manager());
    }
    // We do not add any more settings to ohd telemetry - the param set(s) are complete
    ohdTelemetry->settings_generic_ready();
    // now telemetry can send / receive data via wifibroadcast
    ohdTelemetry->set_link_handle(ohdInterface->get_link_handle());
    m_console->info("All OpenHD modules running");

    // run forever, everything has its own threads. Note that the only way to break out basically
    // is when one of the modules encounters an exception.
    const bool any_debug_enabled=(options.enable_interface_debugging || options.enable_telemetry_debugging || options.enable_video_debugging);
    static bool quit=false;
    // https://unix.stackexchange.com/questions/362559/list-of-terminal-generated-signals-eg-ctrl-c-sigint
    signal(SIGTERM, [](int sig){
      std::cerr<<"Got SIGTERM, exiting\n";
      quit= true;
    });
    signal(SIGQUIT,[](int sig){
      std::cerr<<"Got SIGQUIT, exiting\n";
      quit= true;
    });
    const auto run_time_begin=std::chrono::steady_clock::now();
    while (!quit) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      if(options.run_time_seconds>=1){
        if(std::chrono::steady_clock::now()-run_time_begin>=std::chrono::seconds(options.run_time_seconds)){
          m_console->warn("Terminating, exceeded run time {}",options.run_time_seconds);
          // we can just break out any time, usefully for checking memory leaks and more.
          break;
        }
      }
      if(ohd_video_air){
        ohd_video_air->restartIfStopped();
      }
      // To make sure this is all tightly packed together, we write it to a stringstream first
      // and then to stdout in one big chunk. Otherwise, some other debug output might stand in between the OpenHD
      // state debug chunk.
      if(any_debug_enabled){
        std::stringstream ss;
        ss<< "-----------OpenHD-state debug begin-----------\n";
        if(options.enable_interface_debugging){
          ss<<ohdInterface->createDebug();
        }
        if(options.enable_video_debugging && ohd_video_air){
          ss<< ohd_video_air->createDebug();
        }
        if(options.enable_telemetry_debugging){
          ss << ohdTelemetry->createDebug();
        }
        ss<<"-------------OpenHD-state debug end-------------";
        m_console->debug(ss.str());
      }
    }
    // --- terminate openhd, most likely requested by a developer with sigterm
    m_console->debug("Terminating openhd");
    // Stop any communication between modules, to eliminate any issues created by threads during cleanup
    ohd_action_handler->disable_all_callables();
    // dirty, wait a bit to make sure none of those action(s) are called anymore
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // unique ptr would clean up for us, but this way we are a bit more verbose
    // since some of those modules talk to each other, this is a bit prone to failures.
    if(ohd_video_air){
      m_console->debug("Terminating ohd_video_air - begin");
      ohd_video_air.reset();
      m_console->debug("Terminating ohd_video_air - end");
    }
    if(ohd_video_ground){
      m_console->debug("Terminating ohd_video_ground- begin");
      ohd_video_ground.reset();
      m_console->debug("Terminating ohd_video_ground - end");
    }
    if(ohdTelemetry){
      m_console->debug("Terminating ohd_telemetry - begin");
      ohdTelemetry.reset();
      m_console->debug("Terminating ohd_telemetry - end");
    }
    if(ohdInterface){
      m_console->debug("Terminating ohd_interface - begin");
      ohdInterface.reset();
      m_console->debug("Terminating ohd_interface - end");
    }
  } catch (std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << "Unknown exception occurred" << std::endl;
    exit(1);
  }
  openhd::remove_currently_running_file();
  return 0;
}
