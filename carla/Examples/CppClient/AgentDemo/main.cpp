// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// AgentDemo — end-to-end driveability test for the C++ port of
// agents.navigation.* (issue carla-simulator/carla#9554).
//
// Spawns a random vehicle at a random valid spawn point, wraps it with
// BehaviorAgent (Normal profile), picks a faraway spawn point as the
// destination, and ticks the agent every frame applying its control.
// The spectator camera is kept chase-positioned so the user can watch.
//
// Run against a live CARLA server. Usage:
//   carla-agent-demo                  (defaults: localhost:2000)
//   carla-agent-demo HOST PORT
//   carla-agent-demo HOST PORT MAP    (e.g. Town01)
//   carla-agent-demo HOST PORT MAP BEHAVIOR  (cautious/normal/aggressive)

#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/Vehicle.h>
#include <carla/client/World.h>
#include <carla/geom/Transform.h>

#include <carla/agents/navigation/BehaviorAgent.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

namespace cc = carla::client;
namespace cg = carla::geom;
namespace can = carla::agents::navigation;

using namespace std::chrono_literals;

namespace {

  std::atomic<bool> g_stop{false};

  void HandleSigint(int) { g_stop = true; }

  template <typename RangeT, typename RNG>
  auto &RandomChoice(const RangeT &range, RNG &rng) {
    if (range.empty()) {
      throw std::runtime_error("RandomChoice: empty range");
    }
    std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(rng)];
  }

  can::BehaviorAgent::Behavior ParseBehavior(const std::string &name) {
    if (name == "cautious")   return can::BehaviorAgent::Behavior::Cautious;
    if (name == "aggressive") return can::BehaviorAgent::Behavior::Aggressive;
    return can::BehaviorAgent::Behavior::Normal;
  }

  struct Args {
    std::string host  = "localhost";
    uint16_t    port  = 2000;
    std::string map;
    std::string behavior = "normal";
  };

  Args ParseArgs(int argc, char **argv) {
    Args a;
    if (argc >= 2) a.host = argv[1];
    if (argc >= 3) a.port = static_cast<uint16_t>(std::stoi(argv[2]));
    if (argc >= 4) a.map  = argv[3];
    if (argc >= 5) a.behavior = argv[4];
    return a;
  }

}  // namespace

