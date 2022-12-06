//
// Created by consti10 on 02.05.22.
//

#include <OHDTelemetry.h>
#include <camera_discovery.h>
#include <ohd_interface.h>
#include <ohd_video.h>

#include <iostream>
#include <memory>
#include <sstream>

#include "ohd_common/openhd-global-constants.hpp"
#include "ohd_common/openhd-platform-discover.hpp"
#include "ohd_common/openhd-platform.hpp"
#include "ohd_common/openhd-profile-json.hpp"
#include "ohd_common/openhd-profile.hpp"
#include "ohd_common/openhd-spdlog.hpp"
#include "ohd_common/openhd-temporary-air-or-ground.h"
#include "ohd_common/openhd-rpi-gpio.h"
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

static const char optstr[] = "?:agfcr:b";
static const struct option long_options[] = {
    {"air", no_argument, nullptr, 'a'},
    {"ground", no_argument, nullptr, 'g'},
    {"force-dummy-camera", no_argument, nullptr, 'f'},
    {"force-custom-unmanaged-camera", no_argument, nullptr, 'b'},
    {"force-ip-camera", no_argument, nullptr, 'd'},
    {"clean-start", no_argument, nullptr, 'c'},
    {"debug-interface", no_argument, nullptr, 'x'}, // just use the long options
    {"debug-telemetry", no_argument, nullptr, 'y'},
    {"debug-video", no_argument, nullptr, 'z'},
    {"no-qt-autostart", no_argument, nullptr, 'w'},
    {"run-time_seconds", required_argument, nullptr, 'r'},
    {nullptr, 0, nullptr, 0},
};

struct OHDRunOptions {
  bool run_as_air=false;
  bool force_dummy_camera=false;
  bool force_custom_unmanaged_camera=false;
  bool force_ip_camera=false;
  bool reset_all_settings=false;
  bool reset_frequencies=false;
  bool enable_interface_debugging=false;
  bool enable_telemetry_debugging=false;
  bool enable_video_debugging=false;
  bool no_qt_autostart=false;
  int run_time_seconds=-1; //-1= infinite, only usefully for debugging
  // used to detect if we are launched by a developer working on openhd or
  // "normally" started as a service. TODO generalize / find a better approach here.
  bool developer_mode=false;
};

static OHDRunOptions parse_run_parameters(int argc, char *argv[]){
  OHDRunOptions ret{};
  int c;
  // If this value gets set, we assume a developer is working on OpenHD and skip the discovery via file(s).
  std::optional<bool> commandline_air=std::nullopt;
  bool commandline_force_dummy_camera=false;
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
        ret.developer_mode=true;
        break;
      case 'g':
        // Already set, e.g. --air is already used
        if(commandline_air!=std::nullopt){
          std::cerr<<"Please use either air or ground as param\n";
          exit(1);
        }
        commandline_air= false;
        ret.developer_mode=true;
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
      case 'f':commandline_force_dummy_camera= true;
        break;
      case 'r':
        ret.run_time_seconds= atoi(tmp_optarg);
        break;
      case 'b':
        ret.force_custom_unmanaged_camera=true;
        break;
      case 'd':
        ret.force_ip_camera= true;
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
            "--force-dummy-camera -f [Run as air, always use dummy camera (even if real cam is found)] \n"<<
            "--force-custom-unmanaged-camera [only on air,custom unmanaged camera in openhd,cannot be autodetected] \n"<<
            "--force-ip-camera [only on air, ip camera, cannot be autodetected] \n"<<
            "--run-time_seconds -r [Manually specify run time (default infinite),for debugging] \n";
        exit(1);
    }
  }
  if(commandline_air==std::nullopt && !commandline_force_dummy_camera){
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
    if(commandline_force_dummy_camera){
      commandline_air=true;
    }
    assert(commandline_air.has_value());
    ret.run_as_air=commandline_air.value();
    ret.force_dummy_camera=commandline_force_dummy_camera;
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
  if(OHDFilesystemUtil::exists("/boot/openhd/force_custom_unmanaged_camera.txt")){
    ret.force_custom_unmanaged_camera= true;
  }
  if(OHDFilesystemUtil::exists("/boot/openhd/force_ip_camera.txt")){
    ret.force_ip_camera= true;
  }
  if(ret.force_custom_unmanaged_camera && ret.force_dummy_camera){
    openhd::log::get_default()->warn("Dummy camera overrides custom unmanaged camera");
    ret.force_custom_unmanaged_camera= false;
  }
  if(ret.force_ip_camera && ret.force_dummy_camera){
    openhd::log::get_default()->warn("Dummy camera overrides ip camera");
    ret.force_ip_camera= false;
  }
  if(ret.force_ip_camera && ret.force_custom_unmanaged_camera){
    openhd::log::get_default()->warn("Custom unmanaged camera overrides ip camera");
    ret.force_ip_camera= false;
  }
  return ret;
}

