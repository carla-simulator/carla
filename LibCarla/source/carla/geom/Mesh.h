// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include <carla/geom/Vector3D.h>
#include <carla/geom/Vector2D.h>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include "Util/ProceduralCustomMesh.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  /// Material that references the vertex index start and end of
  /// a mesh where it is affecting.
  struct MeshMaterial {

    MeshMaterial(
        const std::string &new_name,
        size_t start = 0u,
        size_t end = 0u)
      : name(new_name),
        index_start(start),
        index_end(end) {}

    const std::string name;

    size_t index_start;

    size_t index_end;

  };

  /// Mesh data container, validator and exporter.
  class Mesh {
  public:

    using vertex_type = Vector3D;
    using normal_type = Vector3D;
    using index_type = size_t;
    using uv_type = Vector2D;
    using material_type = MeshMaterial;

    // =========================================================================
    // -- Constructor ----------------------------------------------------------
    // =========================================================================

    Mesh(const std::vector<vertex_type> &vertices = {},
        const std::vector<normal_type> &normals = {},
        const std::vector<index_type> &indexes = {},
        const std::vector<uv_type> &uvs = {})
      : _vertices(vertices),
        _normals(normals),
        _indexes(indexes),
        _uvs(uvs) {}

    // =========================================================================
    // -- Validate methods -----------------------------------------------------
    // =========================================================================

    /// Check if the mesh can be valid or not.
    bool IsValid() const;

    // =========================================================================
    // -- Mesh build methods ---------------------------------------------------
    // =========================================================================

    /// Adds a triangle strip to the mesh, vertex order is counterclockwise.
    void AddTriangleStrip(const std::vector<vertex_type> &vertices);

    /// Adds a triangle fan to the mesh, vertex order is counterclockwise.
    void AddTriangleFan(const std::vector<vertex_type> &vertices);

    /// Appends a vertex to the vertices list.
    void AddVertex(vertex_type vertex);

    /// Appends a vertex to the vertices list.
    void AddVertices(const std::vector<vertex_type> &vertices);

    /// Appends a normal to the normal list.
    void AddNormal(normal_type normal);

    /// Appends a index to the indexes list.
    void AddIndex(index_type index);

    /// Appends a vertex to the vertices list, they will be read 3 in 3.
    void AddUV(uv_type uv);

    /// Starts applying a new material to the new added triangles.
    void AddMaterial(const std::string &material_name);

    /// Stops applying the material to the new added triangles.
    void EndMaterial();

    // =========================================================================
    // -- Export methods -------------------------------------------------------
    // =========================================================================

    /// Returns a string containing the mesh encoded in OBJ.
    /// Units are in meters. It is in Unreal space.
    std::string GenerateOBJ() const;

    /// Returns a string containing the mesh encoded in OBJ.
    /// Units are in meters. This function exports the OBJ file
    /// specifically to be consumed by Recast library.
    /// Changes the build face direction and the coordinate space.
    std::string GenerateOBJForRecast() const;

    /// Returns a string containing the mesh encoded in PLY.
    /// Units are in meters.
    std::string GeneratePLY() const;

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    const std::vector<vertex_type> &GetVertices() const;

    std::vector<vertex_type> &GetVertices();

    size_t GetVerticesNum() const;

    const std::vector<normal_type> &GetNormals() const;

    const std::vector<index_type> &GetIndexes() const;

    size_t GetIndexesNum() const;

    const std::vector<uv_type> &GetUVs() const;

    const std::vector<material_type> &GetMaterials() const;

    /// Returns the index of the last added vertex (number of vertices).
    size_t GetLastVertexIndex() const;

    /// Merges two meshes into a single mesh
    Mesh &operator+=(const Mesh &rhs);

    friend Mesh operator+(const Mesh &lhs, const Mesh &rhs);

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    operator FProceduralCustomMesh() const {
      FProceduralCustomMesh Mesh;

      // Build the mesh
      for (const auto Vertex : GetVertices())
      {
        // From meters to centimeters
        Mesh.Vertices.Add(FVector{1e2f * Vertex.x, 1e2f * Vertex.y, 1e2f * Vertex.z});
      }

      const auto Indexes = GetIndexes();
      TArray<FTriIndices> TriIndices;
      for (auto i = 0u; i < Indexes.size(); i += 3)
      {
        FTriIndices Triangle;
        // "-1" since mesh indexes in Unreal starts from index 0.
        Mesh.Triangles.Add(Indexes[i]     - 1);
        // Since Unreal's coords are left handed, invert the last 2 indices.
        Mesh.Triangles.Add(Indexes[i + 2] - 1);
        Mesh.Triangles.Add(Indexes[i + 1] - 1);

        Triangle.v0 = Indexes[i]     - 1;
        Triangle.v1 = Indexes[i + 2] - 1;
        Triangle.v2 = Indexes[i + 1] - 1;
        TriIndices.Add(Triangle);
      }

      // Compute the normals
      TArray<FVector> Normals;
      Mesh.Normals.Init(FVector::UpVector, Mesh.Vertices.Num());

      for (const auto &Triangle : TriIndices) {
        FVector Normal;
        const FVector U = Mesh.Vertices[Triangle.v1] - Mesh.Vertices[Triangle.v0];
        const FVector V = Mesh.Vertices[Triangle.v2] - Mesh.Vertices[Triangle.v0];
        Normal.X = (U.Y * V.Z) - (U.Z * V.Y);
        Normal.Y = (U.Z * V.X) - (U.X * V.Z);
        Normal.Z = (U.X * V.Y) - (U.Y * V.X);
        Normal = -Normal;
        Normal = Normal.GetSafeNormal(.0001f);
        if (Normal != FVector::ZeroVector)
        {
          // fix to prevent ugly x-fighting in geometries with very large curvatures,
          // ensures that all road geometry is facing upwards
          if (FVector::DotProduct(Normal, FVector(0,0,1)) < 0)
          {
            Normal = -Normal;
          }
          Mesh.Normals[Triangle.v0] = Normal;
          Mesh.Normals[Triangle.v1] = Normal;
          Mesh.Normals[Triangle.v2] = Normal;
        }
      }

      return Mesh;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

  private:

    // =========================================================================
    // -- Private data members -------------------------------------------------
    // =========================================================================

    std::vector<vertex_type> _vertices;

    std::vector<normal_type> _normals;

    std::vector<index_type> _indexes;

    std::vector<uv_type> _uvs;

    std::vector<material_type> _materials;
  };

} // namespace geom
} // namespace carla
