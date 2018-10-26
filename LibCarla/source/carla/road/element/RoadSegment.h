// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/road/element/RoadInfo.h"
#include "carla/road/element/Types.h"

#include <limits>
#include <memory>
#include <set>
#include <vector>

namespace carla {
namespace road {

  class MapBuilder;

namespace element {

  class RoadSegment {
  public:

    RoadSegment(id_type id) : _id(id) {}

    RoadSegment(RoadSegmentDefinition &&def)
      : _id(def.GetId()),
        _successors_is_start(std::move(def._successor_is_start)),
        _predecessors_is_start(std::move(def._predecessors_is_start)),
        _geom(std::move(def._geom)) {
      for (auto &&a : def._info) {
        _info.insert(std::move(a));
      }
    }

    id_type GetId() const {
      return _id;
    }

    // returns single info given a type and a distance
    template <typename T>
    const T *GetInfo(double dist) const {
      auto up_bound = decltype(_info)::reverse_iterator(_info.upper_bound(dist));
      auto it = MakeRoadInfoIterator<T>(up_bound, _info.rend());
      return it.IsAtEnd() ? nullptr : *it;
    }

    // returns single info given a type and a distance
    template <typename T>
    const T *GetInfoReverse(double dist) const {
      auto lo_bound = _info.lower_bound(dist);
      auto it = MakeRoadInfoIterator<T>(lo_bound, _info.end());
      return it.IsAtEnd() ? nullptr : *it;
    }

    // returns info vector given a type and a distance
    std::vector<std::shared_ptr<const RoadInfo>> GetInfos(double dist) const {
      // @todo
      (void)dist;
      return std::vector<std::shared_ptr<const RoadInfo>>();
    }

    void PredEmplaceBack(RoadSegment *s) {
      _predecessors.emplace_back(s);
    }

    void SuccEmplaceBack(RoadSegment *s) {
      _successors.emplace_back(s);
    }

    bool HaveSuccessors() const {
      return _successors.size() > 0;
    }

    bool HavePredecessors() const {
      return _predecessors.size() > 0;
    }

    std::vector<id_type> GetSuccessorsIds() const {
      std::vector<id_type> s_vec;
      for (auto &&succ : _successors) {
        s_vec.emplace_back(succ->GetId());
      }
      return s_vec;
    }

    std::vector<bool> GetSuccessorsIsSTart() const {
      return _successors_is_start;
    }

    std::vector<id_type> GetPredecessorsIds() const {
      std::vector<id_type> p_vec;
      for (auto &&pred : _predecessors) {
        p_vec.emplace_back(pred->GetId());
      }
      return p_vec;
    }

    std::vector<bool> GetPredecessorsIsStart() const {
      return _predecessors_is_start;
    }

    const std::vector<RoadSegment *> GetSuccessors() const {
      return _successors;
    }

    const std::vector<RoadSegment *> GetPredecessors() const {
      return _predecessors;
    }

    // Search for the last geometry with less start_offset before 'dist'
    DirectedPoint GetDirectedPointIn(double dist) const {
      assert(_length > 0.0);
      if (dist <= 0.0) {
        return DirectedPoint(_geom.front()->GetStartPosition(),
            _geom.front()->GetHeading());
      }
      // if dist is bigguer than len
      if (dist > _length) {
        return _geom.back()->PosFromDist(
            _length - _geom.back()->GetStartOffset());
      }
      for (auto &&g : _geom) {
        if ((g->GetStartOffset() < dist) &&
            (dist <= g->GetStartOffset() + g->GetLength())) {
          return g->PosFromDist(dist - g->GetStartOffset());
        }
      }
      return DirectedPoint::Invalid();
    }

    std::pair<double, double> GetNearestPoint(const geom::Location &loc) const {
      double min = std::numeric_limits<double>::max();
      std::pair<double, double> last = {0.0, 0.0};

      decltype(_geom)::const_iterator nearest_geom;

      for (auto g = _geom.begin(); g !=_geom.end(); ++g) {
        auto d = (*g)->DistanceTo(loc);
        if (d.second < min) {
          last = d;
          min = d.second;
          nearest_geom = g;
        }
      }

      for (auto g = _geom.begin(); g != nearest_geom; ++g) {
        last.first += (*g)->GetLength();
      }

      return last;
    }

    int GetNearestLane(double, const geom::Location &) const {
      /*
      DirectedPoint dp = GetDirectedPointIn(dist);

      double dist_dp_loc = geom::Math::Distance2D(dp.location, loc);

      const RoadInfoLane *road_info_lane = GetInfo<RoadInfoLane>(dist);
      for (auto &&lane_id : road_info_lane->getLanesIDs()) {
        const LaneInfo *info = road_info_lane->getLane(lane_id);
        // search for info width
      }
      */
      return 0;
    }

    const double &GetLength() const {
      return _length;
    }

    void SetLength(double d) {
      _length = d;
    }

  private:

    friend class carla::road::MapBuilder;

    struct LessComp {
      using is_transparent = void;
      bool operator()(
          const std::shared_ptr<RoadInfo> &a,
          const std::shared_ptr<RoadInfo> &b) const {
        return a->d < b->d;
      }
      bool operator()(
          const double &a,
          const std::shared_ptr<RoadInfo> &b) const {
        return a < b->d;
      }
      bool operator()(
          const std::shared_ptr<RoadInfo> &a,
          const double &b) const {
        return a->d < b;
      }
    };

  private:
    id_type _id;
    std::vector<RoadSegment *> _predecessors;
    std::vector<RoadSegment *> _successors;
    std::vector<bool> _successors_is_start;
    std::vector<bool> _predecessors_is_start;
    std::vector<std::unique_ptr<Geometry>> _geom;
    std::multiset<std::shared_ptr<RoadInfo>, LessComp> _info;
    double _length = -1.0;
  };

} // namespace element
} // namespace road
} // namespace carla
