#ifndef TETORIO_NETWORK_SERVER_H
#define TETORIO_NETWORK_SERVER_H

#include "ClientBuffer.h"

#include <cstdint>
#include <functional>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

namespace network {

/**
 * ServerConfig store server configuration.
 */
struct ServerConfig {
  uint16_t port;       // server port number
  int maxConnections;  // maximum concurrent connections
  int maxEvents = 128; // maximum epoll events
};

/**
 * ServerState store server runtime state.
 */
struct ServerState {
  int serverFd = -1;    // server socket file descriptor
  int epollFd = -1;     // epoll file descriptor
  bool running = false; // server running state
};

/**
 * Server accept and manage client connections.
 */
class Server {
public:
  // callback types for server events
  using ClientConnectCallback = std::function<void(int clientFd)>;
  using ClientDisconnectCallback = std::function<void(int clientFd)>;
  using ClientDataCallback =
      std::function<void(int clientFd, const uint8_t *data, size_t len)>;

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

  /**
   * run epoll event loop blocking until server is stopped
   */
  void runEventLoop();

  /**
   * send data to client
   * @param clientFd client socket file descriptor
   * @param data pointer to data to send
   * @param len length of data to send
   * @return true if successful, false if failed
   */
  bool send(int clientFd, const uint8_t *data, size_t len);

  /**
   * broadcast data to all clients
   * @param data pointer to data to send
   * @param len length of data to send
   */
  void broadcast(const uint8_t *data, size_t len);

  /**
   * get all connected client file descriptors
   * @return vector of client file descriptors
   */
  std::vector<int> getClientFds() const;

  /**
   * set client connect callback
   * @param callback callback function
   */
  void setClientConnectCallback(ClientConnectCallback callback) {
    clientConnectCallback_ = std::move(callback);
  }

  /**
   * set client disconnect callback
   * @param callback callback function
   */
  void setClientDisconnectCallback(ClientDisconnectCallback callback) {
    clientDisconnectCallback_ = std::move(callback);
  }

  /**
   * set client data callback
   * @param callback callback function
   */
  void setClientDataCallback(ClientDataCallback callback) {
    clientDataCallback_ = std::move(callback);
  }

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

  /**
   * initialize epoll
   * @return true if successful, false if failed
   */
  bool initEpoll();

  /**
   * add client socket to epoll
   * @param clientFd client socket file descriptor
   * @return true if successful, false if failed
   */
  bool addClient(int clientFd);

  /**
   * remove client socket from epoll
   * @param clientFd client socket file descriptor
   */
  void removeClient(int clientFd);

  /**
   * handle client connection accept
   */
  void handleAccept();

  /**
   * handle client read event
   * @param clientFd client socket file descriptor
   */
  void handleRead(int clientFd);

  /**
   * handle client write event
   * @param clientFd client socket file descriptor
   */
  void handleWrite(int clientFd);

  /**
   * close client connection
   * @param clientFd client socket file descriptor
   */
  void closeClient(int clientFd);

  /**
   * enable EPOLLOUT for client socket
   * @param clientFd client socket file descriptor
   * @return true if successful, false if failed
   */
  bool enableWriteEvent(int clientFd);

  /**
   * disable EPOLLOUT for client socket
   * @param clientFd client socket file descriptor
   * @return true if successful, false if failed
   */
  bool disableWriteEvent(int clientFd);

  /**
   * set client socket options
   * @param clientFd client socket file descriptor
   * @return true if successful, false if failed
   */
  bool setClientSocketOptions(int clientFd);

  ServerConfig config_;                           // server configuration
  ServerState state_;                             // server runtime state
  std::unordered_map<int, ClientBuffer> clients_; // client fd -> send buffer

  // callbacks for server events
  ClientConnectCallback clientConnectCallback_;
  ClientDisconnectCallback clientDisconnectCallback_;
  ClientDataCallback clientDataCallback_;
};

} // namespace network

#endif // TETORIO_NETWORK_SERVER_H
