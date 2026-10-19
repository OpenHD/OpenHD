#include <filesystem>
#include <csignal>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

#include "config_paths.h"
#include "gst_camera_process.h"
#include "gst_debug_helper.h"
#include "openhd_config.h"
#include "video_worker_child.h"
#include "video_worker_frame.h"

namespace {
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;
using namespace openhd::video_worker;
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
template <class Predicate>
void wait_until(Predicate predicate, std::chrono::seconds timeout,
                const char* message) {
  const auto deadline = Clock::now() + timeout;
  while (!predicate()) {
    require(Clock::now() < deadline, message);
    std::this_thread::sleep_for(50ms);
  }
}

std::map<int, pid_t> workers() {
  std::map<int, pid_t> result;
  for (const auto& task :
       std::filesystem::directory_iterator("/proc/self/task")) {
    std::ifstream children(task.path() / "children");
    pid_t pid;
    while (children >> pid) {
      std::ifstream command("/proc/" + std::to_string(pid) + "/cmdline");
      std::vector<std::string> args;
      std::string arg;
      while (std::getline(command, arg, '\0')) args.push_back(arg);
      if (args.size() == 11 &&
          (args[1] == "--gst-camera-worker" ||
           args[1] == "--libcamera-worker" || args[1] == "--mpp-camera-worker"))
        result[std::stoi(args[3])] = pid;
    }
  }
  return result;
}

void test_frames() {
  int sockets[2];
  require(socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockets) == 0,
          "socketpair failed");
  openhd::FragmentedVideoFrame source;
  source.is_idr_frame = true;
  source.is_intra_stream = true;
  source.enable_ultra_secure_encryption = true;
  source.rtp_fragments.push_back(
      std::make_shared<std::vector<uint8_t>>(1400, 42));
  source.dirty_frame = std::make_shared<std::vector<uint8_t>>(90000, 93);
  require(send_frame(sockets[0], 1, source), "frame send failed");
  FrameAssembler assembler;
  Header header;
  std::vector<uint8_t> data;
  bool complete = false;
  while (receive_packet(sockets[1], header, data) == 1)
    complete = assembler.accept(header, data) || complete;
  require(complete, "frame not reconstructed");
  require(*assembler.frame.rtp_fragments.at(0) == *source.rtp_fragments.at(0),
          "RTP bytes changed");
  require(*assembler.frame.dirty_frame == *source.dirty_frame,
          "raw bytes changed");
  require(assembler.frame.creation_time == source.creation_time &&
              assembler.frame.is_idr_frame && assembler.frame.is_intra_stream &&
              assembler.frame.enable_ultra_secure_encryption,
          "frame metadata changed");
  assembler.accept({Type::FrameBegin, 2}, {});
  assembler.accept({Type::Fragment, 2}, {1, 2, 3});
  assembler.accept({Type::FrameBegin, 3}, {});
  require(!assembler.accept({Type::FrameEnd, 2}, {}), "partial frame escaped");
  assembler.accept({Type::Fragment, 3}, {4});
  require(assembler.accept({Type::FrameEnd, 3}, {}),
          "next frame did not recover");
  require(assembler.frame.rtp_fragments.size() == 1 &&
              assembler.frame.rtp_fragments[0]->at(0) == 4,
          "partial frame mixed in");
  // Fill the socket without draining it. Congestion must return immediately.
  const auto begin = Clock::now();
  int sent = 0;
  while (
      send_packet(sockets[0], {Type::Fragment, 4}, data.data(), data.size())) {
    require(++sent < 100000, "socket was not bounded");
  }
  require(Clock::now() - begin < 1s, "congested IPC blocked");
  require(!send_packet(sockets[0], {Type::Raw}, data.data(), kChunkSize + 1),
          "oversized packet accepted");
  close(sockets[0]);
  close(sockets[1]);
}

