#include "raylib.h"
#include <enet/enet.h>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string.h>
#include <string>

#include "codes.h"

struct Vec2f {
  float x = 0, y = 0;
};

struct Player {
  using IdT = enet_uint16;

  Vec2f pos{};
  int ping = 0;

  bool alive = false;

public:
  Player(IdT id, const std::string &name) : m_id(id), m_name(name) {}

  IdT getId() const { return m_id; }
  std::string getName() const { return m_name; }

private:
  IdT m_id = 0;
  std::string m_name{};
};

struct Client {
  Client(const char *ip, enet_uint16 lobbyPort);

  void tick();

  bool alive() const { return !m_failureReason.has_value(); }

  std::string getFailureReason() const {
    return m_failureReason.value_or("No reason");
  }

  void forEachPlayer(std::function<void(Player &)> functor);
  void forThisPlayer(std::function<void(Player &)> functor);

  void fail(const std::string &reason) { m_failureReason = reason; }

  void sendStartRequest() const;

private:
  ENetPeer *getLobbyPeer() const { return m_lobbyPeer; }
  ENetPeer *getGamePeer() const { return m_gameServerPeer; }

  void connectToGameServer(const char *ip, enet_uint16 lobbyPort);

  void tickLobby();
  void tickGame();

private:
  std::optional<std::string> m_failureReason{};

  std::map<Player::IdT, Player> m_players{};

  Player::IdT m_selfId = 0;
  std::string m_selfName = "Unnamed";

  ENetAddress m_lobbyAddress{};
  ENetHost *m_lobbyHost{};
  ENetPeer *m_lobbyPeer{};

  bool m_inGame = false;
  ENetAddress m_gameServerAddress{};
  ENetHost *m_gameServerHost{};
  ENetPeer *m_gameServerPeer{};
};

int main(int argc, const char **argv) {
  int width = 800;
  int height = 600;
  InitWindow(width, height, "w2 MIPT networked");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height) {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  if (enet_initialize() != 0) {
    printf("Cannot init ENet");
    return 1;
  }

  Client client("localhost", 10887);

  uint32_t timeStart = enet_time_get();
  uint32_t lastFragmentedSendTime = timeStart;
  uint32_t lastMicroSendTime = timeStart;
  bool connected = false;
  float posx = GetRandomValue(100, 1000);
  float posy = GetRandomValue(100, 500);
  float velx = 0.f;
  float vely = 0.f;
  while (!WindowShouldClose() && client.alive()) {
    const float dt = GetFrameTime();

    client.tick();

    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);
    constexpr float accel = 30.f;
    velx += ((left ? -1.f : 0.f) + (right ? 1.f : 0.f)) * dt * accel;
    vely += ((up ? -1.f : 0.f) + (down ? 1.f : 0.f)) * dt * accel;
    posx += velx * dt;
    posy += vely * dt;
    velx *= 0.99f;
    vely *= 0.99f;

    if (IsKeyDown(KEY_ENTER)) {
      client.sendStartRequest();
    }

    client.forThisPlayer(
        [&](Player &player) { player.pos.x = posx, player.pos.y = posy; });

    // Should a player disappear its visual representation will not vanish from
    // client's screens... Eh, the fix would require an additional type of
    // packages, which I am too lazy to add.
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText(TextFormat("Current status: %s", "unknown"), 20, 20, 20, WHITE);
    DrawText(TextFormat("My position: (%d, %d)", (int)posx, (int)posy), 20, 40,
             20, WHITE);
    DrawText("List of players:", 20, 60, 20, WHITE);

    unsigned textPosY = 80;
    client.forEachPlayer([&](Player &player) {
      DrawText((std::to_string(player.ping) + " - " + player.getName()).c_str(),
               20, textPosY, 20, GREEN);
      textPosY += 20;
    });
    client.forEachPlayer([&](Player &player) {
      DrawCircleV(Vector2{player.pos.x, player.pos.y}, 10.f, WHITE);
    });
    client.forThisPlayer([&](Player &player) {
      DrawCircleV(Vector2{player.pos.x, player.pos.y}, 5.f, RED);
    });

    EndDrawing();
  }

  printf("Shutting down. Reason: %s\n", client.getFailureReason().c_str());

  return 0;
}

Client::Client(const char *ip, enet_uint16 lobbyPort) {
  m_lobbyHost = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!m_lobbyHost) {
    fail("Cannot create ENet client\n");
    return;
  }

  enet_address_set_host(&m_lobbyAddress, ip);
  m_lobbyAddress.port = lobbyPort;

  m_lobbyPeer = enet_host_connect(m_lobbyHost, &m_lobbyAddress, 2, 0);
  if (!m_lobbyPeer) {
    fail("Cannot connect to lobby");
    return;
  }
}

