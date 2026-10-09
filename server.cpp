#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include<string_view>

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

int32_t writeAll(const int fileDescriptor, std::string &buffer,
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

void msg(const std::string& msg) { std::cerr << msg << "\n"; }

void die(const std::string& msg) {
  int err = errno;
  std::cerr << err << " " << msg << "\n";
  abort();
}

int32_t oneRequest(const int clientSocket) {

  const size_t maxMsg{4096};
  std::string rBuffer(4 + maxMsg, '\0'); // 4 bytes header

  errno = 0;
  /** errno
   *! errno is set to the error code if the syscall failed. However, errno is
   *NOT ! set to 0 if the syscall succeeded; it simply keeps the previous value.
   *That’s ! why the above code sets errno = 0 before read_full() to distinguish
   *the EOF ! case.
   */

  int32_t err = readFull(clientSocket, rBuffer, 4);
  if (err) {
    msg(errno == 0 ? "EOF" : "read() error");
    return err;
  }

  uint32_t len{};
  std::memcpy(&len, rBuffer.data(), 4);
  if (len > maxMsg) {
    msg("too long");
    return -1;
  }

  // request body
  err = readFull(clientSocket, rBuffer, len, 4);
  if (err) {
    msg("read() error");
    return err;
  }

  std::string_view printView{rBuffer.data() + 4, len};
  std::cout << "client says: " << printView << "\n";

  // reply using the same protocol
  std::string_view reply{"world"};
  std::string wBuffer(4 + reply.length(), '\0');

  len = static_cast<uint32_t>(reply.length());
  std::memcpy(wBuffer.data(), &len, 4);
  std::memcpy(wBuffer.data() + 4, reply.data(), len);

  return writeAll(clientSocket, wBuffer, 4 + len);
}


/** Server setup
 * * socket() asks the kernel for a TCP socket and gets a file descriptor back.
 * * setsockopt(SO_REUSEADDR) lets you restart the server right away on the same
 * * port.
 * * bind() attaches the socket to port 1234 on every local address (0.0.0.0).
 * * listen() turns it into a listening socket. From here the kernel accepts
 * * connections on its behalf and queues them.
 * * accept() blocks until a client is in
 * * the queue.
 *   @param fileDescriptor
 */

int main() {

  // AF_INET => IPv4 , AF_INET6 => IPv6
  // SOCK_STREAM => TCP, SOCK_DGRAP => UDP
  int fileDescripctor = socket(AF_INET, SOCK_STREAM, 0);

  // this is needed for most server applications
  int val = 1;
  setsockopt(fileDescripctor, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  /*
  struct sockaddr_in {
  sa_family_t    sin_family; address family: AF_INET
  in_port_t sin_port;       port in network byte order
  struct in_addr sin_addr;  internet address
}; */

  /**
  * * bind
   */
  sockaddr_in address{}; // sockaddr_in holds IPv4 port pair
  address.sin_family = AF_INET;
  address.sin_port =
      htons(1234); // htons => Host To Network short (short = uint16_t)
  address.sin_addr.s_addr = htonl(0);

  int returnVal =
      bind(fileDescripctor, reinterpret_cast<const sockaddr *>(&address),
           sizeof(address));
  if (returnVal)
    die("bind()");

  /**
   * * listen
   */
  returnVal = listen(fileDescripctor, SOMAXCONN); // SOMAXCONN = 4096
  if (returnVal)
    die("listen()");


  while (true) {
    // accept

    sockaddr_in clientAdress{};
    socklen_t adressLength = sizeof(clientAdress);
    int clientSocket =
        accept(fileDescripctor, reinterpret_cast<sockaddr *>(&clientAdress),
               &adressLength); // addrlen => both input and output size
    if (clientSocket < 0)
      continue; // error

    // only serves one client connection at once
    while(true){
      int32_t err = oneRequest(clientSocket);
      if(err) break;
    }
    
    close(clientSocket);
  }
}
