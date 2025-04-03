#include "entity.h"

#include <math.h>

#include "mathUtils.h"

void simulate_entity(Entity& e, float dt)
{
  bool isBraking = sign(e.thr) != 0.f && sign(e.thr) != sign(e.speed);
  float accel = isBraking ? 12.f : 3.f;
  e.speed = move_to(e.speed, clamp(e.thr, -0.3, 1.f) * 10.f, dt, accel);
  e.ori += e.steer * dt * clamp(e.speed, -2.f, 2.f) * 0.3f;
  e.x += cosf(e.ori) * e.speed * dt;
  e.y += sinf(e.ori) * e.speed * dt;
}

Entity lerp(const Entity& alpha, const Entity& beta, float t)
{
  Entity result = alpha;
  result.x = lerp(alpha.x, beta.x, t);
  result.y = lerp(alpha.y, beta.y, t);
  result.ori = lerp(alpha.ori, beta.ori, t);
  result.speed = beta.speed;
  result.thr = beta.thr;
  result.steer = beta.steer;
  return result;
}

BitOutstream& operator<<(BitOutstream& stream, const Entity& entity)
{
  return stream << entity.eid << entity.color << entity.x << entity.y << entity.ori << entity.speed << entity.steer
                << entity.thr;
}

BitInstream& operator>>(BitInstream& stream, Entity& entity)
{
  return stream >> entity.eid >> entity.color >> entity.x >> entity.y >> entity.ori >> entity.speed >> entity.steer >>
         entity.thr;
}
