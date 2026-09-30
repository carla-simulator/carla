// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/multigpu/sensorStreamRegistry.h>
#include <carla/streaming/Client.h>
#include <carla/streaming/Server.h>
#include <carla/streaming/detail/Token.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <thread>

namespace {

  using carla::multigpu::SensorStreamRegistry;
  using carla::streaming::detail::stream_id_type;
  using carla::streaming::detail::token_type;

  stream_id_type StreamIdOf(const carla::streaming::Stream &stream) {
    return token_type(stream.token()).get_stream_id();
  }

  /// A secondary's streaming server, routing sessions by primary stream id.
  struct SecondaryStreams {
    SecondaryStreams() {
      server.SetSessionsUseStreamAliases(true);
      server.AsyncRun(1u);
      registry.OpenEpisode(0u);
    }

    carla::streaming::Server server{TESTING_PORT};
    SensorStreamRegistry registry;
  };

  void WriteMessage(carla::streaming::Stream &stream, const std::string &text) {
    carla::Buffer buffer(boost::asio::buffer(text.c_str(), text.size()));
    stream.Write(carla::BufferView::CreateFrom(std::move(buffer)));
  }

  /// Writes to @a stream until @a received is set or @a attempts writes
  /// (10 ms apart) pass.
  bool WriteUntilReceived(
      carla::streaming::Stream &stream,
      const std::atomic_bool &received,
      int attempts = 200) {
    for (int attempt = 0; attempt < attempts && !received; ++attempt) {
      WriteMessage(stream, "data");
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return received;
  }

  boost::asio::ip::tcp::socket OpenRawSession(
      boost::asio::io_context &io_context,
      uint16_t port,
      stream_id_type stream_id) {
    boost::asio::ip::tcp::socket socket(io_context);
    socket.connect({boost::asio::ip::make_address("127.0.0.1"), port});
    boost::asio::write(socket, boost::asio::buffer(&stream_id, sizeof(stream_id)));
    return socket;
  }

  /// Writes to @a stream until @a socket has data or about two seconds pass.
  bool WriteUntilReadable(carla::streaming::Stream &stream, boost::asio::ip::tcp::socket &socket) {
    for (int attempt = 0; attempt < 200; ++attempt) {
      WriteMessage(stream, "data");
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      boost::system::error_code ec;
      if (socket.available(ec) > 0u) {
        return true;
      }
    }
    return false;
  }

  void Drain(boost::asio::ip::tcp::socket &socket) {
    std::array<char, 1024> buffer;
    boost::system::error_code ec;
    while (socket.available(ec) > 0u) {
      socket.read_some(boost::asio::buffer(buffer), ec);
    }
  }

} // namespace

TEST(streaming, find_token_does_not_create_a_stream_for_an_unknown_id) {
  carla::streaming::Server server(TESTING_PORT);
  EXPECT_FALSE(server.FindToken(42u).has_value());
  EXPECT_FALSE(server.FindToken(42u).has_value());
  EXPECT_FALSE(server.IsEnabledForROS(42u));
}

TEST(streaming, find_token_returns_the_existing_stream_token) {
  carla::streaming::Server server(TESTING_PORT);
  auto stream = server.MakeStream();
  const auto token = server.FindToken(StreamIdOf(stream));
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), StreamIdOf(stream));
}

TEST(streaming, stream_alias_can_be_set_replaced_and_removed) {
  carla::streaming::Server server(TESTING_PORT);
  EXPECT_FALSE(server.FindStreamAlias(100u).has_value());
  server.SetStreamAlias(100u, 3u);
  EXPECT_EQ(server.FindStreamAlias(100u), std::optional<stream_id_type>(3u));
  server.SetStreamAlias(100u, 4u);
  EXPECT_EQ(server.FindStreamAlias(100u), std::optional<stream_id_type>(4u));
  server.RemoveStreamAlias(100u);
  EXPECT_FALSE(server.FindStreamAlias(100u).has_value());
}

TEST(MultiGpuSensorStreams, token_carries_the_primary_stream_id_and_the_secondary_port) {
  SecondaryStreams secondary;
  auto sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true).has_value());

  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), 100u);
  EXPECT_EQ(token->get_port(), secondary.server.GetLocalEndpoint().port());
  EXPECT_EQ(secondary.server.FindStreamAlias(100u), std::optional<stream_id_type>(StreamIdOf(sensor)));
}

