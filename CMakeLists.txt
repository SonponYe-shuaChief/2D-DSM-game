cmake_minimum_required(VERSION 3.10)
project(DSM_2D_Game)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Server executable
add_executable(server server.cpp)

# Client executable
add_executable(client client.cpp)

# Link Windows socket library
if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif()

# Set output directory
set_target_properties(server client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
