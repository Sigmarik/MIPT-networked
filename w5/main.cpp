// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//

#include <cstdio>
#include <enet/enet.h>
#include <functional>
#include <math.h>
#include <vector>

#include "client_entity.h"
#include "constants.h"
#include "protocol.h"
#include "raylib.h"

static std::vector<ClientEntity> entities;
static uint16_t my_entity = invalid_entity;

static uint32_t tickId = 0;
static uint32_t timeDilation = 0;

void on_set_time(NetBitInstream& stream)
{
  deserialize_server_time(stream, tickId);
  // We could have applied a latency correction to the result, but this
  // won't matter as we would have to reverse it every time we interact with the server.
}

void on_new_entity_packet(NetBitInstream& stream)
{
  Entity newEntity;
  deserialize_new_entity(stream, newEntity);
  // TODO: Direct addressing, of course!
  for (const ClientEntity& e : entities)
    if (e.eid == newEntity.eid)
      return; // don't need to do anything, we already have the entity
  entities.push_back(ClientEntity(tickId, newEntity));
}

void on_set_controlled_entity(NetBitInstream& stream)
{
  deserialize_set_controlled_entity(stream, my_entity);
}

void on_snapshot(NetBitInstream& stream)
{
  uint16_t eid = invalid_entity;
  float x = 0.f;
  float y = 0.f;
  float ori = 0.f;
  float speed = 0.f;
  float thr = 0.f;
  float steer = 0.f;
  uint32_t server_tick = 0;
  deserialize_snapshot(stream, server_tick, eid, x, y, ori, speed, thr, steer);
  server_tick = std::min(tickId, server_tick);
  // TODO: Direct addressing, of course!
  for (ClientEntity& e : entities)
    if (e.eid == eid)
    {
      Entity entity = e.display_entity;
      entity.x = x;
      entity.y = y;
      entity.ori = ori;
      entity.steer = steer;
      entity.thr = thr;
      entity.speed = speed;

      e.setAt(server_tick * UNIVERSAL_PHYS_DT, entity);
    }
}

int main(int argc, const char** argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10131;

  ENetPeer* serverPeer = enet_host_connect(client, &address, 2, 0);
  if (!serverPeer)
  {
    printf("Cannot connect to server");
    return 1;
  }

  int width = 600;
  int height = 600;

  InitWindow(width, height, "w5 networked MIPT");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }

  Camera2D camera = {{0, 0}, {0, 0}, 0.f, 1.f};
  camera.target = Vector2{0.f, 0.f};
  camera.offset = Vector2{width * 0.5f, height * 0.5f};
  camera.rotation = 0.f;
  camera.zoom = 10.f;

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  uint32_t lastPhysTick = get_reliable_time();

  bool connected = false;
  while (!WindowShouldClose())
  {
    uint32_t curTime = get_reliable_time();
    float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
      uint32_t curTime = get_reliable_time();
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        send_join(serverPeer);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
      {
        NetBitInstream stream(event.packet);
        switch (get_packet_type(stream))
        {
        case E_SERVER_TO_CLIENT_TIME:
          on_set_time(stream);
          break;
        case E_SERVER_TO_CLIENT_NEW_ENTITY:
          on_new_entity_packet(stream);
          printf("New entity!\n");
          break;
        case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
          on_set_controlled_entity(stream);
          break;
        case E_SERVER_TO_CLIENT_SNAPSHOT:
          on_snapshot(stream);
          break;
        };
      }
      break;
      default:
        break;
      };
    }
    if (my_entity != invalid_entity)
    {
      bool left = IsKeyDown(KEY_LEFT);
      bool right = IsKeyDown(KEY_RIGHT);
      bool up = IsKeyDown(KEY_UP);
      bool down = IsKeyDown(KEY_DOWN);
      // TODO: Direct addressing, of course!
      for (ClientEntity& e : entities)
        if (e.eid == my_entity)
        {
          // Update
          float thr = (up ? 1.f : 0.f) + (down ? -1.f : 0.f);
          float steer = (left ? -1.f : 0.f) + (right ? 1.f : 0.f);

          e.snapshots.back().state.thr = thr;
          e.snapshots.back().state.steer = steer;

          // Send
          send_entity_input(serverPeer, my_entity, thr, steer);
        }
    }

    while (lastPhysTick + UNIVERSAL_PHYS_DT < curTime)
    {
      for (ClientEntity& e : entities)
      {
        e.simulate();
        e.clearOldSnapshots(tickId * UNIVERSAL_PHYS_DT - 3000);
      }

      lastPhysTick += UNIVERSAL_PHYS_DT;
      ++tickId;
    }

    BeginDrawing();
    ClearBackground(GRAY);
    BeginMode2D(camera);
    for (ClientEntity& e : entities)
    {
      e.updateDisplay(tickId * UNIVERSAL_PHYS_DT + curTime - lastPhysTick);
      const Rectangle rect = {e.display_entity.x, e.display_entity.y, 3.f, 1.f};
      DrawRectanglePro(rect, {0.f, 0.5f}, e.display_entity.ori * 180.f / PI, GetColor(e.display_entity.color));
    }

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
