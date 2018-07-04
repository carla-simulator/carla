// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Message.h"

#include <climits>
#include <random>

namespace util {
namespace message {

  shared_message make_empty(size_t size) {
    return size == 0u ?
        std::make_shared<Message>() :
        std::make_shared<Message>(size);
  }

  shared_message make_random(size_t size) {
    if (size == 0u)
      return make_empty();
    using random_bytes_engine = std::independent_bits_engine<
        std::random_device,
        CHAR_BIT,
        unsigned char>;
    random_bytes_engine rbe;
    auto message = make_empty(size);
    std::generate(begin(*message), end(*message), std::ref(rbe));
    return message;
  }

  std::string to_hex_string(const Message &msg, size_t length) {
    length = std::min(static_cast<size_t>(msg.size()), length);
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", msg.data()[i]);
    if (length < msg.size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get());
  }

} // namespace message
} // namespace util
