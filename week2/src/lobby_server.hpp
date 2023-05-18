#pragma once

#include <enet.h>

#include <vector>

class LobbyServer {
 public:
  LobbyServer();
  ~LobbyServer();

  void Run();

 private:
  void OnReceivePacket(const char* data);
  void MC_GameStart();

  void OnNewPlayer(ENetPeer* peer);
  void Client_GameInfo(ENetPeer* peer);

 private:
  ENetHost* host_{nullptr};
  bool game_started_{false};
};