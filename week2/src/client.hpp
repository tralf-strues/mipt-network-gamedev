#pragma once

#include <enet.h>

#include <vector>

class Client {
 public:
  Client();
  ~Client();

  void Run();

 private:
  void ConnectToLobby();
  void ConnectToGameServer(const ENetAddress& address);

  void OnUpdate();

  void OnReceivePacket(const char* data);

  void ProcessInput();

 private:
  ENetHost* host_{nullptr};

  ENetPeer* lobby_peer_{nullptr};
  ENetPeer* game_server_peer_{nullptr};
};