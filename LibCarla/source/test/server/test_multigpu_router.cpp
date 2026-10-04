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
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/router.h>
#include <carla/streaming/detail/Token.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <chrono>
#include <cstring>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

// Exercises Router's session-selection and promise-fulfillment logic without
// a live secondary process: a "session" here is a carla::multigpu::Primary
// built against an unconnected socket, so only Router's own bookkeeping is
// under test, never Primary's Write()/ReadData().

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

  bool IsRejected(std::future<carla::multigpu::SessionInfo> &future) {
    return future.get().session == nullptr;
  }

  bool WaitUntilPending(
      carla::multigpu::Router &router,
      const std::shared_ptr<carla::multigpu::Primary> &session) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline) {
      if (router.TestHasPendingRequest(session)) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
  }

  carla::Buffer MakeBuffer(std::string_view text) {
    return carla::Buffer(reinterpret_cast<const unsigned char *>(text.data()), text.size());
  }

  std::string ToString(const carla::Buffer &buffer) {
    return std::string(reinterpret_cast<const char *>(buffer.data()), buffer.size());
  }

  carla::Buffer MakeTokenReply(carla::streaming::detail::stream_id_type stream_id, uint16_t port) {
    carla::streaming::detail::token_data data;
    data.stream_id = stream_id;
    data.port = port;
    data.protocol = carla::streaming::detail::token_data::protocol::tcp;
    data.address_type = carla::streaming::detail::token_data::address::ip_v4;
    data.address.v4 = boost::asio::ip::make_address_v4("127.0.0.1").to_bytes();
    return carla::Buffer(reinterpret_cast<const unsigned char *>(&data), sizeof(data));
  }

  carla::Buffer MakeBoolReply(bool value) {
    return carla::Buffer(reinterpret_cast<const unsigned char *>(&value), sizeof(value));
  }

  /// Routes @a sensor_id (actor @a actor) through GetToken(); round-robin
  /// must pick @a session.
  bool RouteSensorTo(
      carla::multigpu::Router &router,
      carla::multigpu::PrimaryCommands &commander,
      const std::shared_ptr<carla::multigpu::Primary> &session,
      carla::multigpu::stream_id sensor_id,
      carla::multigpu::actor_id actor,
      uint16_t port) {
    auto routed = std::async(std::launch::async, [&commander, sensor_id, actor]() {
      return commander.GetToken(sensor_id, actor);
    });
    if (!WaitUntilPending(router, session)) {
      return false;
    }
    router.TestHandleResponse(session, MakeTokenReply(sensor_id, port));
    return (routed.wait_for(std::chrono::seconds(2)) == std::future_status::ready) && routed.get().has_value();
  }

  /// Port of the cached token of @a sensor_id; fails the test on a round trip.
  std::optional<uint16_t> CachedPort(
      carla::multigpu::Router &router,
      carla::multigpu::PrimaryCommands &commander,
      carla::multigpu::stream_id sensor_id) {
    auto cached = std::async(std::launch::async, [&commander, sensor_id]() {
      return commander.GetToken(sensor_id, 0u);
    });
    if (cached.wait_for(std::chrono::milliseconds(200)) != std::future_status::ready) {
      router.Stop();
      static_cast<void>(cached.get());
      return std::nullopt;
    }
    const auto token = cached.get();
    return token ? std::optional<uint16_t>(token->get_port()) : std::nullopt;
  }

  struct SentRequest {
    const carla::multigpu::Primary *session;
    carla::multigpu::MultiGPUCommand command;
    std::vector<unsigned char> payload;
  };

  /// Records every request the router sends to one secondary.
  class RequestLog {
  public:
    explicit RequestLog(carla::multigpu::Router &router) {
      router.TestSetRequestObserver([state = _state](
          const carla::multigpu::Primary *session,
          carla::multigpu::MultiGPUCommand command,
          const carla::BufferView &payload) {
        std::scoped_lock<std::mutex> lock(state->mutex);
        state->sent.push_back(SentRequest{
            session, command, std::vector<unsigned char>(payload.data(), payload.data() + payload.size())});
      });
    }

    std::vector<SentRequest> Take() {
      std::scoped_lock<std::mutex> lock(_state->mutex);
      return std::exchange(_state->sent, {});
    }

  private:
    struct State {
      std::mutex mutex;
      std::vector<SentRequest> sent;
    };
    std::shared_ptr<State> _state = std::make_shared<State>();
  };

  ::testing::AssertionResult IsGetToken(
      const SentRequest &sent,
      const std::shared_ptr<carla::multigpu::Primary> &session,
      carla::multigpu::stream_id sensor_id,
      carla::multigpu::actor_id actor) {
    if (sent.session != session.get()) {
      return ::testing::AssertionFailure() << "sent to another session";
    }
    if (sent.command != carla::multigpu::MultiGPUCommand::GET_TOKEN) {
      return ::testing::AssertionFailure() << "command " << static_cast<uint32_t>(sent.command);
    }
    carla::multigpu::GetTokenRequest request{};
    if (sent.payload.size() != sizeof(request)) {
      return ::testing::AssertionFailure() << "payload of " << sent.payload.size() << " bytes";
    }
    std::memcpy(&request, sent.payload.data(), sizeof(request));
    if ((request.stream_id != sensor_id) || (request.actor_id != actor)) {
      return ::testing::AssertionFailure() << "stream " << request.stream_id << ", actor " << request.actor_id;
    }
    return ::testing::AssertionSuccess();
  }

  ::testing::AssertionResult IsEnableRos(
      const SentRequest &sent,
      const std::shared_ptr<carla::multigpu::Primary> &session,
      carla::multigpu::stream_id sensor_id) {
    if (sent.session != session.get()) {
      return ::testing::AssertionFailure() << "sent to another session";
    }
    if (sent.command != carla::multigpu::MultiGPUCommand::ENABLE_ROS) {
      return ::testing::AssertionFailure() << "command " << static_cast<uint32_t>(sent.command);
    }
    carla::multigpu::stream_id payload_id = 0u;
    if (sent.payload.size() != sizeof(payload_id)) {
      return ::testing::AssertionFailure() << "payload of " << sent.payload.size() << " bytes";
    }
    std::memcpy(&payload_id, sent.payload.data(), sizeof(payload_id));
    if (payload_id != sensor_id) {
      return ::testing::AssertionFailure() << "stream " << payload_id;
    }
    return ::testing::AssertionSuccess();
  }

  /// Answers the next request pending on @a session with @a reply.
  bool Answer(
      carla::multigpu::Router &router,
      const std::shared_ptr<carla::multigpu::Primary> &session,
      carla::Buffer reply) {
    if (!WaitUntilPending(router, session)) {
      return false;
    }
    router.TestHandleResponse(session, std::move(reply));
    return true;
  }

  /// Routes sensor 42 (actor 9) to @a session through GetToken().
  bool RouteSensor(
      carla::multigpu::Router &router,
      carla::multigpu::PrimaryCommands &commander,
      const std::shared_ptr<carla::multigpu::Primary> &session,
      uint16_t port) {
    auto routed = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
    if (!WaitUntilPending(router, session)) {
      return false;
    }
    router.TestHandleResponse(session, MakeTokenReply(42u, port));
    return (routed.wait_for(std::chrono::seconds(2)) == std::future_status::ready) && routed.get().has_value();
  }

} // namespace

