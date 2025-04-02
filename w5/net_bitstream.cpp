#include "net_bitstream.h"

NetBitInstream::NetBitInstream(ENetPacket *packet)
    : BitInstream(std::string((const char *)packet->data, packet->dataLength)) {
}

ENetPacket *NetBitOutstream::packet(ENetPacketFlag flag) const {
  std::string data = str();
  return enet_packet_create(data.c_str(), data.size() + 1, flag);
}

void NetBitOutstream::send(ENetPeer *peer, enet_uint8 channel,
                           ENetPacketFlag flag) const {
  ENetPacket *pack = packet(flag);
  enet_peer_send(peer, channel, pack);
}
