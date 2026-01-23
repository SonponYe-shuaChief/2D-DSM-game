cmake_minimum_required(VERSION 3.10)
project(DSM_2D_Game)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ensure UTF-8 source/exec charset so box-drawing maze characters compile on MSVC
if(MSVC)
    add_compile_options(/utf-8)
endif()

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
