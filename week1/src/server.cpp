#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <unordered_set>

#include "message.hpp"
#include "socket_tools.hpp"

class Server {
 public:
  const char* GetReceivePort() const { return "2022"; }
  const char* GetSendPort()    const { return "2023"; }

  Server() = default;
  ~Server() = default;

  bool Init();
  bool Run();

 private:
  void MessageReply(MessageHeader header, const char* data, ssize_t size);

 private:
  int32_t receive_fd_{-1};
  int32_t send_fd_{-1};

  addrinfo send_addr_info_{};

  uint64_t next_user_id_{0};
  std::unordered_set<uint64_t> user_ids_;
};

bool Server::Init() {
  receive_fd_ = CreateDgramSocket(nullptr, GetReceivePort(), nullptr);
  send_fd_    = CreateDgramSocket("localhost", GetSendPort(), &send_addr_info_);

  bool success = (receive_fd_ != -1) && (send_fd_ != -1);

  std::cout << (success ? "Server initialized successfully" : "FAILED to initialize server") << std::endl;

  return success;
}

bool Server::Run() {
  while (true) {
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
        MessageReply(*reinterpret_cast<MessageHeader*>(buffer), buffer + kHeaderSize, num_bytes - kHeaderSize);
      }
    }
  }

  return true;
}

void Server::MessageReply(MessageHeader header, const char* data, ssize_t size) {
  switch (header.type) {
    case MessageType::kConnectionInit: {
      std::cout << "New user added, user_id = " << next_user_id_ << std::endl;

      user_ids_.insert(next_user_id_);

      MessageHeader reply_header{};
      reply_header.type = MessageType::kConnectionInitResult;
      reply_header.connection_init_result.user_id = next_user_id_++;

      ssize_t res =
          sendto(send_fd_, &reply_header, sizeof(reply_header), 0, send_addr_info_.ai_addr, send_addr_info_.ai_addrlen);

      if (res == -1) {
        std::cout << strerror(errno) << std::endl;
      }

      break;
    }

    case MessageType::kPackageSend: {
      std::cout << "Message received [user_id= " << header.package_send.user_id << "]: " << data << std::endl;
      
      MessageHeader reply_header{};
      reply_header.type = MessageType::kPackageSendResult;
      reply_header.package_send_result.success = (user_ids_.count(header.package_send.user_id) != 0);

      ssize_t res =
          sendto(send_fd_, &reply_header, sizeof(reply_header), 0, send_addr_info_.ai_addr, send_addr_info_.ai_addrlen);

      if (res == -1) {
        std::cout << strerror(errno) << std::endl;
      }

      break;
    }

    case MessageType::kKeepAlive: {
      break;
    }

    default: {
      std::cout << "Invalid MessageType received: " << static_cast<uint32_t>(header.type) << std::endl;
      break;
    }
  }
}

int main() {
  Server server{};

  bool init_success = server.Init();
  if (!init_success) {
    return 1;
  }

  bool run_success = server.Run();
  if (!run_success) {
    return 1;
  }

  return 0;
}
