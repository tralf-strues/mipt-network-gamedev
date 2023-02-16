#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstring>
#include <iostream>

#include "message.hpp"
#include "socket_tools.hpp"

class Client {
 public:
  const char* GetSendPort()    const { return "2022"; }
  const char* GetReceivePort() const { return "2023"; }

  Client() = default;
  ~Client() = default;

  bool Init();
  bool Run();

 private:
  void ProcessMessageReply(MessageHeader header);

 private:
  int32_t receive_fd_{-1};
  int32_t send_fd_{-1};

  addrinfo send_addr_info_{};

  bool validated_by_server_{false};
  uint32_t id_{0};
};

bool Client::Init() {
  send_fd_    = CreateDgramSocket("localhost", GetSendPort(), &send_addr_info_);
  receive_fd_ = CreateDgramSocket(nullptr, GetReceivePort(), nullptr);

  bool success = (receive_fd_ != -1) && (send_fd_ != -1);

  std::cout << (success ? "Client initialized successfully" : "FAILED to initialize client") << std::endl;

  return success;
}

bool Client::Run() {
  {
    MessageHeader header{};
    header.type = MessageType::kConnectionInit;

    ssize_t res = sendto(send_fd_, &header, sizeof(header), 0, send_addr_info_.ai_addr, send_addr_info_.ai_addrlen);

    if (res == -1) {
      std::cout << strerror(errno) << std::endl;
    }
  }

  while (true) {
    if (validated_by_server_) {
      std::string input;
      std::cout << "> ";
      std::getline(std::cin, input);

      MessageHeader header{};
      header.type = MessageType::kPackageSend;
      header.package_send.user_id = id_;

      constexpr size_t kBufferMaxSize = 1024;
      static char buffer[kBufferMaxSize];

      std::memcpy(buffer, &header, sizeof(header));
      std::memcpy(buffer + sizeof(header), input.c_str(), input.size() + 1);

      ssize_t res = sendto(send_fd_, buffer, sizeof(header) + input.size() + 1, 0, send_addr_info_.ai_addr,
                           send_addr_info_.ai_addrlen);

      if (res == -1) {
        std::cout << strerror(errno) << std::endl;
      }
    }

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(receive_fd_, &read_set);

    timeval timeout = {0, 100000};  // 100 ms
    select(receive_fd_ + 1, &read_set, nullptr, nullptr, &timeout);

    if (FD_ISSET(receive_fd_, &read_set)) {
      constexpr size_t kBufSize = 1024;
      constexpr size_t kHeaderSize = sizeof(MessageHeader);

      static char buffer[kBufSize];
      memset(buffer, 0, kBufSize);

      ssize_t num_bytes = recvfrom(receive_fd_, buffer, kBufSize - 1, 0, nullptr, nullptr);
      if (num_bytes >= kHeaderSize) {
        ProcessMessageReply(*reinterpret_cast<MessageHeader*>(buffer));
      }
    }
  }

  return true;
}

void Client::ProcessMessageReply(MessageHeader header) {
  switch (header.type) {
    case MessageType::kConnectionInitResult: {
      std::cout << "Validated by server" << std::endl;

      validated_by_server_ = true;
      id_ = header.connection_init_result.user_id;
      break;
    }

    case MessageType::kPackageSendResult: {
      if (!header.package_send_result.success) {
        std::cout << "Unsuccessful package send\n";
      }

      break;
    }

    default: {
      std::cout << "Invalid MessageType received: " << static_cast<uint32_t>(header.type) << std::endl;
      break;
    }
  }
}

int main() {
  Client client{};

  bool init_success = client.Init();
  if (!init_success) {
    return 1;
  }

  bool run_success = client.Run();
  if (!run_success) {
    return 1;
  }

  return 0;
}
