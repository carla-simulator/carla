// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Frame.h"
#include "Model.h"
#include "OsmIngest.h"

namespace twinmodel {

  struct SignalOptions {
    bool generate_traffic_lights = false;
    bool all_junctions_traffic_lights = false;

    /// A signal node is bound to the nearest road end within this distance.
    double snap_m = 25.0;

    /// Crosswalk half width along the road, and half length across it.
    double crosswalk_half_width = 2.0;
  };

  /// Read traffic lights, stop and give-way signs, speed limits and crossings
  /// out of the OSM tags and attach them to the roads they govern.
  void BuildSignals(const OsmData &osm, const LocalFrame &frame,
                    TwinModelData &model, const SignalOptions &options);

} // namespace twinmodel
