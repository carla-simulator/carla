// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/EpisodeProxy.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/sensor/data/Color.h"

namespace carla {
namespace client {

  class DebugHelper {
  public:

    using Color = sensor::data::Color;

    explicit DebugHelper(detail::EpisodeProxy episode)
      : _episode(std::move(episode)) {}

    void DrawPoint(
        const geom::Location &location,
        float size = 0.1f,
        Color color = Color{255u, 0u, 0u},
        float life_time = -1.0f,
        bool persistent_lines = true);

    void DrawLine(
        const geom::Location &begin,
        const geom::Location &end,
        float thickness = 0.1f,
        Color color = Color{255u, 0u, 0u},
        float life_time = -1.0f,
        bool persistent_lines = true);

    void DrawArrow(
        const geom::Location &begin,
        const geom::Location &end,
        float thickness = 0.1f,
        float arrow_size = 0.1f,
        Color color = Color{255u, 0u, 0u},
        float life_time = -1.0f,
        bool persistent_lines = true);

    void DrawBox(
        const geom::BoundingBox &box,
        const geom::Rotation &rotation,
        float thickness = 0.1f,
        Color color = Color{255u, 0u, 0u},
        float life_time = -1.0f,
        bool persistent_lines = true);

    void DrawString(
        const geom::Location &location,
        const std::string &text,
        bool draw_shadow = false,
        Color color = Color{255u, 0u, 0u},
        float life_time = -1.0f,
        bool persistent_lines = true);

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
