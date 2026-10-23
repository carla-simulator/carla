// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/streaming/detail/Dispatcher.h>
#include <carla/streaming/detail/MultiStreamState.h>
#include <carla/streaming/detail/tcp/Server.h>

#include <atomic>
#include <latch>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {

  namespace cs = carla::streaming;
  namespace detail = carla::streaming::detail;

  class StreamListenerState : public testing::Test {
  protected:

    StreamListenerState()
      : server(io, {boost::asio::ip::address_v4::loopback(), 0u}),
        state(detail::token_type(1u, cs::make_endpoint(server.GetLocalEndpoint()))),
        first(std::make_shared<detail::Session>(io, 1s, server)),
        second(std::make_shared<detail::Session>(io, 1s, server)) {}

    ~StreamListenerState() {
      io.stop();
    }

    template <typename Mutation>
    void RaceReaders(Mutation mutation) {
      std::latch ready(1u);
      std::atomic_bool done{false};
      std::atomic_size_t observations{0u};
      std::thread reader([&]() {
        ready.count_down();
        do {
          const auto listening = state.AreClientsListening();
          const auto ros = state.IsEnabledForROS();
          observations += 1u + size_t(listening) + size_t(ros);
        } while (!done.load());
      });
      ready.wait();
      for (size_t i = 0u; i < 5000u; ++i) {
        mutation();
        if (i % 16u == 0u) std::this_thread::yield();
      }
      done = true;
      reader.join();
      EXPECT_GT(observations.load(), 0u);
    }

    boost::asio::io_context io;
    detail::tcp::Server server;
    detail::MultiStreamState state;
    std::shared_ptr<detail::Session> first;
    std::shared_ptr<detail::Session> second;
  };

} // namespace

TEST_F(StreamListenerState, multiple_clients_and_last_disconnect) {
  EXPECT_FALSE(state.AreClientsListening());
  state.ConnectSession(first);
  state.ConnectSession(second);
  EXPECT_TRUE(state.AreClientsListening());
  state.DisconnectSession(first);
  EXPECT_TRUE(state.AreClientsListening());
  state.DisconnectSession(second);
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, last_disconnect_resets_forced_activity) {
  state.ForceActive();
  state.DisconnectSession(first);
  EXPECT_TRUE(state.AreClientsListening());
  state.ConnectSession(first);
  state.DisconnectSession(first);
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, clear_and_disconnect_preserve_ros_visibility) {
  state.EnableForROS();
  state.ForceActive();
  state.ConnectSession(first);
  state.DisconnectSession(first);
  EXPECT_TRUE(state.IsEnabledForROS());
  EXPECT_TRUE(state.AreClientsListening());
  state.ConnectSession(second);
  state.ClearSessions();
  EXPECT_TRUE(state.IsEnabledForROS());
  EXPECT_TRUE(state.AreClientsListening());
  state.DisableForROS();
  EXPECT_FALSE(state.IsEnabledForROS());
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, disabling_ros_keeps_connected_clients) {
  state.ConnectSession(first);
  state.EnableForROS();
  state.DisableForROS();
  EXPECT_TRUE(state.AreClientsListening());
  state.DisconnectSession(first);
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, clear_resets_forced_activity_without_clients) {
  state.ForceActive();
  EXPECT_TRUE(state.AreClientsListening());
  state.ClearSessions();
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, concurrent_session_churn_and_listener_queries) {
  RaceReaders([&]() {
    state.ConnectSession(first);
    state.DisconnectSession(first);
  });
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, concurrent_ros_visibility_and_listener_queries) {
  RaceReaders([&]() {
    state.EnableForROS();
    state.DisableForROS();
  });
  EXPECT_FALSE(state.IsEnabledForROS());
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, concurrent_force_clear_and_listener_queries) {
  RaceReaders([&]() {
    state.ForceActive();
    state.ClearSessions();
  });
  EXPECT_FALSE(state.AreClientsListening());
}

TEST_F(StreamListenerState, concurrent_clients_force_and_ros_visibility) {
  std::latch start(1u);
  std::thread clients([&]() {
    start.wait();
    for (size_t i = 0u; i < 5000u; ++i) {
      state.ConnectSession(first);
      state.DisconnectSession(first);
    }
  });
  std::thread forced([&]() {
    start.wait();
    for (size_t i = 0u; i < 5000u; ++i) {
      state.ForceActive();
      state.ClearSessions();
    }
  });
  std::thread ros([&]() {
    start.wait();
    for (size_t i = 0u; i < 5000u; ++i) {
      state.EnableForROS();
      state.DisableForROS();
    }
    state.EnableForROS();
  });
  start.count_down();
  clients.join();
  forced.join();
  ros.join();
  EXPECT_TRUE(state.IsEnabledForROS());
  state.ClearSessions();
  EXPECT_TRUE(state.AreClientsListening());
  state.DisableForROS();
  EXPECT_FALSE(state.AreClientsListening());
}

TEST(stream_listener_dispatcher, concurrent_stream_lifecycle_and_ros_queries) {
  detail::Dispatcher dispatcher(cs::make_endpoint<boost::asio::ip::tcp>(0u));
  auto persistent = dispatcher.MakeStream();
  const auto id = detail::token_type(persistent.token()).get_stream_id();
  std::latch ready(1u);
  std::atomic_bool done{false};
  std::thread observer([&]() {
    ready.count_down();
    while (!done.load()) {
      dispatcher.EnableForROS(id);
      dispatcher.IsEnabledForROS(id);
      dispatcher.DisableForROS(id);
      dispatcher.SetROS2TopicVisibilityDefaultEnabled(true);
      dispatcher.SetROS2TopicVisibilityDefaultEnabled(false);
    }
  });
  ready.wait();
  for (size_t repetition = 0u; repetition < 50u; ++repetition) {
    std::vector<cs::Stream> streams;
    for (size_t i = 0u; i < 64u; ++i) streams.push_back(dispatcher.MakeStream());
    for (const auto &stream : streams)
      dispatcher.CloseStream(detail::token_type(stream.token()).get_stream_id());
  }
  done = true;
  observer.join();
  dispatcher.DisableForROS(id);
  EXPECT_FALSE(persistent.AreClientsListening());
}

TEST(stream_listener_dispatcher, ros_defaults_and_explicit_overrides) {
  detail::Dispatcher dispatcher(cs::make_endpoint<boost::asio::ip::tcp>(0u));
  auto hidden = dispatcher.MakeStream();
  EXPECT_FALSE(hidden.AreClientsListening());
  dispatcher.SetROS2TopicVisibilityDefaultEnabled(true);
  auto visible = dispatcher.MakeStream();
  const auto id = detail::token_type(visible.token()).get_stream_id();
  EXPECT_TRUE(visible.AreClientsListening());
  dispatcher.DisableForROS(id);
  EXPECT_FALSE(visible.AreClientsListening());
  dispatcher.EnableForROS(id);
  EXPECT_TRUE(dispatcher.IsEnabledForROS(id));
  dispatcher.CloseStream(id);
  EXPECT_FALSE(dispatcher.IsEnabledForROS(id));
}
