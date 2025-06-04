// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GraphTypes.h"
#include "Carla/Util/NonCopyable.h"

#include <vector>

namespace MapGen {

  class RoadSegmentDescription : private NonCopyable
  {
  public:

    void Add(const GraphHalfEdge &Edge) {
      if (Angle == nullptr) {
        Angle = MakeUnique<float>(Edge.Angle);
      } else if (*Angle != Edge.Angle) { /// @todo Use a scale.
        Angle = nullptr;
      }
      _vect.emplace_back(&Edge);
    }

    const GraphHalfEdge &operator[](size_t i) const {
      return *_vect[i];
    }

    size_t Size() const {
      return _vect.size();
    }

    bool IsStraight() const {
      return Angle.IsValid();
    }

    /// @return nullptr if the road segment is not straight.
    const float *GetAngle() const {
      return Angle.Get();
    }

  private:

    TUniquePtr<float> Angle = nullptr;

    std::vector<const GraphHalfEdge *> _vect;
  };

} // namespace MapGen
