#include <atomic>
#include <future>

#include <gtest/gtest.h>

#include <carla/carla_server.h>

#include "carla/Logging.h"
#include "carla/server/ServerTraits.h"

#include "Sensor.h"

using namespace carla::server;
using namespace boost::posix_time;

// These tests assume there is a CARLA client connected to port 2000.
static constexpr uint32_t PORT = 2000u;
static constexpr uint32_t TIMEOUT = 6u * 1000u;

#define SIZE_OF_ARRAY(array) (sizeof(array) / sizeof(array[0u]))

// #define CARLASERVER_TEST_LOG
#ifdef CARLASERVER_TEST_LOG
  template <typename ... Args>
  static inline void test_log(Args &&... args) {
    carla::logging::print(std::cout, "DEBUG:", std::forward<Args>(args)..., '\n');
  }
#else
  template <typename ... Args>
  static inline void test_log(Args &&...) {}
#endif

static auto make_carla_server() {
  const auto deleter = [](void *ptr) {
    test_log("destroying CarlaServer", ptr);
    carla_free_server(ptr);
  };
  auto ptr = std::unique_ptr<void, decltype(deleter)>(carla_make_server(), deleter);
  test_log("created CarlaServer", ptr.get());
  return ptr;
}

TEST(CarlaServerAPI, MakeCarlaServer) {
  auto CarlaServer = make_carla_server();
  ASSERT_TRUE(CarlaServer != nullptr);
}

TEST(CarlaServerAPI, SimBlocking) {
  auto CarlaServerGuard = make_carla_server();
  CarlaServerPtr CarlaServer = CarlaServerGuard.get();
  ASSERT_TRUE(CarlaServer != nullptr);

  std::array<test::Sensor, 5u> sensors;

  carla_sensor_definition sensor_definitions[sensors.size()];

  for (auto i = 0u; i < sensors.size(); ++i) {
    sensor_definitions[i] = sensors[i].definition();
  }

  const carla_transform start_locations[] = {
    {carla_vector3d{0.0f, 0.0f, 0.0f}, carla_vector3d{0.0f, 0.0f, 0.0f}, carla_rotation3d{0.0f, 0.0f, 0.0f}},
    {carla_vector3d{1.0f, 0.0f, 0.0f}, carla_vector3d{1.0f, 0.0f, 0.0f}, carla_rotation3d{0.0f, 0.0f, 0.0f}},
    {carla_vector3d{0.0f, 1.0f, 0.0f}, carla_vector3d{0.0f, 1.0f, 0.0f}, carla_rotation3d{0.0f, 0.0f, 0.0f}},
    {carla_vector3d{1.0f, 1.0f, 0.0f}, carla_vector3d{1.0f, 1.0f, 0.0f}, carla_rotation3d{0.0f, 0.0f, 0.0f}}
  };

  const auto S = CARLA_SERVER_SUCCESS;
  ASSERT_EQ(0, S);

  test_log("###### Begin Test ######");

  test_log("connecting...");
  ASSERT_EQ(S, carla_server_connect(CarlaServer, PORT, TIMEOUT));

  {
    test_log("waiting for new episode...");
    carla_request_new_episode values;
    ASSERT_EQ(S, carla_read_request_new_episode(CarlaServer, values, TIMEOUT));
  }

  for (auto i = 0u; i < 5u; ++i) {
    test_log("###### New Episode ######");
    {
      test_log("sending scene description...");
      const carla_scene_description values{
          // cppcheck-suppress constStatement
          "TestTown01",
          start_locations,
          SIZE_OF_ARRAY(start_locations),
          sensor_definitions,
          SIZE_OF_ARRAY(sensor_definitions)};
      ASSERT_EQ(S, carla_write_scene_description(CarlaServer, values, TIMEOUT));
    }
    {
      test_log("waiting for episode start...");
      carla_episode_start values;
      ASSERT_EQ(S, carla_read_episode_start(CarlaServer, values, TIMEOUT));
    }
    {
      test_log("sending episode ready...");
      const carla_episode_ready values{true};
      ASSERT_EQ(S, carla_write_episode_ready(CarlaServer, values, TIMEOUT));
    }

    std::array<carla_agent, 30u> agents_data;
    for (auto i = 0u; i < agents_data.size(); ++i) {
      agents_data[i].id = i;
      agents_data[i].type = CARLA_SERVER_AGENT_VEHICLE;
      agents_data[i].transform = start_locations[0u];
      agents_data[i].bounding_box = {start_locations[0u], {100.0f, 100.0f, 100.0f}};
      agents_data[i].forward_speed = 50.0f;
    }

    std::atomic_bool done{false};

    // Simulate game thread.
    auto game_thread_result = std::async(std::launch::async, [&](){
      while (!done) {
        {
          carla_measurements measurements;
          measurements.non_player_agents = agents_data.data();
          measurements.number_of_non_player_agents = agents_data.size();
          auto ec = carla_write_measurements(CarlaServer, measurements);
          if (ec != S)
            break;
        }
        {
          carla_control control;
          test_log("waiting for control...");
          auto ec = carla_read_control(CarlaServer, control, TIMEOUT);
          if ((ec != S) && (ec != CARLA_SERVER_TRY_AGAIN)) {
            break;
          }
        }
      }
    });

    // Simulate render thread.
    auto render_thread_result = std::async(std::launch::async, [&](){
      while (!done) {
        for (auto &sensor : sensors) {
          carla_write_sensor_data(CarlaServer, sensor.MakeRandomData());
          std::this_thread::sleep_for(std::chrono::microseconds(150));
        }
      };
    });

    for (;;) {
      carla_request_new_episode new_episode;
      auto ec = carla_read_request_new_episode(CarlaServer, new_episode, 0);
      ASSERT_TRUE((ec == S) || (ec == CARLA_SERVER_TRY_AGAIN));
      if (ec != CARLA_SERVER_TRY_AGAIN) {
        test_log("received new episode request");
        done = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    test_log("waiting for async's future");
    game_thread_result.get();
    render_thread_result.get();
  }
  test_log("###### End Test ######");
}
