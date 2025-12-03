#include "room/RoomManager.h"

#include <iostream>

namespace room {

RoomManager::RoomManager(size_t maxRooms) : maxRooms_(maxRooms) {}

uint32_t RoomManager::createRoom(const std::string &roomName,
                                 uint32_t hostPlayerId) {
  // check if maximum rooms reached
  if (isMaxRoomsReached()) {
    std::cerr << "maximum rooms reached" << std::endl;
    return 0;
  }

  // check if player is already in a room
  if (playerToRoom_.find(hostPlayerId) != playerToRoom_.end()) {
    std::cerr << "player " << hostPlayerId << " is already in a room"
              << std::endl;
    return 0;
  }

  // generate new room ID
  uint32_t roomId = generateRoomId();

  // create new room
  Room room(roomId, roomName, hostPlayerId);

  // store room and player mapping
  rooms_[roomId] = std::move(room);
  playerToRoom_[hostPlayerId] = roomId;

  std::cout << "room created: roomId=" << roomId << ", name=" << roomName
            << ", host=" << hostPlayerId << std::endl;

  // call callback
  if (roomCreatedCallback_) {
    roomCreatedCallback_(roomId);
  }

  return roomId;
}

bool RoomManager::removeRoom(uint32_t roomId) {
  auto it = rooms_.find(roomId);
  if (it == rooms_.end()) {
    return false;
  }

  // remove all players from room mapping
  for (uint32_t playerId : it->second.playerIds) {
    playerToRoom_.erase(playerId);
  }

  // remove room
  rooms_.erase(it);

  std::cout << "room removed: roomId=" << roomId << std::endl;

  // call callback
  if (roomRemovedCallback_) {
    roomRemovedCallback_(roomId);
  }

  return true;
}

Room *RoomManager::getRoom(uint32_t roomId) {
  auto it = rooms_.find(roomId);
  if (it == rooms_.end()) {
    return nullptr;
  }
  return &it->second;
}

const Room *RoomManager::getRoom(uint32_t roomId) const {
  auto it = rooms_.find(roomId);
  if (it == rooms_.end()) {
    return nullptr;
  }
  return &it->second;
}

Room *RoomManager::getRoomByPlayerId(uint32_t playerId) {
  auto it = playerToRoom_.find(playerId);
  if (it == playerToRoom_.end()) {
    return nullptr;
  }
  return getRoom(it->second);
}

const Room *RoomManager::getRoomByPlayerId(uint32_t playerId) const {
  auto it = playerToRoom_.find(playerId);
  if (it == playerToRoom_.end()) {
    return nullptr;
  }
  return getRoom(it->second);
}

uint32_t RoomManager::getRoomIdByPlayerId(uint32_t playerId) const {
  auto it = playerToRoom_.find(playerId);
  if (it == playerToRoom_.end()) {
    return 0;
  }
  return it->second;
}

bool RoomManager::joinRoom(uint32_t roomId, uint32_t playerId) {
  // check if player is already in a room
  if (playerToRoom_.find(playerId) != playerToRoom_.end()) {
    std::cerr << "player " << playerId << " is already in a room" << std::endl;
    return false;
  }

  // find room
  Room *room = getRoom(roomId);
  if (room == nullptr) {
    std::cerr << "room " << roomId << " not found" << std::endl;
    return false;
  }

  // check if room is waiting
  if (!room->isWaiting()) {
    std::cerr << "room " << roomId << " is not waiting for players"
              << std::endl;
    return false;
  }

  // add player to room
  if (!room->addPlayer(playerId)) {
    std::cerr << "failed to add player " << playerId << " to room " << roomId
              << std::endl;
    return false;
  }

  // update player mapping
  playerToRoom_[playerId] = roomId;

  std::cout << "player " << playerId << " joined room " << roomId << std::endl;

  // call callback
  if (playerJoinedCallback_) {
    playerJoinedCallback_(roomId, playerId);
  }

  return true;
}

bool RoomManager::leaveRoom(uint32_t playerId) {
  // find player's room
  auto it = playerToRoom_.find(playerId);
  if (it == playerToRoom_.end()) {
    return false;
  }

  uint32_t roomId = it->second;
  Room *room = getRoom(roomId);
  if (room == nullptr) {
    // inconsistent state, clean up
    playerToRoom_.erase(it);
    return false;
  }

  // remove player from room
  room->removePlayer(playerId);

  // remove player mapping
  playerToRoom_.erase(it);

  std::cout << "player " << playerId << " left room " << roomId << std::endl;

  // call callback
  if (playerLeftCallback_) {
    playerLeftCallback_(roomId, playerId);
  }

  // remove room if empty
  if (room->isEmpty()) {
    removeRoom(roomId);
  }

  return true;
}

bool RoomManager::startGame(uint32_t roomId, uint32_t playerId) {
  Room *room = getRoom(roomId);
  if (room == nullptr) {
    return false;
  }

  // check if player is host
  if (!room->isHost(playerId)) {
    std::cerr << "player " << playerId << " is not host of room " << roomId
              << std::endl;
    return false;
  }

  // start game
  if (!room->startGame()) {
    std::cerr << "failed to start game in room " << roomId << std::endl;
    return false;
  }

  std::cout << "game started in room " << roomId << std::endl;

  // call callback
  if (gameStartedCallback_) {
    gameStartedCallback_(roomId);
  }

  return true;
}

bool RoomManager::finishGame(uint32_t roomId) {
  Room *room = getRoom(roomId);
  if (room == nullptr) {
    return false;
  }

  room->finishGame();

  std::cout << "game finished in room " << roomId << std::endl;

  // call callback
  if (gameFinishedCallback_) {
    gameFinishedCallback_(roomId);
  }

  return true;
}

std::vector<uint32_t> RoomManager::getAllRoomIds() const {
  std::vector<uint32_t> roomIds;
  roomIds.reserve(rooms_.size());

  for (const auto &[roomId, room] : rooms_) {
    roomIds.push_back(roomId);
  }

  return roomIds;
}

std::vector<uint32_t> RoomManager::getWaitingRoomIds() const {
  std::vector<uint32_t> roomIds;

  for (const auto &[roomId, room] : rooms_) {
    if (room.isWaiting()) {
      roomIds.push_back(roomId);
    }
  }

  return roomIds;
}

uint32_t RoomManager::generateRoomId() {
  // simple incrementing ID
  return nextRoomId_++;
}

} // namespace room
