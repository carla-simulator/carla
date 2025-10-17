// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/GeoReferenceParser.h"

#include "carla/Logging.h"
#include "carla/StringUtil.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/GeoProjection.h"
#include "carla/road/MapBuilder.h"
#include "carla/geom/Math.h"

#include <pugixml/pugixml.hpp>

#include <string>
#include <vector>
#include <regex>

namespace carla {
namespace opendrive {
namespace parser {

  static std::string ToLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return char(std::tolower(c)); });
    return s;
  }

  static std::unordered_map<std::string, std::string> ParseProjectionParameters(const std::string &s){

    // +key[=value]; value can be "quoted" or 'quoted' or an unquoted token (no spaces)
    // This also captures bare flags like +south
    std::unordered_map<std::string, std::string> geo_parameters_map;

    static const std::regex re(R"(\+([A-Za-z0-9_]+)(?:=("[^"]*"|'[^']*'|[^ \t\r\n+]+))?)");
    for (std::sregex_iterator it(s.begin(), s.end(), re), end; it != end; ++it) {
        std::string key = (*it)[1].str();
        std::string val = (*it)[2].str();
        geo_parameters_map[key] = val;
    }

    return geo_parameters_map;
  }

  static geom::Ellipsoid CreateEllipsoid(std::unordered_map<std::string, std::string> parameters){
    geom::Ellipsoid ellps;

    if (parameters.find("ellps") != parameters.end()) {
      auto value = ToLowerCase(parameters["ellps"]);
      auto it = geom::custom_ellipsoids.find(value);
      if (it != geom::custom_ellipsoids.end()) {
        ellps.a = it->second.first;
        ellps.f_inv = it->second.second;
      } else {
        auto val = geom::custom_ellipsoids.find("wgs84");
        ellps.a = val->second.first;
        ellps.f_inv = val->second.second;
      }
    }

    // Specific semi-major axis
    if (parameters.find("a") != parameters.end()) {
      ellps.a = std::stod(parameters["a"]);
    }

    // Specific semi-minor axis
    if (parameters.find("b") != parameters.end()) {
      ellps.fromb(std::stod(parameters["b"]));
    } else if (parameters.find("f") != parameters.end()) {
      ellps.fromf(std::stod(parameters["f"]));
    } else if (parameters.find("rf") != parameters.end()) {
      ellps.f_inv = std::stod(parameters["rf"]);
    }

    return ellps;
  }

  static geom::GeoProjection CreateTransverseMercatorProjection(
    std::unordered_map<std::string, std::string> parameters,
    std::string proj_string,
    geom::Ellipsoid ellipsoid){

    geom::TransverseMercatorParams p;
    if (parameters.find("lat_0") != parameters.end()) {
      p.lat_0 = std::stod(parameters["lat_0"]);
    }
    if (parameters.find("lon_0") != parameters.end()) {
      p.lon_0 = std::stod(parameters["lon_0"]);
    }
    if (parameters.find("k") != parameters.end()) {
      p.k = std::stod(parameters["k"]);
    }
    if (parameters.find("lat_0") != parameters.end()) {
      p.x_0 = std::stod(parameters["x_0"]);
    }
    if (parameters.find("lat_0") != parameters.end()) {
      p.y_0 = std::stod(parameters["y_0"]);
    }
    p.ellps = ellipsoid;
    auto projection = geom::GeoProjection::Make(p);
    projection.setPROJString(proj_string);
    return projection;
  }

  static geom::GeoProjection CreateUniversalTransverseMercatorProjection(
    std::unordered_map<std::string, std::string> parameters,
    std::string proj_string,
    geom::Ellipsoid ellipsoid){

    geom::UniversalTransverseMercatorParams p;
    if (parameters.find("zone") != parameters.end()) {
      p.zone = std::stoi(parameters["zone"]);
    } else {
      log_warning("Missing 'zone' parameter for UTM projection. Using default value " + std::to_string(p.zone));
    }
    p.north = (parameters.count("south") > 0) ? false : true;
    p.ellps = ellipsoid;
    auto projection = geom::GeoProjection::Make(p);
    projection.setPROJString(proj_string);
    return projection;
  }

  static geom::GeoProjection CreateWebMercatorProjection(
    std::string proj_string,
    geom::Ellipsoid ellipsoid){

    // Parameters are fixed.
    geom::WebMercatorParams p;
    p.ellps = ellipsoid;
    auto projection = geom::GeoProjection::Make(p);
    projection.setPROJString(proj_string);
    return projection;
  }

  static geom::GeoProjection CreateLambertConformalConicProjection(
    std::unordered_map<std::string, std::string> parameters,
    std::string proj_string,
    geom::Ellipsoid ellipsoid){

    geom::LambertConformalConicParams p;

    if (parameters.find("lon_0") != parameters.end()) {
      p.lon_0 = std::stod(parameters["lon_0"]);
    }
    if (parameters.find("lat_0") != parameters.end()) {
      p.lat_0 = std::stod(parameters["lat_0"]);
    }
    if (parameters.find("lat_1") != parameters.end()) {
      p.lat_1 = std::stod(parameters["lat_1"]);
    } else {
      log_warning("Missing 'lat_1' parameter for LCC projection. Using default value " + std::to_string(p.lat_1));
    }
    if (parameters.find("lat_2") != parameters.end()) {
      p.lat_2 = std::stod(parameters["lat_2"]);
    } else {
      log_warning("Missing 'lat_2' parameter for LCC projection. Using default value " + std::to_string(p.lat_2));
    }
    if (parameters.find("x_0") != parameters.end()) {
      p.x_0 = std::stod(parameters["x_0"]);
    }
    if (parameters.find("y_0") != parameters.end()) {
      p.y_0 = std::stod(parameters["y_0"]);
    }

    p.ellps = ellipsoid;
    auto projection = geom::GeoProjection::Make(p);
    projection.setPROJString(proj_string);
    return projection;
  }

  // TransverseMercator projection with default parameters.
  static geom::GeoProjection CreateDefaultProjection(geom::Ellipsoid ellipsoid){
    geom::TransverseMercatorParams p;

    p.ellps = ellipsoid;
    auto projection = geom::GeoProjection::Make(p);
    return projection;
  }

  static geom::GeoLocation CreateTransverseMercatorGeoReference(std::unordered_map<std::string, std::string> parameters){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    if (parameters.find("lat_0") != parameters.end()) {
      result.latitude = std::stod(parameters["lat_0"]);
    }
    if (parameters.find("lon_0") != parameters.end()) {
      result.longitude = std::stod(parameters["lon_0"]);
    }
    return result;
  }

  static geom::GeoLocation CreateUniversalTransverseMercatorGeoReference(std::unordered_map<std::string, std::string> parameters){
    geom::GeoLocation result{0.0, 0.0, 0.0};

    result.latitude = 0.0;
    if (parameters.find("zone") != parameters.end()) {
      double zone = std::stod(parameters["zone"]);
      result.longitude = geom::Math::ToRadians(6 * zone - 183);
    }
    return result;
  }

  static geom::GeoLocation CreateWebMercatorGeoReference(){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    return result;
  }

  static geom::GeoLocation CreateLamberConic2SPGeoReference(std::unordered_map<std::string, std::string> parameters){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    if (parameters.find("lat_0") != parameters.end()) {
      result.latitude = std::stod(parameters["lat_0"]);
    }
    if (parameters.find("lon_0") != parameters.end()) {
      result.longitude = std::stod(parameters["lon_0"]);
    }
    return result;
  }

  static geom::GeoLocation CreateDefaultGeoReference(){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    return result;
  }

  static geom::GeoProjection ParseGeoProjection(const std::string &proj_string) {
    geom::GeoProjection result;

    auto parameters = ParseProjectionParameters(proj_string);
    auto ellipsoid = CreateEllipsoid(parameters);

    // Get the projection type
    std::string proj;
    if (parameters.find("proj") != parameters.end()) {
      proj = parameters["proj"];
    } else {
      log_warning("cannot find the type of projection, using default transverse mercator");
      return CreateDefaultProjection(ellipsoid);
    }

    // Parse the parameters
    if (proj == "tmerc") {
      return CreateTransverseMercatorProjection(parameters, proj_string, ellipsoid);
    } else if (proj == "utm") {
      return CreateUniversalTransverseMercatorProjection(parameters, proj_string, ellipsoid);
    } else if (proj == "merc") {
      // Fixed projection, doesn't have any parameters
      return CreateWebMercatorProjection(proj_string, ellipsoid); 
    } else if (proj == "lcc") {
      return CreateLambertConformalConicProjection(parameters, proj_string, ellipsoid);
    }

    log_debug("projection '" + proj + "' is not supported, using default transverse mercator.");
    return CreateDefaultProjection(ellipsoid);
  }

  static geom::GeoLocation ParseGeoReference(const std::string &proj_string) {

    auto parameters = ParseProjectionParameters(proj_string);

    // Get the projection type
    std::string proj;
    if (parameters.find("proj") != parameters.end()) {
      proj = parameters["proj"];
    } else {
      log_warning("cannot find the type of projection, using default geolocation");
      return CreateDefaultGeoReference();
    }

    if (proj == "tmerc") {
      return CreateTransverseMercatorGeoReference(parameters);
    } else if (proj == "utm") {
      return CreateUniversalTransverseMercatorGeoReference(parameters);
    } else if (proj == "merc") {
      // Fixed projection, doesn't have any parameters
      return CreateWebMercatorGeoReference();
    } else if (proj == "lcc") {
      return CreateLamberConic2SPGeoReference(parameters);
    }

    log_debug("projection '" + proj + "' is not supported, using default geolocation.");
    return CreateDefaultGeoReference();
  }

  void GeoReferenceParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {
    map_builder.SetGeoReference(ParseGeoReference(
        xml.child("OpenDRIVE").child("header").child_value("geoReference")));

    map_builder.SetGeoProjection(ParseGeoProjection(
      xml.child("OpenDRIVE").child("header").child_value("geoReference")));
  }

} // parser
} // opendrive
} // carla
