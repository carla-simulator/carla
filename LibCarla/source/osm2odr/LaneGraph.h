// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <optional>
#include <string>

#include "Frame.h"
#include "Model.h"
#include "OsmIngest.h"

namespace twinmodel {

  struct LaneGraphOptions {
    /// Ways shorter than this are merged into their neighbour rather than
    /// becoming a road of their own.
    double min_road_length = 1.0;

    /// Radius around a junction node that the incoming roads are trimmed back
    /// by, so the connecting roads have room to turn.
    double junction_radius = 6.0;

    /// Two intersection nodes join one junction when a chain shorter than this
    /// links them and they are closer than this; profiles.py's
    /// JunctionRules.cluster_m, 30 m for the dense European profile.
    double cluster_m = 30.0;

    /// Turns sharper than this are U-turns and get no connection; the
    /// profile's JunctionRules.uturn_deg.
    double uturn_deg = 150.0;

    /// Clip the network to this box in model metres, as the Python pipeline
    /// clips to the requested bbox. Empty keeps every way whole.
    std::optional<geom::Box> clip;

    bool generate_traffic_lights = false;
    bool all_junctions_traffic_lights = false;
  };

  /// OSM ways -> roads, lanes and junctions in model space. The C++ port of
  /// twinmodel/lanegraph.py's build_lanegraph.
  TwinModelData BuildLaneGraph(
      const OsmData &osm,
      const LocalFrame &frame,
      const std::string &name,
      const LaneGraphOptions &options = LaneGraphOptions());

} // namespace twinmodel
