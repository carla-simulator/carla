// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LaneParser.h"

namespace carla {
namespace opendrive {
namespace parser {

  void LaneParser::ParseLane(
      const pugi::xml_node &xmlNode,
      std::vector<types::LaneInfo> &out_lane) {
    for (pugi::xml_node lane = xmlNode.child("lane"); lane; lane = lane.next_sibling("lane")) {
      types::LaneInfo currentLane;

      currentLane.attributes.type = lane.attribute("type").value();
      currentLane.attributes.level = lane.attribute("level").value();
      currentLane.attributes.id = std::atoi(lane.attribute("id").value());

      ParseLaneSpeed(lane, currentLane.lane_speed);
      ParseLaneWidth(lane, currentLane.lane_width);

      ParseLaneLink(lane.child("link"), currentLane.link);
      ParseLaneRoadMark(lane, currentLane.road_marker);

      out_lane.emplace_back(std::move(currentLane));
    }
  }

  void LaneParser::ParseLaneWidth(
      const pugi::xml_node &xmlNode,
      std::vector<types::LaneWidth> &out_lane_width) {
    for (pugi::xml_node laneWidth = xmlNode.child("width");
        laneWidth;
        laneWidth = laneWidth.next_sibling("width")) {
      types::LaneWidth laneWidthInfo;

      laneWidthInfo.soffset = std::stod(laneWidth.attribute("sOffset").value());

      laneWidthInfo.width = std::stod(laneWidth.attribute("a").value());
      laneWidthInfo.slope = std::stod(laneWidth.attribute("b").value());

      laneWidthInfo.vertical_curvature = std::stod(laneWidth.attribute("c").value());
      laneWidthInfo.curvature_change = std::stod(laneWidth.attribute("d").value());

      out_lane_width.emplace_back(laneWidthInfo);
    }
  }

  void LaneParser::ParseLaneLink(
      const pugi::xml_node &xmlNode,
      std::unique_ptr<types::LaneLink> &out_lane_link) {
    const pugi::xml_node predecessorNode = xmlNode.child("predecessor");
    const pugi::xml_node successorNode = xmlNode.child("successor");

    out_lane_link =
        (predecessorNode || successorNode) ? std::make_unique<opendrive::types::LaneLink>() : nullptr;
    if (out_lane_link == nullptr) {
      return;
    }

    out_lane_link->predecessor_id = predecessorNode ? std::atoi(predecessorNode.attribute("id").value()) : 0;
    out_lane_link->successor_id = successorNode ? std::atoi(successorNode.attribute("id").value()) : 0;
  }

  void LaneParser::ParseLaneOffset(
      const pugi::xml_node &xmlNode,
      std::vector<types::LaneOffset> &out_lane_offset) {
    types::LaneOffset lanesOffset;

    lanesOffset.s = std::stod(xmlNode.attribute("s").value());
    lanesOffset.a = std::stod(xmlNode.attribute("a").value());
    lanesOffset.b = std::stod(xmlNode.attribute("b").value());
    lanesOffset.c = std::stod(xmlNode.attribute("c").value());
    lanesOffset.d = std::stod(xmlNode.attribute("d").value());

    out_lane_offset.emplace_back(lanesOffset);
  }

  void LaneParser::ParseLaneRoadMark(
      const pugi::xml_node &xmlNode,
      std::vector<types::LaneRoadMark> &out_lane_mark) {

    for (pugi::xml_node road_mark = xmlNode.child("roadMark");
         road_mark; road_mark = road_mark.next_sibling("roadMark")) {
      types::LaneRoadMark roadMarker;

      if (road_mark.attribute("sOffset") != nullptr) {
        roadMarker.soffset = std::stod(road_mark.attribute("sOffset").value());
      }

      if (road_mark.attribute("width") != nullptr) {
        roadMarker.width = std::stod(road_mark.attribute("width").value());
      }

      if (road_mark.attribute("type") != nullptr) {
        roadMarker.type = road_mark.attribute("type").value();
      }

      if (road_mark.attribute("weight") != nullptr) {
        roadMarker.weigth = road_mark.attribute("weight").value();
      }

      if (road_mark.attribute("material") != nullptr) {
        roadMarker.color = road_mark.attribute("material").value();
      }

      if (road_mark.attribute("color") != nullptr) {
        roadMarker.color = road_mark.attribute("color").value();
      }

      if (road_mark.attribute("laneChange") != nullptr) {
        roadMarker.lane_change = road_mark.attribute("laneChange").value();
      }

      out_lane_mark.emplace_back(roadMarker);
    }
  }

  void LaneParser::ParseLaneSpeed(
      const pugi::xml_node &xmlNode,
      std::vector<types::LaneSpeed> &out_lane_speed) {
    for (pugi::xml_node laneSpeed = xmlNode.child("speed");
        laneSpeed;
        laneSpeed = laneSpeed.next_sibling("speed")) {
      types::LaneSpeed lane_speed = { 0.0, 0.0 };

      lane_speed.soffset = std::stod(laneSpeed.attribute("sOffset").value());
      lane_speed.max_speed = std::stod(laneSpeed.attribute("max").value());

      out_lane_speed.emplace_back(lane_speed);
    }
  }

  void LaneParser::Parse(
      const pugi::xml_node &xmlNode,
      types::Lanes &out_lanes) {
    LaneParser laneParser;

    for (pugi::xml_node laneSection = xmlNode.child("laneOffset");
        laneSection;
        laneSection = laneSection.next_sibling("laneOffset")) {
      laneParser.ParseLaneOffset(laneSection, out_lanes.lane_offset);
    }

    for (pugi::xml_node laneSection = xmlNode.child("laneSection");
        laneSection;
        laneSection = laneSection.next_sibling("laneSection")) {
      types::LaneSection laneSec;
      laneSec.start_position = std::stod(laneSection.attribute("s").value());

      pugi::xml_node lane = laneSection.child("left");
      laneParser.ParseLane(lane, laneSec.left);

      lane = laneSection.child("center");
      laneParser.ParseLane(lane, laneSec.center);

      lane = laneSection.child("right");
      laneParser.ParseLane(lane, laneSec.right);

      out_lanes.lane_sections.emplace_back(std::move(laneSec));
    }
  }

} // parser
} // opendrive
} // carla
