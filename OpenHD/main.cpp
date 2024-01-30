//
// Created by consti10 on 02.05.22.
//

#include <OHDTelemetry.h>
#include <camera_discovery.h>
#include <getopt.h>
#include <ohd_interface.h>
#include <ohd_video_air.h>
#include <ohd_video_ground.h>

#include <csignal>
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

// |-------------------------------------------------------------------------------|
// |                         OpenHD core executable                                |
// | Weather you run as air (creates openhd air unit) or run as ground             |
// | (creates openhd ground unit) needs to be specified by either using the command|
// | line param (development) or using a text file (openhd images)                 |
// | Read the code documentation in this project for more info.                    |
// |-------------------------------------------------------------------------------|

// A few run time options, only for development. Way more configuration (during development)
// can be done by using the hardware.config file
static const char optstr[] = "?:agcwr:qh:f:";
static const struct option long_options[] = {
    {"air", no_argument, nullptr, 'a'},
    {"ground", no_argument, nullptr, 'g'},
    {"clean-start", no_argument, nullptr, 'c'},
    {"no-qt-autostart", no_argument, nullptr, 'w'},
    {"run-time-seconds", required_argument, nullptr, 'r'},
    {"continue-without-wb-card", no_argument, nullptr, 'q'},
    {"hardware-config-file", required_argument, nullptr, 'h'},
    {"rf-metrics", required_argument, nullptr, 'f'},
    {nullptr, 0, nullptr, 0},
};

struct OHDRunOptions {
  bool run_as_air=false;
  bool reset_all_settings=false;
  bool reset_frequencies=false;
  bool no_qopenhd_autostart=false;
  int run_time_seconds=-1; //-1= infinite, only usefully for debugging
  // do not wait for a card supporting injection (for development)
  bool continue_without_wb_card=false;
  // Specify the hardware.config file, otherwise,
  // the default location (and default values if no file exists at the default location) is used
  std::optional<std::string> hardware_config_file;
  int rf_metrics_level=0;
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
      case 'w':ret.no_qopenhd_autostart = true;
        break;
      case 'r':
        ret.run_time_seconds= atoi(tmp_optarg);
        break;
      case 'q':
        ret.continue_without_wb_card= true;
        break;
      case 'h':
         ret.hardware_config_file=tmp_optarg;
         break;
      case 'f':
         ret.rf_metrics_level=atoi(tmp_optarg);
         break ;
      case '?':
      default:{
          std::stringstream ss;
          ss << "Usage: \n";
          ss <<"--air -a          [Run as air, creates dummy camera if no camera is found] \n";
          ss <<"--ground -g       [Run as ground, no camera detection] \n";
          ss <<"--clean-start -c  [Wipe all persistent settings OpenHD has written, can fix any boot issues when switching hw around] \n";
          ss <<"--no-qt-autostart [disable auto start of QOpenHD on ground] \n";
          ss <<"--run-time-seconds -r [Manually specify run time (default infinite),for debugging] \n";
          ss <<"--continue-without-wb-card -q [continue the startup process even though no monitor mode card has been found yet] \n";
          ss <<"--hardware-config-file -h [specify path to hardware.config file]\n";
          ss <<"--rf-metrics -f [print a ton of rf metrics to stdout, for debugging. Try 1..3]\n";
          std::cout<<ss.str()<<std::flush;
      }
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
  OHDFilesystemUtil::write_file("/usr/local/share/openhd_platform/unconfigured","NULL");

  // Create the folder structure for the (per-module-specific) settings if needed
  openhd::generateSettingsDirectoryIfNonExists();

  // Generate the keys and delete pw if needed
  OHDInterface::generate_keys_from_pw_if_exists_and_delete();

