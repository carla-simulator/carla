// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ThreadGroup.h>
#include <carla/streaming/low_level/Client.h>
#include <carla/streaming/low_level/Server.h>
#include <carla/streaming/detail/tcp/Client.h>
#include <carla/streaming/detail/tcp/Server.h>

#include <atomic>

TEST(streaming_low_level, sending_strings) {
  using namespace util::buffer;
  using namespace carla::streaming;
  using namespace carla::streaming::detail;
  using namespace carla::streaming::low_level;

  constexpr auto number_of_messages = 5'000u;
  const std::string message_text = "Hello client!";

  std::atomic_size_t message_count{0u};

  boost::asio::io_service io_service;

  Server<tcp::Server> srv(io_service, TESTING_PORT);
  srv.SetTimeout(1s);

  auto stream = srv.MakeStream();

  Client<tcp::Client> c;
  c.Subscribe(io_service, stream.token(), [&](auto message) {
    ++message_count;
    ASSERT_NE(message, nullptr);
    ASSERT_EQ(message->size(), message_text.size());
    const std::string msg = as_string(*message);
    ASSERT_EQ(msg, message_text);
  });

  carla::ThreadGroup threads;
  threads.CreateThreads(
      std::max(2u, std::thread::hardware_concurrency()),
      [&]() { io_service.run(); });

  for (auto i = 0u; i < number_of_messages; ++i) {
    stream << message_text;
  }

  std::this_thread::sleep_for(1s);
  io_service.stop();

  std::cout << "client received " << message_count << " messages\n";
}
