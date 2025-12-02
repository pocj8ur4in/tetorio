#include "network/Server.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace network {

Server::Server(uint16_t port, int maxConnections)
    : config_{port, maxConnections} {
  std::cout << "server constructor called" << std::endl;
}

Server::~Server() { stop(); }

bool Server::start() {
  if (state_.running) {
    std::cerr << "server is already running" << std::endl;
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

  // initialize epoll
  if (!initEpoll()) {
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

  // close all client connections
  for (auto it = clients_.begin(); it != clients_.end();) {
    closeClient(it->first);
    it = clients_.erase(it);
  }

  // close epoll file descriptor
  if (state_.epollFd >= 0) {
    close(state_.epollFd);
    state_.epollFd = -1;
  }

  // close server socket file descriptor
  if (state_.serverFd >= 0) {
    close(state_.serverFd);
    state_.serverFd = -1;
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

int Server::accept(struct sockaddr_in *clientAddr) {
  if (!state_.running || state_.serverFd < 0) {
    return -1;
  }

  struct sockaddr_in addr;
  socklen_t addrLen = sizeof(addr);
  struct sockaddr_in *addrPtr = clientAddr ? clientAddr : &addr;

  // accept client connection
  int clientFd = ::accept(
      state_.serverFd, reinterpret_cast<struct sockaddr *>(addrPtr), &addrLen);

  if (clientFd < 0) {
    // no connection available, which is normal in non-blocking mode
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return -1;
    }
    // real error occurred
    std::cerr << "failed to accept client connection: " << strerror(errno)
              << std::endl;
    return -1;
  }

  // set client socket to non-blocking mode
  if (!setNonBlocking(clientFd)) {
    close(clientFd);
    return -1;
  }

  // log client connection
  char clientIp[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addrPtr->sin_addr, clientIp, INET_ADDRSTRLEN);
  uint16_t clientPort = ntohs(addrPtr->sin_port);
  std::cout << "client connected: " << clientIp << ":" << clientPort
            << " (fd: " << clientFd << ")" << std::endl;

  return clientFd;
}

bool Server::initEpoll() {
  // create epoll instance
  state_.epollFd = epoll_create1(0);
  if (state_.epollFd < 0) {
    std::cerr << "failed to create epoll instance: " << strerror(errno)
              << std::endl;
    return false;
  }

  // add server socket to epoll
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET; // edge-triggered mode
  ev.data.fd = state_.serverFd;

  if (epoll_ctl(state_.epollFd, EPOLL_CTL_ADD, state_.serverFd, &ev) < 0) {
    std::cerr << "failed to add server socket to epoll: " << strerror(errno)
              << std::endl;
    close(state_.epollFd);
    state_.epollFd = -1;
    return false;
  }

  return true;
}

bool Server::addClient(int clientFd) {
  // register client socket to epoll
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
  ev.data.fd = clientFd;

  if (epoll_ctl(state_.epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0) {
    std::cerr << "failed to add client socket to epoll: " << strerror(errno)
              << std::endl;
    return false;
  }

  // initialize client buffer
  clients_[clientFd] = ClientBuffer{};

  return true;
}

bool Server::enableWriteEvent(int clientFd) {
  auto it = clients_.find(clientFd);
  if (it == clients_.end() || it->second.wantWrite) {
    return true;
  }

  // create epoll event to register EPOLLOUT event
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
  ev.data.fd = clientFd;

  // modify client socket to register EPOLLOUT event
  if (epoll_ctl(state_.epollFd, EPOLL_CTL_MOD, clientFd, &ev) < 0) {
    std::cerr << "failed to enable EPOLLOUT for client " << clientFd << ": "
              << strerror(errno) << std::endl;
    return false;
  }

  // set wantWrite flag true to indicate EPOLLOUT is enabled
  it->second.wantWrite = true;
  return true;
}

bool Server::disableWriteEvent(int clientFd) {
  auto it = clients_.find(clientFd);
  if (it == clients_.end() || !it->second.wantWrite) {
    return true;
  }

  // create epoll event to unregister EPOLLOUT event
  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
  ev.data.fd = clientFd;

  // modify client socket to unregister EPOLLOUT event
  if (epoll_ctl(state_.epollFd, EPOLL_CTL_MOD, clientFd, &ev) < 0) {
    std::cerr << "failed to disable EPOLLOUT for client " << clientFd << ": "
              << strerror(errno) << std::endl;
    return false;
  }

  // set wantWrite flag false to indicate EPOLLOUT is disabled
  it->second.wantWrite = false;
  return true;
}

bool Server::setClientSocketOptions(int clientFd) {
  // set TCP_NODELAY option (disable Nagle's algorithm)
  int opt = 1;
  if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
    std::cerr << "failed to set TCP_NODELAY: " << strerror(errno) << std::endl;
    return false;
  }

  return true;
}

void Server::removeClient(int clientFd) {
  // remove client socket from epoll
  epoll_ctl(state_.epollFd, EPOLL_CTL_DEL, clientFd, nullptr);

  // remove client from clients map
  clients_.erase(clientFd);
}

void Server::handleAccept() {
  // accept all pending connections in edge-triggered mode
  while (true) {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // use accept4() to set non-blocking mode atomically
    int clientFd = accept4(state_.serverFd,
                           reinterpret_cast<struct sockaddr *>(&clientAddr),
                           &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (clientFd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // no more connections available
        break;
      }

      std::cerr << "failed to accept client connection: " << strerror(errno)
                << std::endl;
      break;
    }

    // set client socket options (TCP_NODELAY)
    if (!setClientSocketOptions(clientFd)) {
      close(clientFd);
      continue;
    }

    // register client socket to epoll
    if (!addClient(clientFd)) {
      close(clientFd);
      continue;
    }

    // log client connection
    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    uint16_t clientPort = ntohs(clientAddr.sin_port);
    std::cout << "client connected: " << clientIp << ":" << clientPort
              << " (fd: " << clientFd << ", total: " << clients_.size() << ")"
              << std::endl;
  }
}

void Server::handleRead(int clientFd) {
  constexpr size_t BUFFER_SIZE = 4096;
  uint8_t buffer[BUFFER_SIZE];

  // read all available data
  while (true) {
    ssize_t n = recv(clientFd, buffer, BUFFER_SIZE, 0);

    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // no more data available, which is normal in non-blocking mode
        break;
      }

      // real error occurred
      std::cerr << "error reading from client " << clientFd << ": "
                << strerror(errno) << std::endl;
      closeClient(clientFd);
      return;
    }

    if (n == 0) {
      // connection closed by client
      std::cout << "client " << clientFd << " disconnected" << std::endl;
      closeClient(clientFd);
      return;
    }

    // TODO: process received data (parse protocol, etc.)
  }
}

void Server::handleWrite(int clientFd) {
  auto it = clients_.find(clientFd);
  if (it == clients_.end()) {
    return;
  }

  ClientBuffer &buf = it->second;
  if (buf.empty()) {
    disableWriteEvent(clientFd);
    return;
  }

  // send all buffered data using offset-based approach
  while (!buf.empty()) {
    ssize_t n = ::send(clientFd, buf.current(), buf.remaining(), MSG_NOSIGNAL);

    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // would block, wait for next epoll write event
        break;
      }

      std::cerr << "error writing to client " << clientFd << ": "
                << strerror(errno) << std::endl;
      closeClient(clientFd);
      return;
    }

    buf.offset += static_cast<size_t>(n);
  }

  // compact buffer if all data sent
  if (buf.empty()) {
    buf.data.clear();
    buf.offset = 0;
    disableWriteEvent(clientFd);
  } else if (buf.offset > 4096) {
    // compact if offset is too large to prevent memory waste
    buf.compact();
  }
}

