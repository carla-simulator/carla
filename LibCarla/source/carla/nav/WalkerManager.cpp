// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/nav/WalkerManager.h"
#include "carla/nav/Navigation.h"

namespace carla {
namespace nav {

    WalkerManager::WalkerManager() {
        _nav = nullptr;
    }

    WalkerManager::~WalkerManager() {
    }

	/// create a new walker route
    bool WalkerManager::AddWalker(ActorId id) {
        WalkerInfo info;
        info.state = WALKER_IDLE;

        // save it
        _walkers[id] = std::move(info);

        return true;
    }

	/// remove a walker route
    bool WalkerManager::RemoveWalker(ActorId id) {
        // search
        auto it = _walkers.find(id);
        if (it == _walkers.end())
            return false;
        _walkers.erase(it);

        return true;
    }

	/// update all routes
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
                        if (dist.SquaredLength() <= 2) {
                            info.state = WALKER_IN_EVENT;
                            _nav->PauseAgent(it.first, true);
                        }
                    }
                    break;

                case WALKER_IN_EVENT:
                    if (ExecuteEvent(it.second, delta)) {
                        SetWalkerNextPoint(it.first);
                    }
                    break;

                case WALKER_STOP:
                    info.state = WALKER_IDLE;
                    break;
            }
        }

        return true;
    }

	/// set a new route from its current position
    bool WalkerManager::SetWalkerRoute(ActorId id) {
        // check
        if (_nav == nullptr)
            return false;

        // set a new random target
        carla::geom::Location location;
        _nav->GetRandomLocation(location, 1, nullptr);

        // set the route
        SetWalkerRoute(id, location);
    }

	/// set a new route from its current position
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
        for (unsigned int i=0; i<path.size(); ++i) {
            // get the type
            switch (area[i]) {
                // random waiting
                case SAMPLE_POLYAREA_GROUND:
                    info.route.emplace_back(WalkerEventIgnore(), std::move(path[i]));
                    break;

                // stop and check
                case SAMPLE_POLYAREA_ROAD:
                case SAMPLE_POLYAREA_CROSS:
                    info.route.emplace_back(WalkerEventStopAndCheck(id, _nav, 3), std::move(path[i]));
                    break;

                default:
                    info.route.emplace_back(WalkerEventIgnore(), std::move(path[i]));
            }
        }

        // assign the first point to go (second in the list)
        SetWalkerNextPoint(id);
        return true;
    }

   	/// set the next point in the route
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

    bool WalkerManager::ExecuteEvent(WalkerInfo &info, double delta) {
        // go to the event
        WalkerRoutePoint &rp = info.route[info.currentIndex];

        // we need to add 'delta' as second parameter, and 'this' as third
        auto bound_visitor = std::bind(WalkerEventVisitor(), std::placeholders::_1, delta, *this);
        return boost::apply_visitor(bound_visitor, rp.event);
    }


} // namespace nav
} // namespace carla
