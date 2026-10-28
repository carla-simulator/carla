// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Geometry.h"

namespace twinmodel {

  using Tags = std::map<std::string, std::string>;

  inline const std::string *FindTag(const Tags &tags, const std::string &key) {
    const auto it = tags.find(key);
    return (it == tags.end()) ? nullptr : &it->second;
  }

  inline bool HasTag(const Tags &tags, const std::string &key, const std::string &value) {
    const std::string *found = FindTag(tags, key);
    return (found != nullptr) && (*found == value);
  }

  enum class LaneType {
    Driving, Sidewalk, Shoulder, Parking, Biking, Border, Restricted, Median, None
  };

  enum class Direction { Forward, Backward, Both };

  enum class MarkingKind { None, Solid, Broken, SolidSolid, SolidBroken, BrokenSolid, BrokenBroken };

  enum class MarkingColor { White, Yellow };

  enum class ContactPoint { Start, End };

  enum class LinkElement { Road, Junction };

  /// OpenDRIVE type/subtype follow road/SignalType.h; the crosswalk is not a
  /// signal in CARLA but an <object type="crosswalk">.
  enum class SignalKind {
    TrafficLight, TrafficLightPedestrian, SpeedLimit, Stop, Yield, PriorityRoad, Crosswalk
  };

  struct SignalType {
    const char *type = "";
    const char *subtype = "-1";
    bool dynamic = false;
  };

  SignalType GetSignalType(SignalKind kind);

  struct Marking {
    MarkingKind kind = MarkingKind::Solid;
    MarkingColor color = MarkingColor::White;
    double width = 0.12;
    std::optional<geom::LineString> geometry;
    std::optional<int> layer;
  };

  /// One lane of a road, numbered outward from the reference line. In CARLA's
  /// right-hand traffic the forward lanes take the negative ids.
  struct Lane {
    int id = 0;
    LaneType type = LaneType::Driving;
    double width = 3.25;
    Direction direction = Direction::Forward;
    std::optional<Marking> marking;
    std::optional<double> speed_limit;
    /// Lane this one continues from / into on the linked road; OpenDRIVE's
    /// <lane><link>, which is what lets a waypoint cross a road boundary.
    std::optional<int> predecessor_lane;
    std::optional<int> successor_lane;
    Tags tags;
  };

  struct RoadLink {
    LinkElement element = LinkElement::Road;
    std::string id;
    std::optional<ContactPoint> contact;
  };

  struct Road {
    std::string id;
    geom::Line3 reference_line;
    std::vector<Lane> lanes;
    std::string name;
    std::string highway;
    std::vector<long long> osm_way_ids;
    std::optional<std::string> junction_id;
    std::optional<RoadLink> predecessor;
    std::optional<RoadLink> successor;
    std::optional<Marking> center_marking;
    Tags tags;
  };

  struct LaneLink {
    int from_lane = 0;
    int to_lane = 0;
  };

  struct Connection {
    std::string id;
    std::string incoming_road;
    std::string connecting_road;
    ContactPoint contact_point = ContactPoint::Start;
    std::vector<LaneLink> lane_links;
  };

  struct Junction {
    std::string id;
    std::optional<geom::Polygon> polygon;
    std::vector<Connection> connections;
    std::vector<long long> osm_node_ids;
    std::string name;
    std::vector<long long> osm_way_ids;
    Tags tags;
  };

  struct Signal {
    std::string id;
    SignalKind kind = SignalKind::TrafficLight;
    std::string road_id;
    double s = 0.0;
    double t = 0.0;
    geom::Point position;
    double heading = 0.0;
    std::optional<double> value;
    std::optional<std::string> controller_id;
    std::string orientation = "+";
    std::vector<std::pair<int, int>> validities;
    double h_offset = 0.0;
    std::optional<long long> osm_node_id;
    Tags tags;
  };

  struct Controller {
    std::string id;
    std::string junction_id;
    std::vector<std::string> signal_ids;
    int sequence = 0;
  };

  struct Elevation {
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;
  };

  /// Everything the OpenDRIVE writer reads. build_lanegraph fills it from the
  /// parsed OSM; the mesh exporter adds surfaces on top of the same roads.
  struct TwinModelData {
    std::string name;
    double origin_lat = 0.0;
    double origin_lon = 0.0;
    std::vector<Road> roads;
    std::vector<Junction> junctions;
    std::vector<Signal> signals;
    std::vector<Controller> controllers;
    Tags tags;

    const Road *FindRoad(const std::string &id) const {
      for (const Road &road : roads) {
        if (road.id == id) {
          return &road;
        }
      }
      return nullptr;
    }
  };

  const char *ToString(LaneType type);
  const char *ToString(Direction direction);
  const char *ToString(MarkingKind kind);
  const char *ToString(MarkingColor color);
  const char *ToString(ContactPoint point);

} // namespace twinmodel