TEST_F(MultiGpuRouterTest, write_to_next_with_no_sessions_rejects_instead_of_hanging) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  // Specifically a rejection value (Router::RejectPromise), not
  // std::future_error(broken_promise): the promise here is never stored
  // anywhere, so if this rejection path were removed the local promise would
  // be destroyed unset and get() would throw instead.
  EXPECT_TRUE(IsRejected(future));
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
  EXPECT_TRUE(IsRejected(future));
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
  EXPECT_TRUE(IsRejected(future));
}

TEST_F(MultiGpuRouterTest, write_to_next_with_connected_session_stays_pending) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  EXPECT_TRUE(router.TestHasPendingRequest(session));
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
  EXPECT_TRUE(IsRejected(future));
  EXPECT_FALSE(router.TestHasPendingRequest(session));
}

TEST_F(MultiGpuRouterTest, response_after_rejection_is_dropped) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  router.TestDisconnectSession(session);
  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);

  // A reply that was already on the wire when the session was dropped must
  // not touch the promise that was just rejected.
  EXPECT_NO_THROW(router.TestHandleResponse(session, MakeBuffer("late reply")));
  EXPECT_TRUE(IsRejected(future));
  EXPECT_FALSE(router.TestHasPendingRequest(session));
}

TEST_F(MultiGpuRouterTest, duplicate_response_does_not_resolve_a_promise_twice) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  router.TestHandleResponse(session, MakeBuffer("first"));
  EXPECT_FALSE(router.TestHasPendingRequest(session));

  // std::promise::set_value() throws if called twice, so this only passes if
  // the first response released the slot.
  EXPECT_NO_THROW(router.TestHandleResponse(session, MakeBuffer("second")));
  EXPECT_NO_THROW(router.TestDisconnectSession(session));

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  auto result = future.get();
  EXPECT_EQ(result.session, session);
  EXPECT_EQ(ToString(result.buffer), "first");
}

TEST_F(MultiGpuRouterTest, stop_rejects_every_pending_request) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router.TestConnectSession(session_a);
  router.TestConnectSession(session_b);

  auto future_a = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  auto future_b = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  ASSERT_TRUE(router.TestHasPendingRequest(session_a));
  ASSERT_TRUE(router.TestHasPendingRequest(session_b));

  router.Stop();

  ASSERT_EQ(future_a.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_EQ(future_b.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(IsRejected(future_a));
  EXPECT_TRUE(IsRejected(future_b));
  EXPECT_FALSE(router.HasClientsConnected());
  // ~Router() calls Stop() again; a second call must be harmless.
  EXPECT_NO_THROW(router.Stop());
}

TEST_F(MultiGpuRouterTest, episode_ready_marker_with_no_pending_promise_triggers_resync_callback) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  callback_count = 0; // TestConnectSession() itself already fired the callback once.

  carla::Buffer marker(
      reinterpret_cast<const unsigned char *>(carla::multigpu::kEpisodeReadyMarker.data()),
      carla::multigpu::kEpisodeReadyMarker.size());
  router.TestHandleResponse(session, std::move(marker));

  EXPECT_EQ(callback_count, 1);
}

TEST_F(MultiGpuRouterTest, session_connecting_after_load_map_loads_the_remembered_map_before_resync) {
  carla::multigpu::Router router(TESTING_PORT);
  RequestLog log(router);
  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  const auto first_load = router.WriteLoadMap("Town03");
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_EQ(sent[0].session, session.get());
  EXPECT_EQ(sent[0].command, carla::multigpu::MultiGPUCommand::LOAD_MAP);
  const auto request = carla::multigpu::ParseLoadMapPayload(
      std::string_view(reinterpret_cast<const char *>(sent[0].payload.data()), sent[0].payload.size()));
  EXPECT_EQ(request.map, "Town03");
  EXPECT_GT(request.load_id, first_load);
  EXPECT_TRUE(router.IsAnySecondaryLoading());
  EXPECT_EQ(callback_count, 0);

  const std::string ready = carla::multigpu::MakeEpisodeReadyMessage(request.load_id);
  router.TestHandleResponse(session, MakeBuffer(ready));
  EXPECT_FALSE(router.IsAnySecondaryLoading());
  EXPECT_EQ(callback_count, 1);
}

TEST_F(MultiGpuRouterTest, session_connecting_without_a_remembered_map_gets_no_load_map) {
  carla::multigpu::Router router(TESTING_PORT);
  RequestLog log(router);
  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  router.TestConnectSession(MakeFakeSession());

  EXPECT_TRUE(log.Take().empty());
  EXPECT_FALSE(router.IsAnySecondaryLoading());
  EXPECT_EQ(callback_count, 1);
}

