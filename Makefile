CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wconversion -O2 -g

all: server client

server: server.cpp hashtable.cpp hashtable.h protocol.h
	$(CXX) $(CXXFLAGS) server.cpp hashtable.cpp -o server

client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f server client

.PHONY: all clean