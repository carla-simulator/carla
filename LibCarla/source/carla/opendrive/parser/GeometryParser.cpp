// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "GeometryParser.h"

#include <cassert>

#define ODP_ASSERT(x, ...)  assert(x)
#define ODP_UNUSED(x)       (void) (x)

void carla::opendrive::parser::GeometryParser::ParseArc(
    const pugi::xml_node &xmlNode,
    carla::opendrive::types::GeometryAttributesArc *out_geometry_arc) {
  out_geometry_arc->type = opendrive::types::GeometryType::ARC;
  out_geometry_arc->curvature = std::stod(xmlNode.attribute("curvature").value());
}

void carla::opendrive::parser::GeometryParser::ParseLine(
    const pugi::xml_node &xmlNode,
    carla::opendrive::types::GeometryAttributesLine *out_geometry_line) {
  ODP_UNUSED(xmlNode);
  out_geometry_line->type = opendrive::types::GeometryType::LINE;
}

void carla::opendrive::parser::GeometryParser::ParseSpiral(
    const pugi::xml_node &xmlNode,
    carla::opendrive::types::GeometryAttributesSpiral *out_geometry_spiral) {
  out_geometry_spiral->type = opendrive::types::GeometryType::SPIRAL;
  out_geometry_spiral->curve_end = std::stod(xmlNode.attribute("curvEnd").value());
  out_geometry_spiral->curve_start = std::stod(xmlNode.attribute("curvStart").value());
}

void carla::opendrive::parser::GeometryParser::Parse(
    const pugi::xml_node &xmlNode,
    std::vector<std::unique_ptr<carla::opendrive::types::GeometryAttributes>> &out_geometry_attributes) {
  carla::opendrive::parser::GeometryParser gometry_parser;

  for (pugi::xml_node roadGeometry = xmlNode.child("geometry");
      roadGeometry;
      roadGeometry = roadGeometry.next_sibling("geometry")) {
    std::unique_ptr<opendrive::types::GeometryAttributes> geometry_attributes;
    std::string firstChildName(roadGeometry.first_child().name());

    if (firstChildName == "arc") {
      geometry_attributes = std::make_unique<opendrive::types::GeometryAttributesArc>();
      gometry_parser.ParseArc(roadGeometry.first_child(),
          static_cast<opendrive::types::GeometryAttributesArc *>(geometry_attributes.get()));
    } else if (firstChildName == "line") {
      geometry_attributes = std::make_unique<opendrive::types::GeometryAttributesLine>();
      gometry_parser.ParseLine(roadGeometry.first_child(),
          static_cast<opendrive::types::GeometryAttributesLine *>(geometry_attributes.get()));
    } else if (firstChildName == "spiral") {
      geometry_attributes = std::make_unique<opendrive::types::GeometryAttributesSpiral>();
      gometry_parser.ParseSpiral(roadGeometry.first_child(),
          static_cast<opendrive::types::GeometryAttributesSpiral *>(geometry_attributes.get()));
    } else {
      ODP_ASSERT(false, "Geometry type unknown");
    }

    geometry_attributes->start_position = std::stod(roadGeometry.attribute("s").value());

    geometry_attributes->start_position_x = std::stod(roadGeometry.attribute("x").value());
    geometry_attributes->start_position_y = std::stod(roadGeometry.attribute("y").value());

    geometry_attributes->heading = std::stod(roadGeometry.attribute("hdg").value());
    geometry_attributes->length = std::stod(roadGeometry.attribute("length").value());

    out_geometry_attributes.emplace_back(std::move(geometry_attributes));
  }
}
