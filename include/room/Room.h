#ifndef TETORIO_ROOM_ROOM_H
#define TETORIO_ROOM_ROOM_H

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace room {

/**
 * GameState store current game room state.
 */
enum class GameState : uint8_t {
  WAITING = 0,  // waiting for players
  PLAYING = 1,  // game in progress
  FINISHED = 2, // game finished
};

/**
 * Room stores game room information and player list.
 */
struct Room {
  uint32_t roomId = 0;                      // unique room ID
  std::string roomName;                     // room name
  uint32_t hostPlayerId = 0;                // host player ID
  std::vector<uint32_t> playerIds;          // player IDs in the room
  GameState gameState = GameState::WAITING; // current game state
  uint8_t maxPlayers = 32;                  // maximum players
  time_t createdAt = 0;                     // room creation timestamp
  time_t startedAt = 0;                     // game start timestamp

  /**
   * default constructor
   */
  Room() = default;

  /**
   * constructor
   * @param id room ID
   * @param name room name
   * @param hostId host player ID
   */
  Room(uint32_t id, const std::string &name, uint32_t hostId)
      : roomId(id), roomName(name), hostPlayerId(hostId),
        createdAt(std::time(nullptr)) {
    playerIds.push_back(hostId);
  }

  /**
   * check if room is full
   * @return true if full, false otherwise
   */
  bool isFull() const {
    return playerIds.size() >= static_cast<size_t>(maxPlayers);
  }

  /**
   * check if room is empty
   * @return true if empty, false otherwise
   */
  bool isEmpty() const { return playerIds.empty(); }

  /**
   * get count of current players in the room
   * @return number of players in the room
   */
  size_t getPlayerCount() const { return playerIds.size(); }

  /**
   * check if player is in the room
   * @param playerId player ID to check
   * @return true if player is in the room, false otherwise
   */
  bool hasPlayer(uint32_t playerId) const {
    for (uint32_t id : playerIds) {
      if (id == playerId) {
        return true;
      }
    }
    return false;
  }

  /**
   * check if player is the host
   * @param playerId player ID to check
   * @return true if player is the host, false otherwise
   */
  bool isHost(uint32_t playerId) const { return hostPlayerId == playerId; }

  /**
   * check if game is in progress
   * @return true if playing, false otherwise
   */
  bool isPlaying() const { return gameState == GameState::PLAYING; }

  /**
   * check if room is waiting for players
   * @return true if waiting, false otherwise
   */
  bool isWaiting() const { return gameState == GameState::WAITING; }

  /**
   * add player to the room
   * @param playerId player ID to add
   * @return true if added, false if room is full or player already in room
   */
  bool addPlayer(uint32_t playerId) {
    if (isFull() || hasPlayer(playerId)) {
      return false;
    }
    playerIds.push_back(playerId);
    return true;
  }

  /**
   * remove player from the room
   * @param playerId player ID to remove
   * @return true if removed, false if player not found
   */
  bool removePlayer(uint32_t playerId) {
    for (auto it = playerIds.begin(); it != playerIds.end(); ++it) {
      if (*it == playerId) {
        playerIds.erase(it);

        // assign new host to first player if host left
        if (hostPlayerId == playerId && !playerIds.empty()) {
          hostPlayerId = playerIds.front();
        }
        return true;
      }
    }
    return false;
  }

  /**
   * start the game
   * @return true if started, false if not enough players or already started
   */
  bool startGame() {
    if (gameState != GameState::WAITING || playerIds.size() < 2) {
      return false;
    }
    gameState = GameState::PLAYING;
    startedAt = std::time(nullptr);
    return true;
  }

  /**
   * finish the game
   */
  void finishGame() { gameState = GameState::FINISHED; }

  /**
   * reset room to waiting state
   */
  void reset() {
    gameState = GameState::WAITING;
    startedAt = 0;
  }
};

} // namespace room

#endif // TETORIO_ROOM_ROOM_H
