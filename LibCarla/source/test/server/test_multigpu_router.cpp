// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/listener.h>
#include <carla/multigpu/primary.h>
#include <carla/multigpu/router.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <chrono>
#include <future>
#include <memory>
#include <thread>

// These tests exercise the multi-GPU router's request/response bookkeeping
// (LibCarla/source/carla/multigpu/router.cpp) without a live secondary
// process. A "session" here is a carla::multigpu::Primary constructed
// directly against an unconnected socket: its Write()/ReadData() paths are
// never exercised, only Router's session-selection and promise-fulfillment
// logic, which is what the target bugs live in.
//
// The io_context must actually be running (as it always is in production,
// via Router::AsyncRun()) before a boost::asio acceptor/socket built on it is
// torn down; tearing one down without ever having run it is a codepath
// production never exercises and is not what these tests are about.

namespace {

  class MultiGpuRouterTest : public ::testing::Test {
  protected:
    boost::asio::io_context _io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _work_guard{
        boost::asio::make_work_guard(_io_context)};
    std::thread _io_thread{[this]() { _io_context.run(); }};
    boost::asio::ip::tcp::endpoint _dummy_endpoint{
        boost::asio::ip::make_address("127.0.0.1"), 0};
    carla::multigpu::Listener _listener{_io_context, _dummy_endpoint};

    std::shared_ptr<carla::multigpu::Primary> MakeFakeSession() {
      return std::make_shared<carla::multigpu::Primary>(
          _io_context, carla::time_duration::seconds(1u), _listener);
    }

    void TearDown() override {
      _work_guard.reset();
      _io_context.stop();
      _io_thread.join();
    }
  };

} // namespace

TEST_F(MultiGpuRouterTest, write_to_next_with_no_sessions_rejects_instead_of_hanging) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  // Specifically std::runtime_error (Router::RejectPromise), not merely
  // std::future_error(broken_promise): the promise here is never stored
  // anywhere, so a bare "some exception was thrown" assertion would also
  // pass if this rejection path were removed entirely and the local promise
  // were simply left to be destroyed unset.
  EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(MultiGpuRouterTest, write_to_one_with_expired_session_rejects_instead_of_hanging) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  std::weak_ptr<carla::multigpu::Primary> expired_session;
  {
    auto session = MakeFakeSession();
    expired_session = session;
  }
  ASSERT_TRUE(expired_session.expired());

  auto future = router.WriteToOne(
      expired_session, carla::multigpu::MultiGPUCommand::ENABLE_ROS, carla::Buffer());

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(MultiGpuRouterTest, write_to_one_with_disconnected_but_still_alive_session_rejects) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  router.TestDisconnectSession(session);
  // `session` is still a live shared_ptr here (held by this test), even
  // though the Router no longer considers it connected. This reproduces the
  // window production can hit when a Primary's socket has already closed
  // (DisconnectSession ran) but the object itself is kept alive a little
  // longer by another in-flight reference: locking the weak_ptr alone would
  // still succeed, so WriteToOne must also check _sessions membership.
  auto future = router.WriteToOne(
      session, carla::multigpu::MultiGPUCommand::ENABLE_ROS, carla::Buffer());

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(MultiGpuRouterTest, write_to_next_with_connected_session_stays_pending) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  router.TestConnectSession(MakeFakeSession());

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  // A real, still-connected session was picked: the request is dispatched
  // and the promise is left pending (no response will ever arrive in this
  // test, since nothing is listening on the other end). This must NOT be
  // rejected immediately by the same code path that handles a missing or
  // expired session.
  EXPECT_EQ(future.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
}

