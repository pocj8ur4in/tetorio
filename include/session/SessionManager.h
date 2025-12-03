#ifndef TETORIO_SESSION_SESSION_MANAGER_H
#define TETORIO_SESSION_SESSION_MANAGER_H

#include "Session.h"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace session {

/**
 * SessionManager manages all sessions.
 */
class SessionManager {
public:
  // callback type for session events
  using SessionCallback = std::function<void(uint32_t playerId)>;

  /**
   * constructor
   * @param heartbeatTimeout heartbeat timeout in seconds (default: 30)
   */
  explicit SessionManager(int heartbeatTimeout = 30);

  /**
   * destructor
   */
  ~SessionManager() = default;

  // copy constructor and assignment operator deleted to prevent copying
  SessionManager(const SessionManager &) = delete;
  SessionManager &operator=(const SessionManager &) = delete;

  /**
   * create a new session for a client
   * @param socketFd client socket file descriptor
   * @return new player ID, 0 if failed
   */
  uint32_t createSession(int socketFd);

  /**
   * remove a session by player ID
   * @param playerId player ID to remove
   * @return true if removed, false if not found
   */
  bool removeSession(uint32_t playerId);

  /**
   * remove a session by socket file descriptor
   * @param socketFd socket file descriptor
   * @return true if removed, false if not found
   */
  bool removeSessionByFd(int socketFd);

  /**
   * get session by player ID
   * @param playerId player ID
   * @return pointer to session, nullptr if not found
   */
  Session *getSession(uint32_t playerId);

  /**
   * get session by player ID for const
   * @param playerId player ID
   * @return pointer to session, nullptr if not found
   */
  const Session *getSession(uint32_t playerId) const;

  /**
   * get session by socket file descriptor
   * @param socketFd socket file descriptor
   * @return pointer to session, nullptr if not found
   */
  Session *getSessionByFd(int socketFd);

  /**
   * get session by socket file descriptor for const
   * @param socketFd socket file descriptor
   * @return pointer to session, nullptr if not found
   */
  const Session *getSessionByFd(int socketFd) const;

  /**
   * get player ID by socket file descriptor
   * @param socketFd socket file descriptor
   * @return player ID, 0 if not found
   */
  uint32_t getPlayerIdByFd(int socketFd) const;

  /**
   * authenticate a player
   * @param playerId player ID
   * @return true if authenticated, false if session not found
   */
  bool authenticate(uint32_t playerId);

  /**
   * update heartbeat for a player
   * @param playerId player ID
   * @return true if updated, false if session not found
   */
  bool updateHeartbeat(uint32_t playerId);

  /**
   * check and remove timed out sessions
   * @return vector of timed out player IDs
   */
  std::vector<uint32_t> checkTimeouts();

  /**
   * set player's room ID
   * @param playerId player ID
   * @param roomId room ID
   * @return true if set, false if session not found
   */
  bool setPlayerRoom(uint32_t playerId, uint32_t roomId);

  /**
   * get all sessions in a room
   * @param roomId room ID
   * @return vector of player IDs in the room
   */
  std::vector<uint32_t> getPlayersInRoom(uint32_t roomId) const;

  /**
   * get all authenticated player IDs
   * @return vector of authenticated player IDs
   */
  std::vector<uint32_t> getAuthenticatedPlayers() const;

  /**
   * get total session count
   * @return number of sessions
   */
  size_t getSessionCount() const { return sessions_.size(); }

  /**
   * get authenticated session count
   * @return number of authenticated sessions
   */
  size_t getAuthenticatedCount() const;

  /**
   * set timeout callback (called when session times out)
   * @param callback callback function
   */
  void setTimeoutCallback(SessionCallback callback) {
    timeoutCallback_ = std::move(callback);
  }

private:
  /**
   * generate a new unique player ID
   * @return new player ID
   */
  uint32_t generatePlayerId();

  std::unordered_map<uint32_t, Session> sessions_; // playerId -> session
  std::unordered_map<int, uint32_t> fdToPlayerId_; // socketFd -> playerId
  uint32_t nextPlayerId_ = 1;                      // next player ID to assign
  int heartbeatTimeout_;            // heartbeat timeout in seconds
  SessionCallback timeoutCallback_; // callback for timeout events
};

} // namespace session

#endif // TETORIO_SESSION_SESSION_MANAGER_H
