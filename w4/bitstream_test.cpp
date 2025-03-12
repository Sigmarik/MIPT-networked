#include "bitstream.h"

#include <gtest/gtest.h>

TEST(Bitstream, Float) {
  BitOutstream outstream;

  outstream << 12.3456f;

  BitInstream instream(outstream.str());

  float result = 0.0f;
  instream >> result;

  EXPECT_EQ(result, 12.3456f);
}

TEST(Bitstream, FloatChain) {
  BitOutstream outstream;

  float value = 12.3456;

  outstream << value << value;

  BitInstream instream(outstream.str());

  float result = 0.0f;
  instream >> result >> result;

  EXPECT_EQ(result, 12.3456f);
}

TEST(Bitstream, Uint8) {
  BitOutstream outstream;

  outstream << (uint8_t)42;

  BitInstream instream(outstream.str());

  uint8_t result = 0;
  instream >> result;

  EXPECT_EQ(result, 42);
}

TEST(Bitstream, Uint8Chain) {
  BitOutstream outstream;

  outstream << (uint8_t)112 << (uint8_t)42;

  BitInstream instream(outstream.str());

  uint8_t result = 0;
  instream >> result >> result;

  EXPECT_EQ(result, 42);
}

TEST(Bitstream, Uint16) {
  BitOutstream outstream;

  outstream << (uint16_t)42;

  BitInstream instream(outstream.str());

  uint16_t result = 0;
  instream >> result;

  EXPECT_EQ(result, 42);
}

TEST(Bitstream, Uint16Chain) {
  BitOutstream outstream;

  for (short _ = 0; _ < 2; ++_)
    outstream << (uint16_t)42;

  BitInstream instream(outstream.str());

  uint16_t result = 0;
  instream >> result >> result;

  EXPECT_EQ(result, 42);
}

TEST(Bitstream, Uint32) {
  BitOutstream outstream;

  outstream << (uint32_t)42;

  BitInstream instream(outstream.str());

  uint32_t result = 0;
  instream >> result;

  EXPECT_EQ(result, 42);
}

TEST(Bitstream, Uint32Chain) {
  BitOutstream outstream;

  for (short _ = 0; _ < 2; ++_)
    outstream << (uint32_t)42;

  BitInstream instream(outstream.str());

  uint32_t result = 0;
  for (short _ = 0; _ < 2; ++_)
    instream >> result;

  EXPECT_EQ(result, 42);
}

TEST(Bitstream, Int8) {
  BitOutstream outstream;

  outstream << (int8_t)-42;

  BitInstream instream(outstream.str());

  int8_t result = 0;
  instream >> result;

  EXPECT_EQ(result, -42);
}

TEST(Bitstream, Int8Chain) {
  BitOutstream outstream;

  for (short _ = 0; _ < 2; ++_)
    outstream << (int8_t)-42;

  BitInstream instream(outstream.str());

  int8_t result = 0;
  for (short _ = 0; _ < 2; ++_)
    instream >> result;

  EXPECT_EQ(result, -42);
}

TEST(Bitstream, Int16) {
  BitOutstream outstream;

  outstream << (int16_t)-42;

  BitInstream instream(outstream.str());

  int16_t result = 0;
  instream >> result;

  EXPECT_EQ(result, -42);
}

TEST(Bitstream, Int16Chain) {
  BitOutstream outstream;

  for (short _ = 0; _ < 2; ++_)
    outstream << (int16_t)-42;

  BitInstream instream(outstream.str());

  int16_t result = 0;
  for (short _ = 0; _ < 2; ++_)
    instream >> result;

  EXPECT_EQ(result, -42);
}

TEST(Bitstream, Int32) {
  BitOutstream outstream;

  outstream << (int32_t)-42;

  BitInstream instream(outstream.str());

  int32_t result = 0;
  instream >> result;

  EXPECT_EQ(result, -42);
}

TEST(Bitstream, Int32Chain) {
  BitOutstream outstream;

  for (short _ = 0; _ < 2; ++_)
    outstream << (int32_t)-42;

  BitInstream instream(outstream.str());

  int32_t result = 0;
  for (short _ = 0; _ < 2; ++_)
    instream >> result;

  EXPECT_EQ(result, -42);
}

TEST(Bitstream, String) {
  BitOutstream outstream;

  outstream << std::string("  Hello, world!  ");

  BitInstream instream(outstream.str());

  std::string message;
  instream >> message;

  EXPECT_EQ(message, "  Hello, world!  ");
}

TEST(Bitstream, StringChain) {
  BitOutstream outstream;

  for (short _ = 0; _ < 2; ++_)
    outstream << std::string("  Hello, world!  ");

  BitInstream instream(outstream.str());

  std::string message;
  for (short _ = 0; _ < 2; ++_)
    instream >> message;

  EXPECT_EQ(message, "  Hello, world!  ");
}

TEST(Bitstream, EmptyString) {
  BitOutstream outstream;

  outstream << std::string{};

  BitInstream instream(outstream.str());

  std::string message;
  instream >> message;

  std::string og;

  EXPECT_EQ(message, og);
}

TEST(Bitstream, Basic) {
  BitOutstream outstream;

  outstream << 42.0f << std::string("  Hello, world!  ") << (uint16_t)13;

  std::string data = outstream.str();

  BitInstream instream(data);

  std::string message;
  uint16_t u16;
  float fl;

  instream >> fl >> message >> u16;

  EXPECT_EQ(fl, 42.0f);
  EXPECT_EQ(u16, 13);
}

TEST(Bitstream, Stress) {
  BitOutstream outstream;

  srand(42);

  using Data = std::variant<std::string, uint16_t, float, int32_t>;
  std::vector<Data> transmission;

  for (unsigned id = 0; id < 10000; ++id) {
    // std::cout << "At " << id << ": ";
    unsigned type = rand() % 4;
    Data package = std::string();
    switch (type) {
    case 0: {
      std::string pack;
      for (unsigned _ = 0; _ < rand() % 100; ++_) {
        pack.push_back((char)rand());
      }
      package = pack;
      outstream << pack;
      // std::cout << "String " << std::quoted(pack);
    } break;
    case 1: {
      uint16_t pack = rand();
      package = pack;
      outstream << pack;
      // std::cout << "Uint16 " << pack;
    } break;
    case 2: {
      int32_t pack = rand();
      package = pack;
      outstream << pack;
      // std::cout << "Int32 " << pack;
    } break;
    case 3: {
      float pack = (float)rand() / (rand() + 1);
      package = pack;
      outstream << pack;
      // std::cout << "Float " << pack;
    } break;
    default:
      assert(false);
    }
    transmission.push_back(package);
  }

  std::string data = outstream.str();

  BitInstream instream(data);

  unsigned id = 0;
  for (const Data &package : transmission) {
    std::visit(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          T variable;
          instream >> variable;
          ASSERT_EQ(variable, arg);
        },
        package);
  }
}
