// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Iterator.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/road/InformationSet.h"
#include "carla/road/Junction.h"
#include "carla/road/LaneSection.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/element/RoadInfo.h"
#include "carla/road/signal/Signal.h"
#include "carla/road/signal/SignalReference.h"
#include "carla/road/element/Geometry.h"

#include <vector>
#include <map>

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

    element::DirectedPoint GetDirectedPointIn (const float s) const;

    /// Returns a pair containing:
    /// - @b first:  distance to the nearest point on the center in
    ///              this road segment from the begining of it (s).
    /// - @b second: Euclidean distance from the nearest point in
    ///              this road segment to p.
    ///   @param loc point to calculate the distance
    const std::pair<float, float> GetNearestPoint(
        const geom::Location &loc) const;

    /// Returns a pointer to the nearest lane, given s relative to Road and
    /// a location
    ///   @param dist distance from the begining of the road to the point you
    ///          want to calculate the distance
    ///   @param loc point to calculate the distance
    const std::pair<const Lane *, float> GetNearestLane(
        const float s, const geom::Location &loc) const;

    template <typename T>
    const T *GetInfo (const float s) const {
      return _info.GetInfo<T>(s);
    }

    auto GetLaneSections() const {
      return MakeListView(
          iterator::make_map_values_const_iterator(_lane_sections.begin()),
          iterator::make_map_values_const_iterator(_lane_sections.end()));
    }

    auto GetLaneSectionsAt(const float s) {
      return MakeListView(
          iterator::make_map_values_iterator(_lane_sections.lower_bound(s)),
          iterator::make_map_values_iterator(_lane_sections.upper_bound(s)));
    }

    auto GetLaneSectionsAt(const float s) const {
      return MakeListView(
          iterator::make_map_values_const_iterator(_lane_sections.lower_bound(s)),
          iterator::make_map_values_const_iterator(_lane_sections.upper_bound(s)));
    }

    /// Get all lanes at a given s
    std::map<LaneId, const Lane *> GetLanesAt(const float s) const;

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
