#include <assert.h>
#include <enet/enet.h>
#include <iostream>
#include <math.h>

#include "entity.h"
#include "net_bitstream.h"
#include "protocol.h"

#include <map>
#include <set>
#include <stdlib.h>
#include <vector>

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer *> controlledMap;
std::set<ENetPeer *> clientPeers{};

static void gen_entity_params(Entity &ent) {
  ent.x = (rand() % 40 - 20) * 30.f;
  ent.y = (rand() % 40 - 20) * 30.f;
  ent.radius = rand() % 10 + 5;
}

static uint16_t create_random_entity() {
  uint16_t newEid = entities.size();
  uint32_t color = 0xff000000 + 0x00440000 * (1 + rand() % 4) +
                   0x00004400 * (1 + rand() % 4) +
                   0x00000044 * (1 + rand() % 4);
  Entity ent = {color, 0, 0, newEid, false, 0.f, 0.f};
  gen_entity_params(ent);
  entities.push_back(ent);
  return newEid;
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host) {
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  uint16_t newEid = create_random_entity();
  const Entity &ent = entities[newEid];

  controlledMap[newEid] = peer;

  // send info about new entity to everyone
  for (ENetPeer *clientPeer : clientPeers)
    send_new_entity(clientPeer, ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
}

void on_state(NetBitInstream &stream) {
  uint16_t eid = invalid_entity;
  float x = 0.f;
  float y = 0.f;
  deserialize_entity_state(stream, eid, x, y);
  for (Entity &e : entities)
    if (e.eid == eid) {
      e.x = x;
      e.y = y;
    }
}

static void announce_full_state(Entity &ent) {
  for (ENetPeer *peer : clientPeers) {
    send_full_entity(peer, ent);
  }
}

static void eat(Entity &alpha, Entity &beta) {
  alpha.radius += beta.radius / 2.0f;
  alpha.points++;
  alpha.eatingCooldown = 0.2f;
  gen_entity_params(beta);
}

static void process_collisions() {
  for (Entity &alphaEnt : entities) {
    if (alphaEnt.eatingCooldown > 0.0f)
      continue;

    for (Entity &betaEnt : entities) {
      if (alphaEnt.eid == betaEnt.eid)
        continue;
      if (alphaEnt.radius <= betaEnt.radius)
        continue;

      float deltaX = alphaEnt.x - betaEnt.x;
      float deltaY = alphaEnt.y - betaEnt.y;
      float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
      if (distance < alphaEnt.radius + betaEnt.radius) {
        // One is fully inside the other
        eat(alphaEnt, betaEnt);
        announce_full_state(alphaEnt);
        announce_full_state(betaEnt);
      }
    }
  }
}

int main(int argc, const char **argv) {
  if (enet_initialize() != 0) {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10131;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server) {
    printf("Cannot create ENet server\n");
    return 1;
  }

  constexpr int numAi = 10;

  for (int i = 0; i < numAi; ++i) {
    uint16_t eid = create_random_entity();
    entities[eid].serverControlled = true;
    controlledMap[eid] = nullptr;
  }

  float sinceLastPack = 0.0f;

  uint32_t lastTime = enet_time_get();
  while (true) {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0) {
      switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host,
               event.peer->address.port);
        clientPeers.insert(event.peer);
        break;
      case ENET_EVENT_TYPE_RECEIVE: {
        NetBitInstream stream(event.packet);
        switch (get_packet_type(stream)) {
        case E_CLIENT_TO_SERVER_JOIN:
          on_join(event.packet, event.peer, server);
          break;
        case E_CLIENT_TO_SERVER_STATE:
          on_state(stream);
          break;
        default:
          assert(false);
        };
        enet_packet_destroy(event.packet);
      } break;
      case ENET_EVENT_TYPE_DISCONNECT: {
        clientPeers.erase(event.peer);
        // TODO: Send player disconnect packet to all other clients.
      } break;
      default:
        break;
      };
    }
    for (Entity &e : entities) {
      if (e.serverControlled) {
        const float diffX = e.targetX - e.x;
        const float diffY = e.targetY - e.y;
        const float dirX = diffX > 0.f ? 1.f : -1.f;
        const float dirY = diffY > 0.f ? 1.f : -1.f;
        constexpr float spd = 50.f;
        e.x += dirX * spd * dt;
        e.y += dirY * spd * dt;
        if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f) {
          e.targetX = (rand() % 40 - 20) * 30.f;
          e.targetY = (rand() % 40 - 20) * 30.f;
        }
      }
    }
    if (!clientPeers.empty()) {
      process_collisions();
    }
    for (Entity &e : entities) {
      e.eatingCooldown -= dt;
    }
    if (sinceLastPack > 0.05) {
      sinceLastPack = 0.0f;
      for (const Entity &e : entities) {
        for (ENetPeer *peer : clientPeers) {
          if (controlledMap[e.eid] != peer) {
            send_snapshot(peer, e.eid, e.x, e.y);
          }
        }
      }
    }
    sinceLastPack += dt;
    // usleep(400000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}