void test_timeout_exit() {
  Child worker;
  char executable[4096];
  const auto size = readlink("/proc/self/exe", executable, sizeof(executable));
  require(size > 0, "cannot resolve test executable");
  require(worker.start(std::string(executable, size), {"--test-gst-timeout"}),
          "cannot launch timeout helper worker");
  wait_until([&] { return worker.reap(); }, 5s,
             "timed out GStreamer helper did not terminate worker");
  require(WIFEXITED(worker.exit_status) && WEXITSTATUS(worker.exit_status) == 0,
          "timeout helper worker failed before observing termination request");
  require(!openhd::TerminateHelper::instance().should_terminate(),
          "child timeout requested parent termination");
  std::cout << "PASS: GStreamer helper timeout terminates only its worker\n";
}

std::shared_ptr<CameraHolder> camera(int index) {
  XCamera camera{};
  camera.index = index;
  camera.camera_type = X_CAM_TYPE_DUMMY_SW;
  auto holder = std::make_shared<CameraHolder>(camera, true);
  auto& settings = holder->unsafe_get_settings();
  settings.enable_streaming = true;
  settings.streamed_video_format = {VideoCodec::H264, 320, 240, 10};
  settings.h26x_bitrate_kbits = 1000;
  settings.air_recording = AIR_RECORDING_OFF;
  return holder;
}

void test_recovery() {
  auto first = camera(0);
  auto second = camera(1);
  std::atomic<int> frames0{0}, frames1{0};
  CameraProcess stream0(
      first,
      [&](int index, const auto& frame) {
        require(index == 0 && !frame.rtp_fragments.empty(),
                "invalid camera 0 frame");
        ++frames0;
      },
      false);
  CameraProcess stream1(
      second,
      [&](int index, const auto& frame) {
        require(index == 1 && !frame.rtp_fragments.empty(),
                "invalid camera 1 frame");
        ++frames1;
      },
      false);
  stream0.start_looping();
  stream1.start_looping();
  auto& actions = openhd::LinkActionHandler::instance();
  wait_until([&] { return frames0 > 10 && frames1 > 10; }, 15s,
             "synthetic cameras did not produce frames");
  wait_until([&] { return workers().size() == 2; }, 5s, "workers missing");
  std::cout << "PASS: two exec'd camera workers produce intact frames\n";

  // Live controls and settings restart use the same parameter callbacks as the
  // application. No test-only fault hooks are installed in production code.
  require(first->set_video_bitrate(2), "bitrate update rejected");
  wait_until(
      [&] { return actions.get_cam_info(0).encoding_bitrate_kbits == 2000; },
      8s, "live bitrate did not reach worker statistics");
  require(first->set_video_width_height_framerate(640, 360, 10),
          "resolution update rejected");
  wait_until([&] { return actions.get_cam_info(0).stream_w == 640; }, 8s,
             "settings snapshot did not restart pipeline");
  std::cout
      << "PASS: live bitrate and settings changes cross the process boundary\n";

  // A link limit must never replace the configured bitrate ceiling, including
  // after a pipeline restart.
  stream0.handle_change_bitrate_request({1000, true});
  wait_until(
      [&] { return actions.get_cam_info(0).encoding_bitrate_kbits == 1000; },
      8s, "link capacity limit not applied");
  require(first->get_settings().h26x_bitrate_kbits == 2000,
          "link adaptation overwrote configured ceiling");
  require(first->set_air_recording(AIR_RECORDING_AUTO_ARM_DISARM),
          "auto recording rejected");
  stream0.handle_update_arming_state(true);
  wait_until([&] { return actions.get_cam_info(0).air_recording_active != 0; },
             10s, "arming did not start recording");
  stream0.handle_update_arming_state(false);
  wait_until([&] { return actions.get_cam_info(0).air_recording_active == 0; },
             10s, "disarming did not stop recording");
  require(first->set_air_recording(AIR_RECORDING_OFF),
          "recording reset rejected");
  std::cout << "PASS: link ceiling and arming-driven recording survive IPC\n";

  const auto original = workers().at(0);
  const auto healthy = workers().at(1);
  auto count0 = frames0.load();
  auto count1 = frames1.load();
  ::kill(original, SIGKILL);
  wait_until(
      [&] {
        const auto pids = workers();
        return pids.count(0) && pids.at(0) != original && frames0 > count0 + 5;
      },
      12s, "camera did not recover after crash");
  require(workers().at(1) == healthy && frames1 > count1,
          "other camera was interrupted by crash");
  require(first->get_settings().h26x_bitrate_kbits == 2000,
          "crash changed parent settings");
  std::cout << "PASS: crashed worker restarts independently\n";

  const auto frozen = workers().at(0);
  ::kill(frozen, SIGSTOP);
  count0 = frames0.load();
  count1 = frames1.load();
  const auto start = Clock::now();
  require(first->set_video_width_height_framerate(320, 240, 10),
          "setting rejected while worker frozen");
  require(Clock::now() - start < 500ms,
          "frozen worker blocked settings callback");
  wait_until([&] { return frames1 > count1 + 10; }, 5s,
             "healthy camera stopped during other camera freeze");
  wait_until(
      [&] {
        const auto pids = workers();
        return pids.count(0) && pids.at(0) != frozen && frames0 > count0 + 5;
      },
      38s, "watchdog did not recover frozen camera");
  require(workers().at(1) == healthy, "watchdog restarted healthy camera");
  std::cout << "PASS: watchdog recovers frozen worker; parent and second "
               "camera stay responsive\n";

  stream0.terminate_looping();
  stream1.terminate_looping();
  require(workers().empty(), "workers were not reaped on shutdown");
  std::cout << "PASS: clean shutdown reaps both workers\n";
}

