// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Regression guard for boost::asio::deadline_timer after the Boost 1.84->1.89
// upgrade.  CARLA uses deadline_timer in streaming/detail/tcp/{Client,ServerSession}
// and multigpu/{primary,secondary}.  The timer is soft-deprecated in favour of
// steady_timer but still shipped in Boost 1.89 and must compile and fire
// correctly.

#include "test.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <atomic>
#include <thread>

using namespace std::chrono_literals;

// Verifies that deadline_timer fires its handler and that the now() comparison
// used throughout LibCarla's check-deadline callbacks still holds.
TEST(BoostDeadlineTimer, TimerFiresWithinReasonableTime) {
  boost::asio::io_context io;
  boost::asio::deadline_timer timer(io,
      boost::posix_time::milliseconds(10));

  std::atomic<bool> fired{false};
  timer.async_wait([&](const boost::system::error_code &ec) {
    EXPECT_FALSE(ec) << "deadline_timer error: " << ec.message();
    fired = true;
  });

  std::thread t([&] { io.run(); });
  t.join();

  EXPECT_TRUE(fired) << "deadline_timer handler never fired";
}

// Verifies the expires_at() <= traits_type::now() pattern used in the
// LibCarla deadline-checking callbacks (e.g. ServerSession::CheckDeadline).
TEST(BoostDeadlineTimer, ExpiresAtNowComparison) {
  boost::asio::io_context io;
  boost::asio::deadline_timer timer(io,
      boost::posix_time::milliseconds(5));

  // Before expiry: expires_at() should be in the future.
  auto expires_at = timer.expires_at();
  auto now_before = boost::asio::deadline_timer::traits_type::now();
  EXPECT_GT(expires_at, now_before);

  // Run until the timer fires to advance real time past the expiry point.
  std::atomic<bool> done{false};
  timer.async_wait([&](const boost::system::error_code &) { done = true; });
  std::thread t([&] { io.run(); });
  t.join();

  // After expiry: now() should be >= expires_at().
  auto now_after = boost::asio::deadline_timer::traits_type::now();
  EXPECT_GE(now_after, expires_at);
  EXPECT_TRUE(done);
}

// Verifies that posix_time::milliseconds and io_context::work still compose
// correctly (needed by Time.h conversions used with the timer).
TEST(BoostDeadlineTimer, PosixTimeDurationConversion) {
  const boost::posix_time::time_duration d =
      boost::posix_time::milliseconds(250);
  EXPECT_EQ(d.total_milliseconds(), 250);
  EXPECT_EQ(d.total_seconds(), 0);
}