// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/PublisherRegistry.h>

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace {

  std::atomic<int> g_live_publishers{0};
  std::atomic<int> g_use_after_free{0};

  struct FakePublisher {
    static constexpr std::uint32_t kAlive = 0xA11CEu;
    static constexpr std::uint32_t kDead = 0xDEADu;

    explicit FakePublisher(std::string topic = {}) : topic(std::move(topic)) {
      ++g_live_publishers;
    }

    virtual ~FakePublisher() {
      magic = kDead;
      --g_live_publishers;
      if (on_destroy) {
        on_destroy();
      }
    }

    void Publish() {
      if (magic != kAlive) {
        ++g_use_after_free;
      }
      ++published;
    }

    std::uint32_t magic{kAlive};
    std::string topic;
    std::atomic<int> published{0};
    std::function<void()> on_destroy;
  };

  struct FakeCamera : FakePublisher {
    using FakePublisher::FakePublisher;
  };

  struct FakeTransform : FakePublisher {
    using FakePublisher::FakePublisher;
  };

  using Registry = carla::ros2::PublisherRegistry<FakePublisher, FakeCamera, FakeTransform>;
  using Naming = Registry::Naming;

  std::shared_ptr<FakeCamera> GetCamera(Registry &registry, void *actor, std::uint32_t id = 7u) {
    return registry.GetOrCreateCamera(actor, id, "rgb", [](const Naming &naming) {
      return std::make_shared<FakeCamera>(naming.base_topic);
    });
  }

  std::shared_ptr<FakePublisher> GetSensor(Registry &registry, void *actor, std::uint32_t id = 7u) {
    return registry.GetOrCreateSensor(actor, id, {"imu"}, [](const Naming &naming) {
      return std::make_shared<FakePublisher>(naming.base_topic);
    });
  }

  Registry::TransformTarget GetTransform(Registry &registry, void *actor) {
    return registry.GetOrCreateTransform(actor, [] { return std::make_shared<FakeTransform>(); });
  }

  /// True if another thread can take the registry lock while this one runs.
  bool LockIsFree(const Registry &registry) {
    auto probe = std::async(std::launch::async, [&registry] { registry.GetTopicOverride(nullptr); });
    return probe.wait_for(std::chrono::seconds(2)) == std::future_status::ready;
  }

} // namespace

TEST(ROS2PublisherRegistry, names_publishers_from_the_registration) {
  Registry registry;
  int vehicle = 0;
  int camera = 0;
  registry.Register(&vehicle, {"hero", "hero", true});
  registry.Register(&camera, {"rgb__", "rgb__", true});
  registry.AddParent(&camera, &vehicle);

  const auto publisher = GetCamera(registry, &camera, 42u);
  ASSERT_NE(publisher, nullptr);
  EXPECT_EQ(publisher->topic, "rt/carla/hero/rgb42");
  EXPECT_EQ(GetCamera(registry, &camera, 99u), publisher);

  const auto transform = GetTransform(registry, &camera);
  ASSERT_NE(transform.publisher, nullptr);
  EXPECT_EQ(transform.parent_frame, "hero");
  EXPECT_EQ(transform.frame_id, "rgb42");
  EXPECT_EQ(GetTransform(registry, &vehicle).parent_frame, "map");
}

TEST(ROS2PublisherRegistry, topic_override_replaces_the_name_and_its_removal_drops_publishers) {
  Registry registry;
  int lidar = 0;
  registry.Register(&lidar, {"lidar", "lidar", true});
  registry.SetTopicOverride(&lidar, "/sensing/lidar/top");
  const auto overridden = GetSensor(registry, &lidar);
  EXPECT_EQ(overridden->topic, "rt/sensing/lidar/top");

  registry.RemoveTopicOverride(&lidar);
  const auto renamed = GetSensor(registry, &lidar);
  EXPECT_NE(renamed, overridden);
  EXPECT_EQ(renamed->topic, "rt/carla/lidar");
}

TEST(ROS2PublisherRegistry, opted_out_actor_gets_no_transform) {
  Registry registry;
  int imu = 0;
  registry.Register(&imu, {"imu", "imu", false});
  EXPECT_EQ(GetTransform(registry, &imu).publisher, nullptr);
  int unregistered = 0;
  EXPECT_EQ(GetTransform(registry, &unregistered).publisher, nullptr);
}

TEST(ROS2PublisherRegistry, release_and_unregister_drop_the_cached_publishers) {
  Registry registry;
  int camera = 0;
  registry.Register(&camera, {"rgb", "rgb", true});
  const auto first = GetCamera(registry, &camera);
  const auto first_transform = GetTransform(registry, &camera).publisher;

  registry.Release(&camera);
  EXPECT_NE(GetCamera(registry, &camera), first);
  EXPECT_NE(GetTransform(registry, &camera).publisher, first_transform);

  registry.Unregister(&camera);
  EXPECT_EQ(GetTransform(registry, &camera).publisher, nullptr);
  EXPECT_EQ(GetCamera(registry, &camera)->topic, "");
}

