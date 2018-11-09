#include "OpenDrive.h"

#include "../road/MapBuilder.h"
#include "carla/Debug.h"

namespace carla {
namespace opendrive {

#define UNUSED(x) (void) x

  static void fnc_generate_roads_data(
      opendrive::types::OpenDriveData &openDriveRoad,
      std::map<int,
      opendrive::types::RoadInformation *> &out_roads) {
    for (size_t i = 0; i < openDriveRoad.roads.size(); ++i) {
      out_roads[openDriveRoad.roads[i].attributes.id] = &openDriveRoad.roads[i];
    }
  }

  static void fnc_generate_junctions_data(
      opendrive::types::OpenDriveData &openDriveRoad,
      std::map<int,
      std::map<int, std::vector<carla::road::lane_junction_t>>> &out_data) {
    for (size_t i = 0; i < openDriveRoad.junctions.size(); ++i) {
      for (size_t j = 0; j < openDriveRoad.junctions[i].connections.size(); ++j) {
        carla::road::lane_junction_t junctionData;
        int junctionID = openDriveRoad.junctions[i].attributes.id;

        int incommingRoad = openDriveRoad.junctions[i].connections[j].attributes.incoming_road;
        int connectingRoad = openDriveRoad.junctions[i].connections[j].attributes.connecting_road;

        junctionData.incomming_road = incommingRoad;
        junctionData.connection_road = connectingRoad;
        junctionData.contact_point = openDriveRoad.junctions[i].connections[j].attributes.contact_point;

        for(size_t k = 0; k < openDriveRoad.junctions[i].connections[j].links.size(); ++k) {
          junctionData.from_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].from);
          junctionData.to_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].to);
        }