TEST(MultiGpuSensorStreams, resolve_is_keyed_by_actor_not_creation_order) {
  SecondaryStreams secondary;
  // The secondary replays sensors in a different order (e.g. a spawn frame
  // was dropped and later recovered by a full resync).
  auto second_sensor = secondary.server.MakeStream();
  auto first_sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 20u, StreamIdOf(second_sensor), true).has_value());
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 10u, StreamIdOf(first_sensor), true).has_value());

  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 10u, 101u).has_value());
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 20u, 102u).has_value());
  EXPECT_EQ(secondary.server.FindStreamAlias(101u), std::optional<stream_id_type>(StreamIdOf(first_sensor)));
  EXPECT_EQ(secondary.server.FindStreamAlias(102u), std::optional<stream_id_type>(StreamIdOf(second_sensor)));
}

TEST(MultiGpuSensorStreams, resolve_before_spawn_reserves_a_stream_the_sensor_adopts) {
  SecondaryStreams secondary;
  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), 100u);
  const auto reserved_id = secondary.server.FindStreamAlias(100u);
  ASSERT_TRUE(reserved_id.has_value());

  const auto again = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(again.has_value());
  EXPECT_EQ(again->get_stream_id(), 100u);
  EXPECT_EQ(secondary.server.FindStreamAlias(100u), reserved_id);

  auto sensor = secondary.server.MakeStream();
  ASSERT_NE(StreamIdOf(sensor), *reserved_id);
  const auto adopted = secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true);
  ASSERT_TRUE(adopted.has_value());
  EXPECT_EQ(StreamIdOf(*adopted), *reserved_id);
  EXPECT_EQ(secondary.server.FindStreamAlias(100u), reserved_id);
}

TEST(MultiGpuSensorStreams, resolve_after_the_sensor_stream_closed_reserves_a_new_one) {
  SecondaryStreams secondary;
  auto sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true).has_value());
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 7u, 100u).has_value());
  secondary.server.CloseStream(StreamIdOf(sensor));

  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(token->get_stream_id(), 100u);
  const auto reserved_id = secondary.server.FindStreamAlias(100u);
  ASSERT_TRUE(reserved_id.has_value());
  EXPECT_NE(*reserved_id, StreamIdOf(sensor));
  EXPECT_TRUE(secondary.server.FindToken(*reserved_id).has_value());
}

TEST(MultiGpuSensorStreams, unbind_removes_the_alias_and_closes_an_unadopted_reservation) {
  SecondaryStreams secondary;
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 7u, 100u).has_value());
  const auto reserved_id = secondary.server.FindStreamAlias(100u);
  ASSERT_TRUE(reserved_id.has_value());

  secondary.registry.Unbind(secondary.server, 7u);
  EXPECT_FALSE(secondary.server.FindStreamAlias(100u).has_value());
  EXPECT_FALSE(secondary.server.FindToken(*reserved_id).has_value());
}

TEST(MultiGpuSensorStreams, episode_change_drops_aliases_and_reservations_but_not_sensor_streams) {
  SecondaryStreams secondary;
  auto sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true).has_value());
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 7u, 100u).has_value());
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 8u, 101u).has_value());
  const auto reserved_id = secondary.server.FindStreamAlias(101u);
  ASSERT_TRUE(reserved_id.has_value());

  static_cast<void>(secondary.registry.BeginEpisodeChange(secondary.server));
  EXPECT_FALSE(secondary.server.FindStreamAlias(100u).has_value());
  EXPECT_FALSE(secondary.server.FindStreamAlias(101u).has_value());
  EXPECT_FALSE(secondary.server.FindToken(*reserved_id).has_value());
  // The sensor's own stream belongs to the sensor, which closes it itself.
  EXPECT_TRUE(secondary.server.FindToken(StreamIdOf(sensor)).has_value());
}

