//
// Created by consti10 on 02.05.22.
//

#include <iostream>
#include <memory>
#include <sstream>

#include "ohd_common/openhd-platform.hpp"
#include "ohd_common/openhd-profile.hpp"
#include "ohd_common/openhd-platform-discover.hpp"
#include "ohd_common/openhd-global-constants.h"

#include <DCameras.h>
#include <OHDInterface.h>
#include <OHDVideo.h>
#include <OHDTelemetry.hpp>

//TODO fix the cmake crap and then we can build a single executable.
static const char optstr[] = "?:da";
static const struct option long_options[] = {
    {"force-air", no_argument, nullptr, 'a'},
    {"force-ground", no_argument, nullptr, 'g'},
    {"clean-start", no_argument, nullptr, 'c'},
	{"debug-interface", no_argument, nullptr, 'x'}, // just use the long options
	{"debug-telemetry", no_argument, nullptr, 'y'},
	{"debug-video", no_argument, nullptr, 'z'},
	{"no-qt-autostart", no_argument, nullptr, 'w'},
    {nullptr, 0, nullptr, 0},
};

struct OHDRunOptions {
  bool force_air = false;
  bool force_ground=false;
  bool clean_start=false;
  bool enable_interface_debugging=false;
  bool enable_telemetry_debugging=false;
  bool enable_video_debugging=false;
  bool no_qt_autostart=false;
};

static OHDRunOptions parse_run_parameters(int argc, char *argv[]){
  OHDRunOptions ret{};
  int c;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 'a':ret.force_air = true;
        break;
      case 'g':ret.force_ground = true;
        break;
      case 'c':ret.clean_start = true;
        break;
	  case 'x':ret.enable_interface_debugging = true;
		break;
	  case 'y':ret.enable_telemetry_debugging = true;
		break;
	  case 'z':ret.enable_video_debugging = true;
		break;
	  case 'w':ret.no_qt_autostart = true;
		break;
      case '?':
      default:
        std::cout << "Usage: \n" <<
            "force-air [Force to boot as air pi, even when no camera is detected] \n" <<
            "force-ground [Force to boot as ground pi,even though one or more cameras are connected] \n"<<
            "clean-start [Wipe all persistent settings OpenHD has written, can fix any boot issues when switching hw around] \n"<<
			"debug-interface [enable interface debugging] \n"<<
			"debug-telemetry [enable telemetry debugging] \n"<<
			"debug-video     [enable video debugging] \n"<<
			"no-qt-autostart [disable auto start of QOpenHD on ground] \n";
        exit(1);
    }
  }
  if(OHDFilesystemUtil::exists("/boot/air.txt") || OHDFilesystemUtil::exists("/boot/Air.txt")){
    ret.force_air=true;
  }
  if(OHDFilesystemUtil::exists("/boot/ground.txt") || OHDFilesystemUtil::exists("/boot/Ground.txt")){
    ret.force_ground=true;
  }
  if(OHDFilesystemUtil::exists("/boot/ohd_clean.txt")){
    ret.clean_start=true;
  }
  if(ret.force_air && ret.force_ground){
    std::cerr << "Cannot force air and ground at the same time\n";
    exit(1);
  }
  return ret;
}

int main(int argc, char *argv[]) {
  // parse some arguments usefully for debugging
  const OHDRunOptions options=parse_run_parameters(argc,argv);

  std::cout << "OpenHD START with " <<"\n"<<
      "force-air:" << OHDUtil::yes_or_no(options.force_air) <<"\n"<<
      "force-ground:" << OHDUtil::yes_or_no(options.force_ground) <<"\n"<<
      "clean-start:" << OHDUtil::yes_or_no(options.clean_start) <<"\n"<<
	  "debug-interface:"<<OHDUtil::yes_or_no(options.enable_interface_debugging) <<"\n"<<
	  "debug-telemetry:"<<OHDUtil::yes_or_no(options.enable_telemetry_debugging) <<"\n"<<
	  "debug-video:"<<OHDUtil::yes_or_no(options.enable_video_debugging) <<"\n"<<
      "no-qt-autostart:"<<OHDUtil::yes_or_no(options.no_qt_autostart) <<"\n";
  std::cout<<"Version number:"<<OHD_VERSION_NUMBER_STRING<<"\n";

  try {
    if(options.clean_start){
      clean_all_settings();
    }
	generateSettingsDirectoryIfNonExists();

    // First discover the platform:
    const auto platform = DPlatform::discover();
    std::cout<<platform->to_string()<<"\n";

    // These are temporary and depend on how the image builder does stuff
    if(platform->platform_type==PlatformType::RaspberryPi){
      if(OHDFilesystemUtil::exists("/boot/ohd_dhclient.txt")){
        // this way pi connects to ethernet hub / router via ethernet.
        OHDUtil::run_command("dhclient eth0",{});
      }
    }else if(platform->platform_type==PlatformType::PC){
      //OHDUtil::run_command("rfkill unblock all",{});
    }

    // Now we need to discover detected cameras, to determine the n of cameras and then
    // decide if we are air or ground unit
    std::vector<Camera> cameras{};
    // To force ground, we just skip the discovery step (0 cameras means ground automatically)
    if (!options.force_ground){
      cameras = DCameras::discover(*platform);
    }
    // and by just adding a dummy camera we automatically become air
    /*if(options.force_air && cameras.empty()) {
      cameras.emplace_back(createDummyCamera());
    }*/
	// TODO what should be the default behaviour on force-air - this way, we always have a sw test camera,
	// which would then still work even if the system detects a camera but for some reason cannot stream from this camera
	if(options.force_air){
	  cameras.resize(0);
	  cameras.emplace_back(createDummyCamera());
	}
    for(const auto& camera:cameras){
      std::cout<<camera.to_string()<<"\n";
    }
    // Now e can crate the immutable profile
    const auto profile=DProfile::discover(static_cast<int>(cameras.size()));

    // Then start ohdInterface, which discovers detected wifi cards and more.
    auto ohdInterface = std::make_shared<OHDInterface>(*platform,*profile);

    // then we can start telemetry, which uses OHDInterface for wfb tx/rx (udp)
    auto ohdTelemetry = std::make_shared<OHDTelemetry>(*platform,* profile);
	// link stats from ohdInterface with telemetry
	ohdInterface->set_stats_callback([&ohdTelemetry](openhd::link_statistics::AllStats stats){
	  ohdTelemetry->set_link_statistics(stats);
	});
	// link interface settings to ohd telemetry
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
    std::unique_ptr<OHDVideo> ohdVideo;
    if (profile->is_air) {
      ohdVideo = std::make_unique<OHDVideo>(*platform,cameras);
      auto settings_components=ohdVideo->get_setting_components();
      if(!settings_components.empty()){
        ohdTelemetry->add_camera_component(0,settings_components.at(0)->get_all_settings());
      }
    }
    // we need to start QOpenHD when we are running as ground
	if(!options.no_qt_autostart){
	  if(!profile->is_air){
		OHDUtil::run_command("systemctl",{" start qopenhd"});
	  }else{
		OHDUtil::run_command("systemctl",{" stop qopenhd"});
	  }
	}
    std::cout << "All OpenHD modules running\n";

    // run forever, everything has its own threads. Note that the only way to break out basically
    // is when one of the modules encounters an exception.
	const bool any_debug_enabled=(options.enable_interface_debugging || options.enable_telemetry_debugging || options.enable_video_debugging);
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
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
		ss<<"-------------OpenHD-state debug end-------------\n";
		std::cout<<ss.str();
	  }
    }
  } catch (std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << "Unknown exception occurred" << std::endl;
    exit(1);
  }
}
