// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/MeshFactory.h>

#include <vector>

#include <carla/geom/Vector3D.h>

namespace carla {
namespace geom {

  /// We use this epsilon to shift the waypoints away from the edges of the lane
  /// sections to avoid floating point precision errors.
  static constexpr double EPSILON = 10.0 * std::numeric_limits<double>::epsilon();

  std::unique_ptr<Mesh> MeshFactory::Generate(const road::Road &road) const {
    Mesh out_mesh;
    for (auto &&lane_section : road.GetLaneSections()) {
      out_mesh += *Generate(lane_section);
    }
    return std::make_unique<Mesh>(out_mesh);
  }

  std::unique_ptr<Mesh> MeshFactory::Generate(const road::LaneSection &lane_section) const {
    Mesh out_mesh;
    for (auto &&lane_pair : lane_section.GetLanes()) {
      out_mesh += *Generate(lane_pair.second);
    }
    return std::make_unique<Mesh>(out_mesh);
  }

  std::unique_ptr<Mesh> MeshFactory::Generate(const road::Lane &lane) const {
    const double s_start = lane.GetDistance() + EPSILON;
    const double s_end = lane.GetDistance() + lane.GetLength() - EPSILON;
    return Generate(lane, s_start, s_end);
  }

  std::unique_ptr<Mesh> MeshFactory::Generate(
      const road::Lane &lane, const double s_start, const double s_end) const {
    RELEASE_ASSERT(road_param.resolution > 0.0);
    DEBUG_ASSERT(s_start >= 0.0);
    DEBUG_ASSERT(s_end <= lane.GetDistance() + lane.GetLength());
    DEBUG_ASSERT(s_end >= EPSILON);
    DEBUG_ASSERT(s_start < s_end);
    // The lane with lane_id 0 have no physical representation in OpenDRIVE
    Mesh out_mesh;
    if (lane.GetId() == 0) {
      return std::make_unique<Mesh>(out_mesh);
    }
    double s_current = s_start;

    std::vector<geom::Vector3D> vertices;
    if (lane.IsStraight()) {
      // Mesh optimization: If the lane is straight just add vertices at the
      // begining and at the end of it

      const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
      vertices.push_back(edges.first);
      vertices.push_back(edges.second);
    } else {
      // Iterate over the lane's 's' and store the vertices based on it's width
      do {
        // Get the location of the edges of the current lane at the current waypoint
        const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
        vertices.push_back(edges.first);
        vertices.push_back(edges.second);

        // Update the current waypoint's "s"
        s_current += road_param.resolution;
      } while(s_current < s_end);
    }

    // This ensures the mesh is constant and have no gaps between roads,
    // adding geometry at the very end of the lane
    if (s_end - (s_current - road_param.resolution) > EPSILON) {
      const auto edges = lane.GetCornerPositions(s_end, road_param.extra_lane_width);
      vertices.push_back(edges.first);
      vertices.push_back(edges.second);
    }

    // Add the adient material, create the strip and close the material
    out_mesh.AddMaterial(
        lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road");
    out_mesh.AddTriangleStrip(vertices);
    out_mesh.EndMaterial();
    return std::make_unique<Mesh>(out_mesh);
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWithMaxLen(
      const road::Road &road, const double max_len) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;
    for (auto &&lane_section : road.GetLaneSections()) {
      auto section_uptr_list = GenerateWithMaxLen(lane_section, max_len);
      mesh_uptr_list.insert(
          mesh_uptr_list.end(),
          std::make_move_iterator(section_uptr_list.begin()),
          std::make_move_iterator(section_uptr_list.end()));
    }
    return mesh_uptr_list;
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWithMaxLen(
      const road::LaneSection &lane_section, const double max_len) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;
    if (lane_section.GetLength() < max_len) {
      mesh_uptr_list.emplace_back(Generate(lane_section));
    } else {
      double s_current = lane_section.GetDistance() + EPSILON;
      const double s_end = lane_section.GetDistance() + lane_section.GetLength() - EPSILON;
      while(s_current + max_len < s_end) {
        const auto s_until = s_current + max_len;
        Mesh lane_section_mesh;
        for (auto &&lane_pair : lane_section.GetLanes()) {
          lane_section_mesh += *Generate(lane_pair.second, s_current, s_until);
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));
        s_current = s_until;
      }
      if (s_end - s_current > EPSILON) {
        for (auto &&lane_pair : lane_section.GetLanes()) {
          mesh_uptr_list.emplace_back(Generate(lane_pair.second, s_current, s_end));
        }
      }
    }
    return mesh_uptr_list;
  }

} // namespace geom
} // namespace carla