TEST(MultiGpuSensorStreams, bind_and_unbind_are_ignored_until_the_latest_load_opens) {
  SecondaryStreams secondary;
  const auto first_load = secondary.registry.BeginEpisodeChange(secondary.server);
  const auto second_load = secondary.registry.BeginEpisodeChange(secondary.server);
  ASSERT_NE(first_load, second_load);

  // A GET_TOKEN during the load still gets a token right away.
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 7u, 100u).has_value());
  const auto reserved_id = secondary.server.FindStreamAlias(100u);
  ASSERT_TRUE(reserved_id.has_value());

  // Replayed frames of the previous episode reuse its actor ids.
  auto stale_sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(stale_sensor), true).has_value());
  secondary.registry.Unbind(secondary.server, 7u);
  secondary.registry.OpenEpisode(first_load);
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(stale_sensor), true).has_value());
  EXPECT_EQ(secondary.server.FindStreamAlias(100u), reserved_id);
  EXPECT_TRUE(secondary.server.FindToken(*reserved_id).has_value());

  secondary.registry.OpenEpisode(second_load);
  auto sensor = secondary.server.MakeStream();
  const auto adopted = secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true);
  ASSERT_TRUE(adopted.has_value());
  EXPECT_EQ(StreamIdOf(*adopted), *reserved_id);
}

TEST(MultiGpuSensorStreams, close_episode_stops_binding_but_keeps_reservations) {
  SecondaryStreams secondary;
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 7u, 100u).has_value());
  secondary.registry.CloseEpisode();

  auto sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true).has_value());
  EXPECT_TRUE(secondary.server.FindStreamAlias(100u).has_value());
}

TEST(MultiGpuSensorStreams, secondaries_with_colliding_local_ids_hand_out_distinct_primary_ids) {
  SecondaryStreams secondary_a;
  SecondaryStreams secondary_b;
  auto sensor_a = secondary_a.server.MakeStream();
  auto sensor_b = secondary_b.server.MakeStream();
  ASSERT_EQ(StreamIdOf(sensor_a), StreamIdOf(sensor_b));
  EXPECT_FALSE(secondary_a.registry.Bind(secondary_a.server, 7u, StreamIdOf(sensor_a), true).has_value());
  EXPECT_FALSE(secondary_b.registry.Bind(secondary_b.server, 8u, StreamIdOf(sensor_b), true).has_value());

  const auto token_a = secondary_a.registry.Resolve(secondary_a.server, 7u, 14u);
  const auto token_b = secondary_b.registry.Resolve(secondary_b.server, 8u, 15u);
  ASSERT_TRUE(token_a.has_value());
  ASSERT_TRUE(token_b.has_value());
  EXPECT_EQ(token_a->get_stream_id(), 14u);
  EXPECT_EQ(token_b->get_stream_id(), 15u);

  // One client subscribed to both, as the Python client keys streams by id.
  std::atomic_bool received_a{false};
  std::atomic_bool received_b{false};
  carla::streaming::Client client;
  client.AsyncRun(1u);
  client.Subscribe(*token_a, [&received_a](auto) { received_a = true; });
  client.Subscribe(*token_b, [&received_b](auto) { received_b = true; });
  EXPECT_TRUE(WriteUntilReceived(sensor_a, received_a));
  EXPECT_TRUE(WriteUntilReceived(sensor_b, received_b));
}

TEST(MultiGpuSensorStreams, subscription_by_primary_id_reaches_the_adopted_stream_and_unsubscribes_by_it) {
  SecondaryStreams secondary;
  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());

  std::atomic_bool received{false};
  std::atomic_size_t count{0u};
  carla::streaming::Client client;
  client.AsyncRun(1u);
  client.Subscribe(*token, [&count, &received](auto) {
    ++count;
    received = true;
  });

  auto own_stream = secondary.server.MakeStream();
  auto adopted = secondary.registry.Bind(secondary.server, 7u, StreamIdOf(own_stream), true);
  ASSERT_TRUE(adopted.has_value());
  secondary.server.CloseStream(StreamIdOf(own_stream));
  ASSERT_TRUE(WriteUntilReceived(*adopted, received));

  // The primary's own token for the sensor: same stream id, other port.
  token_type primary_token(*token);
  primary_token.set_stream_id(100u);
  client.UnSubscribe(primary_token);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  const auto count_after_unsubscribe = count.load();
  for (int i = 0; i < 20; ++i) {
    WriteMessage(*adopted, "data");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  EXPECT_EQ(count.load(), count_after_unsubscribe);
}

