#include "entity.h"

#include <math.h>

#include "mathUtils.h"

float kTargetAttraction = 0.07;

#define ATTRACT(param) e.param = lerp(e.param, target.param, kTargetAttraction)

void simulate_entity(Entity& e, float dt)
{
  bool isBraking = sign(e.thr) != 0.f && sign(e.thr) != sign(e.speed);
  float accel = isBraking ? 12.f : 3.f;
  e.speed = move_to(e.speed, clamp(e.thr, -0.3, 1.f) * 10.f, dt, accel);
  e.ori += e.steer * dt * clamp(e.speed, -2.f, 2.f) * 0.3f;
  e.x += cosf(e.ori) * e.speed * dt;
  e.y += sinf(e.ori) * e.speed * dt;

  if (e.target)
  {
    Entity& target = *e.target;
    simulate_entity(target, dt);
    ATTRACT(ori);
    ATTRACT(x);
    ATTRACT(y);
    e.speed = target.speed;
    e.steer = target.steer;
    e.thr = target.thr;
  }
}

#undef ATTRACT

Entity::snapshot_t Entity::snapshot()
{
  snapshot_t snapshot;
  snapshot.x = &x;
  snapshot.y = &y;
  snapshot.ori = &ori;
  snapshot.speed = &speed;
  snapshot.steer = &steer;
  snapshot.thr = &thr;
  return snapshot;
}

Entity::const_snapshot_t Entity::snapshot() const
{
  const_snapshot_t snapshot;
  snapshot.x = &x;
  snapshot.y = &y;
  snapshot.ori = &ori;
  snapshot.speed = &speed;
  snapshot.steer = &steer;
  snapshot.thr = &thr;
  return snapshot;
}

template <bool Const>
BitOutstream& operator<<(BitOutstream& stream, const Snapshot<Entity, Const>& entity)
{
  return stream << *entity.x << *entity.y << *entity.ori << *entity.speed << *entity.steer << *entity.thr;
}

template BitOutstream& operator<<(BitOutstream& stream, const Snapshot<Entity, true>& entity);
template BitOutstream& operator<<(BitOutstream& stream, const Snapshot<Entity, false>& entity);

BitOutstream& operator<<(BitOutstream& stream, const Entity& entity)
{
  return stream << entity.eid << entity.color << entity.snapshot();
}

BitInstream& operator>>(BitInstream& stream, const Entity::snapshot_t& entity)
{
  return stream >> *entity.x >> *entity.y >> *entity.ori >> *entity.speed >> *entity.steer >> *entity.thr;
}

BitInstream& operator>>(BitInstream& stream, Entity& entity)
{
  return stream >> entity.eid >> entity.color >> entity.snapshot();
}
