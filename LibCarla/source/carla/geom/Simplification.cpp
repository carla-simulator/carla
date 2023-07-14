// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Simplification.h"
#include "simplify/Simplify.h"

namespace carla {
namespace geom {

  void Simplification::Simplificate(const std::unique_ptr<geom::Mesh>& pmesh){
    Simplify::SimplificationObject Simplification;
    for (carla::geom::Vector3D& current_vertex : pmesh->GetVertices()) {
      Simplify::Vertex v;
      v.p.x = current_vertex.x;
      v.p.y = current_vertex.y;
      v.p.z = current_vertex.z;
      Simplification.vertices.push_back(v);
    }

    for (size_t i = 0; i < pmesh->GetIndexes().size() - 2; i += 3) {
      Simplify::Triangle t;
      t.material = 0;
      auto indices = pmesh->GetIndexes();
      t.v[0] = (indices[i]) - 1;
      t.v[1] = (indices[i + 1]) - 1;
      t.v[2] = (indices[i + 2]) - 1;
      Simplification.triangles.push_back(t);
    }

    // Reduce to the X% of the polys
    float target_size = Simplification.triangles.size();
    Simplification.simplify_mesh((target_size * simplification_percentage));

    pmesh->GetVertices().clear();
    pmesh->GetIndexes().clear();

    for (Simplify::Vertex& current_vertex : Simplification.vertices) {
      carla::geom::Vector3D v;
      v.x = current_vertex.p.x;
      v.y = current_vertex.p.y;
      v.z = current_vertex.p.z;
      pmesh->AddVertex(v);
    }

    for (size_t i = 0; i < Simplification.triangles.size(); ++i) {
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[0]) + 1);
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[1]) + 1);
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[2]) + 1);
    }
  }

} // namespace geom
} // namespace carla