TEST(MultiGpuSensorStreams, subscription_by_an_unaliased_id_is_refused) {
  SecondaryStreams secondary;
  auto sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(sensor), true).has_value());
  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());

  // A stale id from another episode that happens to equal a local stream id.
  token_type stale_token(*token);
  stale_token.set_stream_id(StreamIdOf(sensor));
  std::atomic_bool received{false};
  carla::streaming::Client client;
  client.AsyncRun(1u);
  client.Subscribe(stale_token, [&received](auto) { received = true; });
  for (int i = 0; i < 20; ++i) {
    WriteMessage(sensor, "data");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  EXPECT_FALSE(received);
  client.UnSubscribe(stale_token);
}

TEST(MultiGpuSensorStreams, reused_sensor_keeps_its_stream_and_takes_over_the_reservation) {
  SecondaryStreams secondary;
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 7u, 100u).has_value());
  const auto reserved_id = secondary.server.FindStreamAlias(100u);
  ASSERT_TRUE(reserved_id.has_value());

  auto live_sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(live_sensor), false).has_value());
  EXPECT_EQ(secondary.server.FindStreamAlias(100u), std::optional<stream_id_type>(StreamIdOf(live_sensor)));
  EXPECT_FALSE(secondary.server.FindToken(*reserved_id).has_value());
  EXPECT_TRUE(secondary.server.FindToken(StreamIdOf(live_sensor)).has_value());

  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());
  EXPECT_EQ(secondary.server.FindStreamAlias(100u), std::optional<stream_id_type>(StreamIdOf(live_sensor)));
}

TEST(MultiGpuSensorStreams, early_subscriber_of_a_reused_sensor_reconnects_to_its_stream) {
  SecondaryStreams secondary;
  const auto token = secondary.registry.Resolve(secondary.server, 7u, 100u);
  ASSERT_TRUE(token.has_value());

  std::atomic_bool received{false};
  carla::streaming::Client client;
  client.AsyncRun(1u);
  client.Subscribe(*token, [&received](auto) { received = true; });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto live_sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 7u, StreamIdOf(live_sensor), false).has_value());
  EXPECT_TRUE(WriteUntilReceived(live_sensor, received, 500));
  client.UnSubscribe(*token);
}

TEST(MultiGpuSensorStreams, bind_refuses_a_stream_already_bound_to_another_actor) {
  SecondaryStreams secondary;
  auto sensor = secondary.server.MakeStream();
  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 10u, StreamIdOf(sensor), true).has_value());
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 10u, 101u).has_value());

  EXPECT_FALSE(secondary.registry.Bind(secondary.server, 20u, StreamIdOf(sensor), false).has_value());
  ASSERT_TRUE(secondary.registry.Resolve(secondary.server, 20u, 102u).has_value());
  EXPECT_EQ(secondary.server.FindStreamAlias(101u), std::optional<stream_id_type>(StreamIdOf(sensor)));
  const auto other = secondary.server.FindStreamAlias(102u);
  ASSERT_TRUE(other.has_value());
  EXPECT_NE(*other, StreamIdOf(sensor));
}

TEST(streaming, closing_a_session_after_its_alias_moved_leaves_the_new_stream_session_alone) {
  carla::streaming::Server server(TESTING_PORT);
  server.SetSessionsUseStreamAliases(true);
  server.AsyncRun(1u);
  auto old_stream = server.MakeStream();
  auto new_stream = server.MakeStream();
  const auto port = token_type(old_stream.token()).get_port();

  // Raw sockets, unlike carla::streaming::Client, never reconnect, which
  // would hide a session wrongly dropped by the server.
  boost::asio::io_context io_context;
  server.SetStreamAlias(100u, StreamIdOf(old_stream));
  auto old_session = OpenRawSession(io_context, port, 100u);
  ASSERT_TRUE(WriteUntilReadable(old_stream, old_session));

  server.SetStreamAlias(100u, StreamIdOf(new_stream));
  auto new_session = OpenRawSession(io_context, port, 100u);
  ASSERT_TRUE(WriteUntilReadable(new_stream, new_session));

  // The server notices the closed session when writing to it.
  old_session.close();
  for (int i = 0; i < 20; ++i) {
    WriteMessage(old_stream, "data");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  Drain(new_session);
  EXPECT_TRUE(WriteUntilReadable(new_stream, new_session));
}
