// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/nav/WalkerManager.h"

#include "carla/Logging.h"
#include "carla/client/ActorSnapshot.h"
#include "carla/client/Waypoint.h"
#include "carla/client/World.h"
#include "carla/client/detail/Simulator.h"
#include "carla/nav/Navigation.h"
#include "carla/rpc/Actor.h"

namespace carla {
namespace nav {

    WalkerManager::WalkerManager() {
    }

    WalkerManager::~WalkerManager() {
    }

    // assign the navigation module
    void WalkerManager::SetNav(Navigation *nav) {
        _nav = nav;
    }

    // reference to the simulator to access API functions
    void WalkerManager::SetSimulator(std::weak_ptr<carla::client::detail::Simulator> simulator) {
        _simulator = simulator;
    }

	// create a new walker route
    bool WalkerManager::AddWalker(ActorId id) {
        GetAllTrafficLightWaypoints();

        WalkerInfo info;
        info.state = WALKER_IDLE;

        // save it
        _walkers[id] = std::move(info);

        return true;
    }

	// remove a walker route
    bool WalkerManager::RemoveWalker(ActorId id) {
        // search
        auto it = _walkers.find(id);
        if (it == _walkers.end())
            return false;
        _walkers.erase(it);

        return true;
    }

	// update all routes
    bool WalkerManager::Update(double delta) {

        // check all walkers
        for (auto &it : _walkers) {

            // get the elements
            WalkerInfo &info = it.second;

            // in function of the state
            switch (info.state) {
                case WALKER_IDLE:
                    break;

                case WALKER_WALKING:
                    {
                        // get the target point
                        carla::geom::Location &target = info.route[info.currentIndex].location;
                        // get current position
                        carla::geom::Location current;
                        _nav->GetWalkerPosition(it.first, current);
                        // check distance to the target point
                        carla::geom::Vector3D dist(target.x - current.x, target.z - current.z, target.y - current.y);
                        if (dist.SquaredLength() <= 1) {
                            info.state = WALKER_IN_EVENT;
                        }
                    }
                    break;

                case WALKER_IN_EVENT:
                    switch (ExecuteEvent(it.first, it.second, delta)) {
                        case EventResult::Continue:
                            break;
                        case EventResult::End:
                            // next point in route
                            SetWalkerNextPoint(it.first);
                            break;
                        case EventResult::TimeOut:
                            // unblock changing the route
                            SetWalkerRoute(it.first);
                            break;
                    }
                    break;

                case WALKER_STOP:
                    info.state = WALKER_IDLE;
                    break;
            }
        }

        return true;
    }

	// set a new route from its current position
    bool WalkerManager::SetWalkerRoute(ActorId id) {
        // check
        if (_nav == nullptr)
            return false;

        // set a new random target
        carla::geom::Location location;
        _nav->GetRandomLocation(location, nullptr);

        // set the route
        return SetWalkerRoute(id, location);
    }

	// set a new route from its current position
    bool WalkerManager::SetWalkerRoute(ActorId id, carla::geom::Location to) {
        // check
        if (_nav == nullptr)
            return false;

        // search
        auto it = _walkers.find(id);
        if (it == _walkers.end())
            return false;

        // get it
        WalkerInfo &info = it->second;
        std::vector<carla::geom::Location> path;
        std::vector<unsigned char> area;

        // save both points for the route
        _nav->GetWalkerPosition(id, info.from);
        info.to = to;
        info.currentIndex = 0;
        info.state = WALKER_IDLE;

        // get a route from navigation
        _nav->GetAgentRoute(id, info.from, to, path, area);

        // create each point of the route
        info.route.clear();
        info.route.reserve(path.size());
        unsigned char previous_area = CARLA_AREA_SIDEWALK;
        for (unsigned int i=0; i<path.size(); ++i) {
            // get the type
            switch (area[i]) {
                // do nothing
                case CARLA_AREA_SIDEWALK:
                    info.route.emplace_back(WalkerEventIgnore(), std::move(path[i]), area[i]);
                    break;

                // stop and check
                case CARLA_AREA_ROAD:
                case CARLA_AREA_CROSSWALK:
                    // only if we come from a safe area (sidewalks, grass or crosswalk)
                    if (previous_area != CARLA_AREA_CROSSWALK && previous_area != CARLA_AREA_ROAD)
                        info.route.emplace_back(WalkerEventStopAndCheck(60), std::move(path[i]), area[i]);
                    break;

                default:
                    info.route.emplace_back(WalkerEventIgnore(), std::move(path[i]), area[i]);
            }
            previous_area = area[i];
        }

        // assign the first point to go (second in the list)
        SetWalkerNextPoint(id);
        return true;
    }