TEST_F(MultiGpuRouterTest, session_connected_before_load_map_keeps_the_broadcast_behaviour) {
  carla::multigpu::Router router(TESTING_PORT);
  RequestLog log(router);
  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  EXPECT_EQ(callback_count, 1);

  const auto load_id = router.WriteLoadMap("Town05");
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_EQ(sent[0].session, session.get());
  EXPECT_EQ(sent[0].command, carla::multigpu::MultiGPUCommand::LOAD_MAP);
  EXPECT_TRUE(router.IsAnySecondaryLoading());
  EXPECT_EQ(callback_count, 1);

  router.TestHandleResponse(session, MakeBuffer(carla::multigpu::MakeEpisodeReadyMessage(load_id)));
  EXPECT_FALSE(router.IsAnySecondaryLoading());
  EXPECT_EQ(callback_count, 2);
}

TEST_F(MultiGpuRouterTest, stale_ready_of_an_older_load_does_not_finish_a_late_secondary_load) {
  carla::multigpu::Router router(TESTING_PORT);
  const auto stale_load = router.WriteLoadMap("Town01");
  router.WriteLoadMap("Town03");
  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  ASSERT_TRUE(router.IsAnySecondaryLoading());

  router.TestHandleResponse(session, MakeBuffer(carla::multigpu::MakeEpisodeReadyMessage(stale_load)));

  EXPECT_TRUE(router.IsAnySecondaryLoading());
}

TEST_F(MultiGpuRouterTest, unsolicited_non_marker_data_does_not_trigger_resync_callback) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  callback_count = 0;

  // Anything that is not the exact episode-ready marker must not be
  // misread as that signal, even though it also arrives with no pending
  // promise (e.g. a protocol desync or a stray write from a misbehaving
  // secondary).
  std::string garbage("not the marker");
  carla::Buffer buffer(
      reinterpret_cast<const unsigned char *>(garbage.data()), garbage.size());
  router.TestHandleResponse(session, std::move(buffer));

  EXPECT_EQ(callback_count, 0);
}

TEST_F(MultiGpuRouterTest, episode_ready_marker_is_not_misdelivered_to_a_pending_promise) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  callback_count = 0;

  // A request is already pending on this exact session when the marker
  // arrives -- e.g. a GetToken round trip in flight at the moment this
  // secondary's own level reload completes. The marker must still be
  // recognized as the resync signal, and must NOT be handed to this
  // request as if it were its (garbage) response data.
  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  ASSERT_EQ(future.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

  carla::Buffer marker(
      reinterpret_cast<const unsigned char *>(carla::multigpu::kEpisodeReadyMarker.data()),
      carla::multigpu::kEpisodeReadyMarker.size());
  router.TestHandleResponse(session, std::move(marker));

  EXPECT_EQ(callback_count, 1);
  EXPECT_EQ(future.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

  // The request itself is still healthy: its real reply, once it arrives,
  // resolves normally.
  router.TestHandleResponse(session, MakeBuffer("token reply"));
  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  auto result = future.get();
  EXPECT_EQ(result.session, session);
  EXPECT_EQ(ToString(result.buffer), "token reply");
}

TEST_F(MultiGpuRouterTest, response_with_pending_promise_does_not_trigger_resync_callback) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);

  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });

  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  callback_count = 0;

  auto future = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  ASSERT_EQ(future.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

  // Real response data, distinct from the marker (see the priority test
  // above for the marker-with-pending-promise case).
  router.TestHandleResponse(session, MakeBuffer("token reply"));

  ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  auto result = future.get();
  EXPECT_EQ(result.session, session);
  EXPECT_EQ(ToString(result.buffer), "token reply");
  EXPECT_EQ(callback_count, 0);
}

TEST_F(MultiGpuRouterTest, overlapping_request_to_same_session_is_rejected_not_orphaned) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  // Only one session is connected, so both requests round-robin to it.
  // There is no per-request correlation id in the wire protocol (a session
  // gets exactly one _promises slot, not one per request), so the second
  // request must be refused explicitly rather than silently overwriting --
  // and losing -- the promise the first one is still waiting on.
  auto first = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());
  auto second = router.WriteToNext(carla::multigpu::MultiGPUCommand::GET_TOKEN, carla::Buffer());

  ASSERT_EQ(second.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(IsRejected(second));

  // The first request must be exactly as pending as before the second one
  // arrived: not silently resolved, not silently lost.
  EXPECT_EQ(first.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

  router.TestHandleResponse(session, MakeBuffer("first reply"));
  ASSERT_EQ(first.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  auto result = first.get();
  EXPECT_EQ(result.session, session);
  EXPECT_EQ(ToString(result.buffer), "first reply");
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
  EXPECT_TRUE(IsRejected(future_b));
  EXPECT_EQ(future_a.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  EXPECT_EQ(future_c.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);

  router.TestDisconnectSession(session_a);
  ASSERT_EQ(future_a.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(IsRejected(future_a));
  EXPECT_EQ(future_c.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);

  router.TestDisconnectSession(session_c);
  ASSERT_EQ(future_c.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(IsRejected(future_c));
}

TEST_F(MultiGpuRouterTest, get_token_with_no_secondary_returns_nullopt_and_caches_nothing) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  carla::multigpu::PrimaryCommands commander(router);

  std::optional<carla::multigpu::token_type> token;
  ASSERT_NO_THROW(token = commander.GetToken(42u, 42u));
  EXPECT_FALSE(token.has_value());

  // Had the failure been cached, this would return without a round trip.
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  auto retried = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 42u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeTokenReply(42u, 2001u));
  ASSERT_EQ(retried.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto routed = retried.get();
  ASSERT_TRUE(routed.has_value());
  EXPECT_EQ(routed->get_port(), 2001u);
}

TEST_F(MultiGpuRouterTest, get_token_with_malformed_reply_returns_nullopt_and_caches_nothing) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);

  auto malformed = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 42u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  // Shorter than token_data: reading it as a token would overrun the buffer.
  router->TestHandleResponse(session, MakeBuffer("short"));
  ASSERT_EQ(malformed.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_FALSE(malformed.get().has_value());

  auto retried = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 42u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeTokenReply(42u, 2003u));
  ASSERT_EQ(retried.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = retried.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2003u);
}

TEST_F(MultiGpuRouterTest, get_token_not_ready_reply_returns_nullopt_and_caches_nothing) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);

  auto not_ready = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeBuffer(carla::multigpu::kTokenNotReadyMarker));
  ASSERT_EQ(not_ready.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_FALSE(not_ready.get().has_value());

  auto retried = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeTokenReply(42u, 2005u));
  ASSERT_EQ(retried.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = retried.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), 42u);
}

