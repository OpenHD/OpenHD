/******************************************************************************
 * OpenHD
 * 
 * Licensed under the GNU General Public License (GPL) Version 3.
 * 
 * This software is provided "as-is," without warranty of any kind, express or 
 * implied, including but not limited to the warranties of merchantability, 
 * fitness for a particular purpose, and non-infringement. For details, see the 
 * full license in the LICENSE file provided with this source code.
 * 
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for 
 * civilian and non-military purposes. Use in any military or defense 
 * applications is strictly prohibited unless explicitly and individually 
 * licensed otherwise by the OpenHD Team.
 * 
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 * 
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/


#include <OHDTelemetry.h>
#include <getopt.h>
#include <ohd_interface.h>
#ifdef ENABLE_AIR
#include <ohd_video_air.h>
#endif  // ENABLE_AIR
#include <ohd_video_ground.h>

#include <csignal>
#include <iostream>
#include <memory>
#include <cstdlib>

#include "openhd_buttons.h"
#include "openhd_global_constants.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_spdlog.h"
#include "openhd_temporary_air_or_ground.h"
#include "openhd_config.h"
#include "config_paths.h"

// |-------------------------------------------------------------------------------|
// |                         OpenHD core executable | | Weather you run as air
// (creates openhd air unit) or run as ground             | | (creates openhd
// ground unit) needs to be specified by either using the command| | line param
// (development) or using a text file (openhd images)                 | | Read
// the code documentation in this project for more info.                    |
// |-------------------------------------------------------------------------------|

// A few run time options, only for development. Way more configuration (during
// development) can be done by using the hardware.config file
static const char optstr[] = "?:agcwr:h:";
static const struct option long_options[] = {
    {"air", no_argument, nullptr, 'a'},
    {"ground", no_argument, nullptr, 'g'},
    {"clean-start", no_argument, nullptr, 'c'},
    {"no-qt-autostart", no_argument, nullptr, 'w'},
    {"run-time-seconds", required_argument, nullptr, 'r'},
    {"hardware-config-file", required_argument, nullptr, 'h'},
    {nullptr, 0, nullptr, 0},
};
    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string blue = "\033[94m";
    const std::string reset = "\033[0m";

struct OHDRunOptions {
  bool run_as_air = false;
  bool reset_all_settings = false;
  bool no_qopenhd_autostart=false;
  int run_time_seconds = -1;  //-1= infinite, only usefully for debugging
  // Specify the hardware.config file, otherwise,
  // the default location (and default values if no file exists at the default
  // location) is used
  std::optional<std::string> hardware_config_file;
};

static OHDRunOptions parse_run_parameters(int argc, char *argv[]) {
  OHDRunOptions ret{};
  int c;
  // If this value gets set, we assume a developer is working on OpenHD and skip
  // the discovery via file(s).
  std::optional<bool> commandline_air = std::nullopt;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 'a':
        if (commandline_air != std::nullopt) {
          // Already set, e.g. --ground is already used
          std::cerr << "Please use either air or ground as param\n";
          exit(1);
        }
        commandline_air = true;
        break;
      case 'g':
        // Already set, e.g. --air is already used
        if (commandline_air != std::nullopt) {
          std::cerr << "Please use either air or ground as param\n";
          exit(1);
        }
        commandline_air = false;
        break;
      case 'c':
        ret.reset_all_settings = true;
        break;
      case 'w':
        ret.no_qopenhd_autostart = true;
        break;
      case 'r':
        ret.run_time_seconds = atoi(tmp_optarg);
        break;
      case 'h':
        ret.hardware_config_file = tmp_optarg;
        break;
      case '?':
      default: {
        std::stringstream ss;
        ss << "Usage: \n";
        ss << "--air -a          [Run as air, creates dummy camera if no "
              "camera is found] \n";
        ss << "--ground -g       [Run as ground, no camera detection] \n";
        ss << "--clean-start -c  [Wipe all persistent settings OpenHD has "
              "written, can fix any boot issues when switching hw around] \n";
        ss << "--no-qt-autostart [disable auto start of QOpenHD on ground] \n";
        ss << "--run-time-seconds -r [Manually specify run time (default "
              "infinite),for debugging] \n";
        ss << "--hardware-config-file -h [specify path to hardware.config "
              "file]\n";
        ss << "Use hardware.conf for more configuration\n";
        std::cout << ss.str() << std::flush;
      }
        exit(1);
    }
  }
  if (commandline_air == std::nullopt) {
    // command line parameters not used, use the file(s) for detection (default
    // for normal OpenHD images) The logs/checks here are just to help
    // developer(s) avoid common misconfigurations
    const bool file_run_as_ground_exists = openhd::tmp::file_ground_exists();
    const bool file_run_as_air_exists = openhd::tmp::file_air_exists();
    bool error = false;
    if (file_run_as_air_exists &&
        file_run_as_ground_exists) {  // both files exist
      // Just run as ground
      ret.run_as_air = false;
      error = true;
    }
    if (!file_run_as_air_exists &&
        !file_run_as_ground_exists) {  // no file exists
      // Just run as ground
      ret.run_as_air = false;
      error = true;
    }
    if (!error) {
      if (!file_run_as_air_exists) {
        ret.run_as_air = false;
      } else {
        ret.run_as_air = true;
      }
    }
  } else {
    // command line parameters used, just validate they are not mis-configured
    assert(commandline_air.has_value());
    ret.run_as_air = commandline_air.value();
  }
  // If this file exists, delete all openhd settings resulting in default
  // value(s)
    const auto filePathReset = std::string(getConfigBasePath())+ "reset.txt";
  if (OHDUtil::file_exists_and_delete(filePathReset.c_str())) {
    ret.reset_all_settings = true;
  }
#ifndef ENABLE_AIR
  if (ret.run_as_air) {
    std::cerr << "NOTE: COMPILED WITH GROUND ONLY SUPPORT,RUNNING AS GND"
              << std::endl;
    ret.run_as_air = false;
  }
#endif
  return ret;
}

int main(int argc, char *argv[]) {
  // OpenHD needs to be run as root!
  OHDUtil::terminate_if_not_root();
  if (OHDFilesystemUtil::exists("/run/openhd/hold.pid")) {
      std::exit(0);
  }
  const OHDRunOptions options = parse_run_parameters(argc, argv);
  if (options.hardware_config_file.has_value()) {
    openhd::set_config_file(options.hardware_config_file.value());
  }
  {  // Print all the arguments the OHD main executable is started with
 bool validLicense=false;
 if (OHDFilesystemUtil::exists("/usr/local/share/openhd/license")) {
  validLicense=true;
 }
 std::cout << "\033[2J\033[1;1H"; //clear terminal
 std::stringstream ss;
    ss << openhd::get_ohd_version_as_string() << "\n";
    ss << "\n";
    ss << blue;
    ss << "  #######  ########  ######## ##    ## ##     ## ######## \n";
    ss << " ##     ## ##     ## ##       ###   ## ##     ## ##     ##\n";
    ss << " ##     ## ##     ## ##       ####  ## ##     ## ##     ##\n";
    ss << " ##     ## ########  ######   ## ## ## ######### ##     ##\n";
    ss << " ##     ## ##        ##       ##  #### ##     ## ##     ##\n";
    ss << " ##     ## ##        ##       ##   ### ##     ## ##     ##\n";
    ss << "  #######  ##        ######## ##    ## ##     ## ######## \n";
    ss << reset;
    if (!validLicense) {
      ss << "----------------------- " << blue << "OpenSource" << reset << " -----------------------\n";
    } else {
      ss << "----------------------- " << green << "Enterprise" << reset << " -----------------------\n";
    }
    ss << "\n";

    if (options.run_as_air) {
        ss << "----------------------- " << green << "Air Unit" << reset << " -----------------------\n";
    } else {
        ss << "----------------------- " << red << "Ground Unit" << reset << " ----------------------\n";
    }

    if (options.reset_all_settings) {
        ss << red << "Reset Settings" << reset << "\n";
    }
    ss << "\n";

    // ss << "Git info:Branch:" << git_Branch() << " SHA:" << git_CommitSHA1() << " Dirty:" << OHDUtil::yes_or_no(git_AnyUncommittedChanges()) << "\n";
    std::cout << ss.str() << std::flush;
    // openhd::debug_config();
    // OHDInterface::print_internal_fec_optimization_method();
}
  // Create the folder structure
  openhd::generateSettingsDirectoryIfNonExists();
  const auto platform = OHDPlatform::instance();
  openhd::LEDManager::instance().set_status_loading();
  // Generate the keys and delete pw if needed
  OHDInterface::generate_keys_from_pw_if_exists_and_delete();
  // Parse the program arguments
  // This is the console we use inside main, in general different openhd
  // modules/classes have their own loggers with different tags
  std::shared_ptr<spdlog::logger> m_console =
      openhd::log::create_or_get("main");
  assert(m_console);

  // not guaranteed, but better than nothing, check if openhd is already running
  // (kinda) and print warning if yes.
  openhd::check_currently_running_file_and_write();

  // Create and link all the OpenHD modules.
  try {
    // This results in fresh default values for all modules (e.g. interface,
    // telemetry, video)
    if (options.reset_all_settings) {
      openhd::clean_all_settings();
    }
    if (openhd::ButtonManager::instance().user_wants_reset_openhd_core()) {
      openhd::clean_all_settings();
    }
    // Profile no longer depends on n discovered cameras,
    // But if we are air, we have at least one camera, sw if no camera was found
    const auto profile = DProfile::discover(options.run_as_air);
    write_profile_manifest(profile);

    // we need to start QOpenHD when we are running as ground, or stop / disable
    // it when we are running as air. can be disabled for development purposes.
    // On x20, we do not have qopenhd installed (we run as air only) so we can
    // skip this step
    if(!options.no_qopenhd_autostart){
      if (!openhd::load_config().GEN_NO_QOPENHD_AUTOSTART &&
          !OHDPlatform::instance().is_x20()) {
        if (!profile.is_air) {
          OHDUtil::run_command("systemctl", {"--quiet", "start", "qopenhd", "> /dev/null 2>&1"});
        } else {
          OHDUtil::run_command("systemctl", {"--quiet", "stop", "qopenhd", "> /dev/null 2>&1"});
        }
      }
    }

    // create the global action handler that allows openhd modules to
    // communicate with each other e.g. when the rf link in ohd_interface needs
    // to talk to the camera streams to reduce the bitrate
    openhd::LinkActionHandler::instance();

    // We start ohd_telemetry as early as possible, since even without a link
    // (transmission) it still picks up local log message(s) and forwards them
    // to any ground station clients (e.g. QOpenHD)
    auto ohdTelemetry = std::make_shared<OHDTelemetry>(profile);

    // Then start ohdInterface, which discovers detected wifi cards and more.
    auto ohdInterface = std::make_shared<OHDInterface>( profile);

    // Telemetry allows changing all settings (even from other modules)
    ohdTelemetry->add_settings_generic(ohdInterface->get_all_settings());

    // either one is active, depending on air or ground
    std::unique_ptr<OHDVideoGround> ohd_video_ground = nullptr;
    if (profile.is_ground()) {
      ohd_video_ground =
          std::make_unique<OHDVideoGround>(ohdInterface->get_link_handle());
    }
#ifdef ENABLE_AIR
    std::unique_ptr<OHDVideoAir> ohd_video_air = nullptr;
    if (profile.is_air) {
      auto cameras = OHDVideoAir::discover_cameras();
      ohd_video_air = std::make_unique<OHDVideoAir>(
          cameras, ohdInterface->get_link_handle());
      // First add camera specific settings (primary & secondary camera)
      auto settings_components = ohd_video_air->get_all_camera_settings();
      ohdTelemetry->add_settings_camera_component(0, settings_components[0]);
      ohdTelemetry->add_settings_camera_component(1, settings_components[1]);
      // Then the rest
      ohdTelemetry->add_settings_generic(ohd_video_air->get_generic_settings());
    }
#endif  // ENABLE_AIR
    // We do not add any more settings to ohd telemetry - the param set(s) are
    // complete
    ohdTelemetry->settings_generic_ready();
    // now telemetry can send / receive data via wifibroadcast
    ohdTelemetry->set_link_handle(ohdInterface->get_link_handle());
    std::cout << green << "OpenHD was successfully started." << reset << std::endl;
    openhd::LEDManager::instance().set_status_okay();
    // run forever, everything has its own threads. Note that the only way to
    // break out basically is when one of the modules encounters an exception.
    static bool quit = false;
    // https://unix.stackexchange.com/questions/362559/list-of-terminal-generated-signals-eg-ctrl-c-sigint
    signal(SIGTERM, [](int sig) {
      std::cerr << "Got SIGTERM, exiting\n";
      quit = true;
    });
    signal(SIGQUIT, [](int sig) {
      std::cerr << "Got SIGQUIT, exiting\n";
      quit = true;
    });
    const auto run_time_begin = std::chrono::steady_clock::now();
    while (!quit) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      if (options.run_time_seconds >= 1) {
        if (std::chrono::steady_clock::now() - run_time_begin >=
            std::chrono::seconds(options.run_time_seconds)) {
          m_console->warn("Terminating, exceeded run time {}",
                          options.run_time_seconds);
          // we can just break out any time, usefully for checking memory leaks
          // and more.
          break;
        }
      }
      if (openhd::TerminateHelper::instance().should_terminate()) {
        m_console->debug("Terminating,reason:{}",
                         openhd::TerminateHelper::instance().terminate_reason());
        break;
      }
    }
    // --- terminate openhd, most likely requested by a developer with sigterm
    m_console->debug("Terminating openhd");
    openhd::LEDManager::instance().set_status_stopped();
    // Stop any communication between modules, to eliminate any issues created
    // by threads during cleanup
    openhd::LinkActionHandler::instance().disable_all_callables();
    openhd::ExternalDeviceManager::instance().remove_all();
    // dirty, wait a bit to make sure none of those action(s) are called anymore
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // unique ptr would clean up for us, but this way we are a bit more verbose
    // since some of those modules talk to each other, this is a bit prone to
    // failures.
#ifdef ENABLE_AIR
    if (ohd_video_air) {
      m_console->debug("Terminating ohd_video_air - begin");
      ohd_video_air.reset();
      m_console->debug("Terminating ohd_video_air - end");
    }
#endif
    if (ohd_video_ground) {
      m_console->debug("Terminating ohd_video_ground- begin");
      ohd_video_ground.reset();
      m_console->debug("Terminating ohd_video_ground - end");
    }
    if (ohdTelemetry) {
      m_console->debug("Terminating ohd_telemetry - begin");
      ohdTelemetry.reset();
      m_console->debug("Terminating ohd_telemetry - end");
    }
    if (ohdInterface) {
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