TEST(ROS2PublisherRegistry, publishers_are_built_without_the_lock) {
  Registry registry;
  int camera = 0;
  registry.Register(&camera, {"rgb", "rgb", true});
  bool lock_free = false;
  registry.GetOrCreateCamera(&camera, 1u, "rgb", [&](const Naming &naming) {
    lock_free = LockIsFree(registry);
    return std::make_shared<FakeCamera>(naming.base_topic);
  });
  EXPECT_TRUE(lock_free);
}

TEST(ROS2PublisherRegistry, publishers_are_destroyed_without_the_lock) {
  Registry registry;
  int camera = 0;
  registry.Register(&camera, {"rgb", "rgb", true});
  int destroyed = 0;
  bool lock_free = true;
  auto watch = [&](auto publisher) {
    publisher->on_destroy = [&] {
      ++destroyed;
      lock_free = lock_free && LockIsFree(registry);
    };
  };

  watch(GetCamera(registry, &camera));
  registry.Release(&camera);
  watch(GetCamera(registry, &camera));
  registry.Unregister(&camera);
  watch(GetCamera(registry, &camera));
  registry.SetTopicOverride(&camera, "custom");
  registry.RemoveTopicOverride(&camera);
  registry.Register(&camera, {"rgb", "rgb", true});
  watch(GetSensor(registry, &camera));
  watch(GetTransform(registry, &camera).publisher);
  registry.Clear();

  EXPECT_EQ(destroyed, 5);
  EXPECT_TRUE(lock_free);
}

TEST(ROS2PublisherRegistry, a_publisher_released_while_being_built_is_not_cached) {
  Registry registry;
  int camera = 0;
  registry.Register(&camera, {"rgb", "rgb", true});
  const auto built = registry.GetOrCreateCamera(&camera, 1u, "rgb", [&](const Naming &naming) {
    registry.Release(&camera);
    return std::make_shared<FakeCamera>(naming.base_topic);
  });
  ASSERT_NE(built, nullptr);
  EXPECT_NE(GetCamera(registry, &camera), built);
}

TEST(ROS2PublisherRegistry, concurrent_builders_share_the_first_cached_publisher) {
  Registry registry;
  int camera = 0;
  registry.Register(&camera, {"rgb", "rgb", true});
  std::shared_ptr<FakeCamera> inner;
  int losers_destroyed = 0;
  const auto outer = registry.GetOrCreateCamera(&camera, 1u, "rgb", [&](const Naming &naming) {
    inner = GetCamera(registry, &camera);
    auto loser = std::make_shared<FakeCamera>(naming.base_topic);
    loser->on_destroy = [&] { ++losers_destroyed; };
    return loser;
  });
  EXPECT_EQ(outer, inner);
  EXPECT_EQ(GetCamera(registry, &camera), inner);
  EXPECT_EQ(losers_destroyed, 1);
}

TEST(ROS2PublisherRegistry, publishing_races_registration_release_and_unregister) {
  constexpr int kActors = 8;
  constexpr int kPublishers = 6;
  constexpr int kIterations = 4000;
  std::array<int, kActors> actors{};
  const int live_before = g_live_publishers.load();
  g_use_after_free = 0;
  {
    Registry registry;
    std::atomic<bool> go{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < kPublishers; ++t) {
      threads.emplace_back([&, t] {
        std::mt19937 random(static_cast<unsigned>(t));
        while (!go) {
          std::this_thread::yield();
        }
        for (int i = 0; i < kIterations; ++i) {
          void *actor = &actors[random() % kActors];
          const auto id = static_cast<std::uint32_t>(random() % 1000u);
          if (auto camera = GetCamera(registry, actor, id)) {
            camera->Publish();
          }
          if (auto sensor = GetSensor(registry, actor, id)) {
            sensor->Publish();
          }
          if (auto transform = GetTransform(registry, actor).publisher) {
            transform->Publish();
          }
          if ((i % 64) == 0) {
            registry.Release(actor);
          }
        }
      });
    }

    threads.emplace_back([&] {
      std::mt19937 random(1234u);
      while (!go) {
        std::this_thread::yield();
      }
      for (int i = 0; i < kIterations; ++i) {
        void *actor = &actors[random() % kActors];
        void *parent = &actors[random() % kActors];
        switch (random() % 6u) {
          case 0: registry.Register(actor, {"rgb__", "rgb__", (i % 2) == 0}); break;
          case 1: registry.Unregister(actor); break;
          case 2: registry.AddParent(actor, parent); break;
          case 3: registry.SetTopicOverride(actor, "override"); break;
          case 4: registry.RemoveTopicOverride(actor); break;
          default: registry.Release(actor); break;
        }
        (void)registry.BaseTopicName(actor);
      }
    });

    go = true;
    for (auto &thread : threads) {
      thread.join();
    }
    registry.Clear();
  }
  EXPECT_EQ(g_use_after_free.load(), 0);
  EXPECT_EQ(g_live_publishers.load(), live_before);
}
