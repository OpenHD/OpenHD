#include "video_worker_child.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

// Model a vendor library opening a device before main, after exec.
static const int constructor_fd = open("/dev/zero", O_RDONLY);

int main(int argc, char** argv) {
  if (argc == 3 && std::strcmp(argv[1], "--worker") == 0) {
    if (fcntl(constructor_fd, F_GETFD) < 0) return 10;
    char byte = 1;
    if (read(constructor_fd, &byte, 1) != 1 || byte != 0) return 11;
    // A descriptor opened by the parent must not leak across exec.
    auto* directory = opendir("/proc/self/fd");
    if (!directory) return 12;
    while (auto* item = readdir(directory)) {
      char path[512], target[4096];
      std::snprintf(path, sizeof(path), "/proc/self/fd/%s", item->d_name);
      const auto size = readlink(path, target, sizeof(target) - 1);
      if (size > 0) {
        target[size] = 0;
        if (std::strcmp(target, argv[2]) == 0) return 13;
      }
    }
    closedir(directory);
    int type = 0;
    socklen_t size = sizeof(type);
    return getsockopt(3, SOL_SOCKET, SO_TYPE, &type, &size) == 0 &&
                   type == SOCK_SEQPACKET ? 0 : 14;
  }
  char path[] = "/tmp/openhd-worker-fd-XXXXXX";
  int parent_fd = mkstemp(path);
  if (parent_fd < 0) return 1;
  openhd::video_worker::Child child;
  const bool started = child.start("/proc/self/exe", {"--worker", path});
  close(parent_fd);
  if (!started) { unlink(path); return 2; }
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!child.reap() && std::chrono::steady_clock::now() < deadline)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  unlink(path);
  if (child.pid > 0 || !WIFEXITED(child.exit_status) ||
      WEXITSTATUS(child.exit_status) != 0) return 3;
  std::puts("PASS: parent descriptors closed before exec; constructor descriptors and IPC survive");
}
