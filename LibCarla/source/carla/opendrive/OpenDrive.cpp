#include "OpenDrive.h"
#include "parser/OpenDriveParser.h"

#include "../road/MapBuilder.h"

namespace carla {
namespace opendrive {

#define UNUSED(x) (void) x

  struct lane_junction_t {
    int connection_road = -1;
    int from_lane = 0, to_lane = 0;
  };

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
      std::map<int, std::vector<lane_junction_t>>> &out_data) {
    for (size_t i = 0; i < openDriveRoad.junctions.size(); ++i) {
      for (size_t j = 0; j < openDriveRoad.junctions[i].connections.size(); ++j) {
        lane_junction_t junctionData;
        int junctionID = openDriveRoad.junctions[i].attributes.id;

        int incommingRoad = openDriveRoad.junctions[i].connections[j].attributes.incoming_road;
        int connectingRoad = openDriveRoad.junctions[i].connections[j].attributes.connecting_road;

        junctionData.connection_road = connectingRoad;

        if (openDriveRoad.junctions[i].connections[j].links.size()) {
          junctionData.from_lane = openDriveRoad.junctions[i].connections[j].links[0].from;
          junctionData.to_lane = openDriveRoad.junctions[i].connections[j].links[0].to;
        }

        out_data[junctionID][incommingRoad].push_back(junctionData);
      }
    }
  }

  const road::Map &OpenDrive::Load(const std::string &file) {
    carla::opendrive::types::OpenDriveData open_drive_road;
    OpenDriveParser::Parse(file.c_str(), open_drive_road);
    carla::road::MapBuilder mapBuilder;

    if (open_drive_road.roads.empty()) {
      // TODO(Andrei): Log some type of warning
      return mapBuilder.Build();
    }

    // Generate road and junction information
    using junction_data_t = std::map<int, std::map<int, std::vector<lane_junction_t>>>;
    using road_data_t = std::map<int, carla::opendrive::types::RoadInformation *>;

    junction_data_t junctionsData;
    road_data_t roadData;

    fnc_generate_roads_data(open_drive_road, roadData);
    fnc_generate_junctions_data(open_drive_road, junctionsData);

    // Transforma data for the MapBuilder
    for (road_data_t::iterator it = roadData.begin(); it != roadData.end(); ++it) {
      carla::road::RoadSegmentDefinition roadSegment(it->first);

      if (it->second->road_link.successor != nullptr) {
        if (it->second->road_link.successor->element_type == "junction") {
          std::vector<lane_junction_t> &options =
              junctionsData[it->second->road_link.successor->id][it->first];
          for (size_t i = 0; i < options.size(); ++i) {
            roadSegment.AddSuccessorID(options[i].connection_road);
          }
        } else {
          roadSegment.AddSuccessorID(it->second->road_link.successor->id);
        }

        if (it->second->road_link.predecessor != nullptr) {
          std::vector<lane_junction_t> &options =
              junctionsData[it->second->road_link.predecessor->id][it->first];
          for (size_t i = 0; i < options.size(); ++i) {
            roadSegment.AddPredecessorID(options[i].connection_road);
          }
        }

        for (size_t i = 0; i < it->second->geometry_attributes.size(); ++i) {
          geom::Location loc;
          loc.x = it->second->geometry_attributes[i]->start_position_x;
          loc.y = it->second->geometry_attributes[i]->start_position_y;

          switch (it->second->geometry_attributes[i]->type) {
            case carla::opendrive::types::GeometryType::ARC: {
              carla::opendrive::types::GeometryAttributesArc *arc =
                  (carla::opendrive::types::GeometryAttributesArc *) it->second->geometry_attributes[i].get();

              roadSegment.MakeGeometry<carla::road::GeometryArc>(arc->start_position,
                  arc->length,
                  arc->heading,
                  loc,
                  arc->curvature);

              break;
            }

            case carla::opendrive::types::GeometryType::LINE: {
              carla::opendrive::types::GeometryAttributesLine *line =
                  (carla::opendrive::types::GeometryAttributesLine *) it->second->geometry_attributes[i].get();

              roadSegment.MakeGeometry<carla::road::GeometryLine>(line->start_position,
                  line->length,
                  line->heading,
                  loc);

              break;
            }

            case carla::opendrive::types::GeometryType::SPIRAL: {
              carla::opendrive::types::GeometryAttributesSpiral *spiral =
                  (carla::opendrive::types::GeometryAttributesSpiral *) it->second->geometry_attributes[i].get();

              roadSegment.MakeGeometry<carla::road::GeometrySpiral>(spiral->start_position,
                  spiral->length,
                  spiral->heading,
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
    }

    return mapBuilder.Build();
  }

  const road::Map &OpenDrive::Load(std::istream &input) {
    UNUSED(input);

    carla::road::MapBuilder mapBuilder;
    return mapBuilder.Build();
  }

} // namespace opendrive
} // namespace carla
