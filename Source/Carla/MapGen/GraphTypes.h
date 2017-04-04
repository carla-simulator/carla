// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CityMapDefinitions.h"

#include <vector>

namespace MapGen {

#ifdef CARLA_ROAD_GENERATOR_PRINT_OUT

  /// For debug only purposes.
  template <char C>
  struct DataIndex : private NonCopyable
  {

    DataIndex() : index(++NEXT_INDEX) {}

    static void ResetIndex() {
      NEXT_INDEX = 0u;
    }

    template <typename OSTREAM>
    friend OSTREAM &operator<<(OSTREAM &os, const DataIndex &d) {
      os << C << d.index;
      return os;
    }

  // private:

    uint32 index = 0u;

    static uint32 NEXT_INDEX;
  };

#  define INHERIT_GRAPH_TYPE_BASE_CLASS(c) : public DataIndex<c>
#else
#  define INHERIT_GRAPH_TYPE_BASE_CLASS(c) : private NonCopyable
#endif // CARLA_ROAD_GENERATOR_PRINT_OUT

  struct GraphNode INHERIT_GRAPH_TYPE_BASE_CLASS('n')
  {
    uint32 EdgeCount;
    bool bIsIntersection = true; // at this point every node is an intersection.
    EIntersectionType IntersectionType;
    float Rotation;
    std::vector<float> Rots;
  };

  struct GraphHalfEdge INHERIT_GRAPH_TYPE_BASE_CLASS('e')
  {
    float Angle;
  };

  struct GraphFace INHERIT_GRAPH_TYPE_BASE_CLASS('f')
  {
  };

#undef INHERIT_GRAPH_TYPE_BASE_CLASS

} // namespace MapGen
