// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <vector>

namespace osm2odr {

  struct OSM2ODRSettings {
    // Subtract the offset below from every projected coordinate.
    bool use_offsets = false;
    double offset_x = 0.0;
    double offset_y = 0.0;

    // Width of a driving lane, in metres. 0 keeps the converter's own table,
    // which picks a width per highway type.
    double default_lane_width = 0.0;

    // Width of the sidewalk lane added to a way whose OSM tags declare one.
    // 0 adds none.
    double default_sidewalk_width = 0.0;

    // Height one OSM layer is raised by, so a bridge clears what it crosses.
    double elevation_layer_height = 0.0;

    // Projection written as the <geoReference> of the OpenDRIVE header. A
    // +lat_0/+lon_0 in it also becomes the origin of the local projection.
    std::string proj_string = "+proj=tmerc";

    // Put the origin at the centre of the bounding box instead of its north
    // west corner. CARLA's map generation expects the corner.
    bool center_map = false;

    // Emit a traffic light at junctions OSM tags highway=traffic_signals, or
    // at every junction.
    bool generate_traffic_lights = false;
    bool all_junctions_traffic_lights = false;

    // OSM highway types to keep. Empty keeps the converter's own drivable set.
    std::vector<std::string> osm_highways_types;

    // Highway types that get no traffic light even when one is generated.
    std::vector<std::string> tl_excluded_highways_types;
  };

  // Convert the contents of an OSM XML file to an OpenDRIVE description.
  // Throws std::runtime_error when the input cannot be parsed.
  std::string ConvertOSMToOpenDRIVE(
      std::string osm_file,
      OSM2ODRSettings settings = OSM2ODRSettings());

} // namespace osm2odr
