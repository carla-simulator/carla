// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace carla {
namespace opendrive {
namespace types {

  enum class GeometryType : unsigned int {
    ARC,
    LINE,
    SPIRAL
  };

  struct GeometryAttributes {
    GeometryType type;                      // geometry type
    double length;                          // length of the road section
                                            // [meters]

    double start_position;                  // s-offset [meters]
    double heading;                         // start orientation [radians]

    double start_position_x;                // [meters]
    double start_position_y;                // [meters]
  };

  struct GeometryAttributesArc : public GeometryAttributes {
    double curvature;
  };

  struct GeometryAttributesLine : public GeometryAttributes {
    // Nothing else here
  };

  struct GeometryAttributesSpiral : public GeometryAttributes {
    double curve_start;
    double curve_end;
  };

  /////////////////////////////////////////////////////////////////

  struct LaneAttributes {
    int id;
    std::string type;
    std::string level;
  };

  struct LaneWidth {
    double soffset;                         // start position (s-offset)
                                            // [meters]
    double width;                           // a - width [meters]
    double slope;                           // b
    double vertical_curvature;              // c
    double curvature_change;                // d
  };

  struct LaneRoadMark {
    double soffset;
    double width;

    std::string type;
    std::string weigth;

    std::string color;
    std::string lange_change;
  };

  struct LaneOffset {
    double s, a, b, c, d;
  };

  struct LaneSpeed {
    double soffset;                 // start position(s - offset from the
                                    // current lane section) [meters]
    double max_speed;               // maximum allowed speed [meters/second]
  };

  struct LaneLink {
    int predecessor_id;
    int successor_id;
  };

  struct LaneInfo {
    std::vector<LaneSpeed> lane_speed;

    LaneAttributes attributes;
    std::vector<LaneWidth> lane_width;

    std::vector<LaneRoadMark> road_marker;
    std::unique_ptr<LaneLink> link;
  };

  struct LaneSection {
    double start_position;
    std::vector<LaneInfo> left, center, right;
  };

  struct Lanes {
    std::vector<LaneOffset> lane_offset;
    std::vector<LaneSection> lane_sections;
  };

  /////////////////////////////////////////////////////////////////

  struct ElevationProfile {
    double start_position;                  // (S) start position(s -
                                            // offset)[meters]
    double elevation;                       // (A) elevation [meters]
    double slope;                           // (B)
    double vertical_curvature;              // (C)
    double curvature_change;                // (D)
  };

  struct LateralProfile {
    double start_position;                  // (S) start position(s -
                                            // offset)[meters]
    double elevation;                       // (A) elevation [meters]
    double slope;                           // (B)
    double vertical_curvature;              // (C)
    double curvature_change;                // (D)
  };

  struct RoadProfiles {
    std::vector<ElevationProfile> elevation_profile;
    std::vector<LateralProfile> lateral_profile;
  };

  /////////////////////////////////////////////////////////////////

  struct TrafficSignalInformation {
    int id;

    double start_position;                  // s
    double track_position;                  // t

    double zoffset;                         // z offset from track level
    double value;                           // value of the signal (e.g. speed,
                                            // mass � depending on type)

    std::string name;                       // name of the signal (e.g. gfx bead
                                            // name)
    std::string dynamic;                    // boolean identification whether
                                            // signal is a dynamic
                                            // signal(e.g.traffic light)
    std::string orientation;                // "+" = valid in positive track
                                            // direction; "-" = valid in
                                            // negative track direction; "none"
                                            // = valid in both directions

    std::string country;                    // country code of the signa
    std::string type;                       // type identifier according to
                                            // country code or "-1" / "none"
    std::string subtype;                    // subtype identifier according to
                                            // country code or "-1" / "none"
  };

  /////////////////////////////////////////////////////////////////

  struct RoadAttributes {
    std::string name;
    int id, junction;
    double length;

    RoadAttributes() : id(-1),
                       junction(-1),
                       length(0.0) {}
  };

  struct RoadLinkInformation {
    int id;
    std::string element_type;
    std::string contact_point;

    RoadLinkInformation() : id(-1) {}
  };

  struct RoadLink {
    std::unique_ptr<RoadLinkInformation> successor;
    std::unique_ptr<RoadLinkInformation> predecessor;
  };

  struct RoadInformation {
    RoadLink road_link;
    RoadProfiles road_profiles;

    RoadAttributes attributes;
    Lanes lanes;

    std::vector<TrafficSignalInformation> trafic_signals;
    std::vector<std::unique_ptr<GeometryAttributes>> geometry_attributes;
  };

  /////////////////////////////////////////////////////////////////

  struct JunctionAttribues {
    int id;
    std::string name;

    JunctionAttribues() : id(-1) {}
  };

  struct JunctionConnectionAttributes {
    int id;
    int incoming_road;
    int connecting_road;
    std::string contact_point;

    JunctionConnectionAttributes() : id(-1),
                                     incoming_road(-1),
                                     connecting_road(-1) {}
  };

  struct JunctionLaneLink {
    int from;
    int to;

    JunctionLaneLink() : from(-1),
                         to(-1) {}
  };

  struct JunctionConnection {
    JunctionConnectionAttributes attributes;
    std::vector<JunctionLaneLink> links;
  };

  struct Junction {
    JunctionAttribues attributes;
    std::vector<JunctionConnection> connections;
  };

  struct BoxComponent {
    union {
      struct { double x_pos, y_pos, z_pos;
      };
      double pos[3];
    };
    union {
      struct { double x_rot, y_rot, z_rot;
      };
      double rot[3];
    };
    double scale;
    BoxComponent() : pos{0.0, 0.0, 0.0},
                     rot{0.0, 0.0, 0.0},
                     scale(1.0) {}
  };

  struct TrafficLight {
    union {
      struct { double x_pos, y_pos, z_pos;
      };
      double pos[3];
    };
    union {
      struct { double x_rot, y_rot, z_rot;
      };
      double rot[3];
    };
    double scale;
    std::vector<BoxComponent> box_areas;

    TrafficLight() : pos{0.0, 0.0, 0.0},
                     rot{0.0, 0.0, 0.0},
                     scale(1.0) {}
  };

  struct TrafficLightGroup {
    std::vector<TrafficLight> traffic_lights;
    double red_time, yellow_time, green_time;
  };

  /////////////////////////////////////////////////////////////////

  struct OpenDriveData {
    std::string geoReference;
    std::vector<RoadInformation> roads;
    std::vector<Junction> junctions;
    std::vector<TrafficLightGroup> trafficlightgroups;
  };

  struct Waypoint {
    union {
      struct { double x, y, z;
      };
      double v[3];
    };

    double heading, speed, width;
    double distance_in_section;

    Waypoint() : v{0.0, 0.0, 0.0},
                 heading(0.0),
                 speed(0.0),
                 width(0.0) {}
  };

}
}
}
