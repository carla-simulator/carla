// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/streaming/Message.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>

namespace util {
namespace message {

  using carla::streaming::Message;

  using shared_message = std::shared_ptr<Message>;
  using const_shared_message = std::shared_ptr<const Message>;

  static inline shared_message make_empty() {
    return std::make_shared<Message>();
  }

  shared_message make_empty(size_t size);

  shared_message make_random(size_t size);

  template <typename T>
  static inline shared_message make(const T &buffer) {
    return std::make_shared<Message>(boost::asio::buffer(buffer));
  }

  static inline std::string as_string(const Message &msg) {
    return {reinterpret_cast<const char *>(msg.data()), msg.size()};
  }

  std::string to_hex_string(const Message &msg, size_t length = 16u);

} // namespace message
} // namespace util

namespace carla {
namespace streaming {

  static inline unsigned char *begin(Message &msg) {
    return msg.data();
  }

  static inline const unsigned char *begin(const Message &msg) {
    return msg.data();
  }

  static inline unsigned char *end(Message &msg) {
    return msg.data() + msg.size();
  }

  static inline const unsigned char *end(const Message &msg) {
    return msg.data() + msg.size();
  }

  static inline std::ostream &operator<<(std::ostream &out, const Message &msg) {
    out << "[" << msg.size() << " bytes] " << util::message::to_hex_string(msg);
    return out;
  }

  static inline bool operator==(const Message &lhs, const Message &rhs) {
    return
        (lhs.size() == rhs.size()) &&
        std::equal(begin(lhs), end(lhs), begin(rhs));
  }

  static inline bool operator!=(const Message &lhs, const Message &rhs) {
    return !(lhs == rhs);
  }

} // namespace streaming
} // namespace carla