    // set the next point in the route
    bool WalkerManager::SetWalkerNextPoint(ActorId id) {
        // check
        if (_nav == nullptr)
            return false;

        // search
        auto it = _walkers.find(id);
        if (it == _walkers.end())
            return false;

        // get it
        WalkerInfo &info = it->second;

        // advance next point
        ++info.currentIndex;

        // check the end
        if (info.currentIndex < info.route.size()) {
            // change the state
            info.state = WALKER_WALKING;
            // assign the point to go
            _nav->PauseAgent(id, false);
            _nav->SetWalkerDirectTarget(id, info.route[info.currentIndex].location);
        } else {
            // change the state
            info.state = WALKER_STOP;
            _nav->PauseAgent(id, true);
            // we need a new route from here
            SetWalkerRoute(id);
        }

        return true;
    }

    // get the next point in the route
    bool WalkerManager::GetWalkerNextPoint(ActorId id, carla::geom::Location &location) {
        // check
        if (_nav == nullptr)
            return false;

        // search
        auto it = _walkers.find(id);
        if (it == _walkers.end())
            return false;

        // get it
        WalkerInfo &info = it->second;

        // check the end
        if (info.currentIndex < info.route.size()) {
            location = info.route[info.currentIndex].location;
            return true;
        } else {
            return false;
        }
    }

    bool WalkerManager::GetWalkerCrosswalkEnd(ActorId id, carla::geom::Location &location) {
        // check
        if (_nav == nullptr)
            return false;

        // search
        auto it = _walkers.find(id);
        if (it == _walkers.end())
            return false;

        // get it
        WalkerInfo &info = it->second;

        // check the end of current crosswalk
        unsigned int pos = info.currentIndex;
        while (pos < info.route.size()) {
            if (info.route[pos].areaType != CARLA_AREA_CROSSWALK) {
                location = info.route[pos].location;
                return true;
            }
            ++pos;
        }

        return false;
    }

    EventResult WalkerManager::ExecuteEvent(ActorId id, WalkerInfo &info, double delta) {
        // go to the event
        WalkerRoutePoint &rp = info.route[info.currentIndex];

        // build the visitor structure
        WalkerEventVisitor visitor(this, id, delta);
        // run the event
        return std::visit(visitor, rp.event);
    }

    void WalkerManager::GetAllTrafficLightWaypoints() {
        static bool AlreadyCalculated = false;
        if (AlreadyCalculated) return;

        // the world
        carla::client::World world = _simulator.lock()->GetWorld();

        _traffic_lights.clear();
        std::vector<carla::rpc::Actor> actors = _simulator.lock()->GetAllTheActorsInTheEpisode();
        for (auto actor : actors) {
            carla::client::ActorSnapshot snapshot = _simulator.lock()->GetActorSnapshot(actor.id);
            // check traffic lights only
            if (actor.description.id == "traffic.traffic_light") {
                // get the TL actor
                SharedPtr<carla::client::TrafficLight> tl =
                    std::static_pointer_cast<carla::client::TrafficLight>(world.GetActor(actor.id));
                // get the waypoints where the TL affects
                std::vector<SharedPtr<carla::client::Waypoint>> list = tl->GetStopWaypoints();
                for (auto &way : list) {
                    _traffic_lights.emplace_back(tl, way->GetTransform().location);
                }
            }
        }

        AlreadyCalculated = true;
    }


    // return the trafficlight affecting that position
    SharedPtr<carla::client::TrafficLight> WalkerManager::GetTrafficLightAffecting(
        carla::geom::Location UnrealPos,
        float max_distance) {
            float min_dist = std::numeric_limits<float>::infinity();
            SharedPtr<carla::client::TrafficLight> actor;
            for (auto &&item : _traffic_lights) {
                float dist = UnrealPos.DistanceSquared(item.second);
                if (dist < min_dist) {
                    min_dist = dist;
                    actor = item.first;
                }
            }
            // if distance is not in the limit, then reject the trafficlight
            if (max_distance < 0.0f || min_dist <= max_distance * max_distance) {
                return actor;
            } else {
                return SharedPtr<carla::client::TrafficLight>();
            }
    }


} // namespace nav
} // namespace carla
