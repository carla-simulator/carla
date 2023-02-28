// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/element/RoadInfo.h"
#include "carla/road/element/RoadInfoIterator.h"

#include <vector>
#include <memory>

namespace carla {
namespace road {

  class InformationSet : private MovableNonCopyable {
  public:

    InformationSet() = default;

    InformationSet(std::vector<std::unique_ptr<element::RoadInfo>> &&vec)
      : _road_set(std::move(vec)) {}

    /// Return all infos given a type from the start of the road
    template <typename T>
    std::vector<const T *> GetInfos() const {
      std::vector<const T *> vec;
      auto it = element::MakeRoadInfoIterator<T>(_road_set.GetAll());
      for (; !it.IsAtEnd(); ++it) {
        vec.emplace_back(&*it);
      }
      return vec;
    }

    /// Returns single info given a type and a distance (s) from
    /// the start of the road
    template <typename T>
    const T *GetInfo(const double s) const {
      auto it = element::MakeRoadInfoIterator<T>(_road_set.GetReverseSubset(s));
      return it.IsAtEnd() ? nullptr : &*it;
    }

    /// Return all infos given a type in a given range of the road
    template <typename T>
    std::vector<const T *> GetInfos(const double min_s, const double max_s) const {
      std::vector<const T *> vec;
      if(min_s < max_s) {
        auto it = element::MakeRoadInfoIterator<T>(
            _road_set.GetSubsetInRange(min_s, max_s)); //reverse
        for (; !it.IsAtEnd(); ++it) {
          vec.emplace_back(&*it);
        }
      } else {
        auto it = element::MakeRoadInfoIterator<T>(
            _road_set.GetReverseSubsetInRange(max_s, min_s)); //reverse
        for (; !it.IsAtEnd(); ++it) {
          vec.emplace_back(&*it);
        }
      }
      return vec;
    }

  private:

    RoadElementSet<std::unique_ptr<element::RoadInfo>> _road_set;
  };

} // road
} // carla
