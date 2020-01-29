// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/element/RoadObject.h"
#include "carla/road/element/RoadObjectIterator.h"

#include <vector>
#include <memory>

namespace carla {
namespace road {

  class ObjectSet : private MovableNonCopyable {
  public:

    ObjectSet() = default;

    ObjectSet(std::vector<std::unique_ptr<element::RoadObject>> &&vec)
      : _road_set(std::move(vec)) {}

    /// Return all objects given a type from the start of the road
    template <typename T>
    std::vector<const T *> GetObjects() const {
      std::vector<const T *> vec;
      auto it = element::MakeRoadObjectIterator<T>(_road_set.GetAll());
      for (; !it.IsAtEnd(); ++it) {
        vec.emplace_back(&*it);
      }
      return vec;
    }

    /// Returns single object given a type and a distance (s) from
    /// the start of the road
    template <typename T>
    const T *GetObject(const double s) const {
      auto it = element::MakeRoadObjectIterator<T>(_road_set.GetReverseSubset(s));
      return it.IsAtEnd() ? nullptr : &*it;
    }

  private:

    RoadElementSet<std::unique_ptr<element::RoadObject>> _road_set;
  };

} // road
} // carla
