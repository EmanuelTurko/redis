#pragma once

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <string_view>
#include <sys/socket.h>
#include <vector>
#include <unistd.h>

inline constexpr uint32_t maxMsg{32 << 20};
inline constexpr size_t maxArgs{200 * 1000};

class Fd {
public:
  Fd(){_val = -1;}
  explicit Fd(int fd) { _val = fd; }
  ~Fd() {
    if (_val != -1)
      close(_val);
  }

  Fd(const Fd &) = delete;
  Fd &operator=(const Fd &) = delete;

  Fd(Fd &&other) {
    _val = other._val;
    other._val = -1;
  }
  Fd &operator=(Fd &&other) {
    if (this != &other) {
      if (_val != -1)
        close(_val);

      _val = other._val;
      other._val = -1;
    }
    return *this;
  }
  auto get() const ->int { return _val; }

private:
  int _val;
};

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

inline auto getMonotonicMilliseconds()-> std::uint64_t{
  timespec time{};
  clock_gettime(CLOCK_MONOTONIC, &time);
  return static_cast<uint64_t>(time.tv_sec * 1000 + time.tv_nsec / 1000 / 1000);
}

enum class Tag : uint8_t{
    Nil    = 0,    // nil
    Error  = 1,    // error code + msg
    String = 2,    // string
    Int64  = 3,    // int64
    Double = 4,    // double
    Array  = 5,    // array
};

enum class TagError : std::uint32_t{
  Unknown = 1,
  TooBig = 2,
  BadType = 3,
  BadArg = 4,
};

enum class Types : std::uint32_t{
  Init = 0,
  String = 1, // string
  ZSet = 2,   // sorted set
};