TEST_F(MultiGpuRouterTest, get_token_always_carries_the_primary_stream_id) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);

  auto pending = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  // A secondary-local stream id must never reach the client.
  router->TestHandleResponse(session, MakeTokenReply(5u, 2006u));
  ASSERT_EQ(pending.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = pending.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), 42u);
  EXPECT_EQ(token->get_port(), 2006u);
}

TEST_F(MultiGpuRouterTest, cached_token_of_a_disconnected_secondary_is_routed_again) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router->TestConnectSession(session_a);
  router->TestConnectSession(session_b);
  carla::multigpu::PrimaryCommands commander(router);

  auto first = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session_a));
  router->TestHandleResponse(session_a, MakeTokenReply(42u, 2007u));
  ASSERT_EQ(first.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_TRUE(first.get().has_value());

  router->TestDisconnectSession(session_a);
  auto rerouted = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session_b));
  router->TestHandleResponse(session_b, MakeTokenReply(42u, 2008u));
  ASSERT_EQ(rerouted.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = rerouted.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2008u);
}

TEST(MultiGpuCommands, token_not_ready_marker_is_distinct_from_other_replies) {
  EXPECT_NE(carla::multigpu::kTokenNotReadyMarker, carla::multigpu::kEpisodeReadyMarker);
  EXPECT_LT(carla::multigpu::kTokenNotReadyMarker.size(), sizeof(carla::streaming::detail::token_data));
}

TEST(MultiGpuCommands, get_token_request_starts_with_the_stream_id) {
  const carla::multigpu::GetTokenRequest request{42u, 9u};
  carla::streaming::detail::stream_id_type leading_stream_id = 0u;
  std::memcpy(&leading_stream_id, &request, sizeof(leading_stream_id));
  EXPECT_EQ(leading_stream_id, 42u);
}

TEST_F(MultiGpuRouterTest, concurrent_get_token_calls_are_serialized_not_rejected) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);

  // Both requests target the single secondary. The router refuses a second
  // in-flight request on one session, so PrimaryCommands must queue the
  // second caller behind the first rather than let it be rejected.
  auto first = std::async(std::launch::async, [&commander]() { return commander.GetToken(1u, 1u); });
  auto second = std::async(std::launch::async, [&commander]() { return commander.GetToken(2u, 2u); });

  ASSERT_TRUE(WaitUntilPending(*router, session));
  // Gives the other caller time to reach the router, so an unserialized
  // implementation would have its request refused right here.
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  router->TestHandleResponse(session, MakeTokenReply(100u, 3001u));

  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeTokenReply(101u, 3002u));

  ASSERT_EQ(first.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_EQ(second.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto first_token = first.get();
  const auto second_token = second.get();
  ASSERT_TRUE(first_token.has_value());
  ASSERT_TRUE(second_token.has_value());
  const std::set<uint16_t> ports{first_token->get_port(), second_token->get_port()};
  EXPECT_EQ(ports, (std::set<uint16_t>{3001u, 3002u}));
}

TEST_F(MultiGpuRouterTest, get_token_fails_cleanly_when_secondary_disconnects_mid_request) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);

  auto pending = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 42u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));

  // The secondary dies with the GET_TOKEN in flight. In CarlaUnreal an
  // exception escaping GetToken() here crashes the primary (see
  // Router::RejectPromise), so the failure must come back as a value.
  router->TestDisconnectSession(session);

  ASSERT_EQ(pending.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  std::optional<carla::multigpu::token_type> token;
  ASSERT_NO_THROW(token = pending.get());
  EXPECT_FALSE(token.has_value());

  // The primary side is still usable: a secondary that connects afterwards
  // serves the next request.
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  auto next = std::async(std::launch::async, [&commander]() { return commander.GetToken(43u, 43u); });
  ASSERT_TRUE(WaitUntilPending(*router, replacement));
  router->TestHandleResponse(replacement, MakeTokenReply(43u, 2004u));
  ASSERT_EQ(next.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto next_token = next.get();
  ASSERT_TRUE(next_token.has_value());
  EXPECT_EQ(next_token->get_stream_id(), 43u);
}

TEST_F(MultiGpuRouterTest, get_token_after_failed_request_routes_to_remaining_secondary) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router->TestConnectSession(session_a);
  router->TestConnectSession(session_b);
  carla::multigpu::PrimaryCommands commander(router);

  auto failed = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 42u); });
  ASSERT_TRUE(WaitUntilPending(*router, session_a));
  router->TestDisconnectSession(session_a);
  ASSERT_EQ(failed.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_FALSE(failed.get().has_value());

  // The failure must not have been cached: retrying the same sensor routes
  // to the surviving secondary and resolves with its reply.
  auto retried = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 42u); });
  ASSERT_TRUE(WaitUntilPending(*router, session_b));
  router->TestHandleResponse(session_b, MakeTokenReply(42u, 2002u));
  ASSERT_EQ(retried.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = retried.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), 42u);
  EXPECT_EQ(token->get_port(), 2002u);

  // Once routed, the token is served from the cache without another round
  // trip (nothing would answer one here).
  const auto cached = commander.GetToken(42u, 42u);
  ASSERT_TRUE(cached.has_value());
  EXPECT_EQ(cached->get_port(), 2002u);
  EXPECT_FALSE(router->TestHasPendingRequest(session_b));
}