int main(int argc, char **argv) try {
  std::signal(SIGINT,  HandleSigint);
  std::signal(SIGTERM, HandleSigint);

  const auto args = ParseArgs(argc, argv);

  std::mt19937_64 rng{std::random_device{}()};

  cc::Client client(args.host, args.port);
  client.SetTimeout(40s);
  std::cout << "Client " << client.GetClientVersion()
            << "  ↔  Server " << client.GetServerVersion() << "\n";

  // Use the currently loaded world unless the user asked for a specific map.
  auto world = args.map.empty() ? client.GetWorld()
                                : client.LoadWorld(args.map);
  std::cout << "Map: " << world.GetMap()->GetName() << "\n";

  // Spawn a vehicle at a random valid spawn point.
  auto map         = world.GetMap();
  auto spawn_pts   = map->GetRecommendedSpawnPoints();
  if (spawn_pts.empty()) {
    throw std::runtime_error("No spawn points available on this map");
  }
  auto spawn_tx    = RandomChoice(spawn_pts, rng);

  // Prefer agile cars over trucks/vans for the demo — large vehicles get
  // stuck on tight corners while the agent is still warming up.
  auto bp_library  = world.GetBlueprintLibrary();
  auto vehicles_bp = bp_library->Filter("vehicle.tesla.model3");
  if (vehicles_bp->size() == 0) {
    vehicles_bp = bp_library->Filter("vehicle.audi.*");
  }
  if (vehicles_bp->size() == 0) {
    vehicles_bp = bp_library->Filter("vehicle.*");
  }
  if (vehicles_bp->size() == 0) {
    throw std::runtime_error("No vehicle blueprints in this build");
  }
  auto bp          = RandomChoice(*vehicles_bp, rng);

  auto actor       = world.SpawnActor(bp, spawn_tx);
  auto vehicle     = std::dynamic_pointer_cast<cc::Vehicle>(actor);
  if (!vehicle) {
    actor->Destroy();
    throw std::runtime_error("Spawned actor is not a Vehicle");
  }
  std::cout << "Spawned " << vehicle->GetDisplayId()
            << " at (" << spawn_tx.location.x << ", " << spawn_tx.location.y
            << ", " << spawn_tx.location.z << ")\n";

  // Let physics settle so the vehicle is grounded before the agent engages.
  for (int i = 0; i < 5; ++i) world.WaitForTick(2s);

  // Pick a destination meaningfully ahead of the spawn (positive dot with
  // forward vector). This avoids the agent trying a low-speed U-turn from
  // its first step, which is what saturates the lateral PID and pins the
  // vehicle against geometry.
  const auto fwd = spawn_tx.GetForwardVector();
  cg::Transform dest_tx = spawn_tx;
  float best_score = -1.0f;
  for (int tries = 0; tries < 64; ++tries) {
    const auto &cand = RandomChoice(spawn_pts, rng);
    const float dx = cand.location.x - spawn_tx.location.x;
    const float dy = cand.location.y - spawn_tx.location.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 80.0f || dist > 400.0f) continue;
    const float forward_dot = (dx * fwd.x + dy * fwd.y) / dist;
    if (forward_dot < 0.2f) continue;     // require destination broadly ahead
    // Score combines forward bias and reasonable distance.
    const float score = forward_dot * std::min(dist, 300.0f);
    if (score > best_score) {
      best_score = score;
      dest_tx    = cand;
    }
  }
  if (best_score < 0.0f) {
    // Fall back: any spawn point >50m away that is at least vaguely ahead.
    for (const auto &cand : spawn_pts) {
      if (cand.location.Distance(spawn_tx.location) > 50.0f) {
        dest_tx = cand;
        break;
      }
    }
  }
  std::cout << "Destination: ("
            << dest_tx.location.x << ", " << dest_tx.location.y
            << ", " << dest_tx.location.z << ")\n";

  // Wrap with BehaviorAgent.
  auto behavior = ParseBehavior(args.behavior);
  std::cout << "Behavior:    " << args.behavior << "\n";
  can::BehaviorAgent agent(vehicle, behavior);
  agent.SetDestination(dest_tx.location);
  std::cout << "Agent route set; entering control loop. Ctrl-C to stop.\n";

  auto spectator = world.GetSpectator();

  size_t step = 0;
  while (!g_stop && !agent.Done()) {
    // Wait for the next server tick (works in both sync and async modes).
    world.WaitForTick(2s);

    // Run one step of the agent and apply.
    auto control = agent.RunStep();
    vehicle->ApplyControl(control);

    // Chase camera: 6m behind, 2.5m up, slight pitch down, looking forward.
    auto vt = vehicle->GetTransform();
    cg::Transform cam = vt;
    cam.location -= 6.0f * vt.GetForwardVector();
    cam.location.z += 2.5f;
    cam.rotation.pitch = -10.0f;
    spectator->SetTransform(cam);

    if (++step % 50 == 0) {
      const auto &v = vehicle->GetVelocity();
      const float speed_kmh =
          3.6f * std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
      std::cout << "  step " << step << "  throttle=" << control.throttle
                << " brake=" << control.brake << " steer=" << control.steer
                << "  speed=" << speed_kmh << " km/h\n";
    }
  }

  if (agent.Done()) {
    std::cout << "Agent reached destination after " << step << " steps.\n";
  } else {
    std::cout << "Stopped by user after " << step << " steps.\n";
  }

  vehicle->Destroy();
  std::cout << "Vehicle destroyed.\n";
  return 0;
} catch (const cc::TimeoutException &e) {
  std::cerr << "carla-agent-demo: timeout: " << e.what() << "\n";
  return 1;
} catch (const std::exception &e) {
  std::cerr << "carla-agent-demo: " << e.what() << "\n";
  return 2;
}
