# Distributed Shared Memory (DSM) 2D Grid Game

A multiplayer maze game demonstrating DSM concepts with TCP networking, client-side prediction, and multiple consistency modes.

## Project Structure

```
├── SharedState.h    - Shared data structures (Player, GameState)
├── server.cpp       - Authoritative server with select() multiplexing
├── client.cpp       - DSM client with transparency wrapper
└── README.md        - This file
```

## Features

### Shared State (SharedState.h)
- **Memory-aligned structs** using `#pragma pack(push, 1)`
- **10x10 Unicode maze** with box-drawing characters
- **Player struct**: id, x, y, isActive
- **GameState struct**: grid[10][10] and 4 players

### Authoritative Server (server.cpp)
- **TCP Server** on port 5000
- **Master copy** of GameState
- **select() multiplexing** handles 2-4 clients without threads
- **Move validation**: Rejects moves into walls (█)
- **Broadcast**: Sends updated state to all clients after valid moves

### DSM Client (client.cpp)
- **DSMMemory class**: Transparency wrapper hiding networking
- **Two consistency modes**:
  - **Sequential**: Every move sent immediately
  - **Release**: Moves buffered, sent on ENTER key press
- **Client-side prediction**: Move locally, snap back if server corrects
- **ANSI rendering**: Smooth 60 FPS display with colored players

## Building

### Using g++ (MinGW on Windows)
```bash
# Compile server
g++ -o server.exe server.cpp -lws2_32

# Compile client
g++ -o client.exe client.cpp -lws2_32
```

### Using MSVC (Visual Studio)
```bash
# Compile server
cl /EHsc server.cpp ws2_32.lib /Fe:server.exe

# Compile client
cl /EHsc client.cpp ws2_32.lib /Fe:client.exe
```

## Running

### 1. Start the Server
```bash
./server.exe
```
The server will listen on port 5000 and wait for clients.

### 2. Start Client(s)
Open new terminal windows and run:
```bash
./client.exe
```

Choose consistency mode:
- **1**: Sequential (immediate)
- **2**: Release (buffered)

### 3. Play
- **W/A/S/D**: Move up/left/down/right
- **ENTER**: Release buffered moves (Release mode only)
- **Q**: Quit

## How It Works

### DSM Transparency
The `DSMMemory` class provides a simple interface:
```cpp
dsm.movePlayer(dx, dy);  // Networking hidden inside
```

### Sequential Consistency
```
Client: Press 'W' → Predict move → Send to server → Server validates → Broadcast
```

### Release Consistency
```
Client: Press 'W' → Buffer move → Predict locally
Client: Press 'D' → Buffer move → Predict locally
Client: Press ENTER → Send all buffered moves → Server processes → Broadcast
```

### Client-Side Prediction
Clients move immediately for responsive gameplay, but "snap back" if the server rejects the move (e.g., tried to walk into a wall).

## Technical Details

- **Protocol**: TCP sockets (reliable, ordered)
- **Port**: 5000
- **Max Players**: 4
- **Grid Size**: 10x10
- **Multiplexing**: `select()` system call (no threads)
- **State Sync**: Full GameState broadcast (sizeof ~140 bytes)
- **Rendering**: ANSI escape codes for terminal graphics

## DSM Concepts Demonstrated

1. **Shared Memory Model**: GameState struct shared across processes
2. **Consistency Models**: Sequential vs. Release
3. **Transparency**: DSMMemory hides networking complexity
4. **Caching**: Client maintains local copy with prediction
5. **Invalidation**: Server corrections override client predictions
6. **Master-Slave**: Server is authoritative, clients are replicas

## Troubleshooting

**"Connection failed"**: Make sure server is running first
**"Server full"**: Maximum 4 players connected
**Walls render wrong**: Ensure terminal supports Unicode (UTF-8)
**Laggy movement**: Network latency - try localhost only

Enjoy your DSM maze game! 🎮
