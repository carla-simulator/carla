// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/streaming/Message.h>

#include <cstring>
#include <string>
#include <vector>

using namespace util::message;

TEST(streaming_message, to_from_string) {
  const std::string str = "The quick brown fox jumps over the lazy dog";
  Message msg(boost::asio::buffer(str));
  ASSERT_EQ(msg.size(), str.size());
  const std::string result = as_string(msg);
  ASSERT_EQ(result, str);
}

TEST(streaming_message, to_from_vector) {
  constexpr auto size = 1000u;
  using T = size_t;
  std::vector<T> v;
  v.reserve(size);
  for (auto i = 0u; i < size; ++i) {
    v.emplace_back(i);
  }
  Message msg(boost::asio::buffer(v));
  ASSERT_EQ(msg.size(), sizeof(T) * size);
  auto begin = reinterpret_cast<const T *>(msg.data());
  std::vector<T> result(begin, begin + size);
  ASSERT_EQ(result, v);
}

TEST(streaming_message, memcpy) {
  const std::string str = "The quick brown fox jumps over the lazy dog";
  Message msg(str.size());
  ASSERT_EQ(msg.size(), str.size());
  auto buffer = msg.buffer();
  std::memcpy(buffer.data(), str.data(), buffer.size());
  const std::string result = as_string(msg);
  ASSERT_EQ(result, str);
}

TEST(streaming_message, message_too_big) {
  ASSERT_THROW(Message(4294967296ul), std::invalid_argument);
}
