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
#include <camera.hpp>
#include <ohd_video_air.h>
#include <ohd_video_air_generic_settings.h>
#endif  // ENABLE_AIR
#include <ohd_video_ground.h>

#include <csignal>
#include <exception>
#include <iostream>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <optional>
#include <vector>

#include "openhd_buttons.h"
#include "openhd_global_constants.hpp"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "openhd_sock.h"
#include "openhd_spdlog.h"
#include "openhd_temporary_air_or_ground.h"
#include "openhd_config.h"
#include "openhd_util_filesystem.h"
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
static const char optstr[] = "?:agcwort:h:";
static const struct option long_options[] = {
    {"air", no_argument, nullptr, 'a'},
    {"ground", no_argument, nullptr, 'g'},
    {"clean-start", no_argument, nullptr, 'c'},
    {"no-qt-autostart", no_argument, nullptr, 'w'},
    {"no-hotspot", no_argument, nullptr, 'o'},
    {"record-only", no_argument, nullptr, 'r'},
    {"run-time-seconds", required_argument, nullptr, 't'},
    {"hardware-config-file", required_argument, nullptr, 'h'},
    {"openhd_uart_telemetry", optional_argument, nullptr, 0},
    {nullptr, 0, nullptr, 0},
};
    const std::string red = "\033[31m";
    const std::string green = "\033[32m";
    const std::string blue = "\033[94m";
    const std::string reset = "\033[0m";

struct OHDRunOptions {
  bool run_as_air = false;
  bool reset_all_settings = false;
  bool reset_from_sysutil = false;
  bool record_only = false;
  bool no_qopenhd_autostart=false;
  bool no_hotspot=false;
  int run_time_seconds = -1;  //-1= infinite, only usefully for debugging
  // Specify the hardware.config file, otherwise,
  // the default location (and default values if no file exists at the default
  // location) is used
  std::optional<std::string> hardware_config_file;
  std::optional<std::string> openhd_uart_telemetry_device;
};

