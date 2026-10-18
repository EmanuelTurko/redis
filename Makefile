CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wconversion -O2 -fsanitize=address -g

all: server client

server: server.cpp protocol.h hashtable.h hashtable.cpp avl.h avl.cpp zset.h zset.cpp util.h list.h
	$(CXX) $(CXXFLAGS) server.cpp hashtable.cpp avl.cpp zset.cpp -o server

client: client.cpp protocol.h
	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f server client

.PHONY: all clean