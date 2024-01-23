// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

enum class MaterialParameter
{
  Tex_Normal,
  Tex_Ao_Roughness_Metallic_Emissive,
  Tex_Diffuse,
  Tex_Emissive
};

std::string MaterialParameterToString(MaterialParameter material_parameter);

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::MaterialParameter);
