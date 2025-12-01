#include "network/Server.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace {
std::atomic<bool> g_running{true};
network::Server *g_server = nullptr;

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    std::cout << "\nreceived termination signal: shutting down server..."
              << std::endl;
    g_running = false;
    if (g_server) {
      g_server->stop();
    }
  }
}
} // namespace

int main(int argc, char *argv[]) {
  // default value
  uint16_t port = 10000;

  // process command line argument
  if (argc > 1) {
    try {
      int portArg = std::stoi(argv[1]);
      if (portArg < 1 || portArg > 65535) {
        std::cerr << "port number must be between 1 and 65535" << std::endl;
        return 1;
      }
      port = static_cast<uint16_t>(portArg);
    } catch (const std::exception &e) {
      std::cerr << "invalid port number: " << argv[1] << std::endl;
      return 1;
    }
  }

  // register signal handler
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  // create and start server
  network::Server server(port);
  g_server = &server;

  if (!server.start()) {
    std::cerr << "failed to start server" << std::endl;
    return 1;
  }

  std::cout << "server is ready to accept connections" << std::endl;

  while (g_running && server.isRunning()) {
    // accept client connections
    int clientFd = server.accept();
    if (clientFd >= 0) {
      // TODO: handle client connection (store in session manager, etc.)
      close(clientFd);
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
      // real error occurred
      std::cerr << "error accepting connection" << std::endl;
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}
