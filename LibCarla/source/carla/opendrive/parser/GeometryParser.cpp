// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/GeometryParser.h"

#include "carla/road/MapBuilder.h"

#include <third-party/pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  using RoadId = road::RoadId;

  struct GeometryArc {
    double curvature  { 0.0 };
  };

  struct GeometrySpiral {
    double curvStart  { 0.0 };
    double curvEnd    { 0.0 };
  };

  struct GeometryPoly3 {
    double a    { 0.0 };
    double b    { 0.0 };
    double c    { 0.0 };
    double d    { 0.0 };
  };

  struct GeometryParamPoly3 {
    double aU             { 0.0 };
    double bU             { 0.0 };
    double cU             { 0.0 };
    double dU             { 0.0 };
    double aV             { 0.0 };
    double bV             { 0.0 };
    double cV             { 0.0 };
    double dV             { 0.0 };
    std::string p_range   { "arcLength" };
  };

  struct Geometry {
    RoadId road_id      { 0u };
    double s            { 0.0 };
    double x            { 0.0 };
    double y            { 0.0 };
    double hdg          { 0.0 };
    double length       { 0.0 };
    std::string type    { "line" };
    GeometryArc arc;
    GeometrySpiral spiral;
    GeometryPoly3 poly3;
    GeometryParamPoly3 param_poly3;
  };

  void GeometryParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<Geometry> geometry;

    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {

      // parse plan view
      pugi::xml_node node_plan_view = node_road.child("planView");
      if (node_plan_view) {
        // all geometry
        for (pugi::xml_node node_geo : node_plan_view.children("geometry")) {
          Geometry geo;

          // get road id
          geo.road_id = node_road.attribute("id").as_uint();

          // get common properties
          geo.s = node_geo.attribute("s").as_double();
          geo.x = node_geo.attribute("x").as_double();
          geo.y = node_geo.attribute("y").as_double();
          geo.hdg = node_geo.attribute("hdg").as_double();
          geo.length = node_geo.attribute("length").as_double();

          // check geometry type
          pugi::xml_node node = node_geo.first_child();
          geo.type = node.name();
          if (geo.type == "arc") {
            geo.arc.curvature = node.attribute("curvature").as_double();
          } else if (geo.type == "spiral") {
            geo.spiral.curvStart = node.attribute("curvStart").as_double();
            geo.spiral.curvEnd = node.attribute("curvEnd").as_double();
          } else if (geo.type == "poly3") {
            geo.poly3.a = node.attribute("a").as_double();
            geo.poly3.b = node.attribute("b").as_double();
            geo.poly3.c = node.attribute("c").as_double();
            geo.poly3.d = node.attribute("d").as_double();
          } else if (geo.type == "paramPoly3") {
            geo.param_poly3.aU = node.attribute("aU").as_double();
            geo.param_poly3.bU = node.attribute("bU").as_double();
            geo.param_poly3.cU = node.attribute("cU").as_double();
            geo.param_poly3.dU = node.attribute("dU").as_double();
            geo.param_poly3.aV = node.attribute("aV").as_double();
            geo.param_poly3.bV = node.attribute("bV").as_double();
            geo.param_poly3.cV = node.attribute("cV").as_double();
            geo.param_poly3.dV = node.attribute("dV").as_double();
            geo.param_poly3.p_range = node.attribute("pRange").value();
          }

          // add it
          geometry.emplace_back(geo);
        }
      }
    }

    // map_builder calls
    for (const auto& geo : geometry) {
      carla::road::Road *road = map_builder.GetRoad(geo.road_id);
      if (geo.type == "line") {
        map_builder.AddRoadGeometryLine(road, geo.s, geo.x, geo.y, geo.hdg, geo.length);
      } else if (geo.type == "arc") {
        map_builder.AddRoadGeometryArc(road, geo.s, geo.x, geo.y, geo.hdg, geo.length, geo.arc.curvature);
      } else if (geo.type == "spiral") {
        map_builder.AddRoadGeometrySpiral(road,
            geo.s,
            geo.x,
            geo.y,
            geo.hdg,
            geo.length,
            geo.spiral.curvStart,
            geo.spiral.curvEnd);
      } else if (geo.type == "poly3") {
        map_builder.AddRoadGeometryPoly3(road,
            geo.s,
            geo.x,
            geo.y,
            geo.hdg,
            geo.length,
            geo.poly3.a,
            geo.poly3.b,
            geo.poly3.c,
            geo.poly3.d);
      } else if (geo.type == "paramPoly3") {
        map_builder.AddRoadGeometryParamPoly3(road,
            geo.s,
            geo.x,
            geo.y,
            geo.hdg,
            geo.length,
            geo.param_poly3.aU,
            geo.param_poly3.bU,
            geo.param_poly3.cU,
            geo.param_poly3.dU,
            geo.param_poly3.aV,
            geo.param_poly3.bV,
            geo.param_poly3.cV,
            geo.param_poly3.dV,
            geo.param_poly3.p_range);
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
