#include "bitstream.h"

#include <arpa/inet.h>

template <> BitOutstream &BitOutstream::operator<<(const std::string &data) {
  *this << (uint16_t)data.size();
  m_stream << data;
  return *this;
}

template <> BitOutstream &BitOutstream::operator<<(const _Float32 &data) {
  m_stream << std::string((char *)&data, sizeof(_Float32));
  return *this;
}

template <> BitOutstream &BitOutstream::operator<<(const int8_t &data) {
  return *this << (uint8_t)data;
}
template <> BitOutstream &BitOutstream::operator<<(const int16_t &data) {
  return *this << (uint16_t)data;
}
template <> BitOutstream &BitOutstream::operator<<(const int32_t &data) {
  return *this << (uint32_t)data;
}

template <> BitOutstream &BitOutstream::operator<<(const uint8_t &data) {
  m_stream << std::string((char *)&data, sizeof(uint8_t));
  return *this;
}
template <> BitOutstream &BitOutstream::operator<<(const uint16_t &data) {
  uint16_t netData = htons(data);
  m_stream << std::string((char *)&netData, sizeof(netData));
  return *this;
}
template <> BitOutstream &BitOutstream::operator<<(const uint32_t &data) {
  uint32_t netData = htonl(data);
  m_stream << std::string((char *)&netData, sizeof(netData));
  return *this;
}

template <> BitInstream &BitInstream::operator>>(std::string &data) {
  uint16_t length = 0;
  *this >> length;
  data = std::string(m_view.begin(), length);
  m_view.remove_prefix(length);
  return *this;
}
template <> BitInstream &BitInstream::operator>>(_Float32 &data) {
  data = *(_Float32 *)m_view.begin();
  m_view.remove_prefix(sizeof(_Float32));
  return *this;
}

template <> BitInstream &BitInstream::operator>>(int8_t &data) {
  data = *(int8_t *)m_view.begin();
  m_view.remove_prefix(sizeof(int8_t));
  return *this;
}
template <> BitInstream &BitInstream::operator>>(int16_t &data) {
  data = ntohs(*(int16_t *)m_view.begin());
  m_view.remove_prefix(sizeof(int16_t));
  return *this;
}
template <> BitInstream &BitInstream::operator>>(int32_t &data) {
  data = ntohl(*(int32_t *)m_view.begin());
  m_view.remove_prefix(sizeof(int32_t));
  return *this;
}

template <> BitInstream &BitInstream::operator>>(uint8_t &data) {
  data = *(uint8_t *)m_view.begin();
  m_view.remove_prefix(sizeof(data));
  return *this;
}
template <> BitInstream &BitInstream::operator>>(uint16_t &data) {
  data = ntohs(*(uint16_t *)m_view.begin());
  m_view.remove_prefix(sizeof(data));
  return *this;
}
template <> BitInstream &BitInstream::operator>>(uint32_t &data) {
  data = ntohl(*(uint32_t *)m_view.begin());
  m_view.remove_prefix(sizeof(data));
  return *this;
}
