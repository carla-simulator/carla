// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/LaneSection.h"

#include <map>
#include <unordered_map>

namespace carla {
namespace road {

  class LaneSectionMap
    : private std::multimap<double, LaneSection>,
      private MovableNonCopyable {
    using Super = std::multimap<double, LaneSection>;
  public:

    LaneSection &Emplace(SectionId id, double s) {
      LaneSection &result = Super::emplace(s, LaneSection{id, s})->second;
      _by_id.emplace(result.GetId(), &result);
      return result;
    }

    LaneSection &GetById(SectionId id) {
      return *_by_id.at(id);
    }

    const LaneSection &GetById(SectionId id) const {
      return *_by_id.at(id);
    }

    using Super::find;
    using Super::upper_bound;
    using Super::lower_bound;

    using Super::begin;
    using Super::rbegin;
    using Super::end;
    using Super::rend;

  private:

    std::unordered_map<SectionId, LaneSection *> _by_id;
  };

} // road
} // carla