TEST_F(MultiGpuRouterTest, enable_for_ros_without_any_secondary_returns_instead_of_recursing) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  carla::multigpu::PrimaryCommands commander(router);

  // GetToken() fails immediately here, so EnableForROS() must give up rather
  // than retry its routing recursively without bound, and must report the
  // failure back to the caller instead of silently succeeding.
  auto done = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(7u, 7u); });
  ASSERT_EQ(done.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  bool enabled = true;
  ASSERT_NO_THROW(enabled = done.get());
  EXPECT_FALSE(enabled);
}

TEST_F(MultiGpuRouterTest, enable_for_ros_refused_by_the_secondary_returns_false) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session, 2010u));

  auto refused = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeBoolReply(false));
  ASSERT_EQ(refused.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_FALSE(refused.get());

  auto accepted = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeBoolReply(true));
  ASSERT_EQ(accepted.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(accepted.get());
}

TEST_F(MultiGpuRouterTest, enable_for_ros_on_a_disconnected_secondary_routes_the_sensor_again) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router->TestConnectSession(session_a);
  router->TestConnectSession(session_b);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session_a, 2011u));
  router->TestDisconnectSession(session_a);

  auto enabled = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session_b));
  router->TestHandleResponse(session_b, MakeTokenReply(42u, 2012u));
  ASSERT_TRUE(WaitUntilPending(*router, session_b));
  router->TestHandleResponse(session_b, MakeBoolReply(true));
  ASSERT_EQ(enabled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(enabled.get());

  const auto token = commander.GetToken(42u, 9u);
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2012u);
}

TEST_F(MultiGpuRouterTest, ros_queries_on_a_disconnected_secondary_return_without_a_round_trip) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router->TestConnectSession(session_a);
  router->TestConnectSession(session_b);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session_a, 2013u));
  router->TestDisconnectSession(session_a);

  EXPECT_FALSE(commander.IsEnabledForROS(42u));
  commander.DisableForROS(42u);
  EXPECT_FALSE(router->TestHasPendingRequest(session_b));

  // The next request routes the sensor again.
  auto rerouted = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session_b));
  router->TestHandleResponse(session_b, MakeTokenReply(42u, 2014u));
  ASSERT_EQ(rerouted.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = rerouted.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2014u);
}

TEST_F(MultiGpuRouterTest, load_map_forgets_cached_tokens) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session, 2015u));

  commander.SendLoadMap("/Game/Carla/Maps/Town10HD_Opt");
  auto routed = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  router->TestHandleResponse(session, MakeTokenReply(42u, 2016u));
  ASSERT_EQ(routed.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = routed.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2016u);
}

TEST_F(MultiGpuRouterTest, new_secondary_takes_over_the_sensors_of_a_lost_one) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2020u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2020u));
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  static_cast<void>(log.Take());

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  // Targeted at the new session and issued one sensor at a time.
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(7u, 2020u)));
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(8u, 2021u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 2u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 2u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));
  EXPECT_TRUE(IsGetToken(sent[1], replacement, 43u, 10u));

  EXPECT_EQ(CachedPort(*router, commander, 42u), std::optional<uint16_t>(2020u));
  EXPECT_EQ(CachedPort(*router, commander, 43u), std::optional<uint16_t>(2021u));

  auto enabled = std::async(std::launch::async, [&commander]() { return commander.IsEnabledForROS(42u); });
  ASSERT_TRUE(Answer(*router, replacement, MakeBoolReply(true)));
  ASSERT_EQ(enabled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(enabled.get());

  // The episode-ready path fires the same callback but brings no new session.
  router->TestHandleResponse(replacement, MakeBuffer(carla::multigpu::kEpisodeReadyMarker));
  EXPECT_EQ(commander.RerouteLostSensors(), 0u);
  EXPECT_FALSE(router->TestHasPendingRequest(replacement));
}

TEST_F(MultiGpuRouterTest, reroute_without_lost_sensors_is_a_no_op) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, session, 42u, 9u, 2022u));
  auto added = MakeFakeSession();
  router->TestConnectSession(added);

  EXPECT_EQ(commander.RerouteLostSensors(), 0u);
  EXPECT_FALSE(router->TestHasPendingRequest(session));
  EXPECT_FALSE(router->TestHasPendingRequest(added));
  EXPECT_EQ(CachedPort(*router, commander, 42u), std::optional<uint16_t>(2022u));
}

TEST_F(MultiGpuRouterTest, reroute_after_load_map_is_a_no_op) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2023u));
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);

  commander.SendLoadMap("/Game/Carla/Maps/Town10HD_Opt");

  EXPECT_EQ(commander.RerouteLostSensors(), 0u);
  EXPECT_FALSE(router->TestHasPendingRequest(replacement));
}

TEST_F(MultiGpuRouterTest, sensor_refused_by_the_new_secondary_is_routed_lazily_later) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2024u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2024u));
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  // Sent when the secondary cannot resolve the sensor actor to a stream.
  ASSERT_TRUE(Answer(*router, replacement, MakeBuffer(carla::multigpu::kTokenNotReadyMarker)));
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(43u, 2024u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 1u);
  EXPECT_EQ(CachedPort(*router, commander, 43u), std::optional<uint16_t>(2024u));

  auto retried = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2025u)));
  ASSERT_EQ(retried.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = retried.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2025u);
}

