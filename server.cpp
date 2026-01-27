// Shared game state definitions and structures
#include "SharedState.h"

// Console output for logging
#include <iostream>

// Keep winsock2 before windows.h and trim Windows headers
#define WIN32_LEAN_AND_MEAN

// Core Windows sockets API for network communication
#include <winsock2.h>

// Windows API functions
#include <windows.h>

// TCP/IP protocol definitions and utilities
#include <ws2tcpip.h>

// Dynamic array for managing multiple client connections
#include <vector>

// String manipulation functions (memset, etc.)
#include <cstring>


class AuthoritativeServer {
private:
    SOCKET serverSocket;
    std::vector<SOCKET> clientSockets;
    GameState masterState;
    int nextPlayerId;

    void initializeGameState() {
        // Copy maze layout into game state
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                masterState.grid[i][j] = MAZE_LAYOUT[i][j];
            }
        }

        // Initialize all players as inactive
        for (int i = 0; i < 4; i++) {
            masterState.players[i].id = -1;
            masterState.players[i].x = 1;
            masterState.players[i].y = 1;
            masterState.players[i].isActive = false;
        }

        nextPlayerId = 0;
    }

    bool isLegalMove(int x, int y) {
        // Check bounds
        if (x < 0 || x >= 10 || y < 0 || y >= 10) {
            return false;
        }

        // Check if position is a wall
        char cell = masterState.grid[y][x];
        return (cell == ' '); // Only spaces are walkable


        //check if position is occupied by another player
        for (int i = 0; i < 4; i++) {
            if (masterState.players[i].isActive && 
                masterState.players[i].x == x && 
                masterState.players[i].y == y) {
                return false;
            }
        }
        return true;
    }

    void broadcastState() {
        for (SOCKET client : clientSockets) {
            int sent = send(client, (char*)&masterState, sizeof(GameState), 0);
            if (sent == SOCKET_ERROR) {
                std::cerr << "Failed to broadcast to client" << std::endl;
            }
        }
    }

    void handleNewConnection() {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET newClient = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);

        if (newClient == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            return;
        }

        // Find available player slot
        int playerSlot = -1;
        for (int i = 0; i < 4; i++) {
            if (!masterState.players[i].isActive) {
                playerSlot = i;
                break;
            }
        }

        if (playerSlot == -1) {
            std::cout << "Server full, rejecting connection" << std::endl;
            closesocket(newClient);
            return;
        }

        // Activate player
        masterState.players[playerSlot].id = nextPlayerId++;
        masterState.players[playerSlot].x = 1 + playerSlot * 2; // Spread starting positions
        masterState.players[playerSlot].y = 1;
        masterState.players[playerSlot].isActive = true;

        clientSockets.push_back(newClient);

        std::cout << "Client connected. Assigned Player " << playerSlot 
                  << " (ID: " << masterState.players[playerSlot].id << ")" << std::endl;

        // Send initial state to all clients
        broadcastState();
    }

    void handleClientData(SOCKET clientSocket, int clientIndex) {
        // Receive move request: playerId, dx, dy
        struct MoveRequest {
            int32_t playerId;
            int32_t dx;
            int32_t dy;
        };

        MoveRequest req;
        int received = recv(clientSocket, (char*)&req, sizeof(MoveRequest), 0);

        if (received <= 0) {
            // Client disconnected
            std::cout << "Client disconnected" << std::endl;

            // Deactivate player
            for (int i = 0; i < 4; i++) {
                if (masterState.players[i].id == req.playerId || 
                    (received == 0 && i == clientIndex)) {
                    masterState.players[i].isActive = false;
                    break;
                }
            }

            closesocket(clientSocket);
            clientSockets.erase(clientSockets.begin() + clientIndex);
            broadcastState();
            return;
        }

        // Find player
        Player* player = nullptr;
        for (int i = 0; i < 4; i++) {
            if (masterState.players[i].id == req.playerId && masterState.players[i].isActive) {
                player = &masterState.players[i];
                break;
            }
        }

        if (!player) {
            std::cerr << "Invalid player ID: " << req.playerId << std::endl;
            return;
        }

        // Calculate new position
        int newX = player->x + req.dx;
        int newY = player->y + req.dy;

        // Validate move
        if (isLegalMove(newX, newY)) {
            player->x = newX;
            player->y = newY;
            std::cout << "Player " << req.playerId << " moved to (" << newX << ", " << newY << ")" << std::endl;
        } else {
            std::cout << "Player " << req.playerId << " attempted illegal move to (" 
                      << newX << ", " << newY << ") - REJECTED" << std::endl;
        }

        // Broadcast updated state to all clients
        broadcastState();
    }

public:
    AuthoritativeServer() : serverSocket(INVALID_SOCKET), nextPlayerId(0) {
        initializeGameState();
    }

    bool initialize(int port) {
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
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Server initialized on port " << port << std::endl;
        return true;
    }

    void run() {
        std::cout << "Server running. Waiting for clients..." << std::endl;

        while (true) {
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(serverSocket, &readSet);

            SOCKET maxSocket = serverSocket;
            for (SOCKET client : clientSockets) {
                FD_SET(client, &readSet);
                if (client > maxSocket) {
                    maxSocket = client;
                }
            }

            // Use select() to handle multiple clients without threads
            timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int activity = select(maxSocket + 1, &readSet, nullptr, nullptr, &timeout);

            if (activity == SOCKET_ERROR) {
                std::cerr << "Select error" << std::endl;
                break;
            }

            // Check for new connections
            if (FD_ISSET(serverSocket, &readSet)) {
                handleNewConnection();
            }

            // Check for client data
            for (int i = 0; i < clientSockets.size(); ) {
                if (FD_ISSET(clientSockets[i], &readSet)) {
                    handleClientData(clientSockets[i], i);
                    // Don't increment i if client was removed
                    if (i < clientSockets.size() && FD_ISSET(clientSockets[i], &readSet)) {
                        i++;
                    }
                } else {
                    i++;
                }
            }
        }
    }

    ~AuthoritativeServer() {
        for (SOCKET client : clientSockets) {
            closesocket(client);
        }
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
    }
};

int main() {
    AuthoritativeServer server;
    
    if (!server.initialize(5000)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    server.run();

    return 0;
}