  // Parse the program arguments, also uses the "yes if file exists" pattern for some params
  const OHDRunOptions options=parse_run_parameters(argc,argv);
  if(options.hardware_config_file.has_value()){
      openhd::set_config_file(options.hardware_config_file.value());
  }
  {  // Print all the arguments the OHD main executable is started with
      std::stringstream ss;
      ss<<"OpenHD START with \n";
      ss<<"air:"<<  OHDUtil::yes_or_no(options.run_as_air)<<"\n";
      ss<<"reset_all_settings:" << OHDUtil::yes_or_no(options.reset_all_settings) <<"\n";
      ss<<"reset_frequencies:" << OHDUtil::yes_or_no(options.reset_frequencies) <<"\n";
      ss<<"no-qopenhd-autostart:"<<OHDUtil::yes_or_no(options.no_qopenhd_autostart) <<"\n";
      ss<<"run_time_seconds:"<<options.run_time_seconds<<"\n";
      ss<<"continue_without_wb_card:"<<OHDUtil::yes_or_no(options.continue_without_wb_card)<<"\n";
      ss<<"hardware-config-file:["<<options.hardware_config_file.value_or("DEFAULT")<<"]\n";
      ss<<"Version number:"<<openhd::VERSION_NUMBER_STRING<<"\n";
      ss<<"Git info:Branch:"<<git_Branch()<<" SHA:"<<git_CommitSHA1()<<" Dirty:"<<OHDUtil::yes_or_no(git_AnyUncommittedChanges())<<"\n";
      std::cout<<ss.str()<<std::flush;
      openhd::debug_config();
      OHDInterface::print_internal_fec_optimization_method();
  }
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

    // And start the blinker (TODO LED output is really dirty right now).
    auto alive_blinker=std::make_unique<openhd::GreenLedAliveBlinker>(*platform,profile->is_air);

    // create the global action handler that allows openhd modules to communicate with each other
    // e.g. when the rf link in ohd_interface needs to talk to the camera streams to reduce the bitrate
    auto ohd_action_handler=std::make_shared<openhd::ActionHandler>();
    ohd_action_handler->rf_metrics_level=options.rf_metrics_level;

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
      OHDUtil::run_command("systemctl",{"stop","qopenhd"});

      // Now we need to discover camera(s) if we are on the air
      auto cameras = OHDVideoAir::discover_cameras(*platform);

      // Now print the actual cameras used by OHD. Of course, this prints nothing on ground (where we have no cameras connected).
      for(const auto& camera:cameras){
        m_console->info(camera.to_long_string());
      }

      ohd_video_air = std::make_unique<OHDVideoAir>(*platform,cameras,ohd_action_handler,ohdInterface->get_link_handle());
      // Let telemetry handle the settings via mavlink
      auto settings_components= ohd_video_air->get_all_camera_settings();
      for(int i=0;i<settings_components.size();i++){
        ohdTelemetry->add_settings_camera_component(i, settings_components.at(i)->get_all_settings());
      }
      ohdTelemetry->add_settings_generic(ohd_video_air->get_generic_settings());
      ohd_video_air->set_ext_devices_manager(ohdInterface->get_ext_devices_manager());
    }else{
      ohd_video_ground = std::make_unique<OHDVideoGround>(ohdInterface->get_link_handle());
      ohd_video_ground->set_ext_devices_manager(ohdInterface->get_ext_devices_manager());

      // we need to start QOpenHD when we are running as ground, or stop / disable it when we are running as air.
      // can be disabled for development purposes.
      if(!options.no_qopenhd_autostart){
          OHDUtil::run_command("systemctl",{"start","qopenhd"});
      }
    }

    // We do not add any more settings to ohd telemetry - the param set(s) are complete
    ohdTelemetry->settings_generic_ready();
    // now telemetry can send / receive data via wifibroadcast
    ohdTelemetry->set_link_handle(ohdInterface->get_link_handle());
    m_console->info("All OpenHD modules running");

    // run forever, everything has its own threads. Note that the only way to break out basically
    // is when one of the modules encounters an exception.
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
