// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CityAreaDescription.h"
#include "GraphTypes.h"
#include "RoadSegmentDescription.h"

#include <vector>

namespace MapGen {

  class DoublyConnectedEdgeList;

  class GraphParser : private NonCopyable
  {
  public:

    explicit GraphParser(DoublyConnectedEdgeList &Dcel);

    bool HasRoadSegments() const {
      return !RoadSegments.empty();
    }

    bool HasCityAreas() const {
      return !CityAreas.empty();
    }

    size_t RoadSegmentCount() const {
      return RoadSegments.size();
    }

    size_t CityAreaCount() const {
      return CityAreas.size();
    }

    const RoadSegmentDescription &GetRoadSegmentAt(size_t i) const {
      return *RoadSegments[i];
    }

    const CityAreaDescription &GetCityAreaAt(size_t i) const {
      return *CityAreas[i];
    }

    TUniquePtr<RoadSegmentDescription> PopRoadSegment() {
      TUniquePtr<RoadSegmentDescription> ptr{RoadSegments.back().Release()};
      RoadSegments.pop_back();
      return ptr;
    }

    TUniquePtr<CityAreaDescription> PopCityArea() {
      TUniquePtr<CityAreaDescription> ptr{CityAreas.back().Release()};
      CityAreas.pop_back();
      return ptr;
    }

  private:

    using RoadSegmentList = std::vector<TUniquePtr<RoadSegmentDescription>>;

    using CityAreaList = std::vector<TUniquePtr<CityAreaDescription>>;

    RoadSegmentList RoadSegments;

    CityAreaList CityAreas;
  };

} // namespace MapGen
