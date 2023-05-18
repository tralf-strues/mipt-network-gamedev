#pragma once

#include <enet.h>

#include <vector>

constexpr uint32_t kNamesCount = 5;

const char* kPlayerNames[kNamesCount] = {
  "Gabriel",
  "Elliot",
  "Nicholas",
  "Wesley",
  "Jaxon"
};

struct PlayerInfo {
  uint32_t id;
  const char* name;

  ENetPeer* peer{nullptr};
};

class GameServer {
 public:
  GameServer();
  ~GameServer();

  void Run();

 private:
  void OnUpdate();
  void MC_SendTime();
  void MC_SendPing();

  void OnNewPlayer(const PlayerInfo& player);
  void MC_OnNewPlayer(const PlayerInfo& player);
  void Client_OnNewPlayer(const PlayerInfo& player);

 private:
  ENetHost* host_{nullptr};
  uint32_t next_name_idx_{0};

  std::vector<PlayerInfo> players_;
};