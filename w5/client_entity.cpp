#include "client_entity.h"

#include <assert.h>
#include <iostream>

#include "constants.h"

ClientEntity::ClientEntity(uint32_t tick_id, const Entity& entity)
{
  eid = entity.eid;
  display_entity = entity;
  snapshots.push_back({tick_id, entity});
  snapshots.push_back({tick_id + 1, entity});

  assert(snapshots.size() >= 2);
}

void ClientEntity::simulate()
{
  Entity entity = snapshots.back().state;
  uint32_t new_tick_id = snapshots.back().tick_id + 1;
  simulate_entity(entity, UNIVERSAL_PHYS_DT * 0.001f);
  snapshots.push_back({new_tick_id, entity});
}

void ClientEntity::updateDisplay(uint32_t time)
{
  assert(snapshots.size() >= 2);

  Entity target = display_entity;

  // Add a 0.05 seconds delay to user input? Suuuure...
  uint32_t requested_time = time - UNIVERSAL_PHYS_DT;

  if (requested_time > snapshots.back().tick_id * UNIVERSAL_PHYS_DT)
  {
    // There is a room for extrapolation here.
    target = snapshots.back().state;
  }
  else if (requested_time < snapshots.front().tick_id * UNIVERSAL_PHYS_DT)
  {
    // There is a room for extrapolation here.
    target = snapshots.front().state;
  }
  else
  {
    long long id = snapshots.size() - 2;
    while (id > 0 && snapshots[id].tick_id * UNIVERSAL_PHYS_DT > requested_time)
    {
      --id;
    }
    TimedSnapshot& alpha = snapshots[id];
    TimedSnapshot& beta = snapshots[id + 1];
    uint32_t alpha_time = alpha.tick_id * UNIVERSAL_PHYS_DT;
    uint32_t beta_time = beta.tick_id * UNIVERSAL_PHYS_DT;
    float interp = (requested_time - alpha_time) / (float)(beta_time - alpha_time);
    target = lerp(alpha.state, beta.state, interp);
  }

  display_entity = lerp(display_entity, target, 0.07);
}

void ClientEntity::setAt(uint32_t time, Entity entity)
{
  assert(snapshots.size() >= 2);

  uint32_t highest_id = snapshots.back().tick_id;

  uint32_t tick = time / UNIVERSAL_PHYS_DT;

  while (snapshots.size() > 1 && snapshots.back().tick_id > tick)
  {
    snapshots.pop_back();
  }

  if (snapshots.back().tick_id >= tick)
  {
    snapshots.back() = {tick, entity};
  }
  else if (snapshots.back().tick_id < tick)
  {
    snapshots.push_back({tick, entity});
  }

  while (snapshots.back().tick_id < highest_id || snapshots.size() < 2)
  {
    simulate();
  }
}

void ClientEntity::clearOldSnapshots(uint32_t threshold)
{
  assert(snapshots.size() >= 2);

  while (snapshots.size() > 2 && snapshots.front().tick_id < threshold)
  {
    snapshots.pop_front();
  }
}
