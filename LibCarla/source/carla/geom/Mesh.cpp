// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/Mesh.h>

#include <string>
#include <sstream>
#include <carla/geom/Math.h>
#include <ios>

namespace carla {
namespace geom {

  bool Mesh::IsValid() const {
    // should be at least some one vertex
    if (_vertices.empty()) {
      std::cout << "Mesh validation error: there are no vertices in the mesh." << std::endl;
      return false;
    }

    // if there are indices, the amount must be multiple of 3
    if (!_indexes.empty() && _indexes.size() % 3 != 0) {
      std::cout << "Mesh validation error: the index amount must be multiple of 3." << std::endl;
      return false;
    }

    if (!_materials.empty() && _materials.back().index_end == 0) {
      std::cout << "Mesh validation error: last material was not closed." << std::endl;
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

  void Mesh::AddMaterial(const std::string &material_name) {
    const size_t open_index = _indexes.size();
    if (!_materials.empty()) {
      if (_materials.back().index_end == 0) {
        std::cout << "last material was not closed, closing it..." << std::endl;
        EndMaterial();
      }
    }
    if (open_index % 3 != 0) {
      std::cout << "open_index % 3 != 0" << std::endl;
      return;
    }
    _materials.emplace_back(material_name, open_index, 0);
  }

  void Mesh::EndMaterial() {
    const size_t close_index = _indexes.size();
    if (_materials.empty() ||
        _materials.back().index_start == close_index ||
        _materials.back().index_end != 0) {
      std::cout << "WARNING: Bad end of material. Material not started." << std::endl;
      return;
    }
    if (_indexes.empty() || close_index % 3 != 0) {
      std::cout << "WARNING: Bad end of material. Face not started/ended." << std::endl;
      return;
    }
    _materials.back().index_end = close_index;
  }

  std::string Mesh::GenerateOBJ() const {
    if (!IsValid()) {
      return "";
    }
    std::stringstream out;
    out << std::fixed; // Avoid using scientific notation

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
      auto it_m = _materials.begin();
      auto it = _indexes.begin();
      size_t index_counter = 0u;
      while (it != _indexes.end()) {
        // While exist materials
        if (it_m != _materials.end()) {
          // If the current material ends at this index
          if (it_m->index_end == index_counter) {
            ++it_m;
          }
          // If the current material start at this index
          if (it_m->index_start == index_counter) {
            out << "\nusemtl " << it_m->name << std::endl;
          }
        }

        // Add the actual face using the 3 consecutive indices
        out << "f " << *it; ++it;
        out << " " << *it; ++it;
        out << " " << *it << std::endl; ++it;

        index_counter += 3;
      }
    }

    return out.str();
  }

  std::string Mesh::GenerateOBJForRecast() const {
    if (!IsValid()) {
      return "";
    }
    std::stringstream out;
    out << std::fixed; // Avoid using scientific notation

    out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
    for (auto &v : _vertices) {
      // Switched "y" and "z" for Recast library
      out << "v " << v.x << " " << v.z << " " << v.y << std::endl;
    }

    if (!_indexes.empty()) {
      out << std::endl << "# Polygonal face element." << std::endl;
      auto it_m = _materials.begin();
      auto it = _indexes.begin();
      size_t index_counter = 0u;
      while (it != _indexes.end()) {
        // While exist materials
        if (it_m != _materials.end()) {
          // If the current material ends at this index
          if (it_m->index_end == index_counter) {
            ++it_m;
          }
          // If the current material start at this index
          if (it_m->index_start == index_counter) {
            out << "\nusemtl " << it_m->name << std::endl;
          }
        }
        // Add the actual face using the 3 consecutive indices
        // Changes the face build direction to clockwise since
        // the space has changed.
        out << "f " << *it; ++it;
        const auto i_2 = *it; ++it;
        const auto i_3 = *it; ++it;
        out << " " << i_3 << " " << i_2 << std::endl;
        index_counter += 3;
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

  size_t Mesh::GetLastVertexIndex() const {
    return _vertices.size();
  }

  const std::vector<Mesh::vertex_type> &Mesh::GetVertices() const {
    return _vertices;
  }

  const std::vector<Mesh::normal_type> &Mesh::GetNormals() const {
    return _normals;
  }

  const std::vector<Mesh::index_type> &Mesh::GetIndexes() const {
    return _indexes;
  }

  const std::vector<Mesh::uv_type> &Mesh::GetUVs() const {
    return _uvs;
  }

  const std::vector<Mesh::material_type> &Mesh::GetMaterials() const {
    return _materials;
  }

} // namespace geom
} // namespace carla
