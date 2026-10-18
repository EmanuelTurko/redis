# redis

A small Redis-like in-memory key-value server and command-line client, written in C++17. It follows the [Build Your Own Redis](https://build-your-own.org/redis) guide, reworked in idiomatic C++ (classes, RAII, `enum class`, the standard library threading primitives).

## Features

- Single-threaded, non-blocking event loop built on `poll()`, serving many clients at once
- Length-prefixed binary request/response protocol with pipelining support
- String keys stored in a hashtable with progressive rehashing
- Sorted sets backed by an AVL tree (ordered by score, then name) plus a hashtable (lookup by name)
- Key expiration (TTL) driven by a min-heap of expiry times
- Idle connections closed after 5 seconds without activity
- Large sorted sets (over 1000 members) freed on a background thread pool so deletes don't block the event loop

## Commands

| Command | Description | Reply |
|---|---|---|
| `set key value` | Set a string value | nil |
| `get key` | Get a string value | string, or nil if missing |
| `del key` | Delete a key | int: 1 if deleted, 0 if not found |
| `keys` | List all keys | array of strings |
| `pexpire key ms` | Set a time-to-live in milliseconds (negative removes it) | int: 1 if the key exists, 0 if not |
| `pttl key` | Remaining time-to-live in milliseconds | int: ms left, -1 if no TTL, -2 if not found |
| `zadd zset score name` | Add a member to a sorted set, or update its score | int: 1 if added, 0 if updated |
| `zrem zset name` | Remove a member from a sorted set | int: 1 if removed, 0 if not found |
| `zscore zset name` | Get a member's score | double, or nil if missing |
| `zquery zset score name offset limit` | Seek to the first member >= (score, name), skip `offset` members, then return results | array of name, score pairs (`limit` caps the number of array elements) |

Using a string command on a sorted set (or the reverse) returns a type error. Unknown commands return an error.

## Protocol

Every message, in both directions, is a 4-byte length followed by the body. Integers are written in host byte order (little-endian on x86). Messages are capped at 32 MB.

**Request:** `nstr` (4 bytes), then for each argument a 4-byte length and its bytes.

```
+------+------+-------+------+-------+-----
| nstr | len1 | str1  | len2 | str2  | ...
+------+------+-------+------+-------+-----
```

**Response:** a tagged value. The first byte is the type tag:

| Tag | Type | Payload |
|---|---|---|
| 0 | Nil | none |
| 1 | Error | 4-byte code, 4-byte length, message |
| 2 | String | 4-byte length, bytes |
| 3 | Int64 | 8 bytes |
| 4 | Double | 8 bytes |
| 5 | Array | 4-byte element count, then the elements |

Error codes: 1 unknown command, 2 response too big, 3 wrong type, 4 bad argument.

## Architecture

- **Event loop** (`server.cpp`): each iteration builds a `pollfd` list from the listening socket and every connection's intent (read, write or close), waits in `poll()` with a timeout taken from the nearest timer, then handles accepts, reads, writes and expired timers. Each connection keeps incoming and outgoing byte buffers so partial reads and writes are handled without blocking.
- **Key space**: a global `HashMap` of `Entry` objects. Each entry holds a string or a `ZSet`, and its index in the TTL heap.
- **Intrusive data structures**: hashtable, AVL tree, list and heap nodes are embedded in the objects they index. `containerOf` recovers the owning object from a node pointer.
- **Timers**: connections live in a `DList` ordered by last activity, so the oldest idle connection is always at the front. Key expirations live in a `Heap`. Expired keys are processed in batches of at most 2000 per loop iteration so a mass expiry can't stall the server.
- **Background deletion**: a `ThreadPool` of 4 worker threads (`std::thread`, `std::mutex`, `std::condition_variable`) takes work from a queue. Deleting a large sorted set is handed to it, after its TTL is removed on the main thread.
- **Resource management**: the `Fd` class owns a file descriptor and closes it in its destructor; connections are held as `std::unique_ptr<Connection>`.

## File layout

| File | Contents |
|---|---|
| `server.cpp` | Event loop, connection handling, request parsing, response serialization, command handlers, timers |
| `client.cpp` | Command-line client: sends one command and prints the decoded reply |
| `protocol.h` | Shared protocol constants, `Fd` RAII wrapper, buffer helpers, logging helpers, monotonic clock, `Tag` / `TagError` / `Types` enums |
| `hashtable.h` / `.cpp` | `HashMap`: chaining hashtable with progressive rehashing between two tables |
| `avl.h` / `.cpp` | `AVLTree`: self-balancing tree with subtree counts for rank-based offsets |
| `zset.h` / `.cpp` | `ZNode` and `ZSet`: sorted set combining `AVLTree` and `HashMap` |
| `list.h` | `DList`: intrusive circular doubly linked list |
| `heap.h` / `.cpp` | `Heap`: binary min-heap that keeps each item's position up to date |
| `threadpool.h` / `.cpp` | `ThreadPool`: fixed-size worker pool with a work queue |
| `util.h` | `containerOf` macro, FNV string hash, `stringToInt` / `stringToDouble` |
| `Makefile` | Build rules |

## Build

Requires Linux (or WSL) and a C++17 compiler (g++).

```sh
make          # builds ./server and ./client
make clean    # removes the binaries
```

The build uses `-std=c++17 -Wall -Wextra -Wconversion -O2 -fsanitize=address -g -pthread`.

## Run

Start the server; it listens on port 1234 on all interfaces:

```sh
./server
```

In another terminal, send commands with the client:

```sh
./client set a 1
./client get a
./client keys
./client pexpire a 10000
./client pttl a
./client del a

./client zadd scores 1.5 alice
./client zadd scores 2 bob
./client zscore scores alice
./client zquery scores 1 "" 0 10
./client zrem scores bob
```
