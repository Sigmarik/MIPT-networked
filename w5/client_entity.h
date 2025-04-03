#pragma once

#include <deque>

#include "entity.h"

struct ClientEntity
{
  uint16_t eid = 0;

  Entity display_entity{};

  struct TimedSnapshot
  {
    uint32_t tick_id = 0;
    Entity state{};
  };

  std::deque<TimedSnapshot> snapshots{};

  explicit ClientEntity(uint32_t tick_id, const Entity& entity);

  // Progress the simulation by a standard delta time
  void simulate();

  void updateDisplay(uint32_t time);

  void setAt(uint32_t time, Entity entity);

  void clearOldSnapshots(uint32_t threshold);
};