TEST_F(MultiGpuRouterTest, disconnect_rejects_pending_promise_for_that_session) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  ASSERT_EQ(future.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

  router.TestDisconnectSession(session);

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(MultiGpuRouterTest, overlapping_request_to_same_session_is_rejected_not_orphaned) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  router.TestConnectSession(MakeFakeSession());

  // Only one session is connected, so both requests round-robin to it.
  // There is no per-request correlation id in the wire protocol (a session
  // gets exactly one _promises slot, not one per request), so the second
  // request must be refused explicitly rather than silently overwriting --
  // and losing -- the promise the first one is still waiting on.
  auto first = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  auto second = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  ASSERT_EQ(second.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(second.get(), std::runtime_error);

  // The first request must be exactly as pending as before the second one
  // arrived: not silently resolved, not silently lost.
  EXPECT_EQ(first.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
}

TEST_F(MultiGpuRouterTest, write_to_next_round_robins_across_connected_sessions) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  auto session_c = MakeFakeSession();
  router.TestConnectSession(session_a);
  router.TestConnectSession(session_b);
  router.TestConnectSession(session_c);

  auto future_a = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  auto future_b = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  auto future_c = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  // None of the three responses will ever arrive (nothing is listening on
  // the other end of any of these sessions), so all three requests are
  // still pending right now -- but round-robin must have routed each of
  // them to a *different* session (connection order A, B, C). Disconnecting
  // one session at a time and checking which future flips from pending to
  // rejected is the oracle for "which request went where": if two requests
  // had collided on the same session, disconnecting that session would flip
  // two futures instead of one, and TestConnectSession/TestDisconnectSession
  // are the only way to observe routing without a live secondary.
  ASSERT_EQ(future_a.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
  ASSERT_EQ(future_b.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
  ASSERT_EQ(future_c.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

  router.TestDisconnectSession(session_b);
  ASSERT_EQ(future_b.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(future_b.get(), std::runtime_error);
  EXPECT_EQ(future_a.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  EXPECT_EQ(future_c.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);

  router.TestDisconnectSession(session_a);
  ASSERT_EQ(future_a.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(future_a.get(), std::runtime_error);
  EXPECT_EQ(future_c.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);

  router.TestDisconnectSession(session_c);
  ASSERT_EQ(future_c.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_THROW(future_c.get(), std::runtime_error);
}

TEST(MultiGpuRouterProductionShutdown, stop_releases_listening_port_despite_reference_cycle) {
  // Reproduces the exact production call shape (CarlaServer.cpp:
  // make_shared<Router>, then SetCallbacks(), then AsyncRun()) rather than
  // the MultiGpuRouterTest fixture's simplified one, because the regression
  // this guards against only exists in that shape: SetCallbacks() makes
  // _commander hold a shared_ptr<Router> back to this object (Router ->
  // PrimaryCommands -> Router), so ~Router() never runs once it has been
  // called -- Stop() itself, not destruction, must release the OS-level
  // listening socket. This was a real regression introduced while fixing a
  // different bug (a double Listener::Stop() call that only crashed in a
  // test/destructor-driven teardown) and confirmed by adversarial review.
  // A fixed, non-zero port, not TESTING_PORT (0): Router::GetLocalEndpoint()
  // returns the endpoint it was constructed with, not the acceptor's actual
  // bound local_endpoint() -- with port 0 (ephemeral) it would report "port
  // 0" while the OS silently assigned a real port underneath, and the probe
  // below would then always trivially succeed on a *different*, fresh
  // ephemeral port instead of ever re-checking the one actually in question.
  constexpr uint16_t kFixedTestPort = 17654u;
  auto router = std::make_shared<carla::multigpu::Router>(kFixedTestPort);
  router->SetCallbacks();
  router->AsyncRun(1u);
  const auto endpoint = router->GetLocalEndpoint();

  // SetCallbacks()/Listen() only posts the initial async_accept; give the
  // background thread a short, bounded window to actually run it and arm
  // the pending accept operation whose handler captures shared_from_this()
  // -- without that, this test would not exercise the mechanism it exists
  // to guard, regardless of the fix's correctness.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  router->Stop();

  // `router` is deliberately still alive and its SetCallbacks() reference
  // cycle still intact at this point -- exactly as in production, where
  // Stop() is called but the Router itself is never destroyed. The check
  // below must not depend on ~Router()/~Listener() ever running: breaking
  // the cycle (or letting `router` go out of scope) before this point would
  // let ordinary destruction release the port and mask the very regression
  // this test exists to catch.
  {
    boost::asio::io_context probe_io_context;
    EXPECT_NO_THROW({
      carla::multigpu::Listener probe(probe_io_context, endpoint);
    }) << "Router::Stop() did not release its listening port";
  }

  // Cleanup only, after the assertion above: break the self-reference cycle
  // so this test's Router doesn't leak for the rest of the process (this is
  // a test-hygiene concern only -- production leaks it on purpose, for the
  // whole process lifetime, which is exactly the condition just tested).
  router->GetCommander().set_router(nullptr);
}