void test_libcamera_recovery() {
  // This executable doubles as a fake rpicam-vid. It emits Annex-B NALs, then
  // stalls once and ignores SIGINT so both the watchdog and forced stop run.
  char executable[4096];
  const auto size = readlink("/proc/self/exe", executable, sizeof(executable));
  require(size > 0, "cannot resolve fake encoder");
  const std::string app(executable, size);
  const auto state = std::string(getVideoPath()) + "fake-libcamera";
  setenv("OPENHD_LIBCAMERA_APP", app.c_str(), 1);
  setenv("OPENHD_FAKE_LIBCAMERA_STATE", state.c_str(), 1);
  auto holder = camera(0);
  std::atomic<int> frames{0};
  CameraProcess stream(
      holder, [&](int, const auto&) { ++frames; }, false,
      CameraProcess::Backend::Libcamera);
  stream.start_looping();
  wait_until([&] { return frames > 5; }, 10s,
             "fake libcamera produced no frames");
  const auto worker = workers().at(0);
  const auto count = frames.load();
  require(holder->set_video_bitrate(3), "libcamera bitrate rejected");
  wait_until(
      [&] {
        return openhd::LinkActionHandler::instance()
                   .get_cam_info(0)
                   .encoding_bitrate_kbits == 3000;
      },
      10s, "libcamera bitrate not applied");
  // The first fake encoder stalls; a subsequent invocation produces frames.
  wait_until([&] { return std::filesystem::exists(state + ".stalled"); }, 10s,
             "fake encoder did not enter stall");
  const auto stalled_count = frames.load();
  wait_until([&] { return frames > stalled_count + 10; }, 20s,
             "libcamera no-data watchdog did not restart encoder");
  require(workers().at(0) == worker, "libcamera watchdog killed outer worker");
  require(frames > count, "libcamera did not resume output");
  std::cout << "PASS: libcamera no-data watchdog and forced encoder shutdown "
               "recover\n";
  const auto before_crash = frames.load();
  pid_t nested_encoder = -1;
  std::ifstream(state + ".pid") >> nested_encoder;
  require(nested_encoder > 0, "fake encoder PID missing");
  ::kill(worker, SIGKILL);
  wait_until(
      [&] {
        const auto pids = workers();
        return pids.count(0) && pids.at(0) != worker &&
               frames > before_crash + 5;
      },
      12s, "libcamera worker crash did not recover");
  wait_until([&] {
    std::ifstream status("/proc/" + std::to_string(nested_encoder) + "/stat");
    int pid;
    std::string command;
    char state;
    return !(status >> pid >> command >> state) || state == 'Z';
  }, 5s, "nested encoder survived worker death");
  const auto stop_begin = Clock::now();
  stream.terminate_looping();
  require(Clock::now() - stop_begin < 8s, "libcamera shutdown was not bounded");
  require(workers().empty(), "libcamera worker not reaped");
  unsetenv("OPENHD_LIBCAMERA_APP");
  unsetenv("OPENHD_FAKE_LIBCAMERA_STATE");
  std::cout << "PASS: libcamera worker crash recovery and bounded shutdown\n";
}

