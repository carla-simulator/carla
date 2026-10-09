// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#ifdef __linux__

#include <carla/multigpu/secondary.h>

#include <boost/asio/ip/tcp.hpp>

#include <filesystem>
#include <memory>
#include <thread>

using namespace std::chrono_literals;

namespace {

  std::size_t CountThreads() {
    return static_cast<std::size_t>(
        std::distance(std::filesystem::directory_iterator("/proc/self/task"), {}));
  }

} // namespace

TEST(multigpu_secondary, retrying_a_refused_connection_does_not_add_threads) {
  using namespace carla::multigpu;

  constexpr auto observation_window = 3500ms;
  constexpr std::size_t worker_threads = 2u;

  uint16_t unused_port = 0u;
  {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor(
        io_context,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 0));
    unused_port = acceptor.local_endpoint().port();
  }

  const auto threads_before = CountThreads();
  auto secondary = std::make_shared<Secondary>("127.0.0.1", unused_port, [](auto, auto) {});
  secondary->Connect();
  std::this_thread::sleep_for(observation_window);
  const auto threads_after = CountThreads();
  secondary->Stop();

  ASSERT_LE(threads_after, threads_before + worker_threads);
}

#endif // __linux__
