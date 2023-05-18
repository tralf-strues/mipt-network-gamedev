#include "game_server.hpp"

#include <chrono>
#include <sstream>

GameServer::GameServer() {
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10887;

  host_ = enet_host_create(&address, 32, 2, 0, 0);
  assert(host_);
}

GameServer::~GameServer() { enet_host_destroy(host_); }

void GameServer::Run() {
  while (true) {
    OnUpdate();

    ENetEvent event;

    while (enet_host_service(host_, &event, 10) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
          uint32_t id = static_cast<uint32_t>(players_.size());
          OnNewPlayer(players_.emplace_back(PlayerInfo{id, kPlayerNames[next_name_idx_], event.peer}));
          next_name_idx_ = (next_name_idx_ + 1) % kNamesCount;
          break;
        }
        case ENET_EVENT_TYPE_RECEIVE: {
          printf("Packet received '%s'\n", event.packet->data);
          enet_packet_destroy(event.packet);
          break;
        }
        default:
          break;
      };
    }
  }
}

void GameServer::OnUpdate() {
  static uint32_t last_message_time = enet_time_get();
  uint32_t cur_time = enet_time_get();

  if (cur_time - last_message_time > 3000) {
    MC_SendTime();
    MC_SendPing();

    last_message_time = cur_time;
  }
}

void GameServer::MC_SendTime() {
  std::stringstream message;
  message << "Server time: " << enet_time_get();

  ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_host_broadcast(host_, 0, packet);
}

void GameServer::MC_SendPing() {
  std::stringstream message;
  
  message << "Player ping:\n";
  for (const auto& player : players_) {
    message << "    - [name = \"" << player.name << "\", ping = " << player.peer->lastRoundTripTime << "]\n";
  }

  ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_UNSEQUENCED);
  enet_host_broadcast(host_, 1, packet);
}

void GameServer::OnNewPlayer(const PlayerInfo& player) {
  printf("New player \"%s\" with id = %u connected\n", player.name, player.id);

  MC_OnNewPlayer(player);
  Client_OnNewPlayer(player);
}

void GameServer::MC_OnNewPlayer(const PlayerInfo& player) {
  std::stringstream message;
  message << "New player \"" << player.name << "\" with id = " << player.id << " connected";

  ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_host_broadcast(host_, 0, packet);
}

void GameServer::Client_OnNewPlayer(const PlayerInfo& new_player) {
  std::stringstream message;
  
  message << "Player list:\n";
  for (const auto& player : players_) {
    message << "    - [name = \"" << player.name << "\", id = " << player.id << "]\n";
  }

  ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(new_player.peer, 0, packet);
}

int main() {
  GameServer game_server{};
  game_server.Run();

  return 0;
}