static OHDRunOptions parse_run_parameters(int argc, char *argv[]) {
  OHDRunOptions ret{};
  int c;
  // If this value gets set, we assume a developer is working on OpenHD and skip
  // the discovery via file(s).
  std::optional<bool> commandline_air = std::nullopt;
  int option_index = 0;
  while ((c = getopt_long(argc, argv, optstr, long_options, &option_index)) !=
         -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 0: {
        const std::string option_name = long_options[option_index].name;
        if (option_name == "openhd_uart_telemetry") {
          if (optarg != nullptr) {
            ret.openhd_uart_telemetry_device = optarg;
          } else {
            ret.openhd_uart_telemetry_device = "/dev/serial1";
          }
        }
        break;
      }
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
      case 'o':
        ret.no_hotspot = true;
        break;
      case 'r':
        if (commandline_air != std::nullopt && commandline_air.value() == false) {
          std::cerr << "Record-only requires air mode\n";
          exit(1);
        }
        commandline_air = true;
        ret.record_only = true;
        break;
      case 't':
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
        ss << "--no-hotspot      [disable WiFi hotspot on ground] \n";
        ss << "--record-only -r  [Record video without streaming it] \n";
        ss << "--run-time-seconds -t [Manually specify run time (default "
              "infinite),for debugging] \n";
        ss << "--hardware-config-file -h [specify path to hardware.config "
              "file]\n";
        ss << "--openhd_uart_telemetry [optional serial device, default "
              "/dev/serial1] \n";
        ss << "Use hardware.conf for more configuration\n";
        std::cout << ss.str() << std::flush;
      }
        exit(1);
    }
  }
  const auto sysutil_settings = openhd::request_sysutil_settings();
  if (sysutil_settings.has_value() && sysutil_settings->has_reset &&
      sysutil_settings->reset_requested) {
    ret.reset_all_settings = true;
    ret.reset_from_sysutil = true;
  }
  if (commandline_air == std::nullopt) {
    // command line parameters not used, use the file(s) for detection (default
    // for normal OpenHD images) The logs/checks here are just to help
    // developer(s) avoid common misconfigurations
    const bool has_run_mode =
        sysutil_settings.has_value() && sysutil_settings->has_run_mode;
    const bool run_as_air =
        has_run_mode && sysutil_settings->run_as_air;
    const bool run_record_only =
        has_run_mode && sysutil_settings->run_record_only;
    bool error = false;
    if (!has_run_mode) {  // no sysutils setting exists
      // Just run as ground
      ret.run_as_air = false;
      error = true;
    } else {
      ret.run_as_air = run_as_air;
      ret.record_only = run_record_only;
    }
  } else {
    // command line parameters used, just validate they are not mis-configured
    assert(commandline_air.has_value());
    ret.run_as_air = commandline_air.value();
    openhd::SysutilSettingsUpdate update{};
    if (ret.record_only) {
      update.run_mode = "record";
    } else {
      update.run_as_air = ret.run_as_air;
    }
    (void)openhd::update_sysutil_settings(update);
  }
  if (ret.record_only) {
    ret.no_hotspot = true;
    ret.no_qopenhd_autostart = true;
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
  auto& reporter = openhd::Reporter::instance();
  reporter.report(openhd::State::Booting);
  const OHDRunOptions options = parse_run_parameters(argc, argv);
  if (options.hardware_config_file.has_value()) {
    openhd::set_config_file(options.hardware_config_file.value());
  }
  // Create the folder structure
  openhd::generateSettingsDirectoryIfNonExists();
  const auto platform = OHDPlatform::instance();
  reporter.report(openhd::State::Starting);
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
    std::vector<std::string> startup_errors;
    // This results in fresh default values for all modules (e.g. interface,
    // telemetry, video)
    bool reset_performed = false;
    if (options.reset_all_settings) {
      openhd::clean_all_settings();
      reset_performed = true;
    }
    if (openhd::ButtonManager::instance().user_wants_reset_openhd_core()) {
      openhd::clean_all_settings();
      reset_performed = true;
    }
    if (options.reset_from_sysutil && reset_performed) {
      openhd::SysutilSettingsUpdate update{};
      update.reset_requested = false;
      (void)openhd::update_sysutil_settings(update);
    }
    // Profile no longer depends on n discovered cameras,
    // But if we are air, we have at least one camera, sw if no camera was found
    const auto profile = DProfile::discover(options.run_as_air);
    write_profile_manifest(profile);

    {  // Print all the arguments the OHD main executable is started with
      bool validLicense = false;
      if (OHDFilesystemUtil::exists("/usr/local/share/openhd/license")) {
        validLicense = true;
      }
      std::cout << "\033[2J\033[1;1H";  // clear terminal
      std::stringstream ss;
      ss << openhd::get_ohd_version_as_string() << "\n";
      ss << "Built: " << __DATE__ << " " << __TIME__ << "\n";
      ss << "Platform: "
         << x_platform_type_to_string(platform.platform_type) << "\n";
#ifdef ENABLE_AIR
      std::string camera_info = "N/A";
      if (profile.is_air) {
        AirCameraGenericSettingsHolder camera_settings;
        camera_info =
            x_cam_type_to_string(
                camera_settings.get_settings().primary_camera_type);
      }
      ss << "Camera: " << camera_info << "\n";
#endif
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
        ss << "----------------------- " << blue << "OpenSource" << reset
           << " -----------------------\n";
      } else {
        ss << "----------------------- " << green << "Enterprise" << reset
           << " -----------------------\n";
      }
      ss << "\n";

      if (options.record_only) {
        ss << "--------------------- " << green << "Record Mode" << reset
           << " ---------------------\n";
      } else if (options.run_as_air) {
        ss << "----------------------- " << green << "Air Unit" << reset
           << " -----------------------\n";
      } else {
        ss << "----------------------- " << red << "Ground Unit" << reset
           << " ----------------------\n";
      }

      if (options.reset_all_settings) {
        ss << red << "Reset Settings" << reset << "\n";
      }
      ss << "\n";

      // ss << "Git info:Branch:" << git_Branch() << " SHA:" << git_CommitSHA1()
      // << " Dirty:" << OHDUtil::yes_or_no(git_AnyUncommittedChanges()) << "\n";
      std::cout << ss.str() << std::flush;
      // openhd::debug_config();
      // OHDInterface::print_internal_fec_optimization_method();
    }

    // we need to start QOpenHD when we are running as ground, or stop / disable
    // it when we are running as air. can be disabled for development purposes.
    // On x20, we do not have qopenhd installed (we run as air only) so we can
    // skip this step
    if (!options.no_qopenhd_autostart) {
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
    std::shared_ptr<OHDTelemetry> ohdTelemetry = nullptr;
    if (!options.record_only) {
      ohdTelemetry = std::make_shared<OHDTelemetry>(profile);
      if (options.openhd_uart_telemetry_device.has_value()) {
        ohdTelemetry->configure_openhd_uart_telemetry(
            options.openhd_uart_telemetry_device);
      }
    }

    // Then start ohdInterface, which discovers detected wifi cards and more.
    std::shared_ptr<OHDInterface> ohdInterface = nullptr;
    if (!options.record_only) {
      ohdInterface = std::make_shared<OHDInterface>(profile, options.no_hotspot);
      if (!ohdInterface->has_real_monitor_mode_cards()) {
        const std::string no_wifi_card_message =
            "No openhd wifibroadcast card found";
        startup_errors.push_back(no_wifi_card_message);
      }
      if (!ohdInterface->has_primary_link()) {
        const std::string no_link_message =
            "No functional link detected (WiFi/Microhard/Ethernet)";
        startup_errors.push_back(no_link_message);
        reporter.report_status("no_link", no_link_message, 10000);
      }

      // Telemetry allows changing all settings (even from other modules)
      if (ohdTelemetry) {
        ohdTelemetry->add_settings_generic(ohdInterface->get_all_settings());
      }
    }

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
      const bool using_dummy_camera = std::any_of(
          cameras.begin(), cameras.end(),
          [](const XCamera& camera) {
            return camera.camera_type == X_CAM_TYPE_DUMMY_SW;
          });
      if (using_dummy_camera) {
        const std::string dummy_camera_message =
            "No physical camera detected; using dummy camera configuration";
        startup_errors.push_back(dummy_camera_message);
      }
      std::shared_ptr<OHDLink> link_handle = nullptr;
      if (ohdInterface) {
        link_handle = ohdInterface->get_link_handle();
      }
      ohd_video_air = std::make_unique<OHDVideoAir>(
          cameras, link_handle, options.record_only);
      if (ohdTelemetry) {
        // First add camera specific settings (primary & secondary camera)
        auto settings_components = ohd_video_air->get_all_camera_settings();
        ohdTelemetry->add_settings_camera_component(0, settings_components[0]);
        ohdTelemetry->add_settings_camera_component(1, settings_components[1]);
        // Then the rest
        ohdTelemetry->add_settings_generic(ohd_video_air->get_generic_settings());
      }
    }
