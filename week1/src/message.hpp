#pragma once

#include <stdint.h>

enum class MessageType : uint32_t {
  kConnectionInit,
  kConnectionInitResult,

  kPackageSend,
  kPackageSendResult,

  kKeepAlive
};

struct MessageHeaderConnectionInit {
};

struct MessageHeaderConnectionInitResult {
  uint32_t user_id{0};
};

struct MessageHeaderPackageSend {
  uint32_t user_id{0};
};

struct MessageHeaderPackageSendResult {
  bool success;
};

struct MessageHeaderKeepAlive {
};

struct MessageHeader {
  MessageType type;

  union {
    MessageHeaderConnectionInit       connection_init;
    MessageHeaderConnectionInitResult connection_init_result;
    MessageHeaderPackageSend          package_send;
    MessageHeaderPackageSendResult    package_send_result;
    MessageHeaderKeepAlive            keep_alive;
  };
};