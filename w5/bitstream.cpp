#include "bitstream.h"

#include <arpa/inet.h>

BitOutstream& operator<<(BitOutstream& stream, const std::string& data)
{
  stream << (uint16_t)data.size();
  stream.m_stream << data;
  return stream;
}

BitOutstream& operator<<(BitOutstream& stream, _Float32 data)
{
  stream.m_stream << std::string((char*)&data, sizeof(_Float32));
  return stream;
}

BitOutstream& operator<<(BitOutstream& stream, int8_t data)
{
  return stream << (uint8_t)data;
}
BitOutstream& operator<<(BitOutstream& stream, int16_t data)
{
  return stream << (uint16_t)data;
}
BitOutstream& operator<<(BitOutstream& stream, int32_t data)
{
  return stream << (uint32_t)data;
}

BitOutstream& operator<<(BitOutstream& stream, uint8_t data)
{
  uint8_t netData = data;
  stream.m_stream << std::string((const char*)&netData, sizeof(uint8_t));
  return stream;
}
BitOutstream& operator<<(BitOutstream& stream, uint16_t data)
{
  uint16_t netData = htons(data);
  stream.m_stream << std::string((const char*)&netData, sizeof(netData));
  return stream;
}
BitOutstream& operator<<(BitOutstream& stream, uint32_t data)
{
  uint32_t netData = htonl(data);
  stream.m_stream << std::string((const char*)&netData, sizeof(netData));
  return stream;
}
BitOutstream& operator<<(BitOutstream& stream, bool data)
{
  return stream << (uint8_t)data;
}

BitInstream& operator>>(BitInstream& stream, std::string& data)
{
  uint16_t length = 0;
  stream >> length;
  data = std::string(stream.m_view.begin(), length);
  stream.m_view.remove_prefix(length);
  return stream;
}
BitInstream& operator>>(BitInstream& stream, _Float32& data)
{
  data = *(_Float32*)stream.m_view.begin();
  stream.m_view.remove_prefix(sizeof(_Float32));
  return stream;
}

BitInstream& operator>>(BitInstream& stream, int8_t& data)
{
  data = *(int8_t*)stream.m_view.begin();
  stream.m_view.remove_prefix(sizeof(int8_t));
  return stream;
}
BitInstream& operator>>(BitInstream& stream, int16_t& data)
{
  data = ntohs(*(int16_t*)stream.m_view.begin());
  stream.m_view.remove_prefix(sizeof(int16_t));
  return stream;
}
BitInstream& operator>>(BitInstream& stream, int32_t& data)
{
  data = ntohl(*(int32_t*)stream.m_view.begin());
  stream.m_view.remove_prefix(sizeof(int32_t));
  return stream;
}

BitInstream& operator>>(BitInstream& stream, uint8_t& data)
{
  data = *(uint8_t*)stream.m_view.begin();
  stream.m_view.remove_prefix(sizeof(data));
  return stream;
}
BitInstream& operator>>(BitInstream& stream, uint16_t& data)
{
  data = ntohs(*(uint16_t*)stream.m_view.begin());
  stream.m_view.remove_prefix(sizeof(data));
  return stream;
}
BitInstream& operator>>(BitInstream& stream, uint32_t& data)
{
  data = ntohl(*(uint32_t*)stream.m_view.begin());
  stream.m_view.remove_prefix(sizeof(data));
  return stream;
}

BitInstream& operator>>(BitInstream& stream, bool& data)
{
  uint8_t input = 0;
  stream >> input;
  data = input;
  return stream;
}
