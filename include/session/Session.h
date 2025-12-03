#ifndef TETORIO_SESSION_SESSION_H
#define TETORIO_SESSION_SESSION_H

#include <cstdint>
#include <ctime>
#include <vector>

namespace session {

/**
 * Session stores player connection and state information.
 */
struct Session {
  int socketFd = -1;                  // socket file descriptor
  uint32_t playerId = 0;              // unique player ID
  uint32_t roomId = 0;                // current room ID
  time_t lastHeartbeat = 0;           // last heartbeat timestamp
  std::vector<uint8_t> receiveBuffer; // receive buffer for incomplete messages
  bool isAuthenticated = false;       // authentication status

  /**
   * default constructor
   */
  Session() = default;

  /**
   * constructor
   * @param fd socket file descriptor
   */
  explicit Session(int fd) : socketFd(fd), lastHeartbeat(std::time(nullptr)) {}

  /**
   * check if player is in a room
   * @return true if in room, false otherwise
   */
  bool isInRoom() const { return roomId != 0; }

  /**
   * update heartbeat timestamp to current time
   */
  void updateHeartbeat() { lastHeartbeat = std::time(nullptr); }

  /**
   * check if heartbeat has timed out
   * @param timeoutSeconds timeout in seconds
   * @return true if timed out, false otherwise
   */
  bool isHeartbeatTimeout(int timeoutSeconds) const {
    return std::difftime(std::time(nullptr), lastHeartbeat) > timeoutSeconds;
  }

  /**
   * append data to receive buffer
   * @param data pointer to data
   * @param len length of data
   */
  void appendToReceiveBuffer(const uint8_t *data, size_t len) {
    receiveBuffer.insert(receiveBuffer.end(), data, data + len);
  }

  /**
   * remove processed data from receive buffer
   * @param len number of bytes to remove from front
   */
  void consumeReceiveBuffer(size_t len) {
    if (len >= receiveBuffer.size()) {
      receiveBuffer.clear();
    } else {
      receiveBuffer.erase(receiveBuffer.begin(),
                          receiveBuffer.begin() + static_cast<ptrdiff_t>(len));
    }
  }

  /**
   * clear receive buffer
   */
  void clearReceiveBuffer() { receiveBuffer.clear(); }

  /**
   * reset session state to initial state
   */
  void reset() {
    playerId = 0;
    roomId = 0;
    receiveBuffer.clear();
    isAuthenticated = false;
    lastHeartbeat = std::time(nullptr);
  }
};

} // namespace session

#endif // TETORIO_SESSION_SESSION_H
