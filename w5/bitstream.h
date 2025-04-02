#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

struct BitOutstream
{
  BitOutstream() = default;

  template <class T>
  friend BitOutstream& operator<<(BitOutstream& stream, const T& data);

  std::string str() const
  {
    return m_stream.str();
  }

  friend BitOutstream& operator<<(BitOutstream& stream, const std::string& data);
  friend BitOutstream& operator<<(BitOutstream& stream, _Float32 data);

  friend BitOutstream& operator<<(BitOutstream& stream, uint8_t data);
  friend BitOutstream& operator<<(BitOutstream& stream, uint16_t data);
  friend BitOutstream& operator<<(BitOutstream& stream, uint32_t data);

  friend BitOutstream& operator<<(BitOutstream& stream, bool data);

private:
  std::stringstream m_stream{};
};

BitOutstream& operator<<(BitOutstream& stream, const std::string& data);
BitOutstream& operator<<(BitOutstream& stream, _Float32 data);

BitOutstream& operator<<(BitOutstream& stream, int8_t data);
BitOutstream& operator<<(BitOutstream& stream, int16_t data);
BitOutstream& operator<<(BitOutstream& stream, int32_t data);

BitOutstream& operator<<(BitOutstream& stream, uint8_t data);
BitOutstream& operator<<(BitOutstream& stream, uint16_t data);
BitOutstream& operator<<(BitOutstream& stream, uint32_t data);

BitOutstream& operator<<(BitOutstream& stream, bool data);

struct BitInstream
{
  BitInstream(const std::string& raw) : m_data(raw), m_view(m_data)
  {
  }

  friend BitInstream& operator>>(BitInstream& stream, std::string& data);
  friend BitInstream& operator>>(BitInstream& stream, _Float32& data);

  friend BitInstream& operator>>(BitInstream& stream, int8_t& data);
  friend BitInstream& operator>>(BitInstream& stream, int16_t& data);
  friend BitInstream& operator>>(BitInstream& stream, int32_t& data);

  friend BitInstream& operator>>(BitInstream& stream, uint8_t& data);
  friend BitInstream& operator>>(BitInstream& stream, uint16_t& data);
  friend BitInstream& operator>>(BitInstream& stream, uint32_t& data);

  friend BitInstream& operator>>(BitInstream& stream, bool& data);

private:
  std::string m_data{};
  std::string_view m_view{};
};

BitInstream& operator>>(BitInstream& stream, std::string& data);
BitInstream& operator>>(BitInstream& stream, _Float32& data);

BitInstream& operator>>(BitInstream& stream, int8_t& data);
BitInstream& operator>>(BitInstream& stream, int16_t& data);
BitInstream& operator>>(BitInstream& stream, int32_t& data);

BitInstream& operator>>(BitInstream& stream, uint8_t& data);
BitInstream& operator>>(BitInstream& stream, uint16_t& data);
BitInstream& operator>>(BitInstream& stream, uint32_t& data);

BitInstream& operator>>(BitInstream& stream, bool& data);
