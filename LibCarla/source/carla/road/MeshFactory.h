// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include <carla/geom/Mesh.h>
#include <carla/road/Road.h>
#include <carla/road/LaneSection.h>
#include <carla/road/Lane.h>
#include <carla/rpc/OpendriveGenerationParameters.h>

namespace carla {
namespace geom {

  /// Mesh helper generator
  class MeshFactory {
  public:

    MeshFactory(rpc::OpendriveGenerationParameters params =
        rpc::OpendriveGenerationParameters());

    // =========================================================================
    // -- Map Related ----------------------------------------------------------
    // =========================================================================

    // -- Basic --

    /// Generates a mesh that defines a road
    std::unique_ptr<Mesh> Generate(const road::Road &road) const;

    /// Generates a mesh that defines a lane section
    std::unique_ptr<Mesh> Generate(const road::LaneSection &lane_section) const;

    /// Generates a mesh that defines a lane from a given s start and end
    std::unique_ptr<Mesh> Generate(
        const road::Lane &lane, const double s_start, const double s_end) const;

    /// Generates a mesh that defines the whole lane
    std::unique_ptr<Mesh> Generate(const road::Lane &lane) const;

    // -- Walls --

    /// Genrates a mesh representing a wall on the road corners to avoid
    /// cars falling down
    std::unique_ptr<Mesh> GenerateWalls(const road::LaneSection &lane_section) const;

    /// Generates a wall-like mesh at the right side of the lane
    std::unique_ptr<Mesh> GenerateRightWall(
        const road::Lane &lane, const double s_start, const double s_end) const;

    /// Generates a wall-like mesh at the left side of the lane
    std::unique_ptr<Mesh> GenerateLeftWall(
        const road::Lane &lane, const double s_start, const double s_end) const;

    // -- Chunked --

    /// Generates a list of meshes that defines a road with a maximum length
    std::vector<std::unique_ptr<Mesh>> GenerateWithMaxLen(
        const road::Road &road) const;

    /// Generates a list of meshes that defines a lane_section with a maximum length
    std::vector<std::unique_ptr<Mesh>> GenerateWithMaxLen(
        const road::LaneSection &lane_section) const;

    /// Generates a list of meshes that defines a road safety wall with a maximum length
    std::vector<std::unique_ptr<Mesh>> GenerateWallsWithMaxLen(
        const road::Road &road) const;

    /// Generates a list of meshes that defines a lane_section safety wall with a maximum length
    std::vector<std::unique_ptr<Mesh>> GenerateWallsWithMaxLen(
        const road::LaneSection &lane_section) const;

    // -- Util --

    /// Generates a chunked road with all the features needed for simulation
    std::vector<std::unique_ptr<Mesh>> GenerateAllWithMaxLen(
        const road::Road &road) const;

    std::unique_ptr<Mesh> MergeAndSmooth(std::vector<std::unique_ptr<Mesh>> &lane_meshes) const;

    // =========================================================================
    // -- Generation parameters ------------------------------------------------
    // =========================================================================

    /// Parameters for the road generation
    struct RoadParameters {
      float resolution                  =  2.0f;
      float max_road_len                = 50.0f;
      float extra_lane_width            =  1.0f;
      float wall_height                 =  0.6f;
      // Road mesh smoothness:
      float max_weight_distance         =  5.0f;
      float same_lane_weight_multiplier =  2.0f;
      float lane_ends_multiplier        =  2.0f;
    };

    RoadParameters road_param;

  };

} // namespace geom
} // namespace carla
