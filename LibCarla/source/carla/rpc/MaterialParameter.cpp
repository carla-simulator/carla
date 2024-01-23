// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MaterialParameter.h"

namespace carla {
namespace rpc {

std::string MaterialParameterToString(MaterialParameter material_parameter)
{
  switch(material_parameter)
  {
    case MaterialParameter::Tex_Normal:                         return "Normal";
    case MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive: return "AO / Roughness / Metallic / Emissive";
    case MaterialParameter::Tex_Diffuse:                      return "Diffuse";
    case MaterialParameter::Tex_Emissive:                       return "Emissive";
    default:                                                    return "Invalid";
  }
}

}
}
