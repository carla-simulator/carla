// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/Mesh.h>

#include <string>
#include <sstream>
#include <ios>
#include <iostream>
#include <fstream>

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

  // e.g:
  // 1   3   5   7
  // #---#---#---#
  // | / | / | / |
  // #---#---#---#
  // 2   4   6   8
  void Mesh::AddTriangleStrip(const std::vector<Mesh::vertex_type> &vertices) {
    if (vertices.size() == 0) {
      return;
    }
    DEBUG_ASSERT(vertices.size() >= 3);
    size_t i = GetVerticesNum() + 2;
    AddVertices(vertices);
    bool index_clockwise = true;
    while (i < GetVerticesNum()) {
      index_clockwise = !index_clockwise;
      if (index_clockwise) {
        AddIndex(i + 1);
        AddIndex(i);
        AddIndex(i - 1);
      } else {
        AddIndex(i - 1);
        AddIndex(i);
        AddIndex(i + 1);
      }
      ++i;
    }
  }

  // e.g:
  // 2   1   6
  // #---#---#
  // | / | \ |
  // #---#---#
  // 3   4   5
  void Mesh::AddTriangleFan(const std::vector<Mesh::vertex_type> &vertices) {
    DEBUG_ASSERT(vertices.size() >= 3);
    const size_t initial_index = GetVerticesNum() + 1;
    size_t i = GetVerticesNum() + 2;
    AddVertices(vertices);
    while (i < GetVerticesNum()) {
      AddIndex(initial_index);
      AddIndex(i);
      AddIndex(i + 1);
      ++i;
    }
  }

  void Mesh::AddVertex(vertex_type vertex) {
    _vertices.push_back(vertex);
  }

  void Mesh::AddVertices(const std::vector<Mesh::vertex_type> &vertices) {
    std::copy(vertices.begin(), vertices.end(), std::back_inserter(_vertices));
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

  void Mesh::AddUVs(const std::vector<uv_type> & uv) {
    std::copy(uv.begin(), uv.end(), std::back_inserter(_uvs));
  }

  void Mesh::AddMaterial(const std::string &material_name) {
    const size_t open_index = _indexes.size();
    if (!_materials.empty()) {
      if (_materials.back().index_end == 0) {
        // @todo: change this comment to a debug warning
        // std::cout << "last material was not closed, closing it..." << std::endl;
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
      // @todo: change this comment to a debug warning
      // std::cout << "WARNING: Bad end of material. Material not started." << std::endl;
      return;
    }
    if (_indexes.empty() || close_index % 3 != 0) {
      // @todo: change this comment to a debug warning
      // std::cout << "WARNING: Bad end of material. Face not started/ended." << std::endl;
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

    // A handful of road-generation edge cases (e.g. a near-zero-length
    // direction vector normalized to a unit vector, or a polynomial
    // evaluated far outside its intended range) can occasionally produce a
    // vertex tens of thousands of kilometres from the rest of the map.
    // RecastBuilder's InputGeom computes its mesh bounding box with
    // rcCalcBounds() over *every* vertex in the OBJ, regardless of whether
    // any face still references it, and Recast then sizes its voxel grid
    // off that bounding box -- so a single such outlier makes navmesh
    // building computationally intractable even though its own geometry is
    // dropped below. Confirmed: on a real corrupted Town03 export this
    // produced a target tile grid of ~2.6M x 45K tiles and RecastBuilder
    // never finished (pegged a CPU core for 10+ minutes, no output);
    // zeroing the ~4 offending vertices dropped that to a sane 12x12 grid
    // and RecastBuilder completed in under a second. Every shipped CARLA
    // map is a few kilometres across at most, so any vertex beyond this
    // (generous, two-orders-of-magnitude-safe) bound is corrupt: its
    // coordinates are zeroed (rather than left in place) so it can't
    // distort the bounding box, and any face still referencing it is
    // dropped too, degrading to a small hole in the navmesh rather than a
    // hung/impossible build.
    constexpr float MaxSaneCoordinate = 1e5f; // 100 km
    const auto is_sane_vertex = [](const vertex_type &v) {
      return std::fabs(v.x) <= MaxSaneCoordinate &&
             std::fabs(v.y) <= MaxSaneCoordinate &&
             std::fabs(v.z) <= MaxSaneCoordinate;
    };

    size_t sanitized_vertices = 0u;
    out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
    for (auto &v : _vertices) {
      if (is_sane_vertex(v)) {
        // Switched "y" and "z" for Recast library
        out << "v " << v.x << " " << v.z << " " << v.y << std::endl;
      } else {
        out << "v 0 0 0" << std::endl;
        ++sanitized_vertices;
      }
    }

    if (!_indexes.empty()) {
      out << std::endl << "# Polygonal face element." << std::endl;
      auto it_m = _materials.begin();
      auto it = _indexes.begin();
      size_t index_counter = 0u;
      size_t skipped_faces = 0u;
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
        // Read the 3 consecutive indices making up this face.
        const auto i_1 = *it; ++it;
        const auto i_2 = *it; ++it;
        const auto i_3 = *it; ++it;
        index_counter += 3;

        // Indexes are 1-based; guard against out-of-range indices too.
        if (i_1 == 0 || i_2 == 0 || i_3 == 0 ||
            i_1 > _vertices.size() || i_2 > _vertices.size() || i_3 > _vertices.size() ||
            !is_sane_vertex(_vertices[i_1 - 1]) ||
            !is_sane_vertex(_vertices[i_2 - 1]) ||
            !is_sane_vertex(_vertices[i_3 - 1])) {
          ++skipped_faces;
          continue;
        }

        // Add the actual face using the 3 consecutive indices
        // Changes the face build direction to clockwise since
        // the space has changed.
        out << "f " << i_1 << " " << i_3 << " " << i_2 << std::endl;
      }
      if (skipped_faces > 0u) {
        std::cout << "Mesh::GenerateOBJForRecast: sanitized " << sanitized_vertices
                   << " out-of-range vertex/vertices and skipped " << skipped_faces
                   << " face(s) referencing them (beyond "
                   << MaxSaneCoordinate << "m from the origin)." << std::endl;
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

  const std::vector<Mesh::vertex_type> &Mesh::GetVertices() const {
    return _vertices;
  }

  std::vector<Mesh::vertex_type> &Mesh::GetVertices() {
    return _vertices;
  }

  size_t Mesh::GetVerticesNum() const {
    return _vertices.size();
  }

  const std::vector<Mesh::normal_type> &Mesh::GetNormals() const {
    return _normals;
  }

  const std::vector<Mesh::index_type> &Mesh::GetIndexes() const {
    return _indexes;
  }

  std::vector<Mesh::index_type>& Mesh::GetIndexes() {
    return _indexes;
  }
  size_t Mesh::GetIndexesNum() const {
    return _indexes.size();
  }

  const std::vector<Mesh::uv_type> &Mesh::GetUVs() const {
    return _uvs;
  }

  const std::vector<Mesh::material_type> &Mesh::GetMaterials() const {
    return _materials;
  }

  size_t Mesh::GetLastVertexIndex() const {
    return _vertices.size();
  }

  Mesh& Mesh::ConcatMesh(const Mesh& rhs, int num_vertices_to_link) {

    if (!rhs.IsValid()){
      return *this += rhs;
    }
    const size_t v_num = GetVerticesNum();
    const size_t i_num = GetIndexesNum();

    _vertices.insert(
      _vertices.end(),
      rhs.GetVertices().begin(),
      rhs.GetVertices().end());

    _normals.insert(
      _normals.end(),
      rhs.GetNormals().begin(),
      rhs.GetNormals().end());

    const size_t vertices_to_link = static_cast<size_t>(num_vertices_to_link);
    const size_t vertex_to_start_concating = v_num - vertices_to_link;
    for( size_t i = 1; i < vertices_to_link; ++i ) {
      _indexes.push_back( vertex_to_start_concating + i );
      _indexes.push_back( vertex_to_start_concating + i  + 1 );
      _indexes.push_back( v_num + i );

      _indexes.push_back( vertex_to_start_concating + i + 1);
      _indexes.push_back( v_num + i + 1);
      _indexes.push_back( v_num + i);
    }

    std::transform(
      rhs.GetIndexes().begin(),
      rhs.GetIndexes().end(),
      std::back_inserter(_indexes),
      [=](size_t index) {return index + v_num; });

    _uvs.insert(
      _uvs.end(),
      rhs.GetUVs().begin(),
      rhs.GetUVs().end());

    std::transform(
      rhs.GetMaterials().begin(),
      rhs.GetMaterials().end(),
      std::back_inserter(_materials),
      [=](MeshMaterial mat) {
        mat.index_start += i_num;
        mat.index_end += i_num;
        return mat;
      });

    return *this;
  }

  Mesh &Mesh::operator+=(const Mesh &rhs) {
    const size_t v_num = GetVerticesNum();
    const size_t i_num = GetIndexesNum();

    _vertices.insert(
        _vertices.end(),
        rhs.GetVertices().begin(),
        rhs.GetVertices().end());

    _normals.insert(
        _normals.end(),
        rhs.GetNormals().begin(),
        rhs.GetNormals().end());

    std::transform(
        rhs.GetIndexes().begin(),
        rhs.GetIndexes().end(),
        std::back_inserter(_indexes),
        [=](size_t index) {return index + v_num;});

    _uvs.insert(
        _uvs.end(),
        rhs.GetUVs().begin(),
        rhs.GetUVs().end());

    std::transform(
        rhs.GetMaterials().begin(),
        rhs.GetMaterials().end(),
        std::back_inserter(_materials),
        [=](MeshMaterial mat) {
          mat.index_start += i_num;
          mat.index_end += i_num;
          return mat;
        });

    return *this;
  }

  Mesh operator+(const Mesh &lhs, const Mesh &rhs) {
    Mesh m = lhs;
    return m += rhs;
  }

} // namespace geom
} // namespace carla