TEST_F(MultiGpuRouterTest, reroute_leaves_sensors_of_live_secondaries_alone) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto lost = MakeFakeSession();
  auto alive = MakeFakeSession();
  router->TestConnectSession(lost);
  router->TestConnectSession(alive);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2026u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, alive, 43u, 10u, 2027u));
  // As the engine does on the tick after these sessions connected.
  ASSERT_EQ(commander.RerouteLostSensors(), 0u);
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(WaitUntilPending(*router, replacement));
  EXPECT_FALSE(router->TestHasPendingRequest(alive));
  router->TestHandleResponse(replacement, MakeTokenReply(42u, 2026u));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 1u);

  EXPECT_EQ(CachedPort(*router, commander, 42u), std::optional<uint16_t>(2026u));
  EXPECT_EQ(CachedPort(*router, commander, 43u), std::optional<uint16_t>(2027u));
  auto enabled = std::async(std::launch::async, [&commander]() { return commander.IsEnabledForROS(43u); });
  ASSERT_TRUE(Answer(*router, alive, MakeBoolReply(true)));
  ASSERT_EQ(enabled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(enabled.get());
}

TEST_F(MultiGpuRouterTest, reroute_stops_when_the_new_secondary_disconnects_mid_way) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2028u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2028u));
  router->TestDisconnectSession(lost);
  auto short_lived = MakeFakeSession();
  router->TestConnectSession(short_lived);
  static_cast<void>(log.Take());

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(WaitUntilPending(*router, short_lived));
  router->TestDisconnectSession(short_lived);
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 0u);
  auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_TRUE(IsGetToken(sent[0], short_lived, 42u, 9u));

  // Both sensors stay lost, including the one in flight, so the next
  // secondary recovers them under their stream ids.
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  auto second_try = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2029u)));
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(43u, 2029u)));
  ASSERT_EQ(second_try.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(second_try.get(), 2u);
  sent = log.Take();
  ASSERT_EQ(sent.size(), 2u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));
  EXPECT_TRUE(IsGetToken(sent[1], replacement, 43u, 10u));
  EXPECT_EQ(CachedPort(*router, commander, 42u), std::optional<uint16_t>(2029u));
  EXPECT_EQ(CachedPort(*router, commander, 43u), std::optional<uint16_t>(2029u));
}

TEST_F(MultiGpuRouterTest, first_new_secondary_takes_every_lost_sensor) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2030u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2030u));
  router->TestDisconnectSession(lost);
  auto first = MakeFakeSession();
  auto second = MakeFakeSession();
  router->TestConnectSession(first);
  router->TestConnectSession(second);
  static_cast<void>(log.Take());

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(Answer(*router, first, MakeTokenReply(42u, 2030u)));
  ASSERT_TRUE(Answer(*router, first, MakeTokenReply(43u, 2030u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 2u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 2u);
  EXPECT_TRUE(IsGetToken(sent[0], first, 42u, 9u));
  EXPECT_TRUE(IsGetToken(sent[1], first, 43u, 10u));
}

TEST_F(MultiGpuRouterTest, next_new_secondary_takes_over_when_the_first_drops_mid_way) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2031u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2031u));
  router->TestDisconnectSession(lost);
  auto first = MakeFakeSession();
  auto second = MakeFakeSession();
  router->TestConnectSession(first);
  router->TestConnectSession(second);
  static_cast<void>(log.Take());

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(WaitUntilPending(*router, first));
  router->TestDisconnectSession(first);
  ASSERT_TRUE(Answer(*router, second, MakeTokenReply(42u, 2032u)));
  ASSERT_TRUE(Answer(*router, second, MakeTokenReply(43u, 2032u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 2u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 3u);
  EXPECT_TRUE(IsGetToken(sent[0], first, 42u, 9u));
  EXPECT_TRUE(IsGetToken(sent[1], second, 42u, 9u));
  EXPECT_TRUE(IsGetToken(sent[2], second, 43u, 10u));
  EXPECT_EQ(CachedPort(*router, commander, 42u), std::optional<uint16_t>(2032u));
}

TEST_F(MultiGpuRouterTest, session_that_left_before_the_call_is_skipped) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2033u));
  router->TestDisconnectSession(lost);
  auto gone = MakeFakeSession();
  router->TestConnectSession(gone);
  router->TestDisconnectSession(gone);
  static_cast<void>(log.Take());

  EXPECT_EQ(commander.RerouteLostSensors(), 0u);
  EXPECT_TRUE(log.Take().empty());

  auto gone_again = MakeFakeSession();
  router->TestConnectSession(gone_again);
  router->TestDisconnectSession(gone_again);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2034u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 1u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));
}

TEST_F(MultiGpuRouterTest, lost_route_survives_ros_queries_and_failed_get_token) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2035u));
  router->TestDisconnectSession(lost);
  static_cast<void>(log.Take());

  EXPECT_FALSE(commander.IsEnabledForROS(42u));
  commander.DisableForROS(42u);
  // No secondary is left to answer, so this fails without a request.
  EXPECT_FALSE(commander.GetToken(42u, 9u).has_value());
  EXPECT_TRUE(log.Take().empty());

  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2035u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 1u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));
}

TEST_F(MultiGpuRouterTest, get_token_reroutes_a_lost_route_before_reroute_runs) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2036u));
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  static_cast<void>(log.Take());

  auto lazy = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2037u)));
  ASSERT_EQ(lazy.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = lazy.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2037u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));

  EXPECT_EQ(commander.RerouteLostSensors(), 0u);
  EXPECT_FALSE(router->TestHasPendingRequest(replacement));
}

TEST_F(MultiGpuRouterTest, forgotten_sensor_is_not_rerouted) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2038u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2038u));
  commander.ForgetSensor(42u);
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  static_cast<void>(log.Take());

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(43u, 2039u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 1u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 1u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 43u, 10u));

  // Forgetting a live route drops its cached token too.
  commander.ForgetSensor(43u);
  auto routed = std::async(std::launch::async, [&commander]() { return commander.GetToken(43u, 10u); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(43u, 2040u)));
  ASSERT_EQ(routed.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  const auto token = routed.get();
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_port(), 2040u);
}

