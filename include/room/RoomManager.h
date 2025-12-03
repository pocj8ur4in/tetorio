#ifndef TETORIO_ROOM_ROOM_MANAGER_H
#define TETORIO_ROOM_ROOM_MANAGER_H

#include "Room.h"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace room {

/**
 * RoomManager manages all game rooms.
 */
class RoomManager {
public:
  // callback types for room events
  using RoomCallback = std::function<void(uint32_t roomId)>;
  using PlayerRoomCallback =
      std::function<void(uint32_t roomId, uint32_t playerId)>;

  /**
   * constructor
   * @param maxRooms maximum number of rooms (default: 100)
   */
  explicit RoomManager(size_t maxRooms = 100);

  /**
   * destructor
   */
  ~RoomManager() = default;

  // copy constructor and assignment operator deleted to prevent copying
  RoomManager(const RoomManager &) = delete;
  RoomManager &operator=(const RoomManager &) = delete;

  /**
   * create a new room
   * @param roomName room name
   * @param hostPlayerId host player ID
   * @return new room ID, 0 if failed
   */
  uint32_t createRoom(const std::string &roomName, uint32_t hostPlayerId);

  /**
   * remove a room by room ID
   * @param roomId room ID to remove
   * @return true if removed, false if not found
   */
  bool removeRoom(uint32_t roomId);

  /**
   * get room by room ID
   * @param roomId room ID
   * @return pointer to room, nullptr if not found
   */
  Room *getRoom(uint32_t roomId);

  /**
   * get room by room ID for const
   * @param roomId room ID
   * @return pointer to room, nullptr if not found
   */
  const Room *getRoom(uint32_t roomId) const;

  /**
   * get room by player ID
   * @param playerId player ID
   * @return pointer to room, nullptr if player is not in any room
   */
  Room *getRoomByPlayerId(uint32_t playerId);

  /**
   * get room by player ID for const
   * @param playerId player ID
   * @return pointer to room, nullptr if player is not in any room
   */
  const Room *getRoomByPlayerId(uint32_t playerId) const;

  /**
   * get room ID by player ID
   * @param playerId player ID
   * @return room ID, 0 if player is not in any room
   */
  uint32_t getRoomIdByPlayerId(uint32_t playerId) const;

  /**
   * join a room
   * @param roomId room ID to join
   * @param playerId player ID
   * @return true if joined, false if room not found, full, or player already in
   * room
   */
  bool joinRoom(uint32_t roomId, uint32_t playerId);

  /**
   * leave a room
   * @param playerId player ID
   * @return true if left, false if player not in any room
   */
  bool leaveRoom(uint32_t playerId);

  /**
   * start game in a room
   * @param roomId room ID
   * @param playerId player ID
   * @return true if started, false if not host or cannot start
   */
  bool startGame(uint32_t roomId, uint32_t playerId);

  /**
   * finish game in a room
   * @param roomId room ID
   * @return true if finished, false if room not found
   */
  bool finishGame(uint32_t roomId);

  /**
   * get all room IDs
   * @return vector of room IDs
   */
  std::vector<uint32_t> getAllRoomIds() const;

  /**
   * get all waiting room IDs
   * @return vector of waiting room IDs
   */
  std::vector<uint32_t> getWaitingRoomIds() const;

  /**
   * get total room count
   * @return number of rooms
   */
  size_t getRoomCount() const { return rooms_.size(); }

  /**
   * check if maximum rooms reached
   * @return true if maximum reached, false otherwise
   */
  bool isMaxRoomsReached() const { return rooms_.size() >= maxRooms_; }

  /**
   * set room created callback
   * @param callback callback function
   */
  void setRoomCreatedCallback(RoomCallback callback) {
    roomCreatedCallback_ = std::move(callback);
  }

  /**
   * set room removed callback
   * @param callback callback function
   */
  void setRoomRemovedCallback(RoomCallback callback) {
    roomRemovedCallback_ = std::move(callback);
  }

  /**
   * set player joined callback
   * @param callback callback function
   */
  void setPlayerJoinedCallback(PlayerRoomCallback callback) {
    playerJoinedCallback_ = std::move(callback);
  }

  /**
   * set player left callback
   * @param callback callback function
   */
  void setPlayerLeftCallback(PlayerRoomCallback callback) {
    playerLeftCallback_ = std::move(callback);
  }

  /**
   * set game started callback
   * @param callback callback function
   */
  void setGameStartedCallback(RoomCallback callback) {
    gameStartedCallback_ = std::move(callback);
  }

  /**
   * set game finished callback
   * @param callback callback function
   */
  void setGameFinishedCallback(RoomCallback callback) {
    gameFinishedCallback_ = std::move(callback);
  }

private:
  /**
   * generate a new unique room ID
   * @return new room ID
   */
  uint32_t generateRoomId();

  std::unordered_map<uint32_t, Room> rooms_;            // roomId -> room
  std::unordered_map<uint32_t, uint32_t> playerToRoom_; // playerId -> roomId
  uint32_t nextRoomId_ = 1; // next room ID to assign
  size_t maxRooms_;         // maximum number of rooms

  // callbacks for room events
  RoomCallback roomCreatedCallback_;
  RoomCallback roomRemovedCallback_;
  PlayerRoomCallback playerJoinedCallback_;
  PlayerRoomCallback playerLeftCallback_;
  RoomCallback gameStartedCallback_;
  RoomCallback gameFinishedCallback_;
};

} // namespace room

#endif // TETORIO_ROOM_ROOM_MANAGER_H
