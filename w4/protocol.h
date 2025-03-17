#pragma once
#include "entity.h"
#include "net_bitstream.h"
#include <cstdint>
#include <enet/enet.h>

enum MessageType : uint8_t {
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_FULL_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_STATE,
  E_SERVER_TO_CLIENT_SNAPSHOT
};

template <> BitOutstream &BitOutstream::operator<<(const Entity &data);
template <> BitInstream &BitInstream::operator>>(Entity &data);

template <> BitOutstream &BitOutstream::operator<<(const MessageType &data);
template <> BitInstream &BitInstream::operator>>(MessageType &data);

void send_join(ENetPeer *peer);
void send_new_entity(ENetPeer *peer, const Entity &ent);
void send_full_entity(ENetPeer *peer, const Entity &ent);
void send_set_controlled_entity(ENetPeer *peer, uint16_t eid);
void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y);
void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y);

MessageType get_packet_type(NetBitInstream &stream);

void deserialize_new_entity(NetBitInstream &stream, Entity &ent);
void deserialize_full_entity(NetBitInstream &stream, Entity &ent);
void deserialize_set_controlled_entity(NetBitInstream &stream, uint16_t &eid);
void deserialize_entity_state(NetBitInstream &stream, uint16_t &eid, float &x,
                              float &y);
void deserialize_snapshot(NetBitInstream &stream, uint16_t &eid, float &x,
                          float &y);
