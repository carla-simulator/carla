// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include <carla/geom/Vector3D.h>
#include <carla/geom/Vector2D.h>
// #include <carla/NonCopyable.h>

namespace carla {
namespace geom {

  /// Mesh data container, validator and exporter
  class Mesh /*: MovableNonCopyable*/ {
  public:

    using vertex_type = Vector3D;
    using normal_type = Vector3D;
    using index_type = size_t;
    using uv_type = Vector2D;

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

    /// Appends a vertex to the vertices list.
    void AddVertex(vertex_type vertex);

    /// Appends a normal to the normal list.
    void AddNormal(normal_type normal);

    /// Appends a index to the indexes list.
    void AddIndex(index_type index);

    /// Appends a vertex to the vertices list, they will be read 3 in 3.
    void AddUV(uv_type uv);

    // =========================================================================
    // -- Export methods -------------------------------------------------------
    // =========================================================================

    /// Returns an string containing the mesh encoded in OBJ.
    std::string GenerateOBJ() const;

    /// Returns an string containing the mesh encoded in PLY.
    std::string GeneratePLY() const;

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    /// Returns the index of the last added vertex (number of vertices)
    size_t GetLastVertexIndex() const;

    /// TODO: delete this, it's just for debug
    void ShowInfo() const;

  private:

    // =========================================================================
    // -- Private data members -------------------------------------------------
    // =========================================================================

    std::vector<vertex_type> _vertices;

    std::vector<normal_type> _normals;

    std::vector<index_type> _indexes;

    std::vector<uv_type> _uvs;

  };

} // namespace geom
} // namespace carla
