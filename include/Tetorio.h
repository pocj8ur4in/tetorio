#ifndef TETORIO_TETORIO_H
#define TETORIO_TETORIO_H

#include "network/Server.h"
#include "room/RoomManager.h"
#include "session/SessionManager.h"

#include <cstdint>

namespace tetorio {

/**
 * Tetorio integrates management of network, session, and room.
 */
class Tetorio {
public:
  /**
   * constructor
   * @param port port number for server
   * @param maxConnections maximum concurrent connections for server
   */
  explicit Tetorio(uint16_t port, int maxConnections = 128);

  /**
   * destructor
   */
  ~Tetorio() = default;

  // copy constructor and assignment operator deleted to prevent copying
  Tetorio(const Tetorio &) = delete;
  Tetorio &operator=(const Tetorio &) = delete;

  /**
   * start the server
   * @return true if successful, false if failed
   */
  bool start();

  /**
   * stop the server
   */
  void stop();

  /**
   * run the event loop (blocking)
   */
  void run();

  /**
   * check if the server is running
   * @return true if running, false otherwise
   */
  bool isRunning() const { return server_.isRunning(); }

  /**
   * get server port
   * @return server port number
   */
  uint16_t getPort() const { return server_.getPort(); }

  /**
   * get session manager
   * @return reference to session manager
   */
  session::SessionManager &getSessionManager() { return sessionManager_; }

  /**
   * get room manager
   * @return reference to room manager
   */
  room::RoomManager &getRoomManager() { return roomManager_; }

  /**
   * send data to a player by player ID
   * @param playerId player ID
   * @param data pointer to data
   * @param len length of data
   * @return true if successful, false if player not found
   */
  bool sendToPlayer(uint32_t playerId, const uint8_t *data, size_t len);

  /**
   * broadcast data to all players in a room
   * @param roomId room ID
   * @param data pointer to data
   * @param len length of data
   */
  void broadcastToRoom(uint32_t roomId, const uint8_t *data, size_t len);

  /**
   * broadcast data to all connected players
   * @param data pointer to data
   * @param len length of data
   */
  void broadcastToAll(const uint8_t *data, size_t len);

private:
  /**
   * handle client connection
   * @param clientFd client socket file descriptor
   */
  void onClientConnect(int clientFd);

  /**
   * handle client disconnection
   * @param clientFd client socket file descriptor
   */
  void onClientDisconnect(int clientFd);

  /**
   * handle received data from client
   * @param clientFd client socket file descriptor
   * @param data pointer to received data
   * @param len length of received data
   */
  void onClientData(int clientFd, const uint8_t *data, size_t len);

  /**
   * handle session timeout
   * @param playerId player ID that timed out
   */
  void onSessionTimeout(uint32_t playerId);

  /**
   * process complete message from session buffer
   * @param playerId player ID
   */
  void processSessionBuffer(uint32_t playerId);

  network::Server server_;
  session::SessionManager sessionManager_;
  room::RoomManager roomManager_;
};

} // namespace tetorio

#endif // TETORIO_TETORIO_H
