// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ThreadGroup.h>
#include <carla/streaming/detail/Dispatcher.h>
#include <carla/streaming/detail/tcp/Client.h>
#include <carla/streaming/detail/tcp/Server.h>

#include <atomic>

TEST(streaming_low_level_tcp, small_message) {
  using namespace carla::streaming;
  using namespace carla::streaming::detail;

  boost::asio::io_service io_service;
  tcp::Server::endpoint ep(boost::asio::ip::tcp::v4(), TESTING_PORT);

  tcp::Server srv(io_service, ep);
  srv.SetTimeout(1s);
  std::atomic_bool done{false};
  std::atomic_size_t message_count{0u};

  const std::string msg = "Hola!";

  srv.Listen([&](std::shared_ptr<tcp::ServerSession> session) {
    ASSERT_EQ(session->get_stream_id(), 1u);
    while (!done) {
      session->Write(carla::Buffer(msg));
      std::this_thread::sleep_for(1ns);
    }
    std::cout << "done!\n";
  });

  Dispatcher dispatcher{make_endpoint<tcp::Client::protocol_type>(ep)};
  auto stream = dispatcher.MakeStream();
  tcp::Client c(io_service, stream.token(), [&](carla::Buffer message) {
    ++message_count;
    ASSERT_FALSE(message.empty());
    ASSERT_EQ(message.size(), 5u);
    const std::string received = util::buffer::as_string(message);
    ASSERT_EQ(received, msg);
  });

  // We need at least two threads because this server loop consumes one.
  carla::ThreadGroup threads;
  threads.CreateThreads(
      std::max(2u, std::thread::hardware_concurrency()),
      [&]() { io_service.run(); });

  std::this_thread::sleep_for(2s);
  io_service.stop();
  done = true;
  std::cout << "client received " << message_count << " messages\n";
  ASSERT_GT(message_count, 10u);
}
