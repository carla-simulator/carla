// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/streaming/Client.h>
#include <carla/streaming/Server.h>

#include <boost/asio/post.hpp>

#include <algorithm>

using namespace carla::streaming;
using namespace std::chrono_literals;

static auto make_special_message(size_t size) {
  std::vector<uint32_t> v(size/sizeof(uint32_t), 42u);
  carla::Buffer msg(v);
  EXPECT_EQ(msg.size(), size);
  carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(msg));

  return BufView;
}

class Benchmark {
public:

  Benchmark(uint16_t port, size_t message_size, double success_ratio)
    : _server(port),
      _client(),
      _message(make_special_message(message_size)),
      _client_callback(),
      _work_to_do(_client_callback),
      _success_ratio(success_ratio) {}

  void AddStream() {
    Stream stream = _server.MakeStream();

    _client.Subscribe(stream.token(), [this](carla::Buffer msg) {
      carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(msg));
      DEBUG_ASSERT_EQ(BufView->size(), _message->size());
      boost::asio::post(_client_callback, [this]() {
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
          std::this_thread::sleep_for(11ms); // ~90FPS.
          {
            CARLA_PROFILE_SCOPE(game, write_to_stream);
            stream.Write(_message);
          }
        }
      });
    }

    const auto expected_number_of_messages = _streams.size() * number_of_messages;
    const auto threshold =
        static_cast<size_t>(_success_ratio * static_cast<double>(expected_number_of_messages));

    for (auto i = 0u; i < 10; ++i) {
      std::cout << "received " << _number_of_messages_received
                << " of " << expected_number_of_messages
                << " messages,";
      if (_number_of_messages_received >= expected_number_of_messages) {
        break;
      }
      std::cout << " waiting..." << std::endl;
      std::this_thread::sleep_for(1s);
    }

    _client_callback.stop();
    _threads.JoinAll();
    std::cout << " done." << std::endl;

#ifdef NDEBUG
    ASSERT_GE(_number_of_messages_received, threshold);
#else
    if (_number_of_messages_received < threshold) {
      carla::log_warning("threshold unmet:", _number_of_messages_received, '/', threshold);
    }
#endif // NDEBUG
  }

private:

  carla::ThreadGroup _threads;

  Server _server;

  Client _client;

  const carla::SharedBufferView _message;

  boost::asio::io_context _client_callback;

  boost::asio::io_context::work _work_to_do;

  const double _success_ratio;

  std::vector<Stream> _streams;

  std::atomic_size_t _number_of_messages_received{0u};
};

static size_t get_max_concurrency() {
  size_t concurrency = std::thread::hardware_concurrency() / 2u;
  return std::max((size_t) 2u, concurrency);
}

static void benchmark_image(
    const size_t dimensions,
    const size_t number_of_streams = 1u,
    const double success_ratio = 1.0) {
  constexpr auto number_of_messages = 100u;
  carla::logging::log("Benchmark:", number_of_streams, "streams at 90FPS.");
  Benchmark benchmark(TESTING_PORT, 4u * dimensions, success_ratio);
  benchmark.AddStreams(number_of_streams);
  benchmark.Run(number_of_messages);
}

TEST(benchmark_streaming, image_200x200) {
  benchmark_image(200u * 200u);
}

TEST(benchmark_streaming, image_800x600) {
  benchmark_image(800u * 600u, 1u, 0.9);
}

TEST(benchmark_streaming, image_1920x1080) {
  benchmark_image(1920u * 1080u, 1u, 0.9);
}

TEST(benchmark_streaming, image_200x200_mt) {
  benchmark_image(200u * 200u, get_max_concurrency());
}

TEST(benchmark_streaming, image_800x600_mt) {
  benchmark_image(800u * 600u, get_max_concurrency(), 0.9);
}

TEST(benchmark_streaming, image_1920x1080_mt) {
  benchmark_image(1920u * 1080u, get_max_concurrency(), 0.9);
}
