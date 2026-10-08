#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void msg(const std::string msg) { std::cerr << msg << "\n"; }

void die(const std::string msg) {
  int err = errno;
  std::cerr << err << " " << msg << "\n";
  abort();
}

void doSomething(int clientSocket) {

  std::string readBuffer{};
  readBuffer.resize(64);

  ssize_t numberOfBytes =
      read(clientSocket, readBuffer.data(), readBuffer.length());
  if (numberOfBytes < 0) {
    msg("read() error");
    return;
  }

  std::cout << "client says: " << readBuffer << std::endl;

  std::string writeBuffer{"world"};

  write(clientSocket, writeBuffer.data(), writeBuffer.length());
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

    doSomething(clientSocket);
    close(clientSocket);
  }
}
