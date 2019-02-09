// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/Buffer.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>

namespace util {
namespace buffer {

  using carla::Buffer;

  using shared_buffer = std::shared_ptr<Buffer>;
  using const_shared_buffer = std::shared_ptr<const Buffer>;

  static inline shared_buffer make_empty(size_t size = 0u) {
    return size == 0u ?
        std::make_shared<Buffer>() :
        std::make_shared<Buffer>(size);
  }

  shared_buffer make_random(size_t size);

  template <typename T>
  static inline shared_buffer make(const T &buffer) {
    return std::make_shared<Buffer>(boost::asio::buffer(buffer));
  }

  static inline std::string as_string(const Buffer &buf) {
    return {reinterpret_cast<const char *>(buf.data()), buf.size()};
  }

  std::string to_hex_string(const Buffer &buf, size_t length = 16u);

} // namespace buffer
} // namespace util

namespace carla {

  static inline std::ostream &operator<<(std::ostream &out, const Buffer &buf) {
    out << "[" << buf.size() << " bytes] " << util::buffer::to_hex_string(buf);
    return out;
  }

  static inline bool operator==(const Buffer &lhs, const Buffer &rhs) {
    return
        (lhs.size() == rhs.size()) &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  static inline bool operator!=(const Buffer &lhs, const Buffer &rhs) {
    return !(lhs == rhs);
  }

} // namespace carla
