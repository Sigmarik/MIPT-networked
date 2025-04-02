#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include "bitstream.h"
#include "snapshot.h"

constexpr uint16_t invalid_entity = -1;

struct Entity;

template <bool Const>
struct Snapshot<Entity, Const>
{
  using FloatPtr = typename std::conditional<Const, const float*, float*>::type;

  FloatPtr x;
  FloatPtr y;
  FloatPtr speed;
  FloatPtr ori;

  FloatPtr thr;
  FloatPtr steer;

  friend struct Entity;
};

struct Entity
{
  uint32_t color = 0xff00ffff;

  float x = 0.f;
  float y = 0.f;
  float speed = 0.f;
  float ori = 0.f;

  float thr = 0.f;
  float steer = 0.f;

  uint16_t eid = invalid_entity;

  using snapshot_t = Snapshot<Entity, false>;
  using const_snapshot_t = Snapshot<Entity, true>;

  snapshot_t snapshot();
  const_snapshot_t snapshot() const;

  std::unique_ptr<Entity> target;
};

template <bool Const>
BitOutstream& operator<<(BitOutstream& stream, const Snapshot<Entity, Const>& entity);
BitOutstream& operator<<(BitOutstream& stream, const Entity& entity);

BitInstream& operator>>(BitInstream& stream, const Entity::snapshot_t& entity);
BitInstream& operator>>(BitInstream& stream, Entity& entity);

void simulate_entity(Entity& e, float dt);
