// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Road.h"
#include "carla/road/Lane.h"
#include "carla/road/MapData.h"
#include "carla/Logging.h"
#include "carla/ListView.h"

#include <boost/optional.hpp>

namespace carla {
namespace road {

  const MapData *Road::GetMap() const {
    return _map_data;
  }

  RoadId Road::GetId() const {
    return _id;
  }

  std::string Road::GetName() const {
    return _name;
  }

  float Road::GetLength() const {
    return _length;
  }

  bool Road::IsJunction() const {
    return _is_junction;
  }

  JuncId Road::GetJunction() const {
    return _junction_id;
  }

  std::vector<const Road *> Road::GetNexts() const {
    std::vector<const Road *> vec;
    for (auto &&next : _nexts) {
      vec.emplace_back(_map_data->GetRoad(next));
    }
    return vec;
  }

  std::vector<const Road *> Road::GetPrevs() const {
    std::vector<const Road *> vec;
    for (auto &&prev : _prevs) {
      vec.emplace_back(_map_data->GetRoad(prev));
    }
    return vec;
  }

  Lane *Road::GetLane(const LaneId id, const float s) {
    // Get a reversed list of elements that have key
    // value GetDistance() <= s
    auto sections = MakeListView(
        std::make_reverse_iterator(_lane_sections.upper_bound(s)),
        _lane_sections.rend());

    auto validate = [&sections](auto &&it) {
      return
          it != sections.end() &&
          it->second.GetDistance() == sections.begin()->second.GetDistance();
    };

    for (auto i = sections.begin(); validate(i); ++i) {
      auto search = i->second.GetLanes().find(id);
      if (search != i->second.GetLanes().end()) {
        return &search->second;
      }
    }

    log_warning("id", id, "at distance", s, "not found in road", _id);
    return nullptr;
  }

  // get the lane on a section next to 's'
  Lane *Road::GetNextLane(const float s, const LaneId lane_id) {

    auto upper = _lane_sections.upper_bound(s);

    while (upper != _lane_sections.end()) {
      // check id
      Lane *ptr = upper->second.GetLane(lane_id);
      if (ptr != nullptr)
        return ptr;
      ++upper;
    }

    return nullptr;
  }

  // get the lane on a section previous to 's'
  Lane *Road::GetPrevLane(const float s, const LaneId lane_id) {

    auto lower = _lane_sections.lower_bound(s);
    auto rlower = std::make_reverse_iterator(lower);

    while (rlower != _lane_sections.rend()) {
      // check id
      Lane *ptr = rlower->second.GetLane(lane_id);
      if (ptr != nullptr)
        return ptr;
      ++rlower;
    }

    return nullptr;
  }

  // get the start and end section with a lan id
  LaneSection *Road::GetStartSection(LaneId id) {
    auto it = _lane_sections.begin();
    while (it != _lane_sections.end()) {
      // check id
      Lane *ptr = it->second.GetLane(id);
      if (ptr != nullptr)
        return &(it->second);
      ++it;
    }

    return nullptr;
  }


  LaneSection *Road::GetEndSection(LaneId id) {
    auto it = _lane_sections.rbegin();
    while (it != _lane_sections.rend()) {
      // check id
      Lane *ptr = it->second.GetLane(id);
      if (ptr != nullptr)
        return &(it->second);
      ++it;
    }

    return nullptr;
  }

    //carla::road::signal::Signal* Road::GetSignal(const SignId /*id*/) const {
      /// @todo: Fix this so it can return an specific Signal
      /*const auto search = _signals.find(id);
      if (search != _signals.end()) {
        return &search->second;
      }*/
    /*  return nullptr;
    }

    carla::road::signal::SignalReference* Road::GetSignalRef(const SignRefId id) const {*/
      /// @todo: Fix this so it can return an specific SignalReference
      /*const auto search = _sign_ref.find(id);
      if (search != _sign_ref.end()) {
        return &search->second;
      }*/
    //  return nullptr;
    //}

    //std::unordered_map<SignId, signal::Signal> Road::getSignals() const {
    //  return _signals;
    //}

    //std::unordered_map<SignId, signal::SignalReference> Road::getSignalReferences() const {
    //  return _sign_ref;
    //}

} // road
} // carla
