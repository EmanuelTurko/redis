
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
#include <algorithm>
inline constexpr size_t maxMsg = 32 << 20;  // likely larger than the kernel buffer

void msg(const std::string_view msg) { std::cerr << msg << "\n"; }

void die(const std::string_view msg) {
  int err = errno;
  std::cerr << err << " " << msg << "\n";
  abort();
}

int32_t readFull(const int fileDescriptor, uint8_t* buffer, size_t len){

  while(len > 0){
    ssize_t returnVal = read(fileDescriptor,buffer,len);
    if(returnVal <= 0) return -1; // error or unexpected EOF

    assert(static_cast<size_t>(returnVal) <= len);
    
    len -= static_cast<size_t>(returnVal);
    buffer += returnVal;
  }

  return 0;
}

int32_t writeAll(const int fileDescriptor, const uint8_t* buffer, size_t len){

  while (len > 0) {
    ssize_t returnVal = write(fileDescriptor, buffer, len);
    if (returnVal <= 0)
      return -1; // error or unexpected EOF

    assert(static_cast<size_t>(returnVal) <= len);

    len -= static_cast<size_t>(returnVal);
    buffer += returnVal;
  }

  return 0;
}

// append to the back
void appendBuffer(std::vector<uint8_t> &buffer,const uint8_t* data,const size_t len){
  buffer.insert(buffer.end(),data,data+len);
}

// the `query` function was simply splited into `send_req` and `read_res`.
int32_t sendRequest(const int fileDescriptor, const uint8_t* text,const size_t len){

  if(len > maxMsg) return -1;

  uint32_t len32{static_cast<uint32_t>(len)};
  std::vector<uint8_t> wBuffer{};

  appendBuffer(wBuffer, reinterpret_cast<const uint8_t *>(&len32), 4);
  appendBuffer(wBuffer,text,len);

  return writeAll(fileDescriptor, wBuffer.data(), wBuffer.size());
}

int32_t readResult(const int fileDescriptor){

  // 4 bytes header
  std::vector<uint8_t> rBuffer(4);
  
  errno = 0;
  
  int32_t err = readFull(fileDescriptor, &rBuffer[0],4);
  if(err){
    if(errno == 0) msg("EOF");
    else msg("read() error");

    return err;
  }

  uint32_t len{};
  std::memcpy(&len,rBuffer.data(),4); //assume little endian
  if(len > maxMsg){
    msg("too long");
    return -1;
  }

  // reply body
  rBuffer.resize(4+len);
  err = readFull(fileDescriptor, &rBuffer[4], len);
  if(err){
    msg("read() error");
    return err;
  }

  // do something
  std::string_view printView{reinterpret_cast<const char *>(&rBuffer[4]),
                             std::min(len, 100u)};
  std::cout << "len: " << len << " data: " << printView << '\n';
  return 0;
}

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

  // multiple pipelined requests
  std::vector<std::string> queryList =
      {
          "hello1", "hello2", "hello3",
          // a large message requires multiple event loop iterations
          std::string(maxMsg, 'z'),
          "hello5",
      };

  for(const auto& query : queryList){
    int32_t err = sendRequest(fileDescriptor, reinterpret_cast<const uint8_t *>(query.data()), query.size());
    if(err) close(fileDescriptor);
  }

  for(size_t i = 0; i<queryList.size(); ++i){
    int32_t err = readResult(fileDescriptor);
    if(err) close(fileDescriptor);
  }

  close(fileDescriptor);

  return 0;
}