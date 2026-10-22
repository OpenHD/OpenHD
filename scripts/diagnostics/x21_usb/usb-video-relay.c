// Temporary bench transport: preserve UDP datagrams across an ADB TCP forward.
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_all(int fd, const void *data, size_t length) {
  const char *bytes = data;
  while (length) {
    ssize_t sent = send(fd, bytes, length, MSG_NOSIGNAL);
    if (sent < 0 && errno == EINTR) continue;
    if (sent <= 0) return -1;
    bytes += sent;
    length -= sent;
  }
  return 0;
}

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  int udp = socket(AF_INET, SOCK_DGRAM, 0);
  int tcp = socket(AF_INET, SOCK_STREAM, 0);
  int reuse = 1, size = 4 * 1024 * 1024;
  setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  /* Root bench helper: avoid the image's 208 KiB rmem_max clipping bursts
   * while Windows pauses TCP reception. Ordinary SO_RCVBUF silently caps it. */
  if (setsockopt(udp, SOL_SOCKET, SO_RCVBUFFORCE, &size, sizeof(size)))
    setsockopt(udp, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
  socklen_t size_length = sizeof(size);
  if (!getsockopt(udp, SOL_SOCKET, SO_RCVBUF, &size, &size_length)) {
    fprintf(stderr, "UDP receive buffer: %d bytes\n", size);
    fflush(stderr);
  }
  struct sockaddr_in address = {.sin_family = AF_INET,
      .sin_addr.s_addr = htonl(INADDR_LOOPBACK), .sin_port = htons(5600)};
  if (udp < 0 || tcp < 0 || bind(udp, (void *)&address, sizeof(address))) {
    perror("UDP relay bind"); return 1;
  }
  address.sin_port = htons(15600);
  if (argc > 2 || (argc == 2 && inet_pton(AF_INET, argv[1], &address.sin_addr) != 1)) return 2;
  if (bind(tcp, (void *)&address, sizeof(address)) || listen(tcp, 1)) {
    perror("TCP relay bind"); return 1;
  }
  for (;;) {
    int client = accept(tcp, NULL, NULL);
    if (client < 0) { if (errno == EINTR) continue; return 1; }
    setsockopt(client, IPPROTO_TCP, TCP_NODELAY, &reuse, sizeof(reuse));
    struct timeval send_timeout = {.tv_sec = 2};
    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout));
    uint8_t framed[65538];
    uint8_t *packet = framed + 2;
    /* Reconnecting viewers need live video, not datagrams queued while absent. */
    while (recv(udp, packet, 65536, MSG_DONTWAIT) > 0) {}
    for (;;) {
      struct pollfd ready[2] = {{.fd = udp, .events = POLLIN},
                                {.fd = client, .events = POLLIN}};
      int available = poll(ready, 2, 2000);
      if (available < 0) { if (errno == EINTR) continue; break; }
      if (ready[1].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) break;
      if (!(ready[0].revents & POLLIN)) continue;
      ssize_t length = recv(udp, packet, 65536, 0);
      if (length < 0 && errno == EINTR) continue;
      if (length <= 0) break;
      framed[0] = (uint16_t)length >> 8;
      framed[1] = (uint16_t)length & 255;
      if (send_all(client, framed, (size_t)length + 2)) break;
    }
    close(client);
  }
}
