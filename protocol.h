#pragma once

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string_view>
#include <vector>

inline constexpr uint32_t maxMsg{32 << 20};
inline constexpr size_t maxArgs{200 * 1000};


inline void msg(const std::string_view message) {
  std::cerr << message << "\n";
}

inline void msgErrno(const std::string_view msg) {
  std::cerr << "errno: " << errno << " " << msg << "\n";
}

inline void die(const std::string_view msg) {
  std::cerr << errno << " " << msg << "\n";
  abort();
}

// append to the back
inline void bufferAppendBytes(std::vector<uint8_t> &buffer, const uint8_t *data,
                  const size_t len) {
  buffer.insert(buffer.end(), data, data + len);
}

// remove from the front
inline void consumeBuffer(std::vector<uint8_t> &buffer, const size_t len) {
  buffer.erase(buffer.begin(), buffer.begin() + len);
}

enum class Tag : uint32_t{
    Nil    = 0,    // nil
    Error  = 1,    // error code + msg
    String = 2,    // string
    Int64  = 3,    // int64
    Double = 4,    // double
    Array  = 5,    // array
};

enum class TagError : uint32_t{
  Unknown = 1,
  TooBig = 2,
};