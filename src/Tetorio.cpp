#include "Tetorio.h"

#include <iostream>

namespace tetorio {

Tetorio::Tetorio(uint16_t port, int maxConnections)
    : server_(port, maxConnections), sessionManager_(30), roomManager_(100) {
  // set server callbacks
  server_.setClientConnectCallback(
      [this](int clientFd) { onClientConnect(clientFd); });

  server_.setClientDisconnectCallback(
      [this](int clientFd) { onClientDisconnect(clientFd); });

  server_.setClientDataCallback(
      [this](int clientFd, const uint8_t *data, size_t len) {
        onClientData(clientFd, data, len);
      });

  // set session timeout callback
  sessionManager_.setTimeoutCallback(
      [this](uint32_t playerId) { onSessionTimeout(playerId); });

  std::cout << "tetorio initialized" << std::endl;
}

bool Tetorio::start() {
  if (!server_.start()) {
    std::cerr << "failed to start server" << std::endl;
    return false;
  }

  std::cout << "server started on port " << server_.getPort() << std::endl;
  return true;
}

void Tetorio::stop() {
  server_.stop();
  std::cout << "server stopped" << std::endl;
}

void Tetorio::run() { server_.runEventLoop(); }

bool Tetorio::sendToPlayer(uint32_t playerId, const uint8_t *data, size_t len) {
  const session::Session *session = sessionManager_.getSession(playerId);
  if (session == nullptr) {
    return false;
  }

  return server_.send(session->socketFd, data, len);
}

void Tetorio::broadcastToRoom(uint32_t roomId, const uint8_t *data,
                              size_t len) {
  std::vector<uint32_t> playerIds = sessionManager_.getPlayersInRoom(roomId);

  for (uint32_t playerId : playerIds) {
    sendToPlayer(playerId, data, len);
  }
}

void Tetorio::broadcastToAll(const uint8_t *data, size_t len) {
  server_.broadcast(data, len);
}

void Tetorio::onClientConnect(int clientFd) {
  // create session for new client
  uint32_t playerId = sessionManager_.createSession(clientFd);
  if (playerId == 0) {
    std::cerr << "failed to create session for client " << clientFd
              << std::endl;
    return;
  }

  std::cout << "session created for client " << clientFd
            << " (playerId: " << playerId << ")" << std::endl;
}

void Tetorio::onClientDisconnect(int clientFd) {
  // get player ID from session
  uint32_t playerId = sessionManager_.getPlayerIdByFd(clientFd);
  if (playerId == 0) {
    return;
  }

  // leave room if in one
  uint32_t roomId = roomManager_.getRoomIdByPlayerId(playerId);
  if (roomId != 0) {
    roomManager_.leaveRoom(playerId);
    std::cout << "player " << playerId << " left room " << roomId
              << " due to disconnect" << std::endl;
  }

  // remove session
  sessionManager_.removeSession(playerId);
  std::cout << "session removed for player " << playerId << std::endl;
}

void Tetorio::onClientData(int clientFd, const uint8_t *data, size_t len) {
  // get session by fd
  session::Session *session = sessionManager_.getSessionByFd(clientFd);
  if (session == nullptr) {
    std::cerr << "session not found for client " << clientFd << std::endl;
    return;
  }

  // update heartbeat
  session->updateHeartbeat();

  // append data to session receive buffer
  session->appendToReceiveBuffer(data, len);

  // process complete messages
  processSessionBuffer(session->playerId);
}

void Tetorio::onSessionTimeout(uint32_t playerId) {
  std::cout << "session timeout for player " << playerId << std::endl;

  // leave room if in one
  uint32_t roomId = roomManager_.getRoomIdByPlayerId(playerId);
  if (roomId != 0) {
    roomManager_.leaveRoom(playerId);
  }

  // NOTE: session will be removed by SessionManager::checkTimeouts()
}

void Tetorio::processSessionBuffer(uint32_t playerId) {
  session::Session *session = sessionManager_.getSession(playerId);
  if (session == nullptr) {
    return;
  }

  // TODO: implement message parsing based protocol
}

} // namespace tetorio
