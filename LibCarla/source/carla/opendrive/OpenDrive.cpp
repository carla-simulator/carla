// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDrive.h"

#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/Debug.h"

namespace carla {
namespace opendrive {

  static void fnc_generate_roads_data(
      opendrive::types::OpenDriveData &openDriveRoad,
      std::map<int, opendrive::types::RoadInformation *> &out_roads) {
    for (size_t i = 0; i < openDriveRoad.roads.size(); ++i) {
      out_roads[openDriveRoad.roads[i].attributes.id] = &openDriveRoad.roads[i];
    }
  }

  static void fnc_generate_junctions_data(
      opendrive::types::OpenDriveData &openDriveRoad,
      std::map<int, std::map<int, std::vector<carla::road::lane_junction_t>>> &out_data) {
    for (size_t i = 0; i < openDriveRoad.junctions.size(); ++i) {
      for (size_t j = 0; j < openDriveRoad.junctions[i].connections.size(); ++j) {
        carla::road::lane_junction_t junctionData;
        const int junctionID = openDriveRoad.junctions[i].attributes.id;

        const int incommingRoad = openDriveRoad.junctions[i].connections[j].attributes.incoming_road;
        const int connectingRoad = openDriveRoad.junctions[i].connections[j].attributes.connecting_road;

        junctionData.incomming_road = incommingRoad;
        junctionData.connection_road = connectingRoad;
        junctionData.contact_point = openDriveRoad.junctions[i].connections[j].attributes.contact_point;

        for (size_t k = 0; k < openDriveRoad.junctions[i].connections[j].links.size(); ++k) {
          junctionData.from_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].from);
          junctionData.to_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].to);
        }