void Client::tick() {
  tickLobby();
  tickGame();
}

void Client::forEachPlayer(std::function<void(Player &)> functor) {
  for (auto &[id, player] : m_players) {
    if (!player.alive)
      continue;
    functor(player);
  }
}

void Client::forThisPlayer(std::function<void(Player &)> functor) {
  if (m_inGame && m_players.contains(m_selfId))
    functor(m_players.at(m_selfId));
}

void Client::sendStartRequest() const {
  if (m_selfId)
    return;
  std::string msg = "/Start, please?";
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                                          ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(m_lobbyPeer, 0, packet);
}

void Client::connectToGameServer(const char *ip, enet_uint16 port) {
  m_gameServerHost = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!m_gameServerHost) {
    fail("Cannot create ENet client\n");
    return;
  }

  enet_address_set_host(&m_gameServerAddress, ip);
  m_gameServerAddress.port = port;

  m_gameServerPeer =
      enet_host_connect(m_gameServerHost, &m_gameServerAddress, 2, 0);
  if (!m_gameServerPeer) {
    fail("Cannot connect to the game server");
    return;
  }

  printf("Connected to the game server.\n");
  m_inGame = true;
}

void Client::tickLobby() {
  if (m_inGame)
    return;

  ENetEvent event;
  while (enet_host_service(m_lobbyHost, &event, 10) > 0) {
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT: {
      printf("Connection with lobby server %x:%u established\n",
             event.peer->address.host, event.peer->address.port);
    } break;
    case ENET_EVENT_TYPE_RECEIVE: {
      printf("Packet received '%s'\n", event.packet->data);
      std::stringstream stream(std::string((const char *)event.packet->data,
                                           event.packet->dataLength));
      std::string ip;
      enet_uint16 port;
      stream >> ip >> port;

      printf("Trying to connect to %s, %u.\n", ip.c_str(), port);

      connectToGameServer(ip.c_str(), port);

      enet_packet_destroy(event.packet);
    } break;
    case ENET_EVENT_TYPE_DISCONNECT: {
      fail("Disconnected from the lobby server");
    } break;
    default:
      break;
    };
  }
}

void Client::tickGame() {
  if (!m_inGame)
    return;

  ENetEvent event;
  while (enet_host_service(m_gameServerHost, &event, 10) > 0) {
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT: {
      printf("Connection with game server %x:%u established\n",
             event.peer->address.host, event.peer->address.port);
    } break;
    case ENET_EVENT_TYPE_RECEIVE: {
      printf("Packet received '%s'\n", event.packet->data);
      std::stringstream stream(std::string((const char *)event.packet->data,
                                           event.packet->dataLength));

      //! TODO: FIX: Quite an ugly piece of code here...
      std::string code;
      stream >> code;
      // The server can potentially use GUIDs instead of peer IDs for
      // identification. This is why we need to listen for this info from the
      // server (or send identification info ourselves).
      if (code == packtype::kSelfId) {
        stream >> m_selfId >> m_selfName;
      } else if (code == packtype::kNewClient) {
        Player::IdT id = 0;
        std::string name;
        stream >> id >> name;
        m_players.insert({id, Player(id, name)});
      } else if (code == packtype::kDataUpdate) {
        forEachPlayer([&](Player &player) { player.alive = false; });

        unsigned count = 0;
        stream >> count;
        for (unsigned _ = 0; _ < count; ++_) {
          Player::IdT id = 0;
          float posX = 0, posY = 0;
          int ping = 0;
          stream >> id >> posX >> posY >> ping;

          if (!m_players.contains(id))
            continue;

          Player &player = m_players.at(id);
          if (id != m_selfId) {
            player.pos.x = posX;
            player.pos.y = posY;
          }
          player.ping = ping;
          player.alive = true;
        }
      }

      if (m_players.contains(m_selfId)) {
        Player &thisPlayer = m_players.at(m_selfId);
        std::string msg = std::to_string(thisPlayer.pos.x) + ' ' +
                          std::to_string(thisPlayer.pos.y);
        ENetPacket *packet = enet_packet_create(msg.c_str(), msg.length() + 1,
                                                ENET_PACKET_FLAG_UNSEQUENCED);
        printf("Sending packet '%s'\n", msg.c_str());
        enet_peer_send(m_gameServerPeer, 1, packet);
      }

      enet_packet_destroy(event.packet);
    } break;
    case ENET_EVENT_TYPE_DISCONNECT: {
      fail("Disconnected from the game server");
    } break;
    default:
      break;
    };
  }
}
