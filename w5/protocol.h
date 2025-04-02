#pragma once
#include <cstdint>
#include <enet/enet.h>

#include "entity.h"
#include "net_bitstream.h"

enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_TIME,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_INPUT,
  E_SERVER_TO_CLIENT_SNAPSHOT
};

void send_join(ENetPeer* peer);
void send_server_time(ENetPeer* peer, uint32_t tickId);
void send_new_entity(ENetPeer* peer, const Entity& ent);
void send_set_controlled_entity(ENetPeer* peer, uint16_t eid);
void send_entity_input(ENetPeer* peer, uint16_t eid, float thr, float steer);
// Who needs structures when you can have a function that takes 9 parameters?
void send_snapshot(ENetPeer* peer, uint32_t tick_id, uint16_t eid, float x, float y, float ori, float speed, float thr,
                   float steer);

MessageType get_packet_type(NetBitInstream& stream);

void deserialize_server_time(NetBitInstream& stream, uint32_t& tickId);
void deserialize_new_entity(NetBitInstream& stream, Entity& ent);
void deserialize_set_controlled_entity(NetBitInstream& stream, uint16_t& eid);
void deserialize_entity_input(NetBitInstream& stream, uint16_t& eid, float& thr, float& steer);
// Who needs structures when you can have a function that takes 9 parameters?
void deserialize_snapshot(NetBitInstream& stream, uint32_t& tick_id, uint16_t& eid, float& x, float& y, float& ori,
                          float& speed, float& thr, float& steer);