        out_data[junctionID][incommingRoad].emplace_back(junctionData);
      }
    }
  }

  // static void test_fnc_generate_junctions_data(
  //     opendrive::types::OpenDriveData &openDriveRoad,
  //     std::vector<carla::road::lane_junction_t> &out_data) {
  //   for (size_t i = 0; i < openDriveRoad.junctions.size(); ++i) {
  //     for (size_t j = 0; j < openDriveRoad.junctions[i].connections.size(); ++j) {
  //       carla::road::lane_junction_t junctionData;
  //       junctionData.junction_id = openDriveRoad.junctions[i].attributes.id;

  //       const int incommingRoad = openDriveRoad.junctions[i].connections[j].attributes.incoming_road;
  //       const int connectingRoad = openDriveRoad.junctions[i].connections[j].attributes.connecting_road;

  //       junctionData.incomming_road = incommingRoad;
  //       junctionData.connection_road = connectingRoad;

  //       for (size_t k = 0; k < openDriveRoad.junctions[i].connections[j].links.size(); ++k) {
  //         junctionData.from_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].from);
  //         junctionData.to_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].to);
  //       }

  //       out_data.emplace_back(junctionData);
  //     }
  //   }
  // }

  // HACK(Andrei):
  static int fnc_get_first_driving_line(opendrive::types::RoadInformation *roadInfo, int id = 0) {
    if (roadInfo == nullptr) {
      return 0;
    }

    for (auto itSec = roadInfo->lanes.lane_sections.rbegin();
        itSec != roadInfo->lanes.lane_sections.rend() - 1;
        ++itSec) {
      for (auto itLane = itSec->left.begin(); itLane != itSec->left.end(); ++itLane) {
        if (itLane->attributes.type == "driving" && itLane->attributes.id == id) {
          id = itLane->link ? itLane->link->predecessor_id : 0;
          break;
        }
      }
    }

    return id;
  }

  SharedPtr<road::Map> OpenDrive::Load(
      const std::string &file,
      XmlInputType inputType,
      std::string *out_error) {
    carla::opendrive::types::OpenDriveData open_drive_road;

    OpenDriveParser::Parse(file.c_str(), open_drive_road, inputType, out_error);
    carla::road::MapBuilder mapBuilder;

    if (open_drive_road.roads.empty()) {
      // TODO(Andrei): Log some type of warning
      return mapBuilder.Build();
    }

    mapBuilder.SetGeoReference(open_drive_road.geoReference);

    mapBuilder.SetTrafficGroupData(open_drive_road.trafficlightgroups);

    mapBuilder.SetTrafficSignData(open_drive_road.trafficsigns);

    // Generate road and junction information
    using junction_data_t = std::map<int, std::map<int, std::vector<carla::road::lane_junction_t>>>;
    using road_data_t = std::map<int, carla::opendrive::types::RoadInformation *>;

    // std::vector<carla::road::lane_junction_t> junctionInfo;
    // test_fnc_generate_junctions_data(open_drive_road, junctionInfo);
    // mapBuilder.SetJunctionInformation(junctionInfo);

    junction_data_t junctionsData;
    road_data_t roadData;

    fnc_generate_roads_data(open_drive_road, roadData);
    fnc_generate_junctions_data(open_drive_road, junctionsData);

    // Transforma data for the MapBuilder
    for (road_data_t::iterator it = roadData.begin(); it != roadData.end(); ++it) {
      carla::road::element::RoadSegmentDefinition road_segment(it->first);
      carla::road::element::RoadInfoLane *RoadInfoLanes =
          road_segment.MakeInfo<carla::road::element::RoadInfoLane>();

      carla::road::element::RoadGeneralInfo *RoadGeneralInfo =
          road_segment.MakeInfo<carla::road::element::RoadGeneralInfo>();
      RoadGeneralInfo->SetJunctionId(it->second->attributes.junction);

      for (size_t i = 0; i < it->second->lanes.lane_offset.size(); ++i) {
        double s = it->second->lanes.lane_offset[i].s;
        double a = it->second->lanes.lane_offset[i].a;
        RoadGeneralInfo->SetLanesOffset(s, a);
      }

      for (auto &&elevation : it->second->road_profiles.elevation_profile) {
        road_segment.MakeInfo<carla::road::element::RoadElevationInfo>(
            elevation.start_position,
            elevation.start_position,
            elevation.elevation,
            elevation.slope,
            elevation.vertical_curvature,
            elevation.curvature_change);
      }

      std::unordered_map<int, int> left_lanes_go_to_successor, left_lanes_go_to_predecessor;
      // std::unordered_map<int, int> center_lanes_go_to_successor, center_lanes_go_to_predecessor;
      std::unordered_map<int, int> right_lanes_go_to_successor, right_lanes_go_to_predecessor;

      for (auto &&left_lanes : it->second->lanes.lane_sections[0].left) {
        if (left_lanes.link != nullptr) {
          if (left_lanes.attributes.type != "driving") {
            continue;
          }
          if (left_lanes.link->successor_id != 0) {
            left_lanes_go_to_successor[left_lanes.attributes.id] = left_lanes.link->successor_id;
          }
          if (left_lanes.link->predecessor_id != 0) {
            left_lanes_go_to_predecessor[left_lanes.attributes.id] = left_lanes.link->predecessor_id;
          }
        }
      }

      // for (auto &&center_lanes : it->second->lanes.lane_sections[0].center) {
      //   if (center_lanes.link != nullptr) {
      //     if (center_lanes.link->successor_id != 0) {
      //       center_lanes_go_to_successor[center_lanes.attributes.id] = center_lanes.link->successor_id;
      //     }
      //     if (center_lanes.link->predecessor_id != 0) {
      //       center_lanes_go_to_predecessor[center_lanes.attributes.id] = center_lanes.link->predecessor_id;
      //     }
      //   }
      // }

      for (auto &&right_lanes : it->second->lanes.lane_sections[0].right) {
        if (right_lanes.link != nullptr) {
          if (right_lanes.attributes.type != "driving") {
            continue;
          }
          if (right_lanes.link->successor_id != 0) {
            right_lanes_go_to_successor[right_lanes.attributes.id] = right_lanes.link->successor_id;
          }
          if (right_lanes.link->predecessor_id != 0) {
            right_lanes_go_to_predecessor[right_lanes.attributes.id] = right_lanes.link->predecessor_id;
          }
        }
      }

      if (it->second->lanes.lane_sections.size() > 1) {
        for (auto itLaneSec = it->second->lanes.lane_sections.begin() + 1;
            itLaneSec != it->second->lanes.lane_sections.end();
            ++itLaneSec) {
          for (auto &&itLanes : itLaneSec->left) {
            for (auto &&itTest : left_lanes_go_to_successor) {
              if (itTest.second == itLanes.attributes.id && itLanes.link) {
                itTest.second = itLanes.link->successor_id;
              }
            }
          }
        }

        // for (auto itLaneSec = it->second->lanes.lane_sections.begin() + 1;
        //     itLaneSec != it->second->lanes.lane_sections.end();
        //     ++itLaneSec) {
        //   for (auto &&itLanes : itLaneSec->center) {
        //     for (auto &&itTest : center_lanes_go_to_successor) {
        //       if (itTest.second == itLanes.attributes.id && itLanes.link) {
        //         itTest.second = itLanes.link->successor_id;
        //       }
        //     }
        //   }
        // }

        for (auto itLaneSec = it->second->lanes.lane_sections.begin() + 1;
            itLaneSec != it->second->lanes.lane_sections.end();
            ++itLaneSec) {
          for (auto &&itLanes : itLaneSec->right) {
            for (auto &&itTest : right_lanes_go_to_successor) {
              if (itTest.second == itLanes.attributes.id && itLanes.link) {
                itTest.second = itLanes.link->successor_id;
              }
            }
          }
        }
      }

      /////////////////////////////////////////////////////////////////////////

      std::vector<carla::opendrive::types::LaneInfo> *lanesLeft =
          &it->second->lanes.lane_sections.front().left;
      std::vector<carla::opendrive::types::LaneInfo> *lanesRight =
          &it->second->lanes.lane_sections.front().right;

      for (size_t i = 0; i < lanesLeft->size(); ++i) {
        RoadInfoLanes->addLaneInfo(lanesLeft->at(i).attributes.id,
            lanesLeft->at(i).lane_width[0].width,
            lanesLeft->at(i).attributes.type);
      }
      for (size_t i = 0; i < lanesRight->size(); ++i) {
        RoadInfoLanes->addLaneInfo(lanesRight->at(i).attributes.id,
            lanesRight->at(i).lane_width[0].width,
            lanesRight->at(i).attributes.type);
      }

      if (it->second->road_link.successor != nullptr) {
        if (it->second->road_link.successor->element_type == "junction") {
          std::vector<carla::road::lane_junction_t> &options =
              junctionsData[it->second->road_link.successor->id][it->first];
          for (size_t i = 0; i < options.size(); ++i) {
            road_segment.AddSuccessorID(options[i].connection_road, options[i].contact_point == "start");

            for (size_t j = 0; j < options[i].from_lane.size(); ++j) {
              bool is_end = options[i].contact_point == "end";
              int to_lane = options[i].to_lane[j];

              if (is_end) {
                to_lane = fnc_get_first_driving_line(roadData[options[i].connection_road], to_lane);
              }

              road_segment.AddNextLaneInfo(options[i].from_lane[j], to_lane, options[i].connection_road);
            }
          }
        } else {
          bool is_start = it->second->road_link.successor->contact_point == "start";
          road_segment.AddSuccessorID(it->second->road_link.successor->id, is_start);

          for (auto &&lanes : right_lanes_go_to_successor) {
            road_segment.AddNextLaneInfo(lanes.first, lanes.second, it->second->road_link.successor->id);
          }

          // for (auto &&lanes : center_lanes_go_to_successor) {
          //   road_segment.AddNextLaneInfo(lanes.first, lanes.second, it->second->road_link.successor->id);
          // }

          for (auto &&lanes : left_lanes_go_to_successor) {
            road_segment.AddNextLaneInfo(lanes.first, lanes.second, it->second->road_link.successor->id);
          }
        }
      }

      if (it->second->road_link.predecessor != nullptr) {
        if (it->second->road_link.predecessor->element_type == "junction") {
          std::vector<carla::road::lane_junction_t> &options =
              junctionsData[it->second->road_link.predecessor->id][it->first];
          for (size_t i = 0; i < options.size(); ++i) {
            road_segment.AddPredecessorID(options[i].connection_road, options[i].contact_point == "start");

            for (size_t j = 0; j < options[i].from_lane.size(); ++j) {
              bool is_end = options[i].contact_point == "end";
              int to_lane = options[i].to_lane[j];

              if (is_end) {
                to_lane = fnc_get_first_driving_line(roadData[options[i].connection_road], to_lane);
              }

              road_segment.AddPrevLaneInfo(options[i].from_lane[j], to_lane, options[i].connection_road);
            }
          }
        } else {
          bool is_start = it->second->road_link.predecessor->contact_point == "start";
          road_segment.AddPredecessorID(it->second->road_link.predecessor->id, is_start);

          for (auto &&lanes : right_lanes_go_to_predecessor) {
            road_segment.AddPrevLaneInfo(lanes.first, lanes.second, it->second->road_link.predecessor->id);
          }

          for (auto &&lanes : left_lanes_go_to_predecessor) {
            road_segment.AddPrevLaneInfo(lanes.first, lanes.second, it->second->road_link.predecessor->id);
          }
        }
      }

      // Add lane change information
      for (auto &&lane_section : it->second->lanes.lane_sections) {

        // Lambda to convert from string to enum LaneChange
        auto string_2_lane_change_enum = [](auto lane_change)->
            carla::road::element::RoadInfoMarkRecord::LaneChange {
          if (lane_change == "increase") {
            return carla::road::element::RoadInfoMarkRecord::LaneChange::Increase;
          }
          else if (lane_change == "decrease") {
            return carla::road::element::RoadInfoMarkRecord::LaneChange::Decrease;
          }
          else if (lane_change == "both") {
            return carla::road::element::RoadInfoMarkRecord::LaneChange::Both;
          }
          else {
            return carla::road::element::RoadInfoMarkRecord::LaneChange::None;
          }
        };

        double start_position = lane_section.start_position;

        // Create a new RoadInfoMarkRecord for each lane section. Each RoadInfoMarkRecord
        // will contain the actual distance from the start of the RoadSegment

        int current_lane_id = 1;

        for (auto &&lane_section_left : lane_section.left) {
          for (auto &&road_marker : lane_section_left.road_marker) {
              road_segment.MakeInfo<carla::road::element::RoadInfoMarkRecord>(
                start_position + road_marker.soffset,
                current_lane_id,
                road_marker.type,
                road_marker.weigth,
                road_marker.color,
                // See OpenDRIVE Format Specification, Rev. 1.4
                // Doc No.: VI2014.107 (5.3.7.2.1.1.4 Road Mark Record)
                "standard", // material
                road_marker.width,
                string_2_lane_change_enum(road_marker.lane_change),
                // @todo: "carla/opendrive/types.h" must be extended to parse the height
                0.0);
          }
          ++current_lane_id;
        }

        current_lane_id = 0;
        for (auto &&lane_section_center : lane_section.center) { // @todo: .[0]!!
          for (auto &&road_marker : lane_section_center.road_marker) {
              road_segment.MakeInfo<carla::road::element::RoadInfoMarkRecord>(
                start_position + road_marker.soffset,
                current_lane_id,
                road_marker.type,
                road_marker.weigth,
                road_marker.color,
                // See OpenDRIVE Format Specification, Rev. 1.4
                // Doc No.: VI2014.107 (5.3.7.2.1.1.4 Road Mark Record)
                "standard", // material
                road_marker.width,
                string_2_lane_change_enum(road_marker.lane_change),
                // @todo: "carla/opendrive/types.h" must be extended to parse the height
                0.0);
          }
        }

        current_lane_id = -1;
        for (auto &&lane_section_right : lane_section.right) {
          for (auto &&road_marker : lane_section_right.road_marker) {
              road_segment.MakeInfo<carla::road::element::RoadInfoMarkRecord>(
                start_position + road_marker.soffset,
                current_lane_id,
                road_marker.type,
                road_marker.weigth,
                road_marker.color,
                // See OpenDRIVE Format Specification, Rev. 1.4
                // Doc No.: VI2014.107 (5.3.7.2.1.1.4 Road Mark Record)
                "standard", // material
                road_marker.width,
                string_2_lane_change_enum(road_marker.lane_change),
                // @todo: "carla/opendrive/types.h" must be extended to parse the height
                0.0);
          }
          --current_lane_id;
        }
      }

      // Add geometry information
      for (size_t i = 0; i < it->second->geometry_attributes.size(); ++i) {
        geom::Location loc;
        loc.x = it->second->geometry_attributes[i]->start_position_x;
        loc.y = -it->second->geometry_attributes[i]->start_position_y;

        switch (it->second->geometry_attributes[i]->type) {
          case carla::opendrive::types::GeometryType::ARC: {
            carla::opendrive::types::GeometryAttributesArc *arc =
                (carla::opendrive::types::GeometryAttributesArc *) it->second->geometry_attributes[i].get();

            road_segment.MakeGeometry<carla::road::element::GeometryArc>(arc->start_position,
                arc->length,
                -arc->heading,
                loc,
                arc->curvature);

            break;
          }

          case carla::opendrive::types::GeometryType::LINE: {
            carla::opendrive::types::GeometryAttributesLine *line =
                (carla::opendrive::types::GeometryAttributesLine *) it->second->geometry_attributes[i].get();

            road_segment.MakeGeometry<carla::road::element::GeometryLine>(line->start_position,
                line->length,
                -line->heading,
                loc);

            break;
          }

          case carla::opendrive::types::GeometryType::SPIRAL: {
            carla::opendrive::types::GeometryAttributesSpiral *spiral =
                (carla::opendrive::types::GeometryAttributesSpiral *) it->second->geometry_attributes[i].get();

            road_segment.MakeGeometry<carla::road::element::GeometrySpiral>(spiral->start_position,
                spiral->length,
                -spiral->heading,
                loc,
                spiral->curve_start,
                spiral->curve_end);

            break;
          }

          default: {
            break;
          }
        }
      }

      mapBuilder.AddRoadSegmentDefinition(road_segment);
    }

    return mapBuilder.Build();
  }

  SharedPtr<road::Map> OpenDrive::Load(std::istream &input, std::string *out_error) {

    std::string fileContent;
    std::string line;

    while (std::getline(input, line)) {
      fileContent.append(line);
    }

    return Load(fileContent, XmlInputType::CONTENT, out_error);
  }

} // namespace opendrive
} // namespace carla
