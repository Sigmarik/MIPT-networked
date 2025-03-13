#pragma once

#include "bitstream.h"
#include <enet/enet.h>

struct NetBitInstream : public BitInstream {
  NetBitInstream(ENetPacket *packet);

  template <class T> NetBitInstream &operator>>(T &data) {
    BitInstream::operator>> <T>(data);
    return *this;
  }
};

struct NetBitOutstream : public BitOutstream {
  using BitOutstream::BitOutstream;

  template <class T> BitOutstream &operator<<(const T &data) {
    BitOutstream::operator<< <T>(data);
    return *this;
  }

  ENetPacket *packet(ENetPacketFlag flag) const;

  void send(ENetPeer *peer, enet_uint8 channel, ENetPacketFlag flag) const;
};
