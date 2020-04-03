// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  /// Seting for map generation from opendrive without additional geometry
  struct OpendriveGenerationParameters {
    OpendriveGenerationParameters(){}
    OpendriveGenerationParameters(
        double v_distance,
        double w_height,
        double a_width,
        bool e_visibility)
      : vertex_distance(v_distance),
        wall_height(w_height),
        additional_width(a_width),
        enable_mesh_visibility(e_visibility)
        {}

    double vertex_distance = 2.f;
    double wall_height = 1.f;
    double additional_width = 0.6f;
    bool enable_mesh_visibility = true;

    MSGPACK_DEFINE_ARRAY(vertex_distance, wall_height, additional_width, enable_mesh_visibility);
  };

}
}