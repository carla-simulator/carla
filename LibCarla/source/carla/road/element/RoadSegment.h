// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/geom/Location.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfo.h"
#include "carla/road/element/Types.h"

#include <limits>
#include <memory>
#include <set>
#include <vector>
#include <algorithm>

namespace carla {
namespace road {

  class MapBuilder;

namespace element {

  class RoadSegment : private NonCopyable {
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

    /// Returns single info given a type and a distance from
    /// the start of the road (negative lanes)
    template <typename T>
    std::shared_ptr<const T> GetInfo(double dist) const {
      auto up_bound = decltype(_info)::reverse_iterator(_info.upper_bound(dist));
      auto it = MakeRoadInfoIterator<T>(up_bound, _info.rend());
      return it.IsAtEnd() ? nullptr : *it;
    }

    /// Returns single info given a type and a distance from
    /// the end of the road (positive lanes)
    template <typename T>
    std::shared_ptr<const T> GetInfoReverse(double dist) const {
      auto lo_bound = _info.lower_bound(dist);
      auto it = MakeRoadInfoIterator<T>(lo_bound, _info.end());
      return it.IsAtEnd() ? nullptr : *it;
    }

    /// Returns info vector given a type and a distance from
    /// the start of the road (negative lanes)
    template <typename T>
    std::vector<std::shared_ptr<const T>> GetInfos(double dist) const {
      auto up_bound = decltype(_info)::reverse_iterator(_info.upper_bound(dist));
      auto it = MakeRoadInfoIterator<T>(up_bound, _info.rend());

      std::vector<std::shared_ptr<const T>> result;
      for (; !it.IsAtEnd(); ++it) {
        result.emplace_back(*it);
      }
      return result;
    }

    /// Returns info vector given a type and a distance from
    /// the end of the road (positive lanes)
    template <typename T>
    std::vector<std::shared_ptr<const T>> GetInfosReverse(double dist) const {
      auto lo_bound = _info.lower_bound(dist);
      auto it = MakeRoadInfoIterator<T>(lo_bound, _info.end());

      std::vector<std::shared_ptr<const T>> result;
      for (; !it.IsAtEnd(); ++it) {
        result.emplace_back(*it);
      }
      return result;
    }

    /// Workaround where we must find a specific (RoadInfoMarkRecord) RoadInfo
    /// that must have lane_id info. In this case this info is used for selecting
    /// only the nearest RoadInfos to the "dist" input.
    std::vector<std::shared_ptr<const RoadInfoMarkRecord>> GetRoadInfoMarkRecord(
        double dist) const {
      auto mark_record_info = GetInfos<RoadInfoMarkRecord>(dist);
      std::vector<std::shared_ptr<const RoadInfoMarkRecord>> result;
      std::unordered_set<int> inserted_lanes;

      for (auto &&mark_record : mark_record_info) {
        const int lane_id = mark_record->GetLaneId();
        const bool is_new_lane = inserted_lanes.insert(lane_id).second;
        if (is_new_lane) {
          result.emplace_back(mark_record);
        }
      }
      return result;
    }

    /// Workaround where we must find a specific (RoadInfoMarkRecord) RoadInfo
    /// that must have lane_id info. In this case this info is used for selecting
    /// only the nearest RoadInfos to the "dist" input. But reversed!
    std::vector<std::shared_ptr<const RoadInfoMarkRecord>> GetRoadInfoMarkRecordReverse(
        double dist) const {
      auto mark_record_info = GetInfosReverse<RoadInfoMarkRecord>(dist);
      std::vector<std::shared_ptr<const RoadInfoMarkRecord>> result;
      std::unordered_set<int> inserted_lanes;

      for (auto &&mark_record : mark_record_info) {
        const int lane_id = mark_record->GetLaneId();
        const bool is_new_lane = inserted_lanes.insert(lane_id).second;
        if (is_new_lane) {
          result.emplace_back(mark_record);
        }
      }
      return result;
    }

