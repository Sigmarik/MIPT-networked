#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

struct BitOutstream {
  BitOutstream() = default;

  template <class T> BitOutstream &operator<<(const T &data);

  std::string str() const { return m_stream.str(); }

private:
  std::stringstream m_stream{};
};

struct BitInstream {
  BitInstream(const std::string &raw) : m_data(raw), m_view(m_data) {}

  template <class T> BitInstream &operator>>(T &data);

private:
  std::string m_data{};
  std::string_view m_view{};
};

template <> BitOutstream &BitOutstream::operator<<(const std::string &data);

template <> BitOutstream &BitOutstream::operator<<(const int8_t &data);
template <> BitOutstream &BitOutstream::operator<<(const int16_t &data);
template <> BitOutstream &BitOutstream::operator<<(const int32_t &data);

template <> BitOutstream &BitOutstream::operator<<(const uint8_t &data);
template <> BitOutstream &BitOutstream::operator<<(const uint16_t &data);
template <> BitOutstream &BitOutstream::operator<<(const uint32_t &data);

template <> BitInstream &BitInstream::operator>>(std::string &data);

template <> BitInstream &BitInstream::operator>>(int8_t &data);
template <> BitInstream &BitInstream::operator>>(int16_t &data);
template <> BitInstream &BitInstream::operator>>(int32_t &data);

template <> BitInstream &BitInstream::operator>>(uint8_t &data);
template <> BitInstream &BitInstream::operator>>(uint16_t &data);
template <> BitInstream &BitInstream::operator>>(uint32_t &data);
