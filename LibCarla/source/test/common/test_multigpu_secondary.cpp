// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#ifdef __linux__

#include <carla/multigpu/secondary.h>

#include <boost/asio/ip/tcp.hpp>

#include <cstdint>
#include <filesystem>
#include <iterator>
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

  constexpr auto first_attempt = 500ms;
  constexpr auto retry_window = 3000ms;

  std::uint16_t unused_port = 0u;
  {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor(
        io_context,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 0));
    unused_port = acceptor.local_endpoint().port();
  }

  auto secondary = std::make_shared<Secondary>("127.0.0.1", unused_port, [](carla::multigpu::MultiGPUCommand, carla::Buffer) {});
  secondary->Connect();
  std::this_thread::sleep_for(first_attempt);
  const auto threads_after_first_attempt = CountThreads();
  std::this_thread::sleep_for(retry_window);
  const auto threads_after_retries = CountThreads();
  secondary->Stop();

  ASSERT_EQ(threads_after_retries, threads_after_first_attempt);
}

#endif // __linux__
