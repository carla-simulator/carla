// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GraphTypes.h"

#include <vector>

namespace MapGen {

  class CARLA_API RoadSegmentDescription : private NonCopyable
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
