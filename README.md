# Distributed Shared Memory (DSM) 2D Grid Game

A multiplayer maze game demonstrating DSM concepts with TCP networking, client-side prediction, and multiple consistency modes.

## Quick Start

### Prerequisites
- **Windows**: MSYS2 with MinGW-w64 (UCRT64) or MSVC
- **Required**: g++ compiler and ws2_32 library (Windows Sockets)

### How to Run

1. **Build the project**
   ```bash
   .\build.bat
   ```
   This compiles both `server.exe` and `client.exe`

2. **Start the server** (in one terminal)
   ```bash
   .\server.exe
   ```
   You should see: `Server initialized on port 5000`

3. **Start client(s)** (in separate terminals - up to 4 clients)
   ```bash
   .\client.exe
   ```
   
4. **Choose consistency mode**
   - Type `1` for **Sequential** (moves sent immediately)
   - Type `2` for **Release** (moves buffered until ENTER)

5. **Play the game**
   - **Arrow Keys** or **WASD**: Move your player
   - **ENTER**: Release buffered moves (Release mode only)
   - **Q**: Quit

### Example Session
```bash
# Terminal 1 - Server
> .\server.exe
Server initialized on port 5000
Server running. Waiting for clients...
Client connected. Assigned Player 0 (ID: 0)

# Terminal 2 - Client 1
> .\client.exe
Select Consistency Mode:
1. Sequential (immediate updates)
2. Release (batch on ENTER)
Choice: 1
Connected to server
Assigned Player ID: 0

# Terminal 3 - Client 2
> .\client.exe
Choice: 2
Connected to server
Assigned Player ID: 1
```

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
- **10x10 ASCII maze** with simple border and wall characters
- **Player struct**: id, x, y, isActive
- **GameState struct**: grid[10][10] and 4 players

### Authoritative Server (server.cpp)
- **TCP Server** on port 5000
- **Master copy** of GameState
- **select() multiplexing** handles 2-4 clients without threads
- **Move validation**: Rejects moves into walls (#)
- **Broadcast**: Sends updated state to all clients after valid moves

### DSM Client (client.cpp)
- **DSMMemory class**: Transparency wrapper hiding networking
- **Two consistency modes**:
  - **Sequential**: Every move sent immediately
  - **Release**: Moves buffered, sent on ENTER key press
- **Client-side prediction**: Move locally, snap back if server corrects
- **Clean rendering**: Simple ASCII display with player numbers and positions

## Building

### Windows with MSYS2 (Recommended)
```bash
# Easy build using provided script
.\build.bat

# Or manually with g++
g++ server.cpp -lws2_32 -o server.exe
g++ client.cpp -lws2_32 -o client.exe
```

### Using MSVC (Visual Studio)
```bash
cl /EHsc server.cpp ws2_32.lib /Fe:server.exe
cl /EHsc client.cpp ws2_32.lib /Fe:client.exe
```

### Using CMake (Alternative)
```bash
cmake -S . -B build
cmake --build build
```

## Running the Game

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
- **Arrow Keys** or **WASD**: Move up/left/down/right
- **ENTER**: Release buffered moves (Release mode only)
- **Q**: Quit

### Game Display
```
=== DSM 2D Grid Game ===
My Player ID: 0
Controls: Arrow Keys or WASD to move, ENTER to release (Release mode), Q to quit

+ - - - - - - - - - + 
| 0 # #       #     | 
|   #   #   #   #   | 
|       #       #   | 
| # #   # # #       | 
|               #   | 
|   # # # # #   #   | 
|                   | 
| #   # # #   #     | 
+ - - - - - - - - - + 

Active Players:
Player 0 (ID 0): (1, 1) <- YOU
```

Players are shown as numbers (0, 1, 2, 3), walls are `#`, borders are `+|-`

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
**"Permission denied" during build**: Close any running server.exe or client.exe
**Windows Defender blocks exe**: Add folder exclusion in Windows Security settings
**Laggy movement**: Network latency - try localhost only

## Notes

- Server must be started before clients
- Up to 4 players can connect simultaneously
- Each player gets a unique starting position in the maze
- Server validates all moves to prevent cheating
- Clean shutdown with Q key, or Ctrl+C to force quit

Enjoy your DSM maze game! 🎮