TEST_F(MultiGpuRouterTest, reroute_enables_ros_again_on_the_new_secondary) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2041u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 43u, 10u, 2041u));
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 44u, 11u, 2041u));
  auto enabled = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(42u, 9u); });
  ASSERT_TRUE(Answer(*router, lost, MakeBoolReply(true)));
  ASSERT_EQ(enabled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_TRUE(enabled.get());
  auto toggled = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(44u, 11u); });
  ASSERT_TRUE(Answer(*router, lost, MakeBoolReply(true)));
  ASSERT_EQ(toggled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_TRUE(toggled.get());
  auto disabled = std::async(std::launch::async, [&commander]() { commander.DisableForROS(44u); });
  ASSERT_TRUE(Answer(*router, lost, MakeBoolReply(true)));
  ASSERT_EQ(disabled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  static_cast<void>(log.Take());

  auto rerouting = std::async(std::launch::async, [&commander]() { return commander.RerouteLostSensors(); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2041u)));
  // Best effort: a refused ENABLE_ROS still counts the sensor as routed.
  ASSERT_TRUE(Answer(*router, replacement, MakeBoolReply(false)));
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(43u, 2041u)));
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(44u, 2041u)));
  ASSERT_EQ(rerouting.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_EQ(rerouting.get(), 3u);
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 4u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));
  EXPECT_TRUE(IsEnableRos(sent[1], replacement, 42u));
  EXPECT_TRUE(IsGetToken(sent[2], replacement, 43u, 10u));
  EXPECT_TRUE(IsGetToken(sent[3], replacement, 44u, 11u));
}

TEST_F(MultiGpuRouterTest, get_token_on_a_lost_route_enables_ros_again) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  RequestLog log(*router);
  auto lost = MakeFakeSession();
  router->TestConnectSession(lost);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensorTo(*router, commander, lost, 42u, 9u, 2042u));
  auto enabled = std::async(std::launch::async, [&commander]() { return commander.EnableForROS(42u, 9u); });
  ASSERT_TRUE(Answer(*router, lost, MakeBoolReply(true)));
  ASSERT_EQ(enabled.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  ASSERT_TRUE(enabled.get());
  router->TestDisconnectSession(lost);
  auto replacement = MakeFakeSession();
  router->TestConnectSession(replacement);
  static_cast<void>(log.Take());

  auto lazy = std::async(std::launch::async, [&commander]() { return commander.GetToken(42u, 9u); });
  ASSERT_TRUE(Answer(*router, replacement, MakeTokenReply(42u, 2043u)));
  ASSERT_TRUE(Answer(*router, replacement, MakeBoolReply(true)));
  ASSERT_EQ(lazy.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(lazy.get().has_value());
  const auto sent = log.Take();
  ASSERT_EQ(sent.size(), 2u);
  EXPECT_TRUE(IsGetToken(sent[0], replacement, 42u, 9u));
  EXPECT_TRUE(IsEnableRos(sent[1], replacement, 42u));
}

TEST_F(MultiGpuRouterTest, load_map_waits_until_every_secondary_is_ready_or_gone) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router.TestConnectSession(session_a);
  router.TestConnectSession(session_b);
  EXPECT_FALSE(router.IsAnySecondaryLoading());

  router.Write(carla::multigpu::MultiGPUCommand::SEND_FRAME, MakeBuffer("frame"));
  EXPECT_FALSE(router.IsAnySecondaryLoading());

  const auto load_id = router.WriteLoadMap("map");
  EXPECT_TRUE(router.IsAnySecondaryLoading());
  router.TestHandleResponse(session_a, MakeBuffer(carla::multigpu::MakeEpisodeReadyMessage(load_id)));
  EXPECT_TRUE(router.IsAnySecondaryLoading());
  router.TestDisconnectSession(session_b);
  EXPECT_FALSE(router.IsAnySecondaryLoading());
}

TEST_F(MultiGpuRouterTest, stale_episode_ready_for_map_a_does_not_clear_map_b_wait) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  const auto map_a_load = router.WriteLoadMap("map_a");
  const auto map_b_load = router.WriteLoadMap("map_b");
  ASSERT_NE(map_a_load, map_b_load);
  ASSERT_TRUE(router.IsAnySecondaryLoading());

  router.TestHandleResponse(session, MakeBuffer(carla::multigpu::MakeEpisodeReadyMessage(map_a_load)));
  EXPECT_TRUE(router.IsAnySecondaryLoading());
  router.TestHandleResponse(session, MakeBuffer(carla::multigpu::MakeEpisodeReadyMessage(map_b_load)));
  EXPECT_FALSE(router.IsAnySecondaryLoading());
}

TEST_F(MultiGpuRouterTest, untagged_episode_ready_does_not_clear_a_tagged_load_wait) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  static_cast<void>(router.WriteLoadMap("map"));
  router.TestHandleResponse(session, MakeBuffer(carla::multigpu::kEpisodeReadyMarker));
  EXPECT_TRUE(router.IsAnySecondaryLoading());
}

TEST_F(MultiGpuRouterTest, stale_episode_ready_still_rearms_resync) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  int callback_count = 0;
  router.SetNewConnectionCallback([&callback_count]() { ++callback_count; });
  auto session = MakeFakeSession();
  router.TestConnectSession(session);
  callback_count = 0;

  const auto first_load = router.WriteLoadMap("map_a");
  static_cast<void>(router.WriteLoadMap("map_b"));
  router.TestHandleResponse(session, MakeBuffer(carla::multigpu::MakeEpisodeReadyMessage(first_load)));

  EXPECT_EQ(callback_count, 1);
  EXPECT_FALSE(router.TestHasPendingRequest(session));
}

TEST(MultiGpuCommands, load_map_payload_round_trips_map_and_load_id) {
  const auto payload = carla::multigpu::MakeLoadMapPayload("/Game/Carla/Maps/Town15", 7u);
  const auto request = carla::multigpu::ParseLoadMapPayload(payload);
  EXPECT_EQ(request.map, "/Game/Carla/Maps/Town15");
  EXPECT_EQ(request.load_id, 7u);
}