int fake_libcamera() {
  std::signal(SIGINT, SIG_IGN);
  const auto* state_env = std::getenv("OPENHD_FAKE_LIBCAMERA_STATE");
  if (!state_env) return 2;
  const std::string state(state_env);
  std::ofstream(state + ".pid") << getpid();
  const bool stall = !std::filesystem::exists(state + ".stalled");
  // RTPHelper deliberately withholds frames until SPS and PPS are available.
  const uint8_t configuration[]{
      0, 0, 0, 1, 0x67, 0x42, 0, 0x1e, 0x95, 0xa8, 0x28, 0x0f,
      0, 0x44, 0xfc, 0xb8, 0x08, 0x80,
      0, 0, 0, 1, 0x68, 0xce, 0x06, 0xe2};
  if (write(STDOUT_FILENO, configuration, sizeof(configuration)) <= 0) return 0;
  std::vector<uint8_t> nalu(1024, 0x55);
  nalu[0] = nalu[1] = nalu[2] = 0;
  nalu[3] = 1;
  nalu[4] = 0x65;
  for (int i = 0;; ++i) {
    if (stall && i >= 30) {
      std::ofstream(state + ".stalled") << getpid();
      while (true) std::this_thread::sleep_for(1s);
    }
    if (write(STDOUT_FILENO, nalu.data(), nalu.size()) <= 0) return 0;
    std::this_thread::sleep_for(30ms);
  }
}
}  // namespace

int main(int argc, char* argv[]) {
  if (argc > 1 && std::string(argv[1]) == "-t") return fake_libcamera();
  if (argc == 2 && std::string(argv[1]) == "--test-gst-timeout") {
    openhd::run_gst_with_timeout_result("injected-stall", 20ms, [] {
      std::this_thread::sleep_for(10s);
      return true;
    });
    while (!openhd::TerminateHelper::instance().should_terminate())
      std::this_thread::sleep_for(10ms);
    std::_Exit(0);
  }
  const int worker_result = run_camera_worker_if_requested(argc, argv);
  if (worker_result >= 0) return worker_result;
  try {
    char temporary[] = "/tmp/openhd-camera-isolation-XXXXXX";
    require(mkdtemp(temporary) != nullptr,
            "cannot create test recording directory");
    struct TemporaryDirectory {
      std::filesystem::path path;
      ~TemporaryDirectory() { std::filesystem::remove_all(path); }
    } directory{temporary};
    const auto path = directory.path.string() + "/";
    setConfigBasePath(path.c_str());
    setVideoPath(path.c_str());
    openhd::set_wifi_monitor_card_emulate_override(true);
    if (argc != 2 || std::string(argv[1]) != "--native-only") {
      test_frames();
      test_timeout_exit();
      test_recovery();
    }
    test_libcamera_recovery();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
