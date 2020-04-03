// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include <carla/geom/Mesh.h>
#include <carla/road/Road.h>
#include <carla/road/LaneSection.h>
#include <carla/road/Lane.h>

namespace carla {
namespace geom {

  /// Mesh helper generator static
  class MeshFactory {
  public:

    MeshFactory() = default;

    // =========================================================================
    // -- Map Related ----------------------------------------------------------
    // =========================================================================

    /// Generates a mesh that defines a road
    std::unique_ptr<Mesh> Generate(const road::Road &road) const;

    /// Generates a mesh that defines a lane section
    std::unique_ptr<Mesh> Generate(const road::LaneSection &lane_section) const;

    /// Generates a mesh that defines a lane
    std::unique_ptr<Mesh> Generate(const road::Lane &lane) const;

    // =========================================================================
    // -- Generation parameters ------------------------------------------------
    // =========================================================================

    /// Generates a mesh that defines a lane
    struct RoadParameters {
      float resolution       = 2.0f;
      float extra_lane_width = 1.0f;
      float wall_height      = 0.6f;
    };

    RoadParameters road_param;

  };

} // namespace geom
} // namespace carla
