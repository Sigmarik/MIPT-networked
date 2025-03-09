#include "bitstream.h"
#include <enet/enet.h>
#include <iostream>
#include <map>
#include <optional>

struct Client {
  using IdT = enet_uint16;

  explicit Client(IdT id, ENetPeer *peer) : m_id(id), m_peer(peer) {}
  ENetPeer *getPeer() const { return m_peer; }

  IdT getId() const { return m_id; }

private:
  IdT m_id{};
  ENetPeer *m_peer{};
};

struct LobbyServer {
  LobbyServer(enet_uint16 publicPort, const std::string &gameServerIp,
              enet_uint16 gameServerPort);

  ~LobbyServer();

  void tick();

  bool alive() const { return !m_failureReason.has_value(); }

  std::string getFailureReason() const {
    return m_failureReason.value_or("No reason");
  }

  void startTheGame();

  void sendServerInfo(Client &client);

protected:
  void fail(const std::string &reason) { m_failureReason = reason; }

private:
  void processNewClient(ENetEvent connEvent);

private:
  std::map<Client::IdT, Client> m_clients{};

  ENetAddress m_lobbyAddress{};
  ENetHost *m_lobbyHost{};

  enet_uint16 m_gameServerPublicPort = 2004;

  std::string m_gameServerIp{};
  ENetAddress m_gameServerAddress{};
  ENetHost *m_gameServerHost{};
  ENetPeer *m_gameServerPeer{};

  std::optional<std::string> m_failureReason{};

  bool m_gameStarted = false;
};

int main(int argc, const char **argv) {
  if (enet_initialize() != 0) {
    // Why such hatred towards iostream?
    printf("Cannot init ENet");
    return 1;
  }

  LobbyServer lobby(10887, "localhost", 8080);

  while (lobby.alive()) {
    lobby.tick();
  }

  printf("Terminating. Reason: %s.\n", lobby.getFailureReason().c_str());

  atexit(enet_deinitialize);
  return 0;
}

LobbyServer::LobbyServer(enet_uint16 publicPort,
                         const std::string &gameServerIp,
                         enet_uint16 gameServerPort) {
  m_lobbyAddress.host = ENET_HOST_ANY;
  m_lobbyAddress.port = publicPort;

  m_lobbyHost = enet_host_create(&m_lobbyAddress, 32, 2, 0, 0);
  if (!m_lobbyHost) {
    fail("Cannot create ENet server");
    return;
  }

  m_gameServerHost = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!m_gameServerHost) {
    fail("Cannot create ENet client");
    return;
  }

  m_gameServerIp = gameServerIp;

  enet_address_set_host(&m_gameServerAddress, gameServerIp.c_str());
  m_gameServerAddress.port = gameServerPort;

  m_gameServerPeer =
      enet_host_connect(m_gameServerHost, &m_gameServerAddress, 2, 0);
  if (!m_gameServerPeer) {
    fail("Cannot connect to the game server");
    return;
  }
}

LobbyServer::~LobbyServer() {
  enet_host_destroy(m_lobbyHost);
  enet_host_destroy(m_gameServerHost);
}

void LobbyServer::tick() {
  ENetEvent event;
  while (enet_host_service(m_lobbyHost, &event, 10) > 0) {
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT: {
      printf("Connection with %x:%u established\n", event.peer->address.host,
             event.peer->address.port);
      processNewClient(event);
    } break;
    case ENET_EVENT_TYPE_RECEIVE: {
      printf("Packet received '%s'\n", event.packet->data);

      // Assume this is a call to start the game regardless of the message
      // content
      startTheGame();

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
}

void LobbyServer::startTheGame() {
  m_gameStarted = true;

  for (auto &[id, client] : m_clients) {
    sendServerInfo(client);
  }
}

void LobbyServer::sendServerInfo(Client &client) {
  std::stringstream serverStream;
  serverStream << client.getId();

  std::string msg = serverStream.str();
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                                          ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(m_gameServerPeer, 0, packet);

  msg = m_gameServerIp + ' ' + std::to_string(m_gameServerPublicPort) + '\0';
  packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                              ENET_PACKET_FLAG_RELIABLE);

  enet_peer_send(client.getPeer(), 0, packet);
}

void LobbyServer::processNewClient(ENetEvent event) {
  Client::IdT id = event.peer->incomingPeerID;
  Client &newClient =
      m_clients.insert({id, Client(id, event.peer)}).first->second;

  if (m_gameStarted) {
    sendServerInfo(newClient);
  }
}
