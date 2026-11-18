// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "osm2odr/OSM2ODR.h"

#include "carla/Logging.h"

#include "osm2odr/Frame.h"
#include "osm2odr/LaneGraph.h"
#include "osm2odr/OsmIngest.h"
#include "osm2odr/Signals.h"
#include "osm2odr/XodrExport.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace osm2odr {

namespace {

  // Read a "+key=value" out of a PROJ string. Returns false when absent.
  bool ProjValue(const std::string &proj, const std::string &key, double &out) {
    const std::string needle = "+" + key + "=";
    const auto start = proj.find(needle);
    if (start == std::string::npos) {
      return false;
    }
    try {
      out = std::stod(proj.substr(start + needle.size()));
    } catch (const std::exception &) {
      return false;
    }
    return true;
  }

  void WarnAboutUnsupported(const OSM2ODRSettings &settings) {
    if (!settings.tl_excluded_highways_types.empty() &&
        !settings.generate_traffic_lights && !settings.all_junctions_traffic_lights) {
      carla::log_warning(
          "Osm2Odr: traffic light exclusions have no effect while traffic light generation is off.");
    }
    if (!settings.osm_highways_types.empty()) {
      carla::log_warning(
          "Osm2Odr: osm_highways_types is not honoured yet; the converter's own drivable set "
          "is used.");
    }
  }

} // namespace

  std::string ConvertOSMToOpenDRIVE(std::string osm_file, OSM2ODRSettings settings) {
    WarnAboutUnsupported(settings);

    twinmodel::OsmData osm = twinmodel::ParseOsmXml(osm_file);

    // The <bounds> of an export is the rectangle the user dragged, but the ways
    // crossing it come out whole, so measure the nodes that actually arrived.
    const twinmodel::BoundingBox extent = osm.NodeExtent();

    double origin_lat = 0.0;
    double origin_lon = 0.0;
    const bool pinned = ProjValue(settings.proj_string, "lat_0", origin_lat) &&
                        ProjValue(settings.proj_string, "lon_0", origin_lon);
    if (!pinned) {
      if (settings.center_map) {
        origin_lat = (extent.south + extent.north) * 0.5;
        origin_lon = (extent.west + extent.east) * 0.5;
      } else {
        // CARLA's map generation expects the north west corner, which puts the
        // network in +x / -y.
        origin_lat = extent.north;
        origin_lon = extent.west;
      }
    }
    const twinmodel::LocalFrame frame(origin_lat, origin_lon);

    twinmodel::LaneGraphOptions options;
    options.generate_traffic_lights = settings.generate_traffic_lights;
    options.all_junctions_traffic_lights = settings.all_junctions_traffic_lights;
    if (osm.bbox.has_value()) {
      double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
      frame.ToLocal(osm.bbox->west, osm.bbox->south, x0, y0);
      frame.ToLocal(osm.bbox->east, osm.bbox->north, x1, y1);
      options.clip = twinmodel::geom::Box(
          twinmodel::geom::Point(std::min(x0, x1), std::min(y0, y1)),
          twinmodel::geom::Point(std::max(x0, x1), std::max(y0, y1)));
    }

    twinmodel::TwinModelData model = twinmodel::BuildLaneGraph(osm, frame, "osm", options);
    if (model.roads.empty()) {
      throw std::runtime_error(
          "Osm2Odr: the OSM data holds no drivable way, nothing to convert.");
    }

    twinmodel::SignalOptions signal_options;
    signal_options.generate_traffic_lights = settings.generate_traffic_lights;
    signal_options.all_junctions_traffic_lights = settings.all_junctions_traffic_lights;
    twinmodel::BuildSignals(osm, frame, model, signal_options);

    if (settings.default_lane_width > 0.0 || settings.default_sidewalk_width > 0.0) {
      for (twinmodel::Road &road : model.roads) {
        for (twinmodel::Lane &lane : road.lanes) {
          if (lane.type == twinmodel::LaneType::Driving && settings.default_lane_width > 0.0) {
            lane.width = settings.default_lane_width;
          }
          if (lane.type == twinmodel::LaneType::Sidewalk && settings.default_sidewalk_width > 0.0) {
            lane.width = settings.default_sidewalk_width;
          }
        }
      }
    }

    if (settings.use_offsets) {
      for (twinmodel::Road &road : model.roads) {
        for (twinmodel::geom::Point3 &point : road.reference_line) {
          point.x -= settings.offset_x;
          point.y -= settings.offset_y;
        }
      }
    }

    return twinmodel::ExportXodr(model, frame);
  }

} // namespace osm2odr
