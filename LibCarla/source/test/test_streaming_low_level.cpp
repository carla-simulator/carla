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

// This is required for low level to properly stop the threads in case of
// exception/assert.
class io_service_running {
public:

  boost::asio::io_service service;

  explicit io_service_running(size_t threads = 2u)
    : _work_to_do(service) {
    _threads.CreateThreads(threads, [this]() { service.run(); });
  }

  ~io_service_running() {
    service.stop();
  }

private:

  boost::asio::io_service::work _work_to_do;

  carla::ThreadGroup _threads;
};

TEST(streaming_low_level, sending_strings) {
  using namespace util::buffer;
  using namespace carla::streaming;
  using namespace carla::streaming::detail;
  using namespace carla::streaming::low_level;

  constexpr auto number_of_messages = 100u;
  const std::string message_text = "Hello client!";

  std::atomic_size_t message_count{0u};

  io_service_running io;

  Server<tcp::Server> srv(io.service, TESTING_PORT);
  srv.SetTimeout(1s);

  auto stream = srv.MakeStream();

  Client<tcp::Client> c;
  c.Subscribe(io.service, stream.token(), [&](auto message) {
    ++message_count;
    ASSERT_EQ(message.size(), message_text.size());
    const std::string msg = as_string(message);
    ASSERT_EQ(msg, message_text);
  });

  for (auto i = 0u; i < number_of_messages; ++i) {
    std::this_thread::sleep_for(2ms);
    stream << message_text;
  }

  std::this_thread::sleep_for(2ms);
  ASSERT_EQ(message_count, number_of_messages);
}

TEST(streaming_low_level, unsubscribing) {
  using namespace util::buffer;
  using namespace carla::streaming;
  using namespace carla::streaming::detail;
  using namespace carla::streaming::low_level;

  constexpr auto number_of_messages = 50u;
  const std::string message_text = "Hello client!";

  io_service_running io;

  Server<tcp::Server> srv(io.service, TESTING_PORT);
  srv.SetTimeout(1s);

  Client<tcp::Client> c;
  for (auto n = 0u; n < 10u; ++n) {
    auto stream = srv.MakeStream();
    std::atomic_size_t message_count{0u};

    c.Subscribe(io.service, stream.token(), [&](auto message) {
      ++message_count;
      ASSERT_EQ(message.size(), message_text.size());
      const std::string msg = as_string(message);
      ASSERT_EQ(msg, message_text);
    });

    for (auto i = 0u; i < number_of_messages; ++i) {
      std::this_thread::sleep_for(2ms);
      stream << message_text;
    }

    std::this_thread::sleep_for(2ms);
    c.UnSubscribe(stream.token());

    for (auto i = 0u; i < number_of_messages; ++i) {
      std::this_thread::sleep_for(2ms);
      stream << message_text;
    }

    ASSERT_EQ(message_count, number_of_messages);
  }
}
