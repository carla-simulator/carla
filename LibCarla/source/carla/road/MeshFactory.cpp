// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/road/MeshFactory.h>

#include <vector>

#include <carla/geom/Vector3D.h>
#include <carla/geom/Rtree.h>

namespace carla {
namespace geom {

  MeshFactory::MeshFactory(rpc::OpendriveGenerationParameters params) {
    road_param.resolution = static_cast<float>(params.vertex_distance);
    road_param.max_road_len = static_cast<float>(params.max_road_length);
    road_param.extra_lane_width = static_cast<float>(params.additional_width);
    road_param.wall_height = static_cast<float>(params.wall_height);
  }

  /// We use this epsilon to shift the waypoints away from the edges of the lane
  /// sections to avoid floating point precision errors.
  static constexpr double EPSILON = 10.0 * std::numeric_limits<double>::epsilon();
  static constexpr double MESH_EPSILON = 50.0 * std::numeric_limits<double>::epsilon();

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
      const auto edges = lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width);
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

  std::unique_ptr<Mesh> MeshFactory::GenerateWalls(const road::LaneSection &lane_section) const {
    Mesh out_mesh;

    const auto min_lane = lane_section.GetLanes().begin()->first == 0 ?
        1 : lane_section.GetLanes().begin()->first;
    const auto max_lane = lane_section.GetLanes().rbegin()->first == 0 ?
        -1 : lane_section.GetLanes().rbegin()->first;

    for (auto &&lane_pair : lane_section.GetLanes()) {
      const auto &lane = lane_pair.second;
      const double s_start = lane.GetDistance() + EPSILON;
      const double s_end = lane.GetDistance() + lane.GetLength() - EPSILON;
      if (lane.GetId() == max_lane) {
        out_mesh += *GenerateLeftWall(lane, s_start, s_end);
      }
      if (lane.GetId() == min_lane) {
        out_mesh += *GenerateRightWall(lane, s_start, s_end);
      }
    }
    return std::make_unique<Mesh>(out_mesh);
  }

  std::unique_ptr<Mesh> MeshFactory::GenerateRightWall(
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
    const geom::Vector3D height_vector = geom::Vector3D(0.f, 0.f, road_param.wall_height);

    std::vector<geom::Vector3D> r_vertices;
    if (lane.IsStraight()) {
      // Mesh optimization: If the lane is straight just add vertices at the
      // begining and at the end of it
      const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
      r_vertices.push_back(edges.first + height_vector);
      r_vertices.push_back(edges.first);
    } else {
      // Iterate over the lane's 's' and store the vertices based on it's width
      do {
        // Get the location of the edges of the current lane at the current waypoint
        const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
        r_vertices.push_back(edges.first + height_vector);
        r_vertices.push_back(edges.first);

        // Update the current waypoint's "s"
        s_current += road_param.resolution;
      } while(s_current < s_end);
    }

    // This ensures the mesh is constant and have no gaps between roads,
    // adding geometry at the very end of the lane
    if (s_end - (s_current - road_param.resolution) > EPSILON) {
      const auto edges = lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width);
      r_vertices.push_back(edges.first + height_vector);
      r_vertices.push_back(edges.first);
    }

    // Add the adient material, create the strip and close the material
    out_mesh.AddMaterial(
        lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road");
    out_mesh.AddTriangleStrip(r_vertices);
    out_mesh.EndMaterial();
    return std::make_unique<Mesh>(out_mesh);
  }

  std::unique_ptr<Mesh> MeshFactory::GenerateLeftWall(
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
    const geom::Vector3D height_vector = geom::Vector3D(0.f, 0.f, road_param.wall_height);

    std::vector<geom::Vector3D> l_vertices;
    if (lane.IsStraight()) {
      // Mesh optimization: If the lane is straight just add vertices at the
      // begining and at the end of it
      const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
      l_vertices.push_back(edges.second);
      l_vertices.push_back(edges.second + height_vector);
    } else {
      // Iterate over the lane's 's' and store the vertices based on it's width
      do {
        // Get the location of the edges of the current lane at the current waypoint
        const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
        l_vertices.push_back(edges.second);
        l_vertices.push_back(edges.second + height_vector);

        // Update the current waypoint's "s"
        s_current += road_param.resolution;
      } while(s_current < s_end);
    }

    // This ensures the mesh is constant and have no gaps between roads,
    // adding geometry at the very end of the lane
    if (s_end - (s_current - road_param.resolution) > EPSILON) {
      const auto edges = lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width);
      l_vertices.push_back(edges.second);
      l_vertices.push_back(edges.second + height_vector);
    }

