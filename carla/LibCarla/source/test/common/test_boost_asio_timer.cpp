// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Regression guard for boost::asio steady_timer after the Boost 1.84 -> 1.90
// upgrade.  CARLA uses steady_timer in streaming/detail/tcp/{Client,ServerSession}
// and multigpu/{primary,secondary} for connection and session deadlines.

#include "test.h"

#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>

#include <atomic>
#include <chrono>
#include <thread>

// Verifies that steady_timer fires its handler within a reasonable wall-clock
// period.
TEST(BoostAsioTimer, TimerFiresWithinReasonableTime) {
  boost::asio::io_context io;
  boost::asio::steady_timer timer{io, std::chrono::milliseconds(10)};

  std::atomic<bool> fired{false};
  timer.async_wait([&](const boost::system::error_code &ec) {
    EXPECT_FALSE(ec) << "steady_timer error: " << ec.message();
    fired = true;
  });

  std::thread t{[&] { io.run(); }};
  t.join();

  EXPECT_TRUE(fired) << "steady_timer handler never fired";
}

// Verifies the expiry() <= steady_clock::now() pattern used in the LibCarla
// deadline-checking callbacks (Primary::StartTimer, ServerSession::StartTimer).
TEST(BoostAsioTimer, ExpiresAtNowComparison) {
  boost::asio::io_context io;
  boost::asio::steady_timer timer{io, std::chrono::milliseconds(5)};

  // Before expiry: expiry() should be in the future.
  auto expires_at = timer.expiry();
  auto now_before = std::chrono::steady_clock::now();
  EXPECT_GT(expires_at, now_before);

  // Run until the timer fires to advance real time past the expiry point.
  std::atomic<bool> done{false};
  timer.async_wait([&](const boost::system::error_code &) { done = true; });
  std::thread t{[&] { io.run(); }};
  t.join();

  // After expiry: now() should be >= expiry().
  auto now_after = std::chrono::steady_clock::now();
  EXPECT_GE(now_after, expires_at);
  EXPECT_TRUE(done);
}

// Verifies that std::chrono::milliseconds and io_context still compose
// correctly (needed by Time.h::time_duration::to_chrono used at every call
// site of steady_timer::expires_after in LibCarla).
TEST(BoostAsioTimer, ChronoDurationComposition) {
  const std::chrono::milliseconds d{250};
  EXPECT_EQ(d.count(), 250);
  EXPECT_EQ(std::chrono::duration_cast<std::chrono::seconds>(d).count(), 0);
}

// Pins the std::chrono::steady_clock::time_point::max() initialization used
// in ServerSession::ServerSession and Primary::Primary to mirror the old
// deadline_timer default of pos_infin.  StartTimer() guards on
// expiry() <= now(); a regression here would fire Close() before the socket
// is set up.
TEST(BoostAsioTimer, InitializedWithMaxIsInDistantFuture) {
  boost::asio::io_context io;
  boost::asio::steady_timer timer{io, std::chrono::steady_clock::time_point::max()};
  EXPECT_GT(timer.expiry(), std::chrono::steady_clock::now());
}

// Documents the trap that prompted the explicit max() init: a default
// constructed steady_timer has expiry() in the past (time_point::min()),
// unlike deadline_timer which defaulted to pos_infin.  If Boost ever changes
// this default, this test breaks loudly and the explicit max() inits can be
// dropped.
TEST(BoostAsioTimer, DefaultConstructedExpiryIsInPast) {
  boost::asio::io_context io;
  boost::asio::steady_timer timer{io};
  EXPECT_LE(timer.expiry(), std::chrono::steady_clock::now());
}

// Mirrors the cancel-before-close pattern in ServerSession::CloseNow and
// Primary::CloseNow: cancelling a pending wait must invoke the handler with
// operation_aborted, not silently drop it.
TEST(BoostAsioTimer, CancelTriggersAbortedError) {
  boost::asio::io_context io;
  boost::asio::steady_timer timer{io, std::chrono::seconds(60)};
  boost::system::error_code captured;
  std::atomic<bool> fired{false};
  timer.async_wait([&](const boost::system::error_code &ec) {
    captured = ec;
    fired = true;
  });
  timer.cancel();
  io.run();
  EXPECT_TRUE(fired);
  EXPECT_EQ(captured, boost::asio::error::operation_aborted);
}
