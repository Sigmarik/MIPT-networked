#pragma once

#include "bitstream.h"
#include <enet/enet.h>

struct NetBitInstream : public BitInstream
{
  NetBitInstream(ENetPacket* packet);
};

template <class T>
NetBitInstream& operator>>(NetBitInstream& stream, T& value)
{
  return (NetBitInstream&)((BitInstream&)stream >> value);
}

struct NetBitOutstream : public BitOutstream
{
  using BitOutstream::BitOutstream;

  ENetPacket* packet(ENetPacketFlag flag) const;

  void send(ENetPeer* peer, enet_uint8 channel, ENetPacketFlag flag) const;
};

template <class T>
NetBitOutstream& operator<<(NetBitOutstream& stream, const T& value)
{
  return (NetBitOutstream&)((BitOutstream&)stream << value);
}
