#include <iostream>
#include <map>
#include <stdlib.h>
#include <vector>

#include <enet/enet.h>

#include "constants.h"
#include "entity.h"
#include "mathUtils.h"
#include "protocol.h"

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer*> controlledMap;
static std::vector<ENetPeer*> peers;
static uint32_t tickId;

void on_join(NetBitInstream& stream, ENetPeer* peer, ENetHost* host)
{
  send_server_time(peer, tickId);
  // send all entities
  for (const Entity& ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  uint16_t maxEid = entities.empty() ? invalid_entity : entities[0].eid;
  for (const Entity& e : entities)
    maxEid = std::max(maxEid, e.eid);
  uint16_t newEid = maxEid + 1;
  uint32_t color =
      0xff000000 + 0x00440000 * (rand() % 4 + 1) + 0x00004400 * (rand() % 4 + 1) + 0x00000044 * (rand() % 4 + 1);
  float x = (rand() % 4) * 5.f;
  float y = (rand() % 4) * 5.f;
  Entity ent = {color, x, y, 0.f, (rand() / RAND_MAX) * 3.141592654f, 0.f, 0.f, newEid};

  controlledMap[newEid] = peer;

  // send info about new entity to everyone
  for (ENetPeer* peer : peers)
    send_new_entity(peer, ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);

  entities.push_back(std::move(ent));
}

void on_input(NetBitInstream& stream)
{
  uint16_t eid = invalid_entity;
  float thr = 0.f;
  float steer = 0.f;
  deserialize_entity_input(stream, eid, thr, steer);
  for (Entity& e : entities)
    if (e.eid == eid)
    {
      e.thr = thr;
      e.steer = steer;
    }
}

int main(int argc, const char** argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10131;

  ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  uint32_t lastTime = enet_time_get();
  uint32_t lastPhysUpdate = enet_time_get();
  uint32_t lastBroadcast = enet_time_get();
  while (true)
  {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        peers.push_back(event.peer);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
      {
        NetBitInstream stream(event.packet);
        switch (get_packet_type(stream))
        {
        case E_CLIENT_TO_SERVER_JOIN:
          on_join(stream, event.peer, server);
          printf("Client joined\n");
          break;
        case E_CLIENT_TO_SERVER_INPUT:
          on_input(stream);
          break;
        };
        enet_packet_destroy(event.packet);
      }
      break;
      default:
        break;
      };
    }
    static int t = 0;
    // There is a possibility to enter a "death spiral" if physics simulation gets too heavy to be processed in
    // real time.
    while (lastPhysUpdate + UNIVERSAL_PHYS_DT < curTime)
    {
      for (Entity& e : entities)
      {
        // simulate
        simulate_entity(e, UNIVERSAL_PHYS_DT * 0.001f);
      }
      lastPhysUpdate += UNIVERSAL_PHYS_DT;
      ++tickId;
    }

    if (lastBroadcast + SERVER_BROADCAST_DT < curTime)
    {
      for (Entity& e : entities)
      {
        // send
        for (ENetPeer* peer : peers)
        {
          // skip this here in this implementation
          // if (controlledMap[e.eid] != peer)
          send_snapshot(peer, tickId, e.eid, e.x, e.y, e.ori, e.speed, e.thr, e.steer);
        }
      }

      lastBroadcast = curTime;
    }
    // We can still sleep, this till only affect the frequency of the updates.
    // And latency, perhaps.
    usleep(20000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}
