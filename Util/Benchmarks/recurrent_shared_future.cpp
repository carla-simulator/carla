// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/RecurrentSharedFuture.h>
#include <carla/client/WorldSnapshot.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <latch>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

int main() {
  constexpr size_t repetitions = 100000;
  constexpr size_t samples = 7;
  std::cout << "historical_waiters,episode_state_refs,median_set_ns,min_set_ns,max_set_ns\n";
  for (size_t count : {0u, 1u, 32u, 128u, 256u}) {
    carla::RecurrentSharedFuture<carla::client::WorldSnapshot> future;
    std::latch timed_out(static_cast<std::ptrdiff_t>(count));
    std::latch release(1u);
    std::vector<std::thread> threads;
    threads.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      threads.emplace_back([&]() {
        future.WaitFor(0ms);
        timed_out.count_down();
        release.wait();
      });
    }
    timed_out.wait();
    auto state = std::make_shared<carla::client::detail::EpisodeState>(42u);
    const carla::client::WorldSnapshot value(state);
    for (size_t i = 0; i < 1000; ++i) {
      future.SetValue(value);
    }
    std::vector<double> measurements;
    for (size_t sample = 0; sample < samples; ++sample) {
      const auto start = Clock::now();
      for (size_t i = 0; i < repetitions; ++i) {
        future.SetValue(value);
      }
      const auto end = Clock::now();
      measurements.push_back(std::chrono::duration<double, std::nano>(end - start).count() / repetitions);
    }
    std::sort(measurements.begin(), measurements.end());
    std::cout << count << ',' << state.use_count() << ',' << std::fixed << std::setprecision(2)
              << measurements[samples / 2] << ',' << measurements.front() << ','
              << measurements.back() << '\n';
    release.count_down();
    for (auto &thread : threads) {
      thread.join();
    }
  }
  carla::RecurrentSharedFuture<int> future;
  for (size_t i = 0; i < 1000; ++i) {
    future.WaitFor(0ms);
  }
  std::vector<double> measurements;
  for (size_t sample = 0; sample < samples; ++sample) {
    const auto start = Clock::now();
    for (size_t i = 0; i < repetitions; ++i) {
      future.WaitFor(0ms);
    }
    const auto end = Clock::now();
    measurements.push_back(std::chrono::duration<double, std::nano>(end - start).count() / repetitions);
  }
  std::sort(measurements.begin(), measurements.end());
  std::cout << "repeated_wait_timeout_ns," << measurements[samples / 2] << ','
            << measurements.front() << ',' << measurements.back() << '\n';
}
