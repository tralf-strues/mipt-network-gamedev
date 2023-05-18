#include "lobby_server.hpp"

#include <sstream>

LobbyServer::LobbyServer() {
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10888;

  host_ = enet_host_create(&address, 32, 2, 0, 0);
  assert(host_);
}

LobbyServer::~LobbyServer() { enet_host_destroy(host_); }

void LobbyServer::Run() {
  while (true) {
    ENetEvent event;

    while (enet_host_service(host_, &event, 10) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
          OnNewPlayer(event.peer);
          break;
        }
        case ENET_EVENT_TYPE_RECEIVE: {
          OnReceivePacket(reinterpret_cast<const char*>(event.packet->data));
          enet_packet_destroy(event.packet);
          break;
        }
        default:
          break;
      };
    }
  }
}

void LobbyServer::OnReceivePacket(const char* data) {
  printf("Packet received '%s'\n", data);

  if (strcmp(data, "Start") == 0) {
    MC_GameStart();
  }
}

void LobbyServer::MC_GameStart() {
  printf("Starting game...\n");

  std::stringstream message;
  message << "Host=localhost port=10887";

  ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_host_broadcast(host_, 0, packet);

  game_started_ = true;
}

void LobbyServer::OnNewPlayer(ENetPeer* peer) {
  printf("New player connected!\n");

  if (game_started_) {
    Client_GameInfo(peer);
  }
}

void LobbyServer::Client_GameInfo(ENetPeer* peer) {
  printf("Redirecting to game server...\n");

  std::stringstream message;
  message << "Host=localhost port=10887";

  ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);
}

int main() {
  LobbyServer lobby_server{};
  lobby_server.Run();

  return 0;
}