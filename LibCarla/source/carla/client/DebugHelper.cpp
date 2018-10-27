// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/DebugHelper.h"

#include "carla/client/detail/Simulator.h"
#include "carla/rpc/DebugShape.h"

namespace carla {
namespace client {

  using Shape = rpc::DebugShape;

  template <typename T>
  static void DrawShape(
      detail::EpisodeProxy &episode,
      const T &primitive,
      rpc::Color color,
      float life_time,
      bool persistent_lines) {
    const Shape shape{primitive, color, life_time, persistent_lines};
    episode.Lock()->DrawDebugShape(shape);
  }

  void DebugHelper::DrawPoint(
      const geom::Location &location,
      float size,
      sensor::data::Color color,
      float life_time,
      bool persistent_lines) {
    Shape::Point point{location, size};
    DrawShape(_episode, point, color, life_time, persistent_lines);
  }

  void DebugHelper::DrawLine(
      const geom::Location &begin,
      const geom::Location &end,
      float thickness,
      sensor::data::Color color,
      float life_time,
      bool persistent_lines) {
    Shape::Line line{begin, end, thickness};
    DrawShape(_episode, line, color, life_time, persistent_lines);
  }

  void DebugHelper::DrawArrow(
      const geom::Location &begin,
      const geom::Location &end,
      float thickness,
      float arrow_size,
      sensor::data::Color color,
      float life_time,
      bool persistent_lines) {
    Shape::Line line{begin, end, thickness};
    Shape::Arrow arrow{line, arrow_size};
    DrawShape(_episode, arrow, color, life_time, persistent_lines);
  }

  void DebugHelper::DrawBox(
      const geom::BoundingBox &box,
      const geom::Rotation &rotation,
      float thickness,
      sensor::data::Color color,
      float life_time,
      bool persistent_lines) {
    Shape::Box the_box{box, rotation, thickness};
    DrawShape(_episode, the_box, color, life_time, persistent_lines);
  }

  void DebugHelper::DrawString(
      const geom::Location &location,
      const std::string &text,
      bool draw_shadow,
      sensor::data::Color color,
      float life_time,
      bool persistent_lines) {
    Shape::String string{location, text, draw_shadow};
    DrawShape(_episode, string, color, life_time, persistent_lines);
  }

} // namespace client
} // namespace carla
