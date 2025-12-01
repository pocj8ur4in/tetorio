#include "network/Server.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace network {

Server::Server(uint16_t port, int maxConnections)
    : config_{port, maxConnections}, state_{-1, false} {
  std::cout << "server constructor called" << std::endl;
}

Server::~Server() { stop(); }

bool Server::start() {
  if (state_.running) {
    std::cerr << "server is already running." << std::endl;
    return false;
  }

  // create and bind socket
  if (!createAndBind()) {
    return false;
  }

  // start listening
  if (!listen()) {
    close(state_.serverFd);
    state_.serverFd = -1;
    return false;
  }

  state_.running = true;
  std::cout << "server is listening on port " << config_.port << std::endl;
  return true;
}

void Server::stop() {
  if (!state_.running) {
    return;
  }

  if (state_.serverFd >= 0) {
    close(state_.serverFd);
    state_.serverFd = -1;
    std::cout << "server has been stopped." << std::endl;
  }

  state_.running = false;
}

bool Server::createAndBind() {
  // create socket
  state_.serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (state_.serverFd < 0) {
    std::cerr << "failed to create socket: " << strerror(errno) << std::endl;
    return false;
  }

  // set socket options
  if (!setSocketOptions(state_.serverFd)) {
    close(state_.serverFd);
    state_.serverFd = -1;
    return false;
  }

  // set non-blocking mode
  if (!setNonBlocking(state_.serverFd)) {
    close(state_.serverFd);
    state_.serverFd = -1;
    return false;
  }

  // set address structure
  struct sockaddr_in serverAddr;
  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY; // receive from all interfaces
  serverAddr.sin_port = htons(config_.port);

  // bind
  if (bind(state_.serverFd, reinterpret_cast<struct sockaddr *>(&serverAddr),
           sizeof(serverAddr)) < 0) {
    std::cerr << "failed to bind port " << config_.port << ": "
              << strerror(errno) << std::endl;
    return false;
  }

  std::cout << "socket has been bound to port " << config_.port << std::endl;
  return true;
}

bool Server::listen() {
  // start listening
  if (::listen(state_.serverFd, config_.maxConnections) < 0) {
    std::cerr << "failed to listen: " << strerror(errno) << std::endl;
    return false;
  }

  return true;
}

bool Server::setNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    std::cerr << "failed to get socket flags: " << strerror(errno) << std::endl;
    return false;
  }

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    std::cerr << "failed to set socket to non-blocking mode: "
              << strerror(errno) << std::endl;
    return false;
  }

  return true;
}

bool Server::setSocketOptions(int fd) {
  // set SO_REUSEADDR option to allow port reuse for socket in TIME_WAIT state
  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    std::cerr << "failed to set SO_REUSEADDR option: " << strerror(errno)
              << std::endl;
    return false;
  }

// set SO_REUSEPORT option to allow multiple sockets to bind and listen the same
// port (only available on linux)
#ifdef SO_REUSEPORT
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    // ignore this error if it fails
    std::cerr << "failed to set SO_REUSEPORT option: " << strerror(errno)
              << std::endl;
  }
#endif

  return true;
}

} // namespace network
