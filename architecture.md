# 2D DSM Game - System Architecture

## 1. Introduction and Background

### Problem Statement
Traditional operating system concepts (process management, memory allocation, scheduling) are abstract and difficult to visualize for students. This 2D DSM (Distributed Shared Memory) game aims to provide an interactive learning experience.

### Motivation
- Make OS concepts tangible through gameplay
- Demonstrate DSM principles in a visual environment
- Provide hands-on experience with concurrent systems

### Objectives
- Implement a functional 2D game with DSM architecture
- Simulate process synchronization and memory sharing
- Create an educational tool for OS concepts

## 2. Literature Review

### Existing Solutions
- Traditional DSM implementations (Ivy, TreadMarks)
- Educational OS simulators (OSP, Nachos)
- Game-based learning platforms

### Related Work
- Process synchronization in multiplayer games
- Memory consistency models
- Distributed systems in gaming

## 3. System Design and Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │  Game Logic  │  │   Renderer   │  │  Input Mgr   │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
└─────────┼──────────────────┼──────────────────┼─────────┘
          │                  │                  │
┌─────────┼──────────────────┼──────────────────┼─────────┐
│         │     DSM Management Layer            │         │
│  ┌──────▼───────┐  ┌──────▼───────┐  ┌──────▼───────┐  │
│  │ Memory Mgr   │  │  Sync Manager│  │ State Manager│  │
│  │ (Paging/     │  │ (Locks/      │  │ (Game State) │  │
│  │  Segmentation│  │  Semaphores) │  │              │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
└─────────┼──────────────────┼──────────────────┼─────────┘
          │                  │                  │
┌─────────┼──────────────────┼──────────────────┼─────────┐
│         │       Operating System Layer        │         │
│  ┌──────▼───────┐  ┌──────▼───────┐  ┌──────▼───────┐  │
│  │   Process    │  │   Thread     │  │   Memory     │  │
│  │   Scheduler  │  │   Pool       │  │   Allocator  │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Component Diagram

```
┌──────────────────────────────────────────────────────────┐
│                      Game Engine                         │
│                                                           │
│  ┌─────────────┐         ┌─────────────┐                │
│  │   Player    │◄────────┤   Entity    │                │
│  │   Manager   │         │   Manager   │                │
│  └──────┬──────┘         └──────┬──────┘                │
│         │                       │                        │
│         │    ┌──────────────────▼─────────┐             │
│         │    │   Collision Detection      │             │
│         │    └────────────────────────────┘             │
│         │                                                │
│  ┌──────▼──────┐         ┌─────────────┐                │
│  │  DSM Memory │◄────────┤   Physics   │                │
│  │   Manager   │         │   Engine    │                │
│  └─────────────┘         └─────────────┘                │
│         │                                                │
│  ┌──────▼──────────────────────────────┐                │
│  │    Synchronization Primitives       │                │
│  │  (Mutexes, Semaphores, Barriers)    │                │
│  └─────────────────────────────────────┘                │
└──────────────────────────────────────────────────────────┘
```

### Process and Thread Architecture

```
Main Game Process
├── Rendering Thread (Priority: High)
│   ├── Draw sprites
│   └── Update display
├── Game Logic Thread (Priority: Normal)
│   ├── Update entities
│   ├── Handle collisions
│   └── Process game rules
├── Input Handler Thread (Priority: High)
│   └── Capture user input
└── DSM Manager Thread (Priority: Normal)
    ├── Memory synchronization
    ├── State consistency
    └── Lock management
```

### Memory Architecture (DSM)

```
┌─────────────────────────────────────────────────────────┐
│                  Virtual Address Space                   │
│                                                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │   Shared     │  │   Player     │  │    Game      │  │
│  │   Memory     │  │   Local      │  │   Assets     │  │
│  │   Region     │  │   Memory     │  │   (Textures) │  │
│  │              │  │              │  │              │  │
│  │ - Game State │  │ - Input Buf  │  │ - Sprites    │  │
│  │ - Entities   │  │ - Temp Data  │  │ - Sounds     │  │
│  │ - Sync Vars  │  │              │  │              │  │
│  └──────┬───────┘  └──────────────┘  └──────────────┘  │
│         │                                                │
└─────────┼────────────────────────────────────────────────┘
          │
    ┌─────▼──────┐
    │  Page Table│
    │  (MMU)     │
    └────────────┘
```

### Data Flow Diagram

```
User Input
    │
    ▼
┌───────────┐      ┌──────────────┐      ┌──────────────┐
│  Input    │─────►│  Game Logic  │─────►│   DSM Mgr    │
│  Handler  │      │  Processing  │      │   (Sync)     │
└───────────┘      └──────┬───────┘      └──────┬───────┘
                          │                     │
                          ▼                     ▼
                   ┌──────────────┐      ┌──────────────┐
                   │   Physics    │      │   Shared     │
                   │   Update     │      │   Memory     │
                   └──────┬───────┘      └──────┬───────┘
                          │                     │
                          └─────────┬───────────┘
                                    ▼
                             ┌──────────────┐
                             │   Renderer   │
                             │   (Display)  │
                             └──────────────┘
```

### Sequence Diagram: Player Movement

```
Player    Input Handler    Game Logic    DSM Manager    Renderer
  │            │               │              │            │
  │──Press W──►│               │              │            │
  │            │──Input Event─►│              │            │
  │            │               │──Lock Req───►│            │
  │            │               │◄─Lock Grant──│            │
  │            │               │              │            │
  │            │               │──Update Pos──┤            │
  │            │               │              │            │
  │            │               │──Unlock─────►│            │
  │            │               │              │            │
  │            │               │──Render Req────────────►  │
  │            │               │              │            │
  │◄───────────────────────────────────────────Draw Frame──│
```

## 4. Implementation Details

### Technologies
- **Language**: C++ (based on existing client.cpp and server.cpp)
- **DSM Simulation**: Custom implementation using threading
- **Synchronization**: Mutex locks, semaphores, condition variables
- **Memory Management**: Manual page table simulation via SharedState.h

### Key Algorithms

#### 1. Memory Consistency Protocol
```
On Write:
1. Acquire lock for memory page
2. Update local copy
3. Invalidate other copies (if strict consistency)
4. Release lock

On Read:
1. Check if local copy is valid
2. If invalid, fetch from shared memory
3. Update local copy
4. Return data
```

#### 2. Process Scheduling
```
Round-robin scheduling for game threads
Priority-based for rendering (high priority)
```

### Code Segments (Key Components)

```cpp
class DSMManager {
private:
    std::map<int, void*> shared_memory;
    std::map<int, std::mutex> locks;
    std::map<int, PageEntry> page_table;
    
public:
    void* read(int address) {
        // Check page validity
        // Fetch if needed
        // Return data
    }
    
    void write(int address, void* data) {
        // Acquire lock
        locks[address].lock();
        // Update memory
        shared_memory[address] = data;
        // Invalidate other copies
        invalidateOtherCopies(address);
        // Release lock
        locks[address].unlock();
    }
    
    void invalidateOtherCopies(int address) {
        // Notify other processes/threads
        // Mark their copies as invalid
    }
};
```

## 5. Results and Evaluation

### Testing Methodology
- Unit tests for DSM operations
- Integration tests for game mechanics
- Performance benchmarking
- Concurrency stress tests

### Performance Metrics
- Frame rate (target: 60 FPS)
- Memory consistency overhead
- Lock contention measurement
- Response time to user input

### Analysis
- Memory access patterns
- Synchronization bottlenecks
- Scalability considerations

## 6. Conclusion and Future Work

### Achievements
- Functional 2D game with DSM concepts
- Visual demonstration of OS principles
- Educational tool for concurrent systems

### Limitations
- Simplified DSM model (not distributed across network)
- Limited scalability
- Performance overhead from synchronization

### Potential Improvements
- Network-based DSM implementation
- More complex memory consistency models
- Additional OS concepts (deadlock detection, resource allocation)
- Enhanced visualization of internal states

---

## Class Diagram

```
┌─────────────────┐
│     Entity      │
├─────────────────┤
│ + id: int       │
│ + position: Vec2│
│ + velocity: Vec2│
├─────────────────┤
│ + update()      │
│ + render()      │
└────────┬────────┘
         │
         ├──────────┬──────────┐
         │          │          │
    ┌────▼───┐ ┌───▼────┐ ┌───▼────┐
    │ Player │ │ Enemy  │ │ Item   │
    └────────┘ └────────┘ └────────┘

┌─────────────────┐       ┌─────────────────┐
│   DSMManager    │◄──────│   GameState     │
├─────────────────┤       ├─────────────────┤
│ + memory: dict  │       │ + entities: list│
│ + locks: dict   │       │ + score: int    │
├─────────────────┤       ├─────────────────┤
│ + read()        │       │ + update()      │
│ + write()       │       │ + serialize()   │
│ + acquire_lock()│       └─────────────────┘
└─────────────────┘
```

## State Diagram: Game States

```
        ┌──────────┐
        │   INIT   │
        └────┬─────┘
             │
             ▼
        ┌──────────┐
    ┌──►│  MENU    │
    │   └────┬─────┘
    │        │ Start
    │        ▼
    │   ┌──────────┐
    │   │ PLAYING  │◄───┐
    │   └────┬─────┘    │
    │        │ Pause    │Resume
    │        ▼          │
    │   ┌──────────┐    │
    │   │  PAUSED  │────┘
    │   └────┬─────┘
    │        │ Quit/Game Over
    │        ▼
    │   ┌──────────┐
    └───│GAME_OVER │
        └──────────┘
```
