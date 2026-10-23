// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include <carla/streaming/detail/MultiStreamState.h>
#include <carla/streaming/detail/tcp/Server.h>

#include <chrono>
#include <cstdint>
#include <iostream>

bool PollStreamListener(carla::streaming::detail::MultiStreamState &state);

int main() {
  namespace cs = carla::streaming;
  namespace detail = carla::streaming::detail;
  boost::asio::io_context io;
  detail::tcp::Server server(io, {boost::asio::ip::address_v4::loopback(), 0u});
  const auto token = detail::token_type(1u, cs::make_endpoint(server.GetLocalEndpoint()));
  std::cout << "{\"sizeof_state\":" << sizeof(detail::MultiStreamState) << ",\"cases\":{";
  std::uint64_t checksum = 0u;
  constexpr int iterations = 10000000;
  constexpr int samples = 7;
  for (int mode = 0; mode < 4; ++mode) {
    detail::MultiStreamState state(token);
    auto session = std::make_shared<detail::Session>(io, std::chrono::seconds(1), server);
    if (mode == 1) state.ForceActive();
    if (mode == 2) state.EnableForROS();
    if (mode == 3) state.ConnectSession(session);
    for (int i = 0; i < 100000; ++i) checksum += PollStreamListener(state);
    const char *name = mode == 0 ? "inactive" : mode == 1 ? "forced" : mode == 2 ? "ros" : "one_client";
    if (mode != 0) std::cout << ',';
    std::cout << '"' << name << "\":[";
    for (int sample = 0; sample < samples; ++sample) {
      const auto start = std::chrono::steady_clock::now();
      for (int i = 0; i < iterations; ++i) checksum += PollStreamListener(state);
      const auto elapsed = std::chrono::steady_clock::now() - start;
      const double ns = std::chrono::duration<double, std::nano>(elapsed).count() / iterations;
      if (sample != 0) std::cout << ',';
      std::cout << ns;
    }
    std::cout << ']';
    if (mode == 3) state.DisconnectSession(session);
  }
  io.stop();
  std::cout << "},\"iterations_per_sample\":" << iterations
            << ",\"checksum\":" << checksum << "}\n";
}
