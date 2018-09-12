// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/streaming/Client.h>
#include <carla/streaming/Server.h>

using namespace carla::streaming;

static auto make_special_message(size_t size) {
  std::vector<uint32_t> v(size/sizeof(uint32_t), 42u);
  carla::Buffer msg(v);
  EXPECT_EQ(msg.size(), size);
  return msg;
}

class Benchmark {
public:

  Benchmark(uint16_t port, size_t message_size)
    : _server(port),
      _client(),
      _message(make_special_message(message_size)),
      _client_callback(),
      _work_to_do(_client_callback) {}

  void AddStream() {
    Stream stream = _server.MakeStream();

    _client.Subscribe(stream.token(), [this](carla::Buffer DEBUG_ONLY(msg)) {
      DEBUG_ASSERT_EQ(msg.size(), _message.size());
      DEBUG_ASSERT(msg == _message);
      _client_callback.post([this]() {
        CARLA_PROFILE_FPS(client, listen_callback);
        ++_number_of_messages_received;
      });
    });

    _streams.push_back(stream);
  }

  void AddStreams(size_t count) {
    for (auto i = 0u; i < count; ++i) {
      AddStream();
    }
  }

  void Run(size_t number_of_messages) {
    _threads.CreateThread([this]() { _client_callback.run(); });
    _server.AsyncRun(_streams.size());
    _client.AsyncRun(_streams.size());

    std::this_thread::sleep_for(1s); // the client needs to be ready so we make
                                     // sure we get all the messages.

    for (auto &&stream : _streams) {
      _threads.CreateThread([=]() mutable {
        for (auto i = 0u; i < number_of_messages; ++i) {
          CARLA_PROFILE_SCOPE(game, write_to_stream);
          stream << _message.buffer();
        }
      });
    }

    for (auto i = 0u; i < 30; ++i) {
      if (_number_of_messages_received >= (_streams.size() * number_of_messages)) {
        break;
      }
      std::cout << "received only " << _number_of_messages_received
                << " messages, waiting..." << std::endl;
      std::this_thread::sleep_for(1s);
    }

    _client_callback.stop();
    _threads.JoinAll();

    std::cout << _number_of_messages_received << " messages received; done.\n";

    _client.Stop();
    _server.Stop();
  }

private:

  carla::ThreadGroup _threads;

  Server _server;

  Client _client;

  const carla::Buffer _message;

  boost::asio::io_service _client_callback;

  boost::asio::io_service::work _work_to_do;

  std::vector<Stream> _streams;

  std::atomic_size_t _number_of_messages_received{0u};
};

static void benchmark_image(
    const size_t dimensions,
    const size_t number_of_streams = 1u) {
  constexpr auto number_of_messages = 100u;
  Benchmark benchmark(TESTING_PORT, 4u * dimensions);
  benchmark.AddStreams(number_of_streams);
  benchmark.Run(number_of_messages);
}

TEST(benchmark_streaming, image_200x200) {
  benchmark_image(200u * 200u);
}

TEST(benchmark_streaming, image_800x600) {
  benchmark_image(800u * 600u);
}

TEST(benchmark_streaming, image_1920x1080) {
  benchmark_image(1920u * 1080u);
}

TEST(benchmark_streaming, image_200x200_mt) {
  benchmark_image(200u * 200u, 9u);
}

TEST(benchmark_streaming, image_800x600_mt) {
  benchmark_image(800u * 600u, 9u);
}

TEST(benchmark_streaming, image_1920x1080_mt) {
  benchmark_image(1920u * 1080u, 9u);
}
