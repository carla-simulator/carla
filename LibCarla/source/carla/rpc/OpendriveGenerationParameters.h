// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  /// Seting for map generation from opendrive without additional geometry
  struct OpendriveGenerationParameters {
    OpendriveGenerationParameters(){}
    OpendriveGenerationParameters(
        double v_distance,
        double max_road_len,
        double w_height,
        double a_width,
        bool smooth_junc,
        bool e_visibility,
        bool e_pedestrian)
      : vertex_distance(v_distance),
        max_road_length(max_road_len),
        wall_height(w_height),
        additional_width(a_width),
        smooth_junctions(smooth_junc),
        enable_mesh_visibility(e_visibility),
        enable_pedestrian_navigation(e_pedestrian)
        {}

    double vertex_distance = 2.0;
    double max_road_length = 50.0;
    double wall_height = 1.0;
    double additional_width = 0.6;
    double vertex_width_resolution = 4.0f;
    float simplification_percentage = 20.0f;
    bool smooth_junctions = true;
    bool enable_mesh_visibility = true;
    bool enable_pedestrian_navigation = true;

    MSGPACK_DEFINE_ARRAY(
        vertex_distance,
        max_road_length,
        wall_height,
        additional_width,
        smooth_junctions,
        enable_mesh_visibility,
        enable_pedestrian_navigation);
  };

}
}
