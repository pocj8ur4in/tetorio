#include "session/SessionManager.h"

#include <iostream>

namespace session {

SessionManager::SessionManager(int heartbeatTimeout)
    : heartbeatTimeout_(heartbeatTimeout) {}

uint32_t SessionManager::createSession(int socketFd) {
  // check if socket already has a session
  if (fdToPlayerId_.find(socketFd) != fdToPlayerId_.end()) {
    std::cerr << "session already exists for socket " << socketFd << std::endl;
    return 0;
  }

  // generate new player ID
  uint32_t playerId = generatePlayerId();

  // create new session
  Session session(socketFd);
  session.playerId = playerId;

  // store session
  sessions_[playerId] = std::move(session);
  fdToPlayerId_[socketFd] = playerId;

  std::cout << "session created: playerId=" << playerId << ", fd=" << socketFd
            << std::endl;

  return playerId;
}

bool SessionManager::removeSession(uint32_t playerId) {
  auto it = sessions_.find(playerId);
  if (it == sessions_.end()) {
    return false;
  }

  // remove from fd map
  int socketFd = it->second.socketFd;
  fdToPlayerId_.erase(socketFd);

  // remove session
  sessions_.erase(it);

  std::cout << "session removed: playerId=" << playerId << ", fd=" << socketFd
            << std::endl;

  return true;
}

bool SessionManager::removeSessionByFd(int socketFd) {
  auto it = fdToPlayerId_.find(socketFd);
  if (it == fdToPlayerId_.end()) {
    return false;
  }

  uint32_t playerId = it->second;
  return removeSession(playerId);
}

Session *SessionManager::getSession(uint32_t playerId) {
  auto it = sessions_.find(playerId);
  if (it == sessions_.end()) {
    return nullptr;
  }
  return &it->second;
}

const Session *SessionManager::getSession(uint32_t playerId) const {
  auto it = sessions_.find(playerId);
  if (it == sessions_.end()) {
    return nullptr;
  }
  return &it->second;
}

Session *SessionManager::getSessionByFd(int socketFd) {
  auto it = fdToPlayerId_.find(socketFd);
  if (it == fdToPlayerId_.end()) {
    return nullptr;
  }
  return getSession(it->second);
}

const Session *SessionManager::getSessionByFd(int socketFd) const {
  auto it = fdToPlayerId_.find(socketFd);
  if (it == fdToPlayerId_.end()) {
    return nullptr;
  }
  return getSession(it->second);
}

uint32_t SessionManager::getPlayerIdByFd(int socketFd) const {
  auto it = fdToPlayerId_.find(socketFd);
  if (it == fdToPlayerId_.end()) {

    return 0;
  }
  return it->second;
}

bool SessionManager::authenticate(uint32_t playerId) {
  Session *session = getSession(playerId);
  if (session == nullptr) {
    return false;
  }

  session->isAuthenticated = true;
  session->updateHeartbeat();

  std::cout << "player authenticated: playerId=" << playerId << std::endl;

  return true;
}

bool SessionManager::updateHeartbeat(uint32_t playerId) {
  Session *session = getSession(playerId);
  if (session == nullptr) {
    return false;
  }

  session->updateHeartbeat();
  return true;
}

std::vector<uint32_t> SessionManager::checkTimeouts() {
  std::vector<uint32_t> timedOut;

  // check if any session has timed out
  for (auto &[playerId, session] : sessions_) {
    if (session.isHeartbeatTimeout(heartbeatTimeout_)) {
      timedOut.push_back(playerId);
    }
  }

  // call timeout callback and remove timed out sessions
  for (uint32_t playerId : timedOut) {
    std::cout << "session timed out: playerId=" << playerId << std::endl;

    if (timeoutCallback_) {
      timeoutCallback_(playerId);
    }

    removeSession(playerId);
  }

  return timedOut;
}

bool SessionManager::setPlayerRoom(uint32_t playerId, uint32_t roomId) {
  Session *session = getSession(playerId);
  if (session == nullptr) {
    return false;
  }

  session->roomId = roomId;
  return true;
}

std::vector<uint32_t> SessionManager::getPlayersInRoom(uint32_t roomId) const {
  std::vector<uint32_t> players;

  // find all players in the room
  for (const auto &[playerId, session] : sessions_) {
    if (session.roomId == roomId) {
      players.push_back(playerId);
    }
  }

  return players;
}

std::vector<uint32_t> SessionManager::getAuthenticatedPlayers() const {
  std::vector<uint32_t> players;

  for (const auto &[playerId, session] : sessions_) {
    if (session.isAuthenticated) {
      players.push_back(playerId);
    }
  }

  return players;
}

size_t SessionManager::getAuthenticatedCount() const {
  size_t count = 0;

  for (const auto &[playerId, session] : sessions_) {
    if (session.isAuthenticated) {
      ++count;
    }
  }

  return count;
}

uint32_t SessionManager::generatePlayerId() {
  // simple incrementing ID (could be improved with UUID or random generation)
  return nextPlayerId_++;
}

} // namespace session
