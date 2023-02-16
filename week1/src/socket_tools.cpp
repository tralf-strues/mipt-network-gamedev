#include "socket_tools.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

// Adaptation of linux man page: https://linux.die.net/man/3/getaddrinfo
static int ChooseDgramSocket(addrinfo* addr_options, bool should_bind, addrinfo* res_addr) {
  for (addrinfo* addr = addr_options; addr != nullptr; addr = addr->ai_next) {
    if (addr->ai_family != AF_INET || addr->ai_socktype != SOCK_DGRAM || addr->ai_protocol != IPPROTO_UDP) {
      continue;
    }

    int sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);  // Create socket
    if (sfd == -1) {
      continue;
    }

    fcntl(sfd, F_SETFL, O_NONBLOCK);

    int true_val = 1;
    setsockopt(sfd, /*level=*/SOL_SOCKET, /*optname=*/SO_REUSEADDR, /*optval=*/&true_val, sizeof(int));

    if (res_addr) {
      *res_addr = *addr;
    }

    if (!should_bind) {
      return sfd;
    }

    if (bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) {
      return sfd;
    }

    close(sfd);
  }

  return -1;
}

int CreateDgramSocket(const char* address, const char* port, addrinfo* res_addr) {
  addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));

  bool is_listener = !address;

  hints.ai_family = AF_INET;  // use IPv4 (rather than IPv6)
  hints.ai_socktype = SOCK_DGRAM;  // use UDP (rather than TCP)
  if (is_listener) {
    hints.ai_flags = AI_PASSIVE;
  }

  addrinfo* result = nullptr;
  if (getaddrinfo(address, port, &hints, &result) != 0) {
    return 1;
  }

  int sfd = ChooseDgramSocket(result, is_listener, res_addr);
 
  // freeaddrinfo(result);

  return sfd;
}
