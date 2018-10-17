// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/Types.h"
#include "carla/road/element/RoadInfo.h"
#include "carla/geom/Location.h"

#include <set>
#include <memory>
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
        _geom(std::move(def._geom)) {
      for (auto &&a : def._info) {
        _info.insert(std::move(a));
      }
    }

    id_type GetId() const {
      return _id;
    }

    template <typename T>
    const std::vector<const T *> GetInfo() const {
      std::vector<const T *> vec;
      for (auto &&info : _info) {
        auto *t = dynamic_cast<const T *>(info.get());
        if (t != nullptr) {
          vec.emplace_back(t);
        }
      }
      return vec;
    }

    // returns single info given a type and a distance
    template <typename T>
    const T *GetInfo(double dist) const {
      if (dist < 0) {
        return nullptr;
      }
      auto up_bound = _info.upper_bound(
          std::make_unique<RoadInfo>(dist));
      T *t_last = nullptr;
      for (auto i = _info.begin(); i != up_bound; ++i) {
        T *t = dynamic_cast<T *>(i->get());
        if (t != nullptr) {
          t_last = t;
        }
      }
      return t_last;
    }

    // returns info vector given a type and a distance
    template <typename T>
    std::vector<T> GetInfos(double dist) const;

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

    std::vector<id_type> GetPredecessorsIds() const {
      std::vector<id_type> p_vec;
      for (auto &&pred : _predecessors) {
        p_vec.emplace_back(pred->GetId());
      }
      return p_vec;
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
          const std::unique_ptr<RoadInfo> &a,
          const std::unique_ptr<RoadInfo> &b) const {
        return a->d < b->d;
      }
    };

    // friend class Map;

    id_type _id;
    std::vector<RoadSegment *> _predecessors;
    std::vector<RoadSegment *> _successors;
    std::vector<std::unique_ptr<Geometry>> _geom;
    std::multiset<std::unique_ptr<RoadInfo>, LessComp> _info;
    double _length = -1.0;
  };

} // namespace element
} // namespace road
} // namespace carla
