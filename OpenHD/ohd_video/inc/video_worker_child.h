#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>

extern char** environ;

namespace openhd::video_worker {
class Child {
 public:
  Child() = default;
  Child(const Child&) = delete;
  Child& operator=(const Child&) = delete;
  ~Child() {
    kill();
    if (fd >= 0) close(fd);
    // Usually already reaped by the supervisor. Never wait indefinitely for a
    // driver stuck in uninterruptible kernel sleep during application shutdown.
    reap();
  }
  bool start(const std::string& executable,
             std::vector<std::string> arguments) {
    if (pid > 0) return false;
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockets) != 0)
      return false;
    // The exec'd worker retains only fd 3 and standard input/output/error.
    // Use a source above 3 so the dup2 action always clears FD_CLOEXEC.
    int source = fcntl(sockets[1], F_DUPFD_CLOEXEC, 4);
    close(sockets[1]);
    if (source < 0) {
      close(sockets[0]);
      return false;
    }
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, source, 3);
    // Close parent descriptors before exec. Libraries such as Rockchip MPP
    // open DMA heaps in ELF constructors; closing descriptors in worker main
    // would also close those newly opened heaps and break the encoder.
    auto* directory = opendir("/proc/self/fd");
    if (!directory) {
      posix_spawn_file_actions_destroy(&actions);
      close(source);
      close(sockets[0]);
      return false;
    }
    while (auto* item = readdir(directory)) {
      const int inherited_fd = std::atoi(item->d_name);
      if (inherited_fd > 3 && inherited_fd != dirfd(directory))
        posix_spawn_file_actions_addclose(&actions, inherited_fd);
    }
    closedir(directory);
    arguments.insert(arguments.begin(), executable);
    std::vector<char*> argv;
    for (auto& arg : arguments) argv.push_back(arg.data());
    argv.push_back(nullptr);
    int result = posix_spawn(&pid, executable.c_str(), &actions, nullptr,
                             argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    close(source);
    if (result != 0) {
      pid = -1;
      close(sockets[0]);
      return false;
    }
    fd = sockets[0];
    int capacity = 4 * 1024 * 1024;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &capacity, sizeof(capacity));
    return true;
  }
  void kill() {
    if (pid > 0) ::kill(pid, SIGKILL);
  }
  bool reap() {
    if (pid <= 0) return true;
    int status;
    pid_t result;
    do {
      result = waitpid(pid, &status, WNOHANG);
    } while (result < 0 && errno == EINTR);
    if (result == pid || (result < 0 && errno == ECHILD)) {
      if (result == pid) exit_status = status;
      pid = -1;
      return true;
    }
    return false;
  }
  int fd = -1;
  pid_t pid = -1;
  int exit_status = -1;
};
}  // namespace openhd::video_worker
