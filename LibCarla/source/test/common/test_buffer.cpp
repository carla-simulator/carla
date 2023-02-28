// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/BufferPool.h>

#include <array>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace util::buffer;

TEST(buffer, compile) {
  carla::Buffer buffer;
  { std::array<boost::asio::const_buffer, 3u> s; buffer.copy_from(s); }
  { std::array<boost::asio::mutable_buffer, 3u> s; buffer.copy_from(s); }
  { std::vector<boost::asio::const_buffer> s; buffer.copy_from(s); }
  { std::vector<boost::asio::mutable_buffer> s; buffer.copy_from(s); }
  { std::list<boost::asio::const_buffer> s; buffer.copy_from(s); }
  { std::list<boost::asio::mutable_buffer> s; buffer.copy_from(s); }
  { std::set<boost::asio::const_buffer> s; buffer.copy_from(s); }
  { std::set<boost::asio::mutable_buffer> s; buffer.copy_from(s); }

  { boost::asio::const_buffer v; buffer.copy_from(v); }
  { boost::asio::mutable_buffer v; buffer.copy_from(v); }
  { int v[3u]; buffer.copy_from(v); }
  { std::vector<int> v; buffer.copy_from(v); }
  { std::string v; buffer.copy_from(v); }
  { std::wstring v; buffer.copy_from(v); }
  { struct C { int x = 0; } v[3u]; buffer.copy_from(v); }
  { struct C { int x = 0; }; std::array<C, 3u> v; buffer.copy_from(v); }
  { struct C { int x = 0; }; std::vector<C> v; buffer.copy_from(v); }
}

TEST(buffer, copy_buffer_sequence) {
  constexpr auto number_of_buffers = 15u;
  const std::string str = "WXdI<x->+<If$ua>$pu1AUBmS]?_PT{3z$B7L(E|?$]";
  std::string message;
  std::array<Buffer, number_of_buffers> buffers;
  std::array<boost::asio::const_buffer, number_of_buffers> sequence;
  for (auto i = 0u; i < number_of_buffers; ++i) {
    message += str;
    buffers[i].copy_from(str);
    sequence[i] = buffers[i].buffer();
  }
  auto result = Buffer(sequence);
  ASSERT_EQ(result.size(), message.size());
  auto result_str = as_string(result);
  ASSERT_EQ(result_str, message);
}

TEST(buffer, to_from_string) {
  const std::string str = "The quick brown fox jumps over the lazy dog";
  Buffer buffer(str);
  ASSERT_EQ(buffer.size(), str.size());
  const std::string result = as_string(buffer);
  ASSERT_EQ(result, str);
}

TEST(buffer, to_from_vector) {
  constexpr auto size = 1000u;
  using T = size_t;
  std::vector<T> v;
  v.reserve(size);
  for (auto i = 0u; i < size; ++i) {
    v.emplace_back(i);
  }
  Buffer buffer(v);
  ASSERT_EQ(buffer.size(), sizeof(T) * size);
  auto begin = reinterpret_cast<const T *>(buffer.data());
  std::vector<T> result(begin, begin + size);
  ASSERT_EQ(result, v);
}

TEST(buffer, copy) {
  auto msg = make_random(1024u);
  auto cpy = make_empty();
  cpy->copy_from(*msg);
  ASSERT_EQ(msg->size(), cpy->size());
  ASSERT_EQ(*cpy, *msg);
}

TEST(buffer, copy_with_offset) {
  const char str0[] = "Hello";
  const char str1[] = "buffer!";
  Buffer buffer;
  auto offset = static_cast<Buffer::size_type>(sizeof(str0));
  buffer.copy_from(
      offset,
      reinterpret_cast<const unsigned char *>(&str1),
      std::strlen(str1));
  std::memcpy(buffer.data(), str0, std::strlen(str0));
  buffer[std::strlen(str0)] = ' ';
  auto str = std::string(str0) + " " + str1;
  ASSERT_EQ(buffer.size(), str.size());
  ASSERT_EQ(as_string(buffer), str.c_str());
}

TEST(buffer, memcpy) {
  auto msg = make_random(1024u);
  auto cpy = make_empty(msg->size());
  ASSERT_EQ(msg->size(), cpy->size());
  auto buffer = cpy->buffer();
  std::memcpy(buffer.data(), msg->data(), buffer.size());
  ASSERT_EQ(*cpy, *msg);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST(buffer, message_too_big) {
  ASSERT_THROW(Buffer(4294967296ul), std::invalid_argument);
  Buffer buf;
  ASSERT_THROW(buf.reset(4294967296ul), std::invalid_argument);
}
#endif // LIBCARLA_NO_EXCEPTIONS

TEST(buffer, buffer_pool) {
  const std::string str = "Hello buffer!";
  auto pool = std::make_shared<carla::BufferPool>();
  {
    auto buff = pool->Pop();
    buff.copy_from(str);
  }
  auto buff1 = pool->Pop();
  ASSERT_EQ(as_string(buff1), str);
  auto buff2 = pool->Pop();
  ASSERT_NE(as_string(buff2), str);
  // Now delete the pool to test the weak reference inside the buffers.
  pool.reset();
}
