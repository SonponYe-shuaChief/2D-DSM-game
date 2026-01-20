#ifndef SHAREDSTATE_H
#define SHAREDSTATE_H

#include <cstdint>

// Ensure consistent memory layout across client and server
#pragma pack(push, 1)

// Player structure - each player has an ID, position, and active state
struct Player {
    int32_t id;
    int32_t x;
    int32_t y;
    bool isActive;
};

// GameState structure - holds the entire game state
struct GameState {
    // 10x10 maze grid using Unicode Box Drawing characters
    // Characters:
    //   ' ' (space) = path/walkable
    //   '█' = solid wall
    //   '╔' '╗' '╚' '╝' '║' '═' = maze borders
    char grid[10][10];
    
    // Array of 4 players
    Player players[4];
};

#pragma pack(pop)

// Maze layout constants
const char MAZE_LAYOUT[10][10] = {
    {'╔', '═', '═', '═', '═', '═', '═', '═', '═', '╗'},
    {'║', ' ', '█', ' ', ' ', ' ', '█', ' ', ' ', '║'},
    {'║', ' ', '█', ' ', '█', ' ', '█', ' ', '█', '║'},
    {'║', ' ', ' ', ' ', '█', ' ', ' ', ' ', '█', '║'},
    {'║', '█', '█', ' ', '█', '█', '█', ' ', ' ', '║'},
    {'║', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '█', '║'},
    {'║', ' ', '█', '█', '█', '█', '█', ' ', '█', '║'},
    {'║', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '║'},
    {'║', '█', ' ', '█', '█', '█', ' ', '█', ' ', '║'},
    {'╚', '═', '═', '═', '═', '═', '═', '═', '═', '╝'}
};

// Player color codes for rendering (ANSI escape sequences)
const char* PLAYER_COLORS[4] = {
    "\033[91m", // Red
    "\033[92m", // Green
    "\033[93m", // Yellow
    "\033[94m"  // Blue
};

#endif // SHAREDSTATE_H
