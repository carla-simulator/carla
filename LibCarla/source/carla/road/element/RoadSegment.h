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
        _geom(std::move(def._geom)),
        _next_lane(std::move(def._next_lane)),
        _prev_lane(std::move(def._prev_lane)) {
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

    // Given the current lane it gives an std::pair of the lane id the and road id
    // where you can go.  First integer of the pair is the lane id and the second the road id.
    //
    // INPUT:
    //    int current_lane_id               for which lane do you want the next
    //
    // OUTPUT:
    //    std::vector<std::pair<int, int>>  return a pair with lane id (first int) and the road id (second int),
    //                                      if no lane has been found the given pair it will be (0, 0) as lane id
    //                                      zero used for the reference line
    std::vector<std::pair<int, int>> GetNextLane(int current_lane_id) const {
      std::map<int, std::vector<std::pair<int, int>>>::const_iterator it = _next_lane.find(current_lane_id);
      return it == _next_lane.end() ? std::vector<std::pair<int, int>>() : it->second;
    }

    // Given the current lane it gives an std::pair vector with the lane id the and road id
    // where you can go. First integer of the pair is the lane id and the second the road id.
    //
    // INPUT:
    //    int current_lane_id               for which lane do you want the next
    //
    // OUTPUT:
    //    std::vector<std::pair<int, int>>  return a pair with lane id (first int) and the road id (second int),
    //                                      if no lane has been found the given pair it will be (0, 0) as lane id
    //                                      zero used for the reference line
    std::vector<std::pair<int, int>> GetPrevLane(int current_lane_id) const {
      std::map<int, std::vector<std::pair<int, int>>>::const_iterator it = _prev_lane.find(current_lane_id);
      return it == _next_lane.end() ? std::vector<std::pair<int, int>>() : it->second;
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
      decltype(_geom)::const_iterator nearest_geom;
      std::pair<double, double> last = {0.0, std::numeric_limits<double>::max()};

      for (auto g = _geom.begin(); g != _geom.end(); ++g) {
        auto d = (*g)->DistanceTo(loc);
        if (d.second < last.second) {
          last = d;
          nearest_geom = g;
        }
      }

      for (auto g = _geom.begin(); g != nearest_geom; ++g) {
        last.first += (*g)->GetLength();
      }

      return last;
    }

    std::pair<int, geom::Location> GetNearestLane(double dist, const geom::Location &loc) const {
      // Because Unreal's coordinates
      const geom::Location corrected_loc = geom::Location(loc.x, -loc.y, loc.z);

      const DirectedPoint dp_center_road = GetDirectedPointIn(dist);
      auto info = GetInfo<RoadInfoLane>(0.0);
      auto lane_offsets_vec = GetInfo<RoadGeneralInfo>(0.0)->GetLanesOffset();
      double lane_offset = 0.0;

      // Get lane offset for the current dist
      for (auto &&i : lane_offsets_vec) {
        if (i.first <= dist) {
          lane_offset = i.second;
        } else {
          break;
        }
      }

      int nearest_lane_id = 0;
      geom::Location nearest_loc;
      double nearest_dist = std::numeric_limits<double>::max();
      double current_width = lane_offset;

      // Left lanes
      for (auto &&current_lane_id :
          info->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left)) {
        DirectedPoint dp_center_lane = dp_center_road;
        const double half_width = info->getLane(current_lane_id)->_width * 0.5;

        current_width += half_width;
        if (info->getLane(current_lane_id)->_type == "driving") {
          dp_center_lane.ApplyLateralOffset(current_width);
          const double current_dist = geom::Math::Distance2D(dp_center_lane.location, corrected_loc);
          if (current_dist < nearest_dist) {
            nearest_dist = current_dist;
            nearest_lane_id = current_lane_id;
            nearest_loc = dp_center_lane.location;
          }
        }
        current_width += half_width;
      }

      current_width = lane_offset;

      // Right lanes
      for (auto &&current_lane_id :
          info->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right)) {
        DirectedPoint dp_center_lane = dp_center_road;
        const double half_width = info->getLane(current_lane_id)->_width * 0.5;

        current_width -= half_width;
        if (info->getLane(current_lane_id)->_type == "driving") {
          dp_center_lane.ApplyLateralOffset(current_width);
          const double current_dist = geom::Math::Distance2D(dp_center_lane.location, corrected_loc);
          if (current_dist < nearest_dist) {
            nearest_dist = current_dist;
            nearest_lane_id = current_lane_id;
            nearest_loc = dp_center_lane.location;
          }
        }
        current_width -= half_width;
      }

      return std::make_pair(nearest_lane_id, nearest_loc);
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

    // first  int     current lane
    // second int     to which lane
    // third  int     to which road
    std::map<int, std::vector<std::pair<int, int>>> _next_lane;
    std::map<int, std::vector<std::pair<int, int>>> _prev_lane;
  };

} // namespace element
} // namespace road
} // namespace carla
