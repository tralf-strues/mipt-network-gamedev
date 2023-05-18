#include "client.hpp"

#include <sstream>
#include <iostream>
#include <thread>

Client::Client() {
  host_ = enet_host_create(nullptr, 2, 2, 0, 0);
  assert(host_);
}

Client::~Client() { enet_host_destroy(host_); }

void Client::Run() {
  ConnectToLobby();

  std::thread input_thread(&Client::ProcessInput, this);
  input_thread.detach();

  while (true) {
    OnUpdate();

    ENetEvent event;

    while (enet_host_service(host_, &event, 10) > 0) {
      switch (event.type) {
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

void Client::ConnectToLobby() {
  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10888;

  lobby_peer_ = enet_host_connect(host_, &address, 2, 0);
}

void Client::ConnectToGameServer(const ENetAddress& address) {
  if (game_server_peer_ == nullptr) {
    game_server_peer_ = enet_host_connect(host_, &address, 2, 0);
    assert(game_server_peer_);
  }
}

void Client::OnUpdate() {
  static uint32_t last_message_time = enet_time_get();
  uint32_t cur_time = enet_time_get();

  if (game_server_peer_ != nullptr && (cur_time - last_message_time) > 3000) {
    std::stringstream message;
    message << "Player time: " << cur_time;

    ENetPacket* packet = enet_packet_create(message.str().c_str(), message.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(game_server_peer_, 0, packet);

    last_message_time = cur_time;
  }
}

void Client::OnReceivePacket(const char* data) {
  if (strstr(data, "Host=") == data) {
    char host[128];
    uint32_t port = 0;
    sscanf(data, "Host=%s port=%u", host, &port);

    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    ConnectToGameServer(address);
  } else {
    printf("%s\n", data);
  }
}

void Client::ProcessInput() {
  while (true) {
    std::string input;
    std::cin >> input;
    if (input == "Start") {
      ENetPacket* packet = enet_packet_create(input.c_str(), input.length() + 1, ENET_PACKET_FLAG_RELIABLE);
      enet_peer_send(lobby_peer_, 0, packet);
    }
  }
}

int main() {
  Client client{};
  client.Run();

  return 0;
}