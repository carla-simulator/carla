// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <optional>
#include <string>

#include "Model.h"

namespace twinmodel {

  /// Per highway class defaults, ported from profiles.py's _EU_CLASSES. The
  /// urban classes sit at 3.5-3.6 m because CARLA's fire truck is 2.90 m wide.
  struct ClassDefaults {
    double lane_width = 3.5;
    int lanes = 2;
    std::optional<double> sidewalk_width;
    std::optional<double> shoulder;
    std::optional<double> shoulder_inner;
  };

  /// The lane layout an OSM way resolves to, before it is cut at junctions.
  struct LaneProfile {
    int forward_lanes = 1;
    int backward_lanes = 1;
    double lane_width = 3.5;
    std::optional<double> sidewalk_width;
    std::optional<double> shoulder;
    std::optional<double> shoulder_inner;
    std::optional<double> speed_limit;
    bool oneway = false;
  };

  /// True when the way carries motor traffic and should become a Road.
  bool IsDrivable(const std::string &highway);

  const ClassDefaults *FindClassDefaults(const std::string &highway);

  /// Resolve the lane layout from the way's tags, falling back to the class
  /// defaults. Honours lanes, lanes:forward/backward, oneway, width, maxspeed.
  LaneProfile ResolveProfile(const std::string &highway, const Tags &tags);

  /// Parse an OSM maxspeed value into m/s; "50", "50 km/h" and "30 mph".
  std::optional<double> ParseMaxSpeed(const std::string &value);

} // namespace twinmodel