#endif  // ENABLE_AIR
    // We do not add any more settings to ohd telemetry - the param set(s) are
    // complete
    if (ohdTelemetry) {
      ohdTelemetry->settings_generic_ready();
      // now telemetry can send / receive data via wifibroadcast
      if (ohdInterface) {
        ohdTelemetry->set_link_handle(ohdInterface->get_link_handle());
      }
    }
    if (startup_errors.empty()) {
      std::cout << green << "OpenHD was successfully started." << reset
                << std::endl;
      reporter.report(openhd::State::Ready);
    } else {
      const auto combined_errors =
          fmt::format("{}", fmt::join(startup_errors, "; "));
      std::cout << red << "OpenHD started with errors:" << reset << std::endl;
      for (const auto& error_message : startup_errors) {
        std::cout << red << " - " << error_message << reset << std::endl;
        m_console->error("Startup issue: {}", error_message);
      }
      reporter.report(openhd::State::Error, combined_errors, 3000);
    }
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
    reporter.report(openhd::State::Stopped);
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
    reporter.report(openhd::State::Error);
    exit(1);
  } catch (...) {
    std::cerr << "Unknown exception occurred" << std::endl;
    reporter.report(openhd::State::Error);
    exit(1);
  }
  openhd::remove_currently_running_file();
  return 0;
}
