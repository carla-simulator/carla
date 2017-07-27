#include <atomic>
#include <future>
#include <iostream>

#include <gtest/gtest.h>

#include <carla/carla_server.h>

#include "carla/Logging.h"
#include "carla/server/ServerTraits.h"

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

  const uint32_t image0[] = {1u, 2u, 3u, 4u};
  const uint32_t image1[] = {1u, 2u, 3u, 4u, 5u, 6u};
  const carla_image images[] = {
    {2u, 2u, 0u, image0},
    {2u, 3u, 1u, image1}
  };

  const carla_transform start_locations[] = {
    {carla_vector3d{0.0f, 0.0f, 0.0f}, carla_vector3d{0.0f, 0.0f, 0.0f}},
    {carla_vector3d{1.0f, 0.0f, 0.0f}, carla_vector3d{1.0f, 0.0f, 0.0f}},
    {carla_vector3d{0.0f, 1.0f, 0.0f}, carla_vector3d{0.0f, 1.0f, 0.0f}},
    {carla_vector3d{1.0f, 1.0f, 0.0f}, carla_vector3d{1.0f, 1.0f, 0.0f}}
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
          start_locations,
          SIZE_OF_ARRAY(start_locations)};
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

    std::atomic_bool done{false};
    auto agent_thread_result = std::async(std::launch::async, [&](){
      while (!done) {
        {
          carla_measurements measurements;
          // test_log("write measurements and images");
          auto ec = carla_write_measurements(CarlaServer, measurements, images, SIZE_OF_ARRAY(images));
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
    agent_thread_result.get();
  }
  test_log("###### End Test ######");
}
