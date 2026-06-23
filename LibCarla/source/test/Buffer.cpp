// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Buffer.h"
#include <climits>
#include <random>

namespace util {
namespace buffer {

  shared_buffer make_random(size_t size) {
    if (size == 0u)
      return make_empty();
    // independent_bits_engine requires an unsigned result type wider than
    // char ([rand.req.genl]/1.6); MSVC enforces this with a static_assert.
    using random_bytes_engine = std::independent_bits_engine<
        std::random_device,
        CHAR_BIT,
        unsigned short>;
    random_bytes_engine rbe;
    auto buffer = make_empty(size);
    std::generate(buffer->begin(), buffer->end(), [&rbe]() {
      return static_cast<unsigned char>(rbe());
    });
    return buffer;
  }

  std::string to_hex_string(const Buffer &buf, size_t length) {
    length = std::min(static_cast<size_t>(buf.size()), length);
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", buf.data()[i]);
    if (length < buf.size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get());
  }

} // namespace buffer
} // namespace util
