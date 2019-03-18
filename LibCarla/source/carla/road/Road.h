// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/InformationSet.h"
#include "carla/road/Junction.h"
#include "carla/road/LaneSection.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/signal/Signal.h"
#include "carla/road/signal/SignalReference.h"

#include <vector>
#include <map>
#include <memory>

namespace carla {
namespace road {

  class MapData;
  class Elevation;
  class MapBuilder;

  class Road : private MovableNonCopyable {
  public:

    Road() = default;
    /// @todo: remove the move constructors
    Road(Road&&) = default;
    Road &operator=(Road&&) = default;

    const MapData *GetMap() const;

    RoadId GetId() const;

    std::string GetName() const;

    float GetLength() const;

    bool IsJunction() const;

    JuncId GetJunction() const;

    Lane *GetLane(const LaneId id, const float s);

    std::vector<const Road *> GetNexts() const;

    std::vector<const Road *> GetPrevs() const;

    carla::road::signal::Signal* GetSignal(const SignId id);

    carla::road::signal::SignalReference* GetSignalRef(const SignRefId id);

    std::unordered_map<SignId, signal::Signal>* getSignals();

    std::unordered_map<SignId, signal::SignalReference>* getSignalReferences();

  private:

    friend MapBuilder;

    MapData *_map_data { nullptr };

    RoadId _id { 0 };

    std::string _name;

    float _length { 0.0f };

    bool _is_junction { false };

    JuncId _junction_id { -1 };

    std::multimap<float, LaneSection> _lane_sections;

    InformationSet _info;

    std::vector<RoadId> _nexts;

    std::vector<RoadId> _prevs;

    std::unordered_map<SignId, signal::Signal> _signals;

    std::unordered_map<SignRefId, signal::SignalReference> _sign_ref;

  };

} // road
} // carla
