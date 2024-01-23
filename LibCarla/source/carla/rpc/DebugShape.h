// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/MsgPackAdaptors.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/rpc/Color.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <variant>
#pragma warning(pop)
#else
#include <variant>
#endif

namespace carla {
namespace rpc {

  class DebugShape {
  public:

    struct Point {
      geom::Location location;
      float size;
      MSGPACK_DEFINE_ARRAY(location, size);
    };

    struct Line {
      geom::Location begin;
      geom::Location end;
      float thickness;
      MSGPACK_DEFINE_ARRAY(begin, end, thickness);
    };

    struct Arrow {
      Line line;
      float arrow_size;
      MSGPACK_DEFINE_ARRAY(line, arrow_size);
    };

    struct Box {
      geom::BoundingBox box;
      geom::Rotation rotation;
      float thickness;
      MSGPACK_DEFINE_ARRAY(box, rotation, thickness);
    };

    struct String {
      geom::Location location;
      std::string text;
      bool draw_shadow;
      MSGPACK_DEFINE_ARRAY(location, text, draw_shadow);
    };

    std::variant<Point, Line, Arrow, Box, String> primitive;

    Color color = {255u, 0u, 0u};

    float life_time = -1.0f;

    bool persistent_lines = true;

    MSGPACK_DEFINE_ARRAY(primitive, color, life_time, persistent_lines);
  };

} // namespace rpc
} // namespace carla
