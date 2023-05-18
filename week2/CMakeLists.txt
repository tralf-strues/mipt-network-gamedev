cmake_minimum_required(VERSION 3.13)

project(mipt-network-gamedev-week2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -O0 -fsanitize=address")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../")

add_subdirectory(../third_party third_party)

# Game Server
add_executable(game_server)

target_link_libraries(game_server PUBLIC enet_static)

target_sources(game_server
  PUBLIC
    src/game_server.hpp

  PRIVATE
    src/game_server.cpp
)

# Lobby Server
add_executable(lobby_server)

target_link_libraries(lobby_server PUBLIC enet_static)

target_sources(lobby_server
  PUBLIC
    src/lobby_server.hpp
  
  PRIVATE
    src/lobby_server.cpp
)

# Client
add_executable(client)

target_link_libraries(client PUBLIC enet_static)

target_sources(client
  PUBLIC
    src/client.hpp
  
  PRIVATE
    src/client.cpp
)