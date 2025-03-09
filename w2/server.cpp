#include <enet/enet.h>
#include <iostream>

#include "codes.h"

#include <chrono>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>

struct Vec2f {
  float x = 0, y = 0;
};

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct Player {
  using IdT = enet_uint16;

  explicit Player(IdT id, ENetPeer *peer)
      : m_id(id), m_name(std::string("Client") + std::to_string(id)),
        m_peer(peer) {}

  IdT getId() const { return m_id; }
  std::string getName() const { return m_name; }
  ENetPeer *getPeer() const { return m_peer; }

public:
  Vec2f pos{};
  int ping = 0;

  TimePoint latestRequest{};

private:
  IdT m_id = 0;
  std::string m_name = "";

  ENetPeer *m_peer{};
};

struct GameServer {
  GameServer(enet_uint16 lobbyConnectorPort, enet_uint16 gameServerPort);

  ~GameServer();

  void tick();

  bool alive() const { return !m_failureReason.has_value(); }

  std::string getFailureReason() const {
    return m_failureReason.value_or("No reason");
  }

  void broadcastData();

protected:
  void fail(const std::string &reason) { m_failureReason = reason; }

private:
  void processNewClient(ENetEvent connEvent);

private:
  std::map<Player::IdT, Player> m_clients{};

  ENetAddress m_lobbyAddress{};
  ENetHost *m_lobbyHost{};

  ENetAddress m_gameServerAddress{};
  ENetHost *m_gameServerHost{};

  std::optional<std::string> m_failureReason{};
  TimePoint m_lastBroadcast{};

  static constexpr float kBroadcastDt = 1.0f / 20.0f;
};

int main(int argc, const char **argv) {
  if (enet_initialize() != 0) {
    printf("Cannot init ENet");
    return 1;
  }

  GameServer server(8080, 2004);

  while (server.alive()) {
    server.tick();
  }

  printf("Shutting down server. Reason: %s.\n",
         server.getFailureReason().c_str());

  atexit(enet_deinitialize);
  return 0;
}

GameServer::GameServer(enet_uint16 lobbyConnectorPort,
                       enet_uint16 gameServerPort) {
  m_lobbyAddress.host = ENET_HOST_ANY;
  m_lobbyAddress.port = lobbyConnectorPort;

  // Seems kinda weird that it is the game server that can have multiple lobbies
  // to it and not the other way around.
  m_lobbyHost = enet_host_create(&m_lobbyAddress, 32, 2, 0, 0);
  if (!m_lobbyHost) {
    fail("Cannot create lobby connector server");
    return;
  }

  m_gameServerAddress.host = ENET_HOST_ANY;
  m_gameServerAddress.port = gameServerPort;

  m_gameServerHost = enet_host_create(&m_gameServerAddress, 32, 2, 0, 0);
  if (!m_gameServerHost) {
    fail("Cannot create game client");
    return;
  }
}

GameServer::~GameServer() {
  enet_host_destroy(m_lobbyHost);
  enet_host_destroy(m_gameServerHost);
}

void GameServer::tick() {
  ENetEvent event;
  while (enet_host_service(m_lobbyHost, &event, 10) > 0) {
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT: {
      printf("Connection with lobby server %x:%u established\n",
             event.peer->address.host, event.peer->address.port);
      // This is an internal connection that *should* be protected.
    } break;
    case ENET_EVENT_TYPE_RECEIVE: {
      printf("Received a packet from a lobby server: '%s'\n",
             event.packet->data);

      enet_packet_destroy(event.packet);
    } break;
    default:
      break;
    };
  }

  while (enet_host_service(m_gameServerHost, &event, 10) > 0) {
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT: {
      printf("Connection with client %x:%u established\n",
             event.peer->address.host, event.peer->address.port);
      // This is an internal connection that *should* be protected.
      processNewClient(event);
    } break;
    case ENET_EVENT_TYPE_RECEIVE: {
      printf("Packet received '%s' from client %u\n", event.packet->data,
             event.peer->incomingPeerID);

      Player &player = m_clients.at(event.peer->incomingPeerID);

      float posX = 0, posY = 0;

      std::stringstream stream(std::string((const char *)event.packet->data,
                                           event.packet->dataLength));
      stream >> posX >> posY;

      auto time = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
          time - player.latestRequest);

      player.ping = duration.count();

      player.pos.x = posX;
      player.pos.y = posY;

      enet_packet_destroy(event.packet);
    } break;
    case ENET_EVENT_TYPE_DISCONNECT: {
      printf("A client has disconnected\n");
      m_clients.erase(event.peer->incomingPeerID);
    } break;
    default:
      break;
    };
  }

  TimePoint now = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now - m_lastBroadcast)
                   .count();

  if (time > kBroadcastDt) {
    broadcastData();
  }
}

void GameServer::broadcastData() {
  std::stringstream stream;
  stream << packtype::kDataUpdate << ' ' << m_clients.size() << ' ';
  for (auto &[id, player] : m_clients) {
    stream << std::fixed << std::setprecision(2) << player.getId() << ' '
           << player.pos.x << ' ' << player.pos.y << ' ' << player.ping << ' ';
  }

  std::string msg = stream.str();

  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                                          ENET_PACKET_FLAG_UNSEQUENCED);

  printf("Sending '%s' to all clients\n", msg.c_str());

  for (auto &[id, player] : m_clients) {
    enet_peer_send(player.getPeer(), 1, packet);
    player.latestRequest = std::chrono::high_resolution_clock::now();
  }
}

void GameServer::processNewClient(ENetEvent connEvent) {
  Player::IdT id = connEvent.peer->incomingPeerID;
  Player &newPlayer =
      m_clients.insert({id, Player(id, connEvent.peer)}).first->second;

  std::string msg = packtype::kSelfId + ' ' +
                    std::to_string(newPlayer.getId()) + ' ' +
                    newPlayer.getName();

  printf("Sending '%s' to the new player.\n", msg.c_str());
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                                          ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(newPlayer.getPeer(), 0, packet);

  msg = packtype::kNewClient + ' ' + std::to_string(newPlayer.getId()) + ' ' +
        newPlayer.getName();

  packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                              ENET_PACKET_FLAG_RELIABLE);

  for (auto &[id, player] : m_clients) {
    printf("Sending '%s' to an old player.\n", msg.c_str());
    enet_peer_send(player.getPeer(), 0, packet);

    std::string historyMsg = packtype::kNewClient + ' ' +
                             std::to_string(player.getId()) + ' ' +
                             player.getName();

    printf("Sending '%s' to the new player.\n", historyMsg.c_str());
    ENetPacket *historyPack = enet_packet_create(
        historyMsg.c_str(), historyMsg.length() + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(newPlayer.getPeer(), 0, historyPack);
  }
}