        out_data[junctionID][incommingRoad].emplace_back(junctionData);
      }
    }
  }

  static void test_fnc_generate_junctions_data(
      opendrive::types::OpenDriveData &openDriveRoad,
      std::vector<carla::road::lane_junction_t> &out_data) {
    for (size_t i = 0; i < openDriveRoad.junctions.size(); ++i) {
      for (size_t j = 0; j < openDriveRoad.junctions[i].connections.size(); ++j) {
        carla::road::lane_junction_t junctionData;
        junctionData.junction_id = openDriveRoad.junctions[i].attributes.id;

        int incommingRoad = openDriveRoad.junctions[i].connections[j].attributes.incoming_road;
        int connectingRoad = openDriveRoad.junctions[i].connections[j].attributes.connecting_road;

        junctionData.incomming_road = incommingRoad;
        junctionData.connection_road = connectingRoad;

        for(size_t k = 0; k < openDriveRoad.junctions[i].connections[j].links.size(); ++k) {
          junctionData.from_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].from);
          junctionData.to_lane.emplace_back(openDriveRoad.junctions[i].connections[j].links[k].to);
        }

         out_data.emplace_back(junctionData);
      }
    }
  }

  // HACK(Andrei):
  static int fnc_get_first_driving_line(opendrive::types::RoadInformation *roadInfo) {
    if(roadInfo == nullptr) {
      printf("In function %s(%d) roadInfo is NULL\n", __FUNCTION__, __LINE__);
      return 0;
    }

    for(auto &&lane : roadInfo->lanes.lane_sections[0].left) {
      if(lane.attributes.type == "driving") {
        return lane.attributes.id;
      }
    }

    printf("In function %s(%d) no left ID found\n", __FUNCTION__, __LINE__);
    return 0;
  }

  SharedPtr<road::Map> OpenDrive::Load(const std::string &file, XmlInputType inputType, std::string *out_error) {
    carla::opendrive::types::OpenDriveData open_drive_road;

    OpenDriveParser::Parse(file.c_str(), open_drive_road, inputType, out_error);
    carla::road::MapBuilder mapBuilder;

    if (open_drive_road.roads.empty()) {
      // TODO(Andrei): Log some type of warning
      return mapBuilder.Build();
    }

    // Generate road and junction information
    using junction_data_t = std::map<int, std::map<int, std::vector<carla::road::lane_junction_t>>>;
    using road_data_t = std::map<int, carla::opendrive::types::RoadInformation *>;

    std::vector<carla::road::lane_junction_t> junctionInfo;
    test_fnc_generate_junctions_data(open_drive_road, junctionInfo);
    mapBuilder.SetJunctionInformation(junctionInfo);

    junction_data_t junctionsData;
    road_data_t roadData;

    fnc_generate_roads_data(open_drive_road, roadData);
    fnc_generate_junctions_data(open_drive_road, junctionsData);

    // Transforma data for the MapBuilder
    for (road_data_t::iterator it = roadData.begin(); it != roadData.end(); ++it) {
      carla::road::element::RoadSegmentDefinition roadSegment(it->first);
      carla::road::element::RoadInfoLane *roadInfoLanes = roadSegment.MakeInfo<carla::road::element::RoadInfoLane>();

      carla::road::element::RoadGeneralInfo *roadGeneralInfo = roadSegment.MakeInfo<carla::road::element::RoadGeneralInfo>();
      roadGeneralInfo->SetJunctionId(it->second->attributes.junction);

      for(size_t i = 0; i < it->second->lanes.lane_offset.size(); ++i) {
        double s = it->second->lanes.lane_offset[i].s;
        double a = it->second->lanes.lane_offset[i].a;
        roadGeneralInfo->SetLanesOffset(s, a);
      }


      /////////////////////////////////////////////////////////////////////////

      std::map<int, int> leftLanesGoToSuccessor, leftLanesGoToPredecessor;
      std::map<int, int> rightLanesGoToSuccessor, rightLanesGoToPredecessor;

      for(auto &&leftlanes : it->second->lanes.lane_sections[0].left) {
        if(leftlanes.link != nullptr) {
          if(leftlanes.attributes.type != "driving") continue;
          if(leftlanes.link->successor_id != 0) leftLanesGoToSuccessor[leftlanes.attributes.id] = leftlanes.link->successor_id;
          if(leftlanes.link->predecessor_id != 0) leftLanesGoToPredecessor[leftlanes.attributes.id] = leftlanes.link->predecessor_id;
        }
      }

      for(auto &&rightlanes : it->second->lanes.lane_sections[0].right) {
        if(rightlanes.link != nullptr) {
          if(rightlanes.attributes.type != "driving") continue;
          if(rightlanes.link->successor_id != 0) rightLanesGoToSuccessor[rightlanes.attributes.id] = rightlanes.link->successor_id;
          if(rightlanes.link->predecessor_id != 0) rightLanesGoToPredecessor[rightlanes.attributes.id] = rightlanes.link->predecessor_id;
        }
      }

      if(it->second->lanes.lane_sections.size() > 1) {
        for(auto itLaneSec = it->second->lanes.lane_sections.begin() + 1; itLaneSec != it->second->lanes.lane_sections.end(); ++itLaneSec) {
          for(auto &&itLanes : itLaneSec->left) {
            for(auto &&itTest : leftLanesGoToSuccessor) {
              if(itTest.second == itLanes.attributes.id && itLanes.link) {
                itTest.second = itLanes.link->successor_id;
              }
            }
            for(auto &&itTest : leftLanesGoToPredecessor) {
              if(itTest.second == itLanes.attributes.id && itLanes.link) {
                itTest.second = itLanes.link->predecessor_id;
              }
            }
          }
        }

        for(auto itLaneSec = it->second->lanes.lane_sections.begin() + 1; itLaneSec != it->second->lanes.lane_sections.end(); ++itLaneSec) {
          for(auto &&itLanes : itLaneSec->right) {
            for(auto &&itTest : rightLanesGoToSuccessor) {
              if(itTest.second == itLanes.attributes.id && itLanes.link) {
                itTest.second = itLanes.link->successor_id;
              }
            }
            for(auto &&itTest : rightLanesGoToPredecessor) {
              if(itTest.second == itLanes.attributes.id && itLanes.link) {
                itTest.second = itLanes.link->predecessor_id;
              }
            }
          }
        }
      }


      /////////////////////////////////////////////////////////////////////////

      std::vector<carla::opendrive::types::LaneInfo> *lanesLeft = &it->second->lanes.lane_sections.front().left;
      std::vector<carla::opendrive::types::LaneInfo> *lanesRight = &it->second->lanes.lane_sections.front().right;

      for(size_t i = 0; i < lanesLeft->size(); ++i) {
        roadInfoLanes->addLaneInfo(lanesLeft->at(i).attributes.id, lanesLeft->at(i).lane_width[0].width, lanesLeft->at(i).attributes.type);
      }
      for(size_t i = 0; i < lanesRight->size(); ++i) {
        roadInfoLanes->addLaneInfo(lanesRight->at(i).attributes.id, lanesRight->at(i).lane_width[0].width, lanesRight->at(i).attributes.type);
      }

      if (it->second->road_link.successor != nullptr) {
        if (it->second->road_link.successor->element_type == "junction") {
          std::vector<carla::road::lane_junction_t> &options =
            junctionsData[it->second->road_link.successor->id][it->first];
          for (size_t i = 0; i < options.size(); ++i) {
            roadSegment.AddSuccessorID(options[i].connection_road, options[i].contact_point == "start");

            for(size_t j = 0; j < options[i].from_lane.size(); ++j) {
              bool is_end = options[i].contact_point == "end";
              int to_lane = options[i].to_lane[j];

              if(is_end && to_lane < 0) {
                printf("TEST ERROR\nFromLane: %d\nToLane: %d\nRoadID: %d\n",
                options[i].from_lane[j],
                to_lane,
                options[i].connection_road
                );
              }

              if(is_end) {
                to_lane = fnc_get_first_driving_line(roadData[options[i].connection_road]);
              }

              roadSegment.AddNextLaneInfo(options[i].from_lane[j], to_lane, options[i].connection_road);
              printf("[suc][% 5d -> % 5d]    [% 3d -> % 3d]\n", it->first, options[i].connection_road, options[i].from_lane[j], to_lane);
            }
          }
        } else {
          bool is_start = it->second->road_link.successor->contact_point == "start";
          roadSegment.AddSuccessorID(it->second->road_link.successor->id, is_start);

          for(auto &&lanes : rightLanesGoToSuccessor) {
            printf("[suc][% 5d -> % 5d]    [% 3d -> % 3d]\n", it->first, it->second->road_link.successor->id, lanes.first, lanes.second);
            roadSegment.AddNextLaneInfo(lanes.first, lanes.second, it->second->road_link.successor->id);
          }

          for(auto &&lanes : leftLanesGoToSuccessor) {
            printf("[suc][% 5d -> % 5d]    [% 3d -> % 3d]\n", it->first, it->second->road_link.successor->id, lanes.first, lanes.second);
            roadSegment.AddNextLaneInfo(lanes.first, lanes.second, it->second->road_link.successor->id);
          }
        }
      }

      if (it->second->road_link.predecessor != nullptr) {
        if (it->second->road_link.predecessor->element_type == "junction") {
          std::vector<carla::road::lane_junction_t> &options =
            junctionsData[it->second->road_link.predecessor->id][it->first];
          for (size_t i = 0; i < options.size(); ++i) {
            roadSegment.AddPredecessorID(options[i].connection_road, options[i].contact_point == "start");

            for(size_t j = 0; j < options[i].from_lane.size(); ++j) {
              bool is_end = options[i].contact_point == "end";
              int to_lane = options[i].to_lane[j];

              if(is_end && to_lane < 0) {
                printf("TEST ERROR\nFromLane: %d\nToLane: %d\nRoadID: %d\n",
                options[i].from_lane[j],
                to_lane,
                options[i].connection_road
                );
              }

              if(is_end) {
                to_lane = fnc_get_first_driving_line(roadData[options[i].connection_road]);
              }

              roadSegment.AddPrevLaneInfo(options[i].from_lane[j], to_lane, options[i].connection_road);
              printf("[pre][% 5d -> % 5d]    [% 3d -> % 3d]\n", it->first, options[i].connection_road, options[i].from_lane[j], to_lane);
            }
          }
        } else {
          bool is_start = it->second->road_link.predecessor->contact_point == "start";
          roadSegment.AddPredecessorID(it->second->road_link.predecessor->id, is_start);

          for(auto &&lanes : rightLanesGoToPredecessor) {
            printf("[pre][% 5d -> % 5d]    [% 3d -> % 3d]\n", it->first, it->second->road_link.predecessor->id, lanes.first, lanes.second);
            roadSegment.AddPrevLaneInfo(lanes.first, lanes.second, it->second->road_link.predecessor->id);
          }

          for(auto &&lanes : leftLanesGoToPredecessor) {
            printf("[pre][% 5d -> % 5d]    [% 3d -> % 3d]\n", it->first, it->second->road_link.predecessor->id, lanes.first, lanes.second);
            roadSegment.AddPrevLaneInfo(lanes.first, lanes.second, it->second->road_link.predecessor->id);
          }
        }
      }

      for (size_t i = 0; i < it->second->geometry_attributes.size(); ++i) {
        geom::Location loc;
        loc.x = it->second->geometry_attributes[i]->start_position_x;
        loc.y = -it->second->geometry_attributes[i]->start_position_y;

        switch (it->second->geometry_attributes[i]->type) {
          case carla::opendrive::types::GeometryType::ARC: {
            carla::opendrive::types::GeometryAttributesArc *arc =
                (carla::opendrive::types::GeometryAttributesArc *) it->second->geometry_attributes[i].get();

            roadSegment.MakeGeometry<carla::road::element::GeometryArc>(arc->start_position,
                arc->length,
                -arc->heading,
                loc,
                arc->curvature);

            break;
          }

          case carla::opendrive::types::GeometryType::LINE: {
            carla::opendrive::types::GeometryAttributesLine *line =
                (carla::opendrive::types::GeometryAttributesLine *) it->second->geometry_attributes[i].get();

            roadSegment.MakeGeometry<carla::road::element::GeometryLine>(line->start_position,
                line->length,
                -line->heading,
                loc);

            break;
          }

          case carla::opendrive::types::GeometryType::SPIRAL: {
            carla::opendrive::types::GeometryAttributesSpiral *spiral =
                (carla::opendrive::types::GeometryAttributesSpiral *) it->second->geometry_attributes[i].get();

            roadSegment.MakeGeometry<carla::road::element::GeometrySpiral>(spiral->start_position,
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

      mapBuilder.AddRoadSegmentDefinition(roadSegment);
    }

    return mapBuilder.Build();
  }

  SharedPtr<road::Map> OpenDrive::Load(std::istream &input, std::string *out_error) {

    std::string fileContent;
    std::string line;

    while(std::getline(input, line))
    {
      fileContent.append(line);
    }

    return Load(fileContent, XmlInputType::CONTENT, out_error);
  }

} // namespace opendrive
} // namespace carla
