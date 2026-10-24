// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/Color.h"
#include "carla/rpc/ObjectLabel.h"
#include "carla/rpc/Transform.h"
#include "carla/rpc/Vector2D.h"
#include "carla/rpc/Vector3D.h"

#include <cstdint>
#include <string>
#include <vector>

namespace carla {
namespace rpc {

  struct CustomMaterialParameters {
    Color base_color{200, 200, 200};
    float roughness = 0.8f;
    float metallic = 0.0f;
    float specular = 0.5f;
    Vector2D uv_scale{1.0f, 1.0f};
    std::string albedo_texture_path;
    std::string normal_texture_path;
    std::string roughness_texture_path;

    MSGPACK_DEFINE_ARRAY(
        base_color,
        roughness,
        metallic,
        specular,
        uv_scale,
        albedo_texture_path,
        normal_texture_path,
        roughness_texture_path);
  };

  struct CustomMeshDescription {
    std::string name = "CustomMesh";
    Transform transform{};
    Vector3D scale{1.0f, 1.0f, 1.0f};
    std::vector<Vector3D> vertices;
    std::vector<int32_t> triangles;
    std::vector<Vector3D> normals;
    std::vector<Vector2D> uvs;
    std::vector<Color> vertex_colors;
    CityObjectLabel semantic_label = CityObjectLabel::Buildings;
    bool enable_collision = true;
    std::string material_preset = "building_concrete";
    CustomMaterialParameters material_params{};

    MSGPACK_DEFINE_ARRAY(
        name,
        transform,
        scale,
        vertices,
        triangles,
        normals,
        uvs,
        vertex_colors,
        semantic_label,
        enable_collision,
        material_preset,
        material_params);
  };

} // namespace rpc
} // namespace carla