TEST(MultiGpuCommands, load_map_payload_without_load_id_parses_as_zero) {
  const std::string legacy("/Game/Carla/Maps/Town15\0", 24u);
  const auto request = carla::multigpu::ParseLoadMapPayload(legacy);
  EXPECT_EQ(request.map, "/Game/Carla/Maps/Town15");
  EXPECT_EQ(request.load_id, 0u);
}

TEST(MultiGpuCommands, episode_ready_message_round_trips_load_id) {
  const auto message = carla::multigpu::MakeEpisodeReadyMessage(42u);
  const auto load_id = carla::multigpu::ParseEpisodeReadyMessage(message);
  ASSERT_TRUE(load_id.has_value());
  EXPECT_EQ(*load_id, 42u);
  EXPECT_EQ(carla::multigpu::ParseEpisodeReadyMessage(carla::multigpu::kEpisodeReadyMarker), 0u);
  EXPECT_FALSE(carla::multigpu::ParseEpisodeReadyMessage("Yes, I'm alive").has_value());
  EXPECT_FALSE(carla::multigpu::ParseEpisodeReadyMessage(message.substr(0u, message.size() - 1u)).has_value());
}

TEST_F(MultiGpuRouterTest, load_map_wait_can_be_abandoned) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  auto session = MakeFakeSession();
  router.TestConnectSession(session);

  static_cast<void>(router.WriteLoadMap("map"));
  ASSERT_TRUE(router.IsAnySecondaryLoading());
  router.StopWaitingForSecondaryLoads();
  EXPECT_FALSE(router.IsAnySecondaryLoading());
}

TEST_F(MultiGpuRouterTest, load_map_without_secondaries_waits_for_nothing) {
  carla::multigpu::Router router(TESTING_PORT);
  router.AsyncRun(1u);
  static_cast<void>(router.WriteLoadMap("map"));
  EXPECT_FALSE(router.IsAnySecondaryLoading());
}

TEST(MultiGpuRouterProductionShutdown, stop_releases_listening_port_despite_reference_cycle) {
  // Reproduces the production call shape (make_shared<Router>, SetCallbacks(),
  // AsyncRun()): SetCallbacks() gives PrimaryCommands a shared_ptr back to
  // this Router, so ~Router() never runs -- Stop() itself, not destruction,
  // must release the listening socket.
  //
  // A bound, ephemeral port obtained up front, not TESTING_PORT (0) passed
  // straight to Router: GetLocalEndpoint() returns the endpoint the Router
  // was constructed with, not the acceptor's actual bound local_endpoint(),
  // so constructing it with port 0 would make the probe below re-check a
  // different, freshly-assigned port instead of the one actually in use.
  uint16_t fixed_test_port;
  {
    boost::asio::io_context probe_io_context;
    carla::multigpu::Listener port_probe(
        probe_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("0.0.0.0"), 0));
    fixed_test_port = port_probe.GetLocalEndpoint().port();
  }

  auto router = std::make_shared<carla::multigpu::Router>(fixed_test_port);
  router->SetCallbacks();
  router->AsyncRun(1u);
  const auto endpoint = router->GetLocalEndpoint();

  // Give the background thread a bounded window to run the initial
  // async_accept SetCallbacks()/Listen() posts, arming the handler whose
  // capture of shared_from_this() this test exists to guard against leaking.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  router->Stop();

  // `router` is deliberately still alive with its reference cycle intact
  // here, exactly as in production: the check below must not depend on
  // ~Router()/~Listener() ever running.
  {
    boost::asio::io_context probe_io_context;
    EXPECT_NO_THROW({
      carla::multigpu::Listener probe(probe_io_context, endpoint);
    }) << "Router::Stop() did not release its listening port";
  }

  // Break the self-reference cycle so this test's Router doesn't leak for
  // the rest of the process; production leaks it on purpose.
  router->GetCommander().set_router(nullptr);
}

TEST_F(MultiGpuRouterTest, is_routed_is_false_before_the_sensor_is_routed) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  router->TestConnectSession(MakeFakeSession());
  carla::multigpu::PrimaryCommands commander(router);
  EXPECT_FALSE(commander.IsRouted(42u));
}

TEST_F(MultiGpuRouterTest, is_routed_is_true_for_the_routed_sensor_only) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session, 2020u));
  EXPECT_TRUE(commander.IsRouted(42u));
  EXPECT_FALSE(commander.IsRouted(43u));
}

TEST_F(MultiGpuRouterTest, is_routed_is_false_once_the_secondary_disconnects_before_any_purge) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session_a = MakeFakeSession();
  auto session_b = MakeFakeSession();
  router->TestConnectSession(session_a);
  router->TestConnectSession(session_b);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session_a, 2021u));

  router->TestDisconnectSession(session_a);
  EXPECT_FALSE(commander.IsRouted(42u));
}

TEST_F(MultiGpuRouterTest, is_routed_is_false_after_load_map) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session, 2022u));

  commander.SendLoadMap("/Game/Carla/Maps/Town10HD_Opt");
  EXPECT_FALSE(commander.IsRouted(42u));
}

TEST_F(MultiGpuRouterTest, is_routed_does_not_wait_for_a_request_in_flight) {
  auto router = std::make_shared<carla::multigpu::Router>(TESTING_PORT);
  router->AsyncRun(1u);
  auto session = MakeFakeSession();
  router->TestConnectSession(session);
  carla::multigpu::PrimaryCommands commander(router);
  ASSERT_TRUE(RouteSensor(*router, commander, session, 2023u));

  auto pending = std::async(std::launch::async, [&commander]() { return commander.GetToken(77u, 10u); });
  ASSERT_TRUE(WaitUntilPending(*router, session));
  auto routed = std::async(std::launch::async, [&commander]() { return commander.IsRouted(42u); });
  ASSERT_EQ(routed.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(routed.get());

  router->TestHandleResponse(session, MakeTokenReply(77u, 2024u));
  ASSERT_EQ(pending.wait_for(std::chrono::seconds(2)), std::future_status::ready);
  EXPECT_TRUE(commander.IsRouted(77u));
}
