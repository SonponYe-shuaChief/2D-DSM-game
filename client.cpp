#include "SharedState.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <vector>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

// Consistency modes
enum ConsistencyMode {
    SEQUENTIAL,  // Send every move immediately
    RELEASE      // Buffer moves, send on ENTER
};

// DSMMemory - Transparency wrapper that hides networking
class DSMMemory {
private:
    SOCKET serverSocket;
    GameState localState;
    GameState predictedState;
    int myPlayerId;
    ConsistencyMode mode;
    
    // For Release mode: buffer of pending moves
    struct Move {
        int32_t dx;
        int32_t dy;
    };
    std::vector<Move> pendingMoves;

    bool connectToServer(const char* host, int port) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            WSACleanup();
            return false;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, host, &serverAddr.sin_addr);

        if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        // Set socket to non-blocking mode for async receive
        u_long nonBlocking = 1;
        ioctlsocket(serverSocket, FIONBIO, &nonBlocking);

        std::cout << "Connected to server" << std::endl;
        return true;
    }

    void sendMoveToServer(int dx, int dy) {
        struct MoveRequest {
            int32_t playerId;
            int32_t dx;
            int32_t dy;
        };

        MoveRequest req;
        req.playerId = myPlayerId;
        req.dx = dx;
        req.dy = dy;

        send(serverSocket, (char*)&req, sizeof(MoveRequest), 0);
    }

    Player* getMyPlayer() {
        for (int i = 0; i < 4; i++) {
            if (predictedState.players[i].id == myPlayerId && predictedState.players[i].isActive) {
                return &predictedState.players[i];
            }
        }
        return nullptr;
    }

    bool isWalkable(int x, int y) {
        if (x < 0 || x >= 10 || y < 0 || y >= 10) return false;
        return predictedState.grid[y][x] == ' ';
    }

public:
    DSMMemory(const char* host, int port, ConsistencyMode consistencyMode) 
        : serverSocket(INVALID_SOCKET), myPlayerId(-1), mode(consistencyMode) {
        
        if (!connectToServer(host, port)) {
            throw std::runtime_error("Failed to connect to server");
        }

        // Wait for initial state
        std::cout << "Waiting for initial game state..." << std::endl;
        
        // Set socket to blocking temporarily
        u_long blocking = 0;
        ioctlsocket(serverSocket, FIONBIO, &blocking);
        
        int received = recv(serverSocket, (char*)&localState, sizeof(GameState), 0);
        if (received != sizeof(GameState)) {
            throw std::runtime_error("Failed to receive initial state");
        }

        // Set back to non-blocking
        u_long nonBlocking = 1;
        ioctlsocket(serverSocket, FIONBIO, &nonBlocking);

        // Copy to predicted state
        predictedState = localState;

        // Find my player ID (the most recently activated player)
        for (int i = 0; i < 4; i++) {
            if (localState.players[i].isActive) {
                myPlayerId = localState.players[i].id;
            }
        }

        std::cout << "Assigned Player ID: " << myPlayerId << std::endl;
        std::cout << "Consistency Mode: " << (mode == SEQUENTIAL ? "SEQUENTIAL" : "RELEASE") << std::endl;
    }

    // Main DSM transparency function
    void movePlayer(int dx, int dy) {
        if (dx == 0 && dy == 0) return;

        if (mode == SEQUENTIAL) {
            // Sequential: send immediately
            Player* player = getMyPlayer();
            if (!player) return;

            // Client-side prediction
            int newX = player->x + dx;
            int newY = player->y + dy;

            if (isWalkable(newX, newY)) {
                player->x = newX;
                player->y = newY;
            }

            // Send to server
            sendMoveToServer(dx, dy);

        } else { // RELEASE mode
            // Buffer the move
            pendingMoves.push_back({dx, dy});

            // Apply locally for immediate feedback
            Player* player = getMyPlayer();
            if (!player) return;

            int newX = player->x + dx;
            int newY = player->y + dy;

            if (isWalkable(newX, newY)) {
                player->x = newX;
                player->y = newY;
            }
        }
    }

    // Release trigger - send all buffered moves
    void releaseUpdates() {
        if (mode == RELEASE && !pendingMoves.empty()) {
            std::cout << "Releasing " << pendingMoves.size() << " buffered moves..." << std::endl;
            
            for (const Move& move : pendingMoves) {
                sendMoveToServer(move.dx, move.dy);
            }
            
            pendingMoves.clear();
        }
    }

    // Update from server (snap back if prediction was wrong)
    void syncWithServer() {
        GameState newState;
        int received = recv(serverSocket, (char*)&newState, sizeof(GameState), 0);

        if (received == sizeof(GameState)) {
            localState = newState;
            
            // Snap back to server state (correcting any wrong predictions)
            predictedState = localState;
        }
    }

    const GameState& getState() const {
        return predictedState;
    }

    int getMyPlayerId() const {
        return myPlayerId;
    }

    ~DSMMemory() {
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
    }
};

