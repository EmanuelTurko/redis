
#include <cerrno>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

void die(const std::string msg){
    int err = errno;
    std::cerr << err << " " << msg << "\n";
    abort();
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

  int fileDescripctor = socket(AF_INET, SOCK_STREAM, 0);
  if (fileDescripctor < 0)
    die("socket()");

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = ntohs(1234);
  address.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

  int returnVal =
      connect(fileDescripctor, reinterpret_cast<const sockaddr *>(&address),
              sizeof(address));
  if (returnVal)
    die("connect()");

  std::string msg = "hello";
  write(fileDescripctor, msg.data(), msg.length());

  std::string writeBuffer{};
  writeBuffer.resize(64);
  ssize_t numberOfBytes =
      read(fileDescripctor, writeBuffer.data(), writeBuffer.length() - 1);
  if (numberOfBytes < 0)
    die("read()");

  std::cout << "server says: " << writeBuffer << "\n";

  close(fileDescripctor);
 }