int main(int argc, char *argv[]) {
  // OpenHD needs to be run as root, otherwise we cannot access/ modify the Wi-Fi cards for example
  // (And there are also many other places where we just need to be root).
  OHDUtil::terminate_if_not_root();

  // parse some arguments usefully for debugging
  const OHDRunOptions options=parse_run_parameters(argc,argv);

  // Log what arguments the OHD main executable is started with.
  std::cout << "OpenHD START with " <<"\n"<<
      "air:"<<  OHDUtil::yes_or_no(options.run_as_air)<<"\n"<<
      "force_dummy_camera:"<<  OHDUtil::yes_or_no(options.force_dummy_camera)<<"\n"<<
      "force_custom_unmanaged_camera:"<<  OHDUtil::yes_or_no(options.force_custom_unmanaged_camera)<<"\n"<<
      "force_ip_camera:"<<  OHDUtil::yes_or_no(options.force_ip_camera)<<"\n"<<
      "reset_all_settings:" << OHDUtil::yes_or_no(options.reset_all_settings) <<"\n"<<
      "reset_frequencies:" << OHDUtil::yes_or_no(options.reset_frequencies) <<"\n"<<
      "debug-interface:"<<OHDUtil::yes_or_no(options.enable_interface_debugging) <<"\n"<<
      "debug-telemetry:"<<OHDUtil::yes_or_no(options.enable_telemetry_debugging) <<"\n"<<
      "debug-video:"<<OHDUtil::yes_or_no(options.enable_video_debugging) <<"\n"<<
      "no-qt-autostart:"<<OHDUtil::yes_or_no(options.no_qt_autostart) <<"\n"<<
      "run_time_seconds:"<<options.run_time_seconds<<"\n"<<
      "developer_mode:"<<OHDUtil::yes_or_no(options.developer_mode)<<"\n";
  std::cout<<"Version number:"<<OHD_VERSION_NUMBER_STRING<<"\n";
  std::cout<<"Git info:Branch:"<<git_Branch()<<" SHA:"<<git_CommitSHA1()<<"Dirty:"<<OHDUtil::yes_or_no(git_AnyUncommittedChanges())<<"\n";
  OHDInterface::print_internal_fec_optimization_method();

  std::shared_ptr<spdlog::logger> m_console=openhd::log::create_or_get("main");
  assert(m_console);

  // First discover the platform:
  const auto platform = DPlatform::discover();
  m_console->info("Detected Platform:"+platform->to_string());

  // Create and link all the OpenHD modules.
  try {
    // This results in fresh default values for all modules (e.g. interface, telemetry, video)
    if(options.reset_all_settings){
      clean_all_settings();
    }
    // or only the wb_link module
    if(options.reset_frequencies){
      clean_all_interface_settings();
    }
    // on rpi, we have the gpio input such that users don't have to create the reset frequencies file
    if(platform->platform_type==PlatformType::RaspberryPi){
      // Or input via rpi gpio 26
      openhd::rpi::gpio26_configure();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if(openhd::rpi::gpio26_user_wants_reset_frequencies()){
        clean_all_interface_settings();
      }
    }

    generateSettingsDirectoryIfNonExists();

    // Profile no longer depends on n discovered cameras,
    // But if we are air, we have at least one camera, sw if no camera was found
    const auto profile=DProfile::discover(options.run_as_air, options.developer_mode);
    write_profile_manifest(*profile);

    // we need to start QOpenHD when we are running as ground, or stop / disable it when we are running as ground.
    if(!options.no_qt_autostart){
      if(!profile->is_air){
        OHDUtil::run_command("systemctl",{" start qopenhd"});
      }else{
        OHDUtil::run_command("systemctl",{" stop qopenhd"});
      }
    }

    // Now we need to discover camera(s) if we are on the air
    std::vector<Camera> cameras{};
    if(profile->is_air){
      if(options.force_dummy_camera){
        // skip camera detection, we want the dummy camera regardless weather a camera is connected or not.
        cameras.emplace_back(createDummyCamera());
      }else if(options.force_custom_unmanaged_camera) {
        // these cameras cannot be autodetected and need to be manually forced/specified
        cameras.emplace_back(createCustomUnmanagedCamera());
      }else if(options.force_ip_camera){
        // these cameras cannot be autodetected and need to be manually forced/specified
        cameras.emplace_back(createCustomIpCamera());
      }else{
        // Issue on rpi: The openhd service is often started before ? (most likely the OS needs to do some internal setup stuff)
        // and then the cameras discovery step is run before the camera is available, and therefore not found. Block up to
        // X seconds here, to give the OS time until the camera is available, only then continue with the dummy camera
        // Since the jetson is also an embedded platform, just like the rpi, I am doing it for it too, even though I never
        // checked if that's actually an issue there
        cameras = DCameras::discover(*platform);
        if(platform->platform_type==PlatformType::RaspberryPi || platform->platform_type==PlatformType::Jetson){
          const auto begin=std::chrono::steady_clock::now();
          while (std::chrono::steady_clock::now()-begin<std::chrono::seconds(10)){
            if(!cameras.empty())break; // break as soon as we have at least one camera
            m_console->debug("Re-running camera discovery step, until camera is found/timeout");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            cameras=DCameras::discover(*platform);
          }
        }
        if(cameras.empty()){
          m_console->warn("No camera found after X seconds, using dummy camera instead");
          cameras.emplace_back(createDummyCamera());
        }
      }
    }
    // Now print the actual cameras used by OHD. Of course, this prints nothing on ground (where we have no cameras connected).
    for(const auto& camera:cameras){
      m_console->info(camera.to_string());
    }
    // And start the blinker (TODO LED output is really dirty right now).
    auto alive_blinker=std::make_unique<openhd::GreenLedAliveBlinker>(*platform,profile->is_air);

    // create the global action handler that allows openhd modules to communicate with each other
    // e.g. when the rf link in ohd_interface needs to talk to the camera streams to reduce the bitrate
    auto ohd_action_handler=std::make_shared<openhd::ActionHandler>();

    // Then start ohdInterface, which discovers detected wifi cards and more.
    auto ohdInterface = std::make_shared<OHDInterface>(*platform,*profile,ohd_action_handler);

    ohd_action_handler->action_set_video_codec_set([&ohdInterface](int codec){
      ohdInterface->set_video_codec(codec);
    });

    // then we can start telemetry, which uses OHDInterface for wfb tx/rx (udp)
    auto ohdTelemetry = std::make_shared<OHDTelemetry>(*platform,* profile,ohd_action_handler);
    // Telemetry allows changing all settings (even from other modules)
    ohdTelemetry->add_settings_generic(ohdInterface->get_all_settings());
    // Now we are done with generic settings, param set is now ready (we don't add any new params anymore)
    ohdTelemetry->settings_generic_ready();
    // Since telemetry handles the data stream(s) to external devices itself, we need to also react to
    // changes to the external device(s) from ohd_interface
    //ohdTelemetry->add_external_ground_station_ip(" 127.0.0.1","192.168.18.229");
    ohdInterface->set_external_device_callback([&ohdTelemetry](const openhd::ExternalDevice& external_device,bool connected){
      if(connected){
        ohdTelemetry->add_external_ground_station_ip(external_device.local_network_ip,external_device.external_device_ip);
      }else{
        ohdTelemetry->remove_external_ground_station_ip(external_device.local_network_ip,external_device.external_device_ip);
      }
    });

    // and start ohdVideo if we are on the air pi
    std::unique_ptr<OHDVideo> ohdVideo= nullptr;
    if (profile->is_air) {
      ohdVideo = std::make_unique<OHDVideo>(*platform,cameras,ohd_action_handler,ohdInterface->get_video_tx_interface());
      auto settings_components=ohdVideo->get_setting_components();
      if(!settings_components.empty()){
        ohdTelemetry->add_settings_camera_component(
            0, settings_components.at(0)->get_all_settings());
      }
    }
    m_console->info("All OpenHD modules running");

    // run forever, everything has its own threads. Note that the only way to break out basically
    // is when one of the modules encounters an exception.
    const bool any_debug_enabled=(options.enable_interface_debugging || options.enable_telemetry_debugging || options.enable_video_debugging);
    static bool quit=false;
    signal(SIGTERM, [](int sig){
      std::cerr<<"Got SIGTERM, exiting\n";
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
      if(ohdVideo){
        ohdVideo->restartIfStopped();
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
        if(options.enable_video_debugging && ohdVideo){
          ss<<ohdVideo->createDebug();
        }
        if(options.enable_telemetry_debugging){
          ss << ohdTelemetry->createDebug();
        }
        ss<<"-------------OpenHD-state debug end-------------";
        m_console->debug(ss.str());
      }
    }
    // Stop any communication between modules, to eliminate any issues created by threads during cleanup
    ohd_action_handler->disable_all_callables();
  } catch (std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << "Unknown exception occurred" << std::endl;
    exit(1);
  }
  return 0;
}
