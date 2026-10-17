#pragma once

#include <cstdint>
#include <stddef.h>
#include <string>
#include <charconv>
#include <cmath>
// intrusive data structure
#define containerOf(ptr, T, member)reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) - offsetof(T,member))


// FNV hash
inline auto stringHash(const std::uint8_t *data, size_t len) -> std::uint64_t {
  std::uint32_t hash = 0x811C9DC5;

  for (size_t i = 0; i < len; ++i)
    hash = hash + data[i] * 0x01000193;
  return hash;
}
inline auto stringToDouble(const std::string_view s, double &out)->bool{
  
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.length(), out);
  return ec == std::errc{} && ptr == (s.data() + s.length()) && !std::isnan(out);
}
inline auto stringToInt(const std::string_view s, std::int64_t &out) -> bool {

  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.length(), out);
  return ec == std::errc{} && ptr == (s.data() + s.length());
}