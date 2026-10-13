
#include "protocol.h"
#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

auto readFull(const int fileDescriptor, std::string &buffer, size_t len,
              const size_t startingOffset = 0) -> int32_t {

  size_t offset{startingOffset};
  while (len > 0) {
    ssize_t returnVal = read(fileDescriptor, buffer.data() + offset, len);
    if (returnVal <= 0)
      return -1; // error or unexpected EOF

    size_t bytesRead = static_cast<size_t>(returnVal);
    assert(bytesRead <= len);

    len -= bytesRead;
    offset += bytesRead;
  }

  return 0;
}
auto writeAll(const int fileDescriptor, std::string_view buffer,
              size_t len) -> int32_t {

  while (len > 0) {
    ssize_t returnVal = write(fileDescriptor, buffer.data(), len);
    if (returnVal <= 0)
      return -1; // error or unexpected EOF

    const size_t bytesWritten = static_cast<size_t>(returnVal);
    assert(bytesWritten <= len);

    len -= bytesWritten;
    buffer.remove_prefix(bytesWritten);
  }

  return 0;
}
// the `query` function was simply splited into `send_req` and `read_res`.
auto sendRequest(const int fileDescriptor,
                 const std::vector<std::string> &cmd) -> int32_t {

  uint32_t len{4};
  for (const std::string &s : cmd)
    len += 4 + s.size();
  if (len > maxMsg)
    return -1;

  std::string wBuffer(4 + maxMsg, '\0');
  std::memcpy(&wBuffer[0], &len, 4);

  uint32_t cmdSize = cmd.size();
  std::memcpy(&wBuffer[4], &cmdSize, 4);

  size_t current{8};

  for (const std::string &s : cmd) {
    uint32_t argLen = static_cast<uint32_t>(s.size());
    std::memcpy(&wBuffer[current], &argLen, 4);
    std::memcpy(&wBuffer[current + 4], s.data(), s.size());
    current += 4 + s.size();
  }

  return writeAll(fileDescriptor, wBuffer, 4 + len);
}
auto printResponse(const uint8_t *data, size_t size) -> int32_t {

  if (size < 1) {
    msg("bad response");
    return -1;
  }

  switch (data[0]) {
  case static_cast<int>(Tag::Nil):
    std::cout << "(nil)\n";
    return 1;
  case static_cast<int>(Tag::Error):
    if (size < 1 + 8) {
      msg("bad response");
      return -1;
    }
    {
      int32_t code{};
      uint32_t len{};

      std::memcpy(&code, &data[1], 4);
      std::memcpy(&len, &data[1 + 4], 4);

      if (size < 1 + 8 + len) {
        msg("bad response");
        return -1;
      }

      std::string_view printView{reinterpret_cast<const char *>(&data[1 + 8]),
                                 len};
      std::cout << "(error) " << code << " " << printView << "\n";
      return 1 + 8 + len;
    }
  case static_cast<int>(Tag::String):
    if (size < 1 + 4) {
      msg("bad response");
      return -1;
    }
    {
      uint32_t len{};
      std::memcpy(&len, &data[1], 4);
      if (size < 1 + 4 + len) {
        msg("bad response");
        return -1;
      }

      std::string_view printView{reinterpret_cast<const char *>(&data[1 + 4]),
                                 len};
      std::cout << "(string) " << " " << printView << "\n";
      return 1 + 4 + len;
    }
  case static_cast<int>(Tag::Int64):
    if (size < 1 + 8) {
      msg("bad response");
      return -1;
    }
    {
      int64_t val{};
      std::memcpy(&val, &data[1], 8);

      std::cout << "(int) " << val << "\n";
      return 1 + 8;
    }
  case static_cast<int>(Tag::Double):
    if (size < 1 + 8) {
      msg("bad response");
      return -1;
    }
    {
      double val{};
      std::memcpy(&val, &data[1], 8);
      std::cout << "(double) " << val << "\n";
      return 1 + 8;
    }
  case static_cast<int>(Tag::Array):
    if (size < 1 + 4) {
      msg("bad response");
      return -1;
    }
    {
      uint32_t len{};
      std::memcpy(&len, &data[1], 4);
      std::cout << "(array) len=" << len << "\n";

      size_t arrayBytes{1 + 4};
      for (uint32_t i = 0; i < len; ++i) {
        int32_t returnVal = printResponse(&data[arrayBytes], size - arrayBytes);
        if (returnVal < 0)
          return returnVal;
        arrayBytes += static_cast<size_t>(returnVal);
      }

      std::cout << "(arr) end\n";
      return static_cast<int32_t>(arrayBytes);
    }

  default:
    msg("bad response");
    return -1;
  }
}
auto readResult(const int fileDescriptor) -> int32_t {

  // 4 bytes header
  std::string rBuffer(4 + maxMsg, '\0');
  errno = 0;

  int32_t err = readFull(fileDescriptor, rBuffer, 4);
  if (err) {
    if (errno == 0)
      msg("EOF");
    else
      msg("read() error");

    return err;
  }

  uint32_t len{};
  std::memcpy(&len, rBuffer.data(), 4); // assume little endian
  if (len > maxMsg) {
    msg("too long");
    return -1;
  }

  // reply body
  err = readFull(fileDescriptor, rBuffer, len, 4);
  if (err) {
    msg("read() error");
    return err;
  }

  // print the result
  int32_t returnVal =
      printResponse(reinterpret_cast<uint8_t *>(&rBuffer[4]), len);
  if (returnVal > 0 && static_cast<uint32_t>(returnVal) != len) {
    msg("bad response");
    return -1;
  }

  return returnVal;
}
int main(int argc, char **argv) {

  int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (fileDescriptor < 0)
    die("socket()");

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = ntohs(1234);
  address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

  int returnVal =
      connect(fileDescriptor, reinterpret_cast<const sockaddr *>(&address),
              sizeof(address));
  if (returnVal)
    die("connect()");

  std::vector<std::string> cmd{};
  for (auto i = 1; i < argc; ++i) {
    cmd.push_back(argv[i]);
  }

  int32_t err = sendRequest(fileDescriptor, cmd);
  if (err)
    close(fileDescriptor);

  err = readResult(fileDescriptor);
  if (err)
    close(fileDescriptor);

  close(fileDescriptor);
  return 0;
}