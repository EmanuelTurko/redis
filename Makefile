CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wconversion -O2 -g

all: server client

server: server.cpp protocol.h hashtable.h hashtable.cpp avl.h avl.cpp
	$(CXX) $(CXXFLAGS) server.cpp hashtable.cpp avl.cpp -o server

client: client.cpp protocol.h
	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f server client

.PHONY: all clean