    // Add the adient material, create the strip and close the material
    out_mesh.AddMaterial(
        lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road");
    out_mesh.AddTriangleStrip(l_vertices);
    out_mesh.EndMaterial();
    return std::make_unique<Mesh>(out_mesh);
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWithMaxLen(
      const road::Road &road) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;
    for (auto &&lane_section : road.GetLaneSections()) {
      auto section_uptr_list = GenerateWithMaxLen(lane_section);
      mesh_uptr_list.insert(
          mesh_uptr_list.end(),
          std::make_move_iterator(section_uptr_list.begin()),
          std::make_move_iterator(section_uptr_list.end()));
    }
    return mesh_uptr_list;
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWithMaxLen(
      const road::LaneSection &lane_section) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;
    if (lane_section.GetLength() < road_param.max_road_len) {
      mesh_uptr_list.emplace_back(Generate(lane_section));
    } else {
      double s_current = lane_section.GetDistance() + EPSILON;
      const double s_end = lane_section.GetDistance() + lane_section.GetLength() - EPSILON;
      while(s_current + road_param.max_road_len < s_end) {
        const auto s_until = s_current + road_param.max_road_len;
        Mesh lane_section_mesh;
        for (auto &&lane_pair : lane_section.GetLanes()) {
          lane_section_mesh += *Generate(lane_pair.second, s_current, s_until);
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));
        s_current = s_until;
      }
      if (s_end - s_current > EPSILON) {
        Mesh lane_section_mesh;
        for (auto &&lane_pair : lane_section.GetLanes()) {
          lane_section_mesh += *Generate(lane_pair.second, s_current, s_end);
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));
      }
    }
    return mesh_uptr_list;
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWallsWithMaxLen(
      const road::Road &road) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;
    for (auto &&lane_section : road.GetLaneSections()) {
      auto section_uptr_list = GenerateWallsWithMaxLen(lane_section);
      mesh_uptr_list.insert(
          mesh_uptr_list.end(),
          std::make_move_iterator(section_uptr_list.begin()),
          std::make_move_iterator(section_uptr_list.end()));
    }
    return mesh_uptr_list;
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWallsWithMaxLen(
      const road::LaneSection &lane_section) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;

    const auto min_lane = lane_section.GetLanes().begin()->first == 0 ?
        1 : lane_section.GetLanes().begin()->first;
    const auto max_lane = lane_section.GetLanes().rbegin()->first == 0 ?
        -1 : lane_section.GetLanes().rbegin()->first;

    if (lane_section.GetLength() < road_param.max_road_len) {
      mesh_uptr_list.emplace_back(GenerateWalls(lane_section));
    } else {
      double s_current = lane_section.GetDistance() + EPSILON;
      const double s_end = lane_section.GetDistance() + lane_section.GetLength() - EPSILON;
      while(s_current + road_param.max_road_len < s_end) {
        const auto s_until = s_current + road_param.max_road_len;
        Mesh lane_section_mesh;
        for (auto &&lane_pair : lane_section.GetLanes()) {
          const auto &lane = lane_pair.second;
          if (lane.GetId() == max_lane) {
            lane_section_mesh += *GenerateLeftWall(lane, s_current, s_until);
          }
          if (lane.GetId() == min_lane) {
            lane_section_mesh += *GenerateRightWall(lane, s_current, s_until);
          }
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));
        s_current = s_until;
      }
      if (s_end - s_current > EPSILON) {
        Mesh lane_section_mesh;
        for (auto &&lane_pair : lane_section.GetLanes()) {
          const auto &lane = lane_pair.second;
          if (lane.GetId() == max_lane) {
            lane_section_mesh += *GenerateLeftWall(lane, s_current, s_end);
          }
          if (lane.GetId() == min_lane) {
            lane_section_mesh += *GenerateRightWall(lane, s_current, s_end);
          }
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));
      }
    }
    return mesh_uptr_list;
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateAllWithMaxLen(
      const road::Road &road) const {
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;

    // Get road meshes
    auto roads = GenerateWithMaxLen(road);
    mesh_uptr_list.insert(
        mesh_uptr_list.end(),
        std::make_move_iterator(roads.begin()),
        std::make_move_iterator(roads.end()));

    // Get wall meshes only if is not a junction
    if (!road.IsJunction()) {
      auto walls = GenerateWallsWithMaxLen(road);

      if (roads.size() == walls.size()) {
        for (size_t i = 0; i < walls.size(); ++i) {
          *mesh_uptr_list[i] += *walls[i];
        }
      } else {
        mesh_uptr_list.insert(
            mesh_uptr_list.end(),
            std::make_move_iterator(walls.begin()),
            std::make_move_iterator(walls.end()));
      }
    }

    return mesh_uptr_list;
  }

  struct VertexWeight {
    Mesh::vertex_type* vertex;
    double weight;
  };
  struct VertexNeighbors {
    Mesh::vertex_type* vertex;
    std::vector<VertexWeight> neighbors;
  };
  struct VertexInfo {
    Mesh::vertex_type * vertex;
    size_t lane_mesh_idx;
    bool is_static;
  };

  // Helper function to compute the weight of neighboring vertices
  static VertexWeight ComputeVertexWeight(
      const MeshFactory::RoadParameters &road_param,
      const VertexInfo &vertex_info,
      const VertexInfo &neighbor_info) {
    const float distance3D = geom::Math::Distance(*vertex_info.vertex, *neighbor_info.vertex);
    // Ignore vertices beyond a certain distance
    if(distance3D > road_param.max_weight_distance) {
      return {neighbor_info.vertex, 0};
    }
    if(abs(distance3D) < EPSILON) {
      return {neighbor_info.vertex, 0};
    }
    float weight = geom::Math::Clamp<float>(1.0f / distance3D, 0.0f, 100000.0f);

    // Additional weight to vertices in the same lane
    if(vertex_info.lane_mesh_idx == neighbor_info.lane_mesh_idx) {
      weight *= road_param.same_lane_weight_multiplier;
      // Further additional weight for fixed verices
      if(neighbor_info.is_static) {
        weight *= road_param.lane_ends_multiplier;
      }
    }
    return {neighbor_info.vertex, weight};
  }

  // Helper function to compute neighborhoord of vertices and their weights
  std::vector<VertexNeighbors> GetVertexNeighborhoodAndWeights(
      const MeshFactory::RoadParameters &road_param,
      std::vector<std::unique_ptr<Mesh>> &lane_meshes) {
    // Build rtree for neighborhood queries
    using Rtree = geom::PointCloudRtree<VertexInfo>;
    using Point = Rtree::BPoint;
    Rtree rtree;
    for (size_t lane_mesh_idx = 0; lane_mesh_idx < lane_meshes.size(); ++lane_mesh_idx) {
      auto& mesh = lane_meshes[lane_mesh_idx];
      for(size_t i = 0; i < mesh->GetVerticesNum(); ++i) {
        auto& vertex = mesh->GetVertices()[i];
        Point point(vertex.x, vertex.y, vertex.z);
        if (i < 2 || i >= mesh->GetVerticesNum() - 2) {
          rtree.InsertElement({point, {&vertex, lane_mesh_idx, true}});
        } else {
          rtree.InsertElement({point, {&vertex, lane_mesh_idx, false}});
        }
      }
    }

    // Find neighbors for each vertex and compute their weight
    std::vector<VertexNeighbors> vertices_neighborhoods;
    for (size_t lane_mesh_idx = 0; lane_mesh_idx < lane_meshes.size(); ++lane_mesh_idx) {
      auto& mesh = lane_meshes[lane_mesh_idx];
      for(size_t i = 0; i < mesh->GetVerticesNum(); ++i) {
        if (i > 2 && i < mesh->GetVerticesNum() - 2) {
          auto& vertex = mesh->GetVertices()[i];
          Point point(vertex.x, vertex.y, vertex.z);
          auto closest_vertices = rtree.GetNearestNeighbours(point, 20);
          VertexNeighbors vertex_neighborhood;
          vertex_neighborhood.vertex = &vertex;
          for(auto& close_vertex : closest_vertices) {
            auto &vertex_info = close_vertex.second;
            if(&vertex == vertex_info.vertex) {
              continue;
            }
            auto vertex_weight = ComputeVertexWeight(
                road_param, {&vertex, lane_mesh_idx, false}, vertex_info);
            if(vertex_weight.weight > 0)
              vertex_neighborhood.neighbors.push_back(vertex_weight);
          }
          vertices_neighborhoods.push_back(vertex_neighborhood);
        }
      }
    }
    return vertices_neighborhoods;
  }

  std::unique_ptr<Mesh> MeshFactory::MergeAndSmooth(std::vector<std::unique_ptr<Mesh>> &lane_meshes) const {
    geom::Mesh out_mesh;

    auto vertices_neighborhoods = GetVertexNeighborhoodAndWeights(road_param, lane_meshes);

    // Laplacian function
    auto Laplacian = [&](const Mesh::vertex_type* vertex, const std::vector<VertexWeight> &neighbors) -> double {
      double sum = 0;
      double sum_weight = 0;
      for(auto &element : neighbors) {
        sum += (element.vertex->z - vertex->z)*element.weight;
        sum_weight += element.weight;
      }
      if(sum_weight > 0)
        return sum / sum_weight;
      else
        return 0;
    };
    // Run iterative algorithm
    double lambda = 0.5;
    int iterations = 100;
    for(int iter = 0; iter < iterations; ++iter) {
      for (auto& vertex_neighborhood : vertices_neighborhoods) {
        auto * vertex = vertex_neighborhood.vertex;
        vertex->z += static_cast<float>(lambda*Laplacian(vertex, vertex_neighborhood.neighbors));
      }
    }

    for(auto &mesh : lane_meshes) {
      out_mesh += *mesh;
    }

    return std::make_unique<Mesh>(out_mesh);
  }

} // namespace geom
} // namespace carla
