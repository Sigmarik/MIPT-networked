#include "protocol.h"

#include <chrono>
#include <cstring> // memcpy
#include <time.h>

uint32_t get_reliable_time()
{
  static auto start_time = std::chrono::steady_clock::now();
  auto current_time = std::chrono::steady_clock::now();
  auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
  return static_cast<uint32_t>(elapsed_time.count());
}

static BitOutstream& operator<<(BitOutstream& stream, const MessageType& type)
{
  return stream << (unsigned)type;
}

static BitInstream& operator>>(BitInstream& stream, MessageType& type)
{
  unsigned value = 0;
  stream >> value;
  type = (MessageType)value;
  return stream;
}

void send_join(ENetPeer* peer)
{
  NetBitOutstream stream;
  stream << E_CLIENT_TO_SERVER_JOIN;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_server_time(ENetPeer* peer, uint32_t tickId)
{
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_TIME;
  stream << (uint32_t)tickId;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_new_entity(ENetPeer* peer, const Entity& ent)
{
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_NEW_ENTITY << ent;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_set_controlled_entity(ENetPeer* peer, uint16_t eid)
{
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY << eid;
  stream.send(peer, 0, ENET_PACKET_FLAG_RELIABLE);
}

void send_entity_input(ENetPeer* peer, uint16_t eid, float thr, float steer)
{
  NetBitOutstream stream;
  stream << E_CLIENT_TO_SERVER_INPUT << eid << thr << steer;
  stream.send(peer, 1, ENET_PACKET_FLAG_UNSEQUENCED);
}

void send_snapshot(ENetPeer* peer, uint32_t tick_id, uint16_t eid, float x, float y, float ori, float speed, float thr,
                   float steer)
{
  NetBitOutstream stream;
  stream << E_SERVER_TO_CLIENT_SNAPSHOT << tick_id << eid << x << y << ori << speed << thr << steer;
  stream.send(peer, 1, ENET_PACKET_FLAG_UNSEQUENCED);
}

MessageType get_packet_type(NetBitInstream& stream)
{
  MessageType type;
  stream >> type;
  return type;
}

void deserialize_server_time(NetBitInstream& stream, uint32_t& tickId)
{
  stream >> tickId;
}

void deserialize_new_entity(NetBitInstream& stream, Entity& ent)
{
  stream >> ent;
}

void deserialize_set_controlled_entity(NetBitInstream& stream, uint16_t& eid)
{
  stream >> eid;
}

void deserialize_entity_input(NetBitInstream& stream, uint16_t& eid, float& thr, float& steer)
{
  stream >> eid >> thr >> steer;
}

void deserialize_snapshot(NetBitInstream& stream, uint32_t& tick_id, uint16_t& eid, float& x, float& y, float& ori,
                          float& speed, float& thr, float& steer)
{
  stream >> tick_id >> eid >> x >> y >> ori >> speed >> thr >> steer;
}