void Server::closeClient(int clientFd) {
  removeClient(clientFd);
  close(clientFd);
  std::cout << "client " << clientFd
            << " closed (remaining: " << clients_.size() << ")" << std::endl;
}

bool Server::send(int clientFd, const uint8_t *data, size_t len) {
  auto it = clients_.find(clientFd);
  if (it == clients_.end()) {
    return false;
  }

  // append data to client buffer
  it->second.append(data, len);

  // enable write event to trigger EPOLLOUT
  return enableWriteEvent(clientFd);
}

void Server::broadcast(const uint8_t *data, size_t len) {
  for (auto &[clientFd, buffer] : clients_) {
    buffer.append(data, len);
    enableWriteEvent(clientFd);
  }
}

void Server::runEventLoop() {
  if (!state_.running) {
    std::cerr << "server is not running" << std::endl;
    return;
  }

  if (state_.epollFd < 0) {
    std::cerr << "epoll is not initialized" << std::endl;
    return;
  }

  std::vector<struct epoll_event> events(config_.maxEvents);
  std::cout << "starting event loop..." << std::endl;

  while (state_.running) {
    // wait for epoll events
    int numEvents = epoll_wait(state_.epollFd, events.data(),
                               static_cast<int>(events.size()), -1);

    if (numEvents < 0) {
      if (errno == EINTR) {
        // continue when interrupted by signal
        continue;
      }

      // real error occurred
      std::cerr << "failed to wait for epoll events: " << strerror(errno)
                << std::endl;
      break;
    }

    // process epoll events
    for (int i = 0; i < numEvents; ++i) {
      int fd = events[i].data.fd;
      uint32_t eventFlags = events[i].events;

      // handle server socket for new connections
      if (fd == state_.serverFd) {
        if (eventFlags & (EPOLLERR | EPOLLHUP)) {
          std::cerr << "server socket error" << std::endl;
          stop();
          return;
        }
        if (eventFlags & EPOLLIN) {
          handleAccept();
        }
        continue;
      }

      // skip if client already removed (by previous event in same batch)
      if (clients_.find(fd) == clients_.end()) {
        continue;
      }

      // check for client errors
      if (eventFlags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        closeClient(fd);
        continue;
      }

      // handle client read event
      if (eventFlags & EPOLLIN) {
        handleRead(fd);
        // check if client still exists after read
        if (clients_.find(fd) == clients_.end()) {
          continue;
        }
      }

      // handle client write event
      if (eventFlags & EPOLLOUT) {
        handleWrite(fd);
      }
    }
  }

  std::cout << "epoll event loop stopped" << std::endl;
}

} // namespace network
