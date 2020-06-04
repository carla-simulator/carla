#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/detail/Client.h>
#include <carla/client/Map.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/WalkerAIController.h>
#include <carla/geom/Transform.h>
#include <carla/rpc/EpisodeSettings.h>

#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>
#include <signal.h>

namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;
namespace crpc = carla::rpc;

using namespace std::chrono_literals;
using namespace std;

typedef carla::SharedPtr<carla::client::Actor> ShrdPtrActor;

bool isStopped;

void sighandler(int sig)
{
    switch(sig)
    {
        case SIGINT:
        cout << "\n***Handling Ctrl+C signal...***" << endl;
        cout.flush();
        isStopped = true;
        break;
    }
    cout << sig << endl;
}


/// Pick a random element from @a range.
template <typename RangeT, typename RNG>
static auto &RandomChoice(const RangeT &range, RNG &&generator)
{
    assert(range.size() > 0u);
    uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(forward<RNG>(generator))]; // KB: this can fail for map
}


int main(int argc, const char *argv[])
{
    isStopped = false;
    signal(SIGINT, sighandler);

    mt19937_64 rng((random_device())());

    auto client = cc::Client("127.0.0.1", 2000);
    client.SetTimeout(10s);

    cout << "Client API version : " << client.GetClientVersion() << '\n';
    cout << "Server API version : " << client.GetServerVersion() << '\n';

    auto m_world = client.LoadWorld("Town02");

    auto traffic_manager = client.GetInstanceTM(8000); //KB: the port
    traffic_manager.SetGlobalDistanceToLeadingVehicle(2.0);
    traffic_manager.SetSynchronousMode(true);

    // Synchronous mode:
    auto defaultSettings = m_world.GetSettings();
    crpc::EpisodeSettings wsettings(true, false, 1.0 / 30); // (synchrone, noRender, interval)
    m_world.ApplySettings(wsettings);
    m_world.SetWeather(crpc::WeatherParameters::ClearNoon);

    // Spawn Vehicles:
    const int number_of_vehicles = 50;
    auto blueprints = m_world.GetBlueprintLibrary()->Filter("vehicle.*");
    auto spawn_points = m_world.GetMap()->GetRecommendedSpawnPoints();
    vector<ShrdPtrActor> vehicles; vehicles.reserve(number_of_vehicles);

    for (int i = 0; i < number_of_vehicles; ++i)
    {
        auto blueprint = RandomChoice(*blueprints, rng);
        // Find a valid spawn point.
        auto transform = RandomChoice(spawn_points, rng);

        // Randomize the blueprint.
        if (blueprint.ContainsAttribute("color"))
        {
            auto &attribute = blueprint.GetAttribute("color");
            blueprint.SetAttribute("color", RandomChoice(attribute.GetRecommendedValues(), rng));
        }
        if (i==0)
            blueprint.SetAttribute("role_name", "hero");
        else
            blueprint.SetAttribute("role_name", "autopilot");

        // Spawn the vehicle.
        auto actor = m_world.TrySpawnActor(blueprint, transform);
        if (!actor) continue;
        // Finish and store the vehicle
        traffic_manager.SetPercentageIgnoreWalkers(actor, 0.0f);
        static_cast<cc::Vehicle*>(actor.get())->SetAutopilot(true);
        vehicles.push_back(actor);
        cout << "Spawned " << vehicles.back()->GetDisplayId() << '\n';
    }

    // Spawn walkers:
    const int number_of_walkers = 50;
    vector<ShrdPtrActor> walkers; walkers.reserve(number_of_walkers);
    vector<ShrdPtrActor> wControllers; wControllers.reserve(number_of_walkers);

    auto w_bp = m_world.GetBlueprintLibrary()->Filter("walker.pedestrian.*"); // "Filter" returns BluePrintLibrary (i.e. wrapper about container of ActorBlueprints)
    auto wc_bp = m_world.GetBlueprintLibrary()->Find("controller.ai.walker"); // "Find" returns pointer to the ActorBlueprint

    vector<float> speeds; speeds.reserve(number_of_walkers);

    for (int i = 0; i < number_of_walkers; ++i)
    {
        auto location = m_world.GetRandomLocationFromNavigation();
        if (!location.has_value()) continue;
        auto walker_bp = RandomChoice(*w_bp, rng);
        if (walker_bp.ContainsAttribute("is_invincible")) walker_bp.SetAttribute("is_invincible", "false");
        auto walker = m_world.TrySpawnActor(walker_bp, location.value());
        if (!walker) continue;

        auto controller = m_world.TrySpawnActor(*wc_bp, cg::Transform(), walker.get());
        if (!controller) continue;

        // Store the walker and its controller
        walkers.push_back(walker);
        speeds.push_back(atof(walker_bp.GetAttribute("speed").GetRecommendedValues()[1].c_str()));
        wControllers.push_back(controller);
        cout << "Spawned " << walkers.back()->GetDisplayId() << '\n';
    }

    m_world.Tick(carla::time_duration(chrono::seconds(10)));

    for (int i = 0; i < wControllers.size(); ++i)
    {
        // KB: important! First Start then any settings like max speed.
        static_cast<cc::WalkerAIController*>(wControllers[i].get())->Start();
        static_cast<cc::WalkerAIController*>(wControllers[i].get())->SetMaxSpeed(speeds[i]);
    }

    m_world.SetPedestriansCrossFactor(0.0f);

    while (!isStopped)
    {
        try
        {
            m_world.Tick(carla::time_duration(1s));
        }
        catch(exception & e) { cout << "Ignoring exception: " << e.what() << endl; }
    }

    m_world.ApplySettings(defaultSettings);
    for (auto v : vehicles) v->Destroy();
    for (auto w : walkers)  w->Destroy();

    return 0;
}
