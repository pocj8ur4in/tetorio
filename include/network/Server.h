#ifndef TETORIO_NETWORK_SERVER_H
#define TETORIO_NETWORK_SERVER_H

#include <cstdint>
#include <netinet/in.h>

namespace network {

/**
 * ServerConfig class store server configuration.
 */
struct ServerConfig {
  uint16_t port;      // server port number
  int maxConnections; // maximum concurrent connections
};

/**
 * ServerState class store server runtime state.
 */
struct ServerState {
  int serverFd; // server socket file descriptor
  bool running; // server running state
};

/**
 * server class accept and manage client connections.
 */
class Server {
public:
  /**
   * constructor
   * @param port server port number
   * @param maxConnections maximum concurrent connections
   */
  explicit Server(uint16_t port, int maxConnections = 128);

  /**
   * destructor
   */
  ~Server();

  // copy constructor and assignment operator deleted to prevent copying
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;

  /**
   * start server
   * @return true if successful, false if failed
   */
  bool start();

  /**
   * stop server
   */
  void stop();

  /**
   * check if server is running
   * @return true if running, false if not
   */
  bool isRunning() const { return state_.running; }

  /**
   * get server socket file descriptor
   * @return server socket file descriptor, -1 if failed
   */
  int getSocketFd() const { return state_.serverFd; }

  /**
   * get server port number
   * @return server port number
   */
  uint16_t getPort() const { return config_.port; }

  /**
   * accept client connection
   * @param clientAddr client address (can be nullptr)
   * @return client socket file descriptor, -1 if no connection or error
   */
  int accept(struct sockaddr_in *clientAddr = nullptr);

private:
  /**
   * create and bind socket
   * @return true if successful, false if failed
   */
  bool createAndBind();

  /**
   * start listening
   * @return true if successful, false if failed
   */
  bool listen();

  /**
   * set socket to non-blocking mode
   * @param fd socket file descriptor
   * @return true if successful, false if failed
   */
  bool setNonBlocking(int fd);

  /**
   * set socket options
   * @param fd socket file descriptor
   * @return true if successful, false if failed
   */
  bool setSocketOptions(int fd);

  ServerConfig config_; // server configuration
  ServerState state_;   // server runtime state
};

} // namespace network

#endif // TETORIO_NETWORK_SERVER_H
