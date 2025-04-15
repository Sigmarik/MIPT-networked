#pragma once
#include <cstdint>
#include <map>

constexpr uint16_t invalid_entity = -1;
constexpr float worldSize = 120.f;
struct Entity
{
  // immutable state
  uint32_t color = 0xff00ffff;
  bool serverControlled = false;

  // mutable state
  float x = 0.f;
  float y = 0.f;
  float vx = 0.f;
  float vy = 0.f;
  float ori = 0.f;
  float omega = 0.f;

  // user input
  float thr = 0.f;
  float steer = 0.f;

  // misc
  uint16_t eid = invalid_entity;
};

struct LOD
{
  LOD() = default;
  LOD(unsigned short ql, float drThr, float dist) : quality(ql), deadReckoningThreshold(drThr), upperBound(dist)
  {
  }

  unsigned short quality = 0;
  float deadReckoningThreshold = 0.1f;
  float upperBound = 0.0f;
};

static constexpr unsigned kNetLODs = 4;
static LOD kLODs[kNetLODs] = {
    LOD{1, 0.01f, 5.0f},
    LOD{1, 0.05f, 20.0f},
    LOD{0, 0.1f, 40.0f},
    LOD{0, 0.5f, -1.0f},
};

struct ServerEntity
{
  Entity entity;
  std::map<uint16_t, Entity> deadReckonings;

  void update(float dt);

  void addReckoning(uint16_t id);

  void synchReckoning(uint16_t id);

  template <class F>
  void forEachReckoning(F &&func)
  {
    for (auto &[id, copy] : deadReckonings)
    {
      func(entity, copy);
    }
  }
};

void simulate_entity(Entity &e, float dt);
