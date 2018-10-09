// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"

#include <cstdio>

namespace carla {
namespace road {

using id_type = size_t;

// Geometry ////////////////////////////////////////////////////////////

enum class GeometryType : unsigned int {
  LINE,
  ARC,
  SPIRAL
};

struct Geometry {
  GeometryType type;   // geometry type
  double length = 1.0; // length of the road section [meters]

  double start_position_offset = 0.0; // s-offset [meters]
  double heading = 0.0;               // start orientation [radians]

  geom::Location start_position; // [meters]

protected:
  Geometry(GeometryType type) : type(type) {}
};

struct GeometryLine : public Geometry {
  GeometryLine() : Geometry(GeometryType::LINE) {}
};

struct GeometryArc : public Geometry {
  double curvature = 0.0;
  GeometryArc() : Geometry(GeometryType::ARC) {}
};

struct GeometrySpiral : public Geometry {
  double curve_start = 0.0;
  double curve_end = 0.0;
  GeometrySpiral() : Geometry(GeometryType::SPIRAL) {}
};

// Roads //////////////////////////////////////////////////////////////

struct RoadInfo {
  // distance from Road's start location
  double d = 0; // [meters]
};

struct SpeedLimit : public RoadInfo {
  double value = 0; // [meters/second]
};

struct RoadSegmentDefinition {
  id_type id = -1;
  std::vector<id_type> predecessor_id;
  std::vector<id_type> successor_id;
  std::vector<Geometry> geom;
  std::multimap<double, RoadInfo> info;
};

} // namespace road
} // namespace carla