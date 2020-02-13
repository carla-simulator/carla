// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/Mesh.h>

#include <string>
#include <sstream>
#include <carla/geom/Math.h>


namespace carla {
namespace geom {

  bool Mesh::IsValid() const {
    // should be at least some one vertex
    if (_vertices.empty()) {
      std::cout << "Mesh validation error: there are no vertices in the mesh." << std::endl;
      return false;
    }

    // if there are indices, the amount must be multiple of 3
    if (!_indexes.empty() and _indexes.size() % 3 != 0) {
      std::cout << "Mesh validation error: the index amount must be multiple of 3." << std::endl;
      return false;
    }

    return true;
  }

  void Mesh::AddVertex(vertex_type vertex) {
    _vertices.push_back(vertex);
  }

  void Mesh::AddNormal(normal_type normal) {
    _normals.push_back(normal);
  }

  void Mesh::AddIndex(index_type index) {
    _indexes.push_back(index);
  }

  void Mesh::AddUV(uv_type uv) {
    _uvs.push_back(uv);
  }

  size_t Mesh::GetLastVertexIndex() const {
    return _vertices.size();
  }

  /// TODO: delete this, it's just for debug
  void Mesh::ShowInfo() const {
    std::cout << "----------------------" << std::endl;
    std::cout << "Vertices: " << _vertices.size() << std::endl;
    std::cout << "Normals:  " << _normals.size() << std::endl;
    std::cout << "Indexes:  " << _indexes.size() << std::endl;
    std::cout << "Uvs:      " << _uvs.size() << std::endl;
    std::cout << "----------------------" << std::endl;
  }

  std::string Mesh::GenerateOBJ() const {
    if (!IsValid()) {
      return "Invalid Mesh";
    }

    std::stringstream out;

    out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
    for (auto &v : _vertices) {
      out << "v " << v.x << " " << v.y << " " << v.z << std::endl;
    }

    if (!_uvs.empty()) {
      out << std::endl << "# List of texture coordinates, in (u, v) coordinates, these will vary between 0 and 1." << std::endl;
      for (auto &vt : _uvs) {
        out << "vt " << vt.x << " " << vt.y << std::endl;
      }
    }

    if (!_normals.empty()) {
      out << std::endl << "# List of vertex normals in (x, y, z) form; normals might not be unit vectors." << std::endl;
      for (auto &vn : _normals) {
        out << "vn " << vn.x << " " << vn.y << " " << vn.z << std::endl;
      }
    }

    if (!_indexes.empty()) {
      out << std::endl << "# Polygonal face element." << std::endl;
      auto it = _indexes.begin();
      while (it != _indexes.end()) {
        out << "f " << *it; ++it;
        out << " " << *it; ++it;
        out << " " << *it << std::endl; ++it;
      }
    }

    return out.str();
  }

  std::string Mesh::GeneratePLY() const {
    if (!IsValid()) {
      return "Invalid Mesh";
    }
    // Generate header
    std::stringstream out;
    return out.str();
  }

} // namespace geom
} // namespace carla