    /// Workaround where we must find a specific (RoadInfoLaneWidth) RoadInfo
    /// that must have lane_id info. In this case this info is used for selecting
    /// only the nearest RoadInfos to the "dist" input.
    std::vector<std::shared_ptr<const RoadInfoLaneWidth>> GetRoadInfoLaneWidth(
        double dist) const {
      auto lane_offset_info = GetInfos<RoadInfoLaneWidth>(dist);
      std::vector<std::shared_ptr<const RoadInfoLaneWidth>> result;
      std::unordered_set<int> inserted_lanes;

      for (auto &&lane_offset : lane_offset_info) {
        const int lane_id = lane_offset->GetLaneId();
        const bool is_new_lane = inserted_lanes.insert(lane_id).second;
        if (is_new_lane) {
          result.emplace_back(lane_offset);
        }
      }
      return result;
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

    /// Given the current lane it gives an std::pair vector with the lane id and road
    /// id where you can go. First integer of the pair is the lane id and the second
    /// the road id.
    ///
    /// @param current_lane_id                for which lane do you want the next
    ///
    /// OUTPUT:
    ///    std::vector<std::pair<int, int>>   return a pair with lane id (first
    ///                                       int) and the road id (second int),
    ///                                       if no lane has been found the given
    ///                                       pair it will be (0, 0) as lane id
    ///                                       zero used for the reference line
    std::vector<std::pair<int, int>> GetNextLane(int current_lane_id) const {
      std::map<int, std::vector<std::pair<int, int>>>::const_iterator it = _next_lane.find(current_lane_id);
      return it == _next_lane.end() ? std::vector<std::pair<int, int>>() : it->second;
    }

    /// Given the current lane it gives an std::pair vector with the lane id and road
    /// id where you can go. First integer of the pair is the lane id and the second
    /// the road id.
    ///
    /// @param current_lane_id                for which lane do you want the next
    ///
    /// OUTPUT:
    ///    std::vector<std::pair<int, int>>   return a pair with lane id (first
    ///                                       int) and the road id (second int),
    ///                                       if no lane has been found the given
    ///                                       pair it will be (0, 0) as lane id
    ///                                       zero used for the reference line
    std::vector<std::pair<int, int>> GetPrevLane(int current_lane_id) const {
      std::map<int, std::vector<std::pair<int, int>>>::const_iterator it = _prev_lane.find(current_lane_id);
      return it == _prev_lane.end() ? std::vector<std::pair<int, int>>() : it->second;
    }

    // Search for the last geometry with less start_offset before 'dist'
    DirectedPoint GetDirectedPointIn(double dist) const {
      assert(_length > 0.0);
      if (dist <= 0.0) {
        return DirectedPointWithElevation(
            dist, DirectedPoint(
                _geom.front()->GetStartPosition(),
                _geom.front()->GetHeading()));
      }

      if (dist >= _length) {
        return DirectedPointWithElevation(
            dist,
            _geom.back()->PosFromDist(_length - _geom.back()->GetStartOffset()));
      }

      for (auto &&g : _geom) {
        if ((g->GetStartOffset() < dist) && (dist <= (g->GetStartOffset() + g->GetLength()))) {
          return DirectedPointWithElevation(
            dist,
            g->PosFromDist(dist - g->GetStartOffset()));
        }
      }

      return DirectedPointWithElevation(
          dist,
          _geom.back()->PosFromDist(dist - _geom.back()->GetStartOffset()));
    }

    /// Returns a pair containing:
    /// - @b first:  distance to the nearest point on the center in
    ///              this road segment from the begining of it.
    /// - @b second: Euclidean distance from the nearest point in
    ///              this road segment to p.
    ///   @param loc point to calculate the distance
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

    /// Returns a the nearest lane id.
    ///   @param dist distance from the begining of the road to the point you
    ///          want to calculate the distance
    ///   @param loc point to calculate the distance
    std::pair<int, double> GetNearestLane(double dist, const geom::Location &loc) const {
      const DirectedPoint dp_center_road = GetDirectedPointIn(dist);
      auto info = GetInfo<RoadInfoLane>(0.0);

      int nearest_lane_id = 0;
      double nearest_dist = std::numeric_limits<double>::max();

      for (auto &&current_lane_id :
          info->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Both)) {
        const auto current_lane_info = info->getLane(current_lane_id);

        if (current_lane_info->_type == "driving") {
          DirectedPoint dp_center_lane = dp_center_road;
          dp_center_lane.ApplyLateralOffset(current_lane_info->_lane_center_offset);

          const double current_dist = geom::Math::Distance2D(dp_center_lane.location, loc);
          if (current_dist < nearest_dist) {
            nearest_dist = current_dist;
            nearest_lane_id = current_lane_id;
          }
        }
      }
      return std::pair<int, double>(nearest_lane_id, nearest_dist);
    }

    const double &GetLength() const {
      return _length;
    }

    void SetLength(double d) {
      _length = d;
    }

  private:

    DirectedPoint DirectedPointWithElevation(double dist, DirectedPoint dp) const {
        const auto elev_info = GetInfo<RoadElevationInfo>(dist);
        if (elev_info) {
          dp.location.z = elev_info->Evaluate(dist, &dp.pitch);
        }
        return dp;
    }

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

    friend class MapBuilder;

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
