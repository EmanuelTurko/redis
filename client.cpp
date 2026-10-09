
#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

void msg(const std::string &msg) { std::cerr << msg << "\n"; }

void die(const std::string &msg) {
  int err = errno;
  std::cerr << err << " " << msg << "\n";
  abort();
}

int32_t readFull(const int fileDescriptor, std::string &buffer,
                 size_t numOfBytes, const size_t startingOffset = 0) {

  size_t offset{startingOffset};

  while (numOfBytes) {
    ssize_t returnVal =
        read(fileDescriptor, buffer.data() + offset, numOfBytes);
    if (returnVal == 0)
      return 0; // "EOF";
    else if (returnVal == -1) {
      if (errno == EINTR)
        continue; // try again
      else
        return -1; // error;
    }

    auto rvSize = static_cast<size_t>(returnVal);
    assert(rvSize <= numOfBytes);

    numOfBytes -= rvSize;
    offset += rvSize;
  }

  return 0;
}

int32_t writeAll(const int fileDescriptor, std::string_view buffer,
                 size_t numOfBytes) {

  size_t offset{};

  while (numOfBytes > 0) {
    ssize_t returnVal =
        write(fileDescriptor, buffer.data() + offset, numOfBytes);
    if (returnVal <= 0)
      return -1; // error

    auto rvSize = static_cast<size_t>(returnVal);
    assert(rvSize <= numOfBytes);

    numOfBytes -= rvSize;
    offset += rvSize;
  }

  return 0;
}

int32_t query(const int fileDescriptor, std::string_view text) {

  constexpr size_t maxMsg{4096};
  uint32_t len = static_cast<uint32_t>(text.length());
  if (len > maxMsg)
    return -1;

  // send request

  std::string wBuffer(4 + maxMsg, '\0');
  std::memcpy(wBuffer.data(), &len, 4);
  std::memcpy(wBuffer.data() + 4, text.data(), len);

  if (int32_t err = writeAll(fileDescriptor, wBuffer, 4 + len))
    return err;

  // 4 bytes header
  std::string rBuffer(4 + maxMsg, '\0');
  errno = 0;

  int32_t err = readFull(fileDescriptor, rBuffer, 4);
  if (err) {
    msg(errno == 0 ? "EOF" : "read() error");
    return err;
  }

  std::memcpy(&len, rBuffer.data(), 4);
  if (len > maxMsg) {
    msg("too long");
    return -1;
  }

  // reply body
  err = readFull(fileDescriptor, rBuffer, len, 4);
  if (err == -1) {
    msg("read() error");
    return err;
  }

  std::string_view printView{rBuffer.data() + 4, len};
  std::cout << "server says: " << printView << "\n";

  return 0;
}

/** Client setup

* * The client creates its own socket and calls connect() to 127.0.0.1:1234. The
* * kernel performs the TCP handshake with the server's kernel. Neither program
* * writes code for that part.

* * On the server, accept() now returns a new socket for this one client.

* * The client write()s "hello". The bytes travel through the kernel to the
* * server's socket.

* * The server read()s them, prints "client says: hello", and write()s "world"
* * back.

* * The client read()s "world", prints it, and closes its socket.
*/

int main() {

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

  int32_t err = query(fileDescriptor, "hello1");
  err = query(fileDescriptor, "hello2");
  
  close(fileDescriptor);
  return 0;
}