// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//
#include "raylib.h"
#include <cstdio>
#include <enet/enet.h>
#include <functional>
#include <math.h>

#include "entity.h"
#include "protocol.h"
#include <vector>

static std::vector<Entity> entities;
static uint16_t my_entity = invalid_entity;

void on_new_entity_packet(NetBitInstream& stream)
{
  Entity newEntity;
  deserialize_new_entity(stream, newEntity);
  // TODO: Direct adressing, of course!
  for (const Entity& e : entities)
    if (e.eid == newEntity.eid)
      return; // don't need to do anything, we already have entity
  newEntity.target = std::make_unique<Entity>();
  newEntity.target->x = newEntity.x;
  newEntity.target->y = newEntity.y;
  newEntity.target->speed = newEntity.speed;
  newEntity.target->thr = newEntity.thr;
  newEntity.target->steer = newEntity.steer;
  entities.push_back(std::move(newEntity));
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
  deserialize_snapshot(stream, eid, x, y, ori, speed, thr, steer);
  // TODO: Direct adressing, of course!
  for (Entity& e : entities)
    if (e.eid == eid)
    {
      e.target->x = x;
      e.target->y = y;
      e.target->ori = ori;
      e.target->thr = thr;
      e.target->steer = steer;
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

  bool connected = false;
  while (!WindowShouldClose())
  {
    float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
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
      for (Entity& e : entities)
        if (e.eid == my_entity)
        {
          // Update
          float thr = (up ? 1.f : 0.f) + (down ? -1.f : 0.f);
          float steer = (left ? -1.f : 0.f) + (right ? 1.f : 0.f);

          e.target->thr = thr;
          e.target->steer = steer;

          // Send
          send_entity_input(serverPeer, my_entity, thr, steer);
        }
    }

    for (Entity& e : entities)
    {
      simulate_entity(e, dt);
    }

    BeginDrawing();
    ClearBackground(GRAY);
    BeginMode2D(camera);
    for (const Entity& e : entities)
    {
      const Rectangle rect = {e.x, e.y, 3.f, 1.f};
      DrawRectanglePro(rect, {0.f, 0.5f}, e.ori * 180.f / PI, GetColor(e.color));
    }

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
