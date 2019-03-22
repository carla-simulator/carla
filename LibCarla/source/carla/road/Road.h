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

    const Lane *GetLane(const LaneId id, const float s) const;

    Lane *GetNextLane(const float s, const LaneId lane_id);

    Lane *GetPrevLane(const float s, const LaneId lane_id);

    // get the start and end section with a lan id
    LaneSection *GetStartSection(LaneId id);
    LaneSection *GetEndSection(LaneId id);

    std::vector<const Road *> GetNexts() const;

    std::vector<const Road *> GetPrevs() const;

    const geom::CubicPolynomial &GetElevationOn(const float s) const;

    carla::road::signal::Signal* GetSignal(const SignId id);

    carla::road::signal::SignalReference* GetSignalRef(const SignRefId id);

    std::unordered_map<SignId, signal::Signal>* getSignals();

    std::unordered_map<SignId, signal::SignalReference>* getSignalReferences();

    /// Returns a directed point on the center of the road (lane 0),
    /// with the corresponding laneOffset and elevation records applied,
    /// on distance "s".
    /// - @ param s distance regarding the road to compute the point
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

  private:

    template <typename MultiMapT>
    static auto GetLessEqualRange(MultiMapT &map, float s) {
      if (map.find(s) == map.end()) {
        auto it = map.lower_bound(s);
        if (it == map.begin()) {
          return std::make_pair(map.end(), map.end());
        }
        s = (--it)->first;
      }
      return std::make_pair(map.lower_bound(s), map.upper_bound(s));
    }

  public:

    auto GetLaneSectionsAt(const float s) {
      auto pair = GetLessEqualRange(_lane_sections, s);
      return MakeListView(
          iterator::make_map_values_iterator(pair.first),
          iterator::make_map_values_iterator(pair.second));
    }

    auto GetLaneSectionsAt(const float s) const {
      auto pair = GetLessEqualRange(_lane_sections, s);
      return MakeListView(
          iterator::make_map_values_const_iterator(pair.first),
          iterator::make_map_values_const_iterator(pair.second));
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