// Game renderer
class GameRenderer {
public:
    static void clearScreen() {
        std::cout << "\033[2J\033[H"; // ANSI clear screen and move cursor to top
    }

    static void render(const GameState& state, int myPlayerId) {
        clearScreen();

        std::cout << "=== DSM 2D Grid Game ===" << std::endl;
        std::cout << "My Player ID: " << myPlayerId << std::endl;
        std::cout << "Controls: WASD to move, ENTER to release (Release mode), Q to quit" << std::endl;
        std::cout << std::endl;

        // Render grid
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                bool playerHere = false;
                
                // Check if any player is at this position
                for (int p = 0; p < 4; p++) {
                    if (state.players[p].isActive && 
                        state.players[p].x == x && 
                        state.players[p].y == y) {
                        
                        // Render player with color
                        std::cout << PLAYER_COLORS[p] << (char)('0' + p) << "\033[0m";
                        playerHere = true;
                        break;
                    }
                }

                if (!playerHere) {
                    // Render grid cell
                    char cell = state.grid[y][x];
                    if (cell == '█') {
                        std::cout << "\033[90m█\033[0m"; // Dark gray for walls
                    } else {
                        std::cout << cell;
                    }
                }
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Active Players:" << std::endl;
        for (int i = 0; i < 4; i++) {
            if (state.players[i].isActive) {
                std::cout << PLAYER_COLORS[i] << "Player " << i 
                          << " (ID " << state.players[i].id << "): (" 
                          << state.players[i].x << ", " << state.players[i].y << ")"
                          << (state.players[i].id == myPlayerId ? " <- YOU" : "")
                          << "\033[0m" << std::endl;
            }
        }
    }
};

int main() {
    std::cout << "=== DSM Client ===" << std::endl;
    std::cout << "Select Consistency Mode:" << std::endl;
    std::cout << "1. Sequential (immediate updates)" << std::endl;
    std::cout << "2. Release (batch on ENTER)" << std::endl;
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;

    ConsistencyMode mode = (choice == 2) ? RELEASE : SEQUENTIAL;

    try {
        DSMMemory dsm("127.0.0.1", 5000, mode);

        GameRenderer::render(dsm.getState(), dsm.getMyPlayerId());

        bool running = true;
        while (running) {
            // Sync with server
            dsm.syncWithServer();

            // Handle input
            if (_kbhit()) {
                char key = _getch();
                
                int dx = 0, dy = 0;
                
                switch (key) {
                    case 'w': case 'W': dy = -1; break;
                    case 's': case 'S': dy = 1; break;
                    case 'a': case 'A': dx = -1; break;
                    case 'd': case 'D': dx = 1; break;
                    case '\r': // ENTER key
                        dsm.releaseUpdates();
                        break;
                    case 'q': case 'Q':
                        running = false;
                        break;
                }

                if (dx != 0 || dy != 0) {
                    dsm.movePlayer(dx, dy);
                }

                GameRenderer::render(dsm.getState(), dsm.getMyPlayerId());
            }

            // Small delay to prevent CPU spinning
            Sleep(16); // ~60 FPS
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Client shutting down..." << std::endl;
    return 0;
}
