#include "protocol.h"

#include <iomanip>
#include <iostream>

template <> BitOutstream &BitOutstream::operator<<(const Entity &entity) {
  *this << entity.eid;
  *this << entity.x << entity.y << entity.targetX << entity.targetY;
  *this << entity.serverControlled;
  *this << entity.color;
  return *this;
}

template <> BitInstream &BitInstream::operator>>(Entity &entity) {
  *this >> entity.eid;
  *this >> entity.x >> entity.y >> entity.targetX >> entity.targetY;
  *this >> entity.serverControlled;
  *this >> entity.color;
  return *this;
}

template <> BitOutstream &BitOutstream::operator<<(const MessageType &data) {
  return *this << (uint8_t)data;
}
template <> BitInstream &BitInstream::operator>>(MessageType &data) {
  return *this >> (uint8_t &)data;
  ;
}

void send_join(ENetPeer *peer) {
  NetBitOutstream stream;
  MessageType mtype = E_CLIENT_TO_SERVER_JOIN;
  stream << mtype;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_new_entity(ENetPeer *peer, const Entity &ent) {
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_NEW_ENTITY;
  stream << ent;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid) {
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY;
  stream << eid;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y) {
  NetBitOutstream stream;
  stream << E_CLIENT_TO_SERVER_STATE;
  stream << eid << x << y;
  stream.send(peer, 1, ENET_PACKET_FLAG_UNSEQUENCED);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y) {
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_SNAPSHOT;
  stream << eid << x << y;
  stream.send(peer, 1, ENET_PACKET_FLAG_UNSEQUENCED);
}

MessageType get_packet_type(NetBitInstream &stream) {
  MessageType type;
  stream >> type;
  return type;
}

void deserialize_new_entity(NetBitInstream &stream, Entity &ent) {
  stream >> ent;
}

void deserialize_set_controlled_entity(NetBitInstream &stream, uint16_t &eid) {
  stream >> eid;
}

void deserialize_entity_state(NetBitInstream &stream, uint16_t &eid, float &x,
                              float &y) {
  stream >> eid >> x >> y;
}

void deserialize_snapshot(NetBitInstream &stream, uint16_t &eid, float &x,
                          float &y) {
  stream >> eid >> x >> y;
}
