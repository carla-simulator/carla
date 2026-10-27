// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "ActorGcTestServer.h"

#if defined(__SANITIZE_ADDRESS__)
#  define CARLA_TEST_ACTOR_GC_ASAN
#elif defined(__clang__)
#  if __has_feature(address_sanitizer)
#    define CARLA_TEST_ACTOR_GC_ASAN
#  endif
#endif

#ifdef CARLA_TEST_ACTOR_GC_ASAN
#  include <sanitizer/asan_interface.h>
#endif

namespace {

  using Simulator = carla::client::detail::Simulator;
  using Gc = carla::client::GarbageCollectionPolicy;

  auto Spawn(const std::shared_ptr<Simulator> &simulator, Gc gc) {
    carla::rpc::ActorDefinition definition;
    definition.id = "vehicle.actor-gc-lifetime-test";
    const carla::client::ActorBlueprint blueprint(definition);
    return simulator->SpawnActor(blueprint, carla::geom::Transform{}, nullptr,
        carla::rpc::AttachmentType::Rigid, gc);
  }

  void CheckLookedUpOwnerRelease(Gc gc) {
    actor_gc_test::ActorServer server;
    std::weak_ptr<Simulator> observed_simulator;
    carla::client::Actor *observed_actor = nullptr;
    {
      auto simulator = server.MakeSimulator();
      observed_simulator = simulator;
      carla::rpc::Actor description;
      description.id = 99u;
      description.description.id = "static.prop.gc-lifetime-test";
      // A looked-up handle has no spawn-frame grace; the actual initial
      // snapshot has no record for this id. No simulation frame is fabricated.
      auto actor = simulator->MakeActor(description, gc);
      ASSERT_FALSE(actor->IsAlive());
      observed_actor = actor.get();
      actor.reset();
      simulator.reset();
    }
    const bool released = observed_simulator.expired();
    EXPECT_TRUE(released);
    EXPECT_EQ(server.DestroyCount(), 0u);
    if (!released && gc == Gc::Enabled) {
      // Baseline-only cleanup after recording the failed ownership check:
      // the leaked actor is the sole remaining strong owner of the simulator.
      delete observed_actor;
    }
    EXPECT_TRUE(observed_simulator.expired());
  }

  TEST(actor_gc_lifetime, inactive_garbage_collected_handle_releases_local_owner) {
    CheckLookedUpOwnerRelease(Gc::Enabled);
  }

  TEST(actor_gc_lifetime, disabled_garbage_collection_releases_local_owner_control) {
    CheckLookedUpOwnerRelease(Gc::Disabled);
  }

  TEST(actor_gc_lifetime, live_garbage_collected_handle_still_destroys_server_actor) {
    actor_gc_test::ActorServer server;
    auto simulator = server.MakeSimulator();
    auto actor = Spawn(simulator, Gc::Enabled);
    ASSERT_TRUE(actor->IsAlive());
    actor.reset();
    EXPECT_EQ(server.DestroyCount(), 1u);
  }

  TEST(actor_gc_lifetime, false_destroy_reply_still_releases_local_owner_control) {
    actor_gc_test::ActorServer server(false);
    auto simulator = server.MakeSimulator();
    std::weak_ptr<Simulator> observed = simulator;
    auto actor = Spawn(simulator, Gc::Enabled);
    ASSERT_TRUE(actor->IsAlive());
    actor.reset();
    EXPECT_EQ(server.DestroyCount(), 1u);
    simulator.reset();
    EXPECT_TRUE(observed.expired());
  }

#ifdef CARLA_TEST_ACTOR_GC_ASAN
  TEST(actor_gc_lifetime, explicitly_destroyed_handle_releases_its_local_allocation) {
    actor_gc_test::ActorServer server;
    auto simulator = server.MakeSimulator();
    auto actor = Spawn(simulator, Gc::Enabled);
    auto *address = actor.get();
    ASSERT_TRUE(actor->Destroy());
    EXPECT_FALSE(actor->Destroy());
    EXPECT_FALSE(actor->IsAlive());
    actor.reset();
    // ASan's public shadow-memory query does not dereference freed storage.
    const bool released = __asan_address_is_poisoned(address) != 0;
    EXPECT_TRUE(released);
    if (!released) {
      delete address;
    }
    EXPECT_EQ(server.DestroyCount(), 1u);
  }
#endif

} // namespace
