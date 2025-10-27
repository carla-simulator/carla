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
#include <utility>
#include <cctype>
#include <stdexcept>

namespace carla {
namespace opendrive {
namespace parser {

  template <typename S>
  static std::string ToLowerCase(S&& s) {
    std::string r = s;
    for (auto& c : r)
      c = static_cast<char>(std::tolower(c));
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

  template <typename M, typename S>
  static bool TryGetParameter(M& parameters, S&& name, std::string& out)
  {
    auto i = parameters.find(std::forward<S>(name));
    if (i == parameters.cend())
      return false;
    out = i->second;
    return true;
  }

  template <typename Map, typename S, typename O>
  static
  std::enable_if_t<std::is_floating_point<O>::value, bool>
  TryGetParameter(Map& parameters, S&& name, O& out)
  {
    auto i = parameters.find(std::forward<S>(name));
    if (i == parameters.cend())
      return false;
    out = static_cast<O>(std::stod(i->second));
    return true;
  }

  template <typename Map, typename S, typename O>
  static
  std::enable_if_t<std::is_integral<O>::value && std::is_unsigned<O>::value, bool>
  TryGetParameter(Map& parameters, S&& name, O& out)
  {
    auto i = parameters.find(std::forward<S>(name));
    if (i == parameters.cend())
      return false;
    out = static_cast<O>(std::stoull(i->second));
    return true;
  }

  template <typename Map, typename S, typename O>
  static
  std::enable_if_t<std::is_integral<O>::value && std::is_signed<O>::value, bool>
  TryGetParameter(Map& parameters, S&& name, O& out)
  {
    auto i = parameters.find(std::forward<S>(name));
    if (i == parameters.cend())
      return false;
    out = static_cast<O>(std::stoll(i->second));
    return true;
  }

  static geom::Ellipsoid CreateEllipsoid(std::unordered_map<std::string, std::string> parameters){
    
    geom::Ellipsoid ellps;
    std::string value;
    double x;

    if (TryGetParameter(parameters, "ellps", value)) {
      value = ToLowerCase(value);
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
    TryGetParameter(parameters, "a", ellps.a);
    TryGetParameter(parameters, "b", x);
    ellps.fromb(x);
    TryGetParameter(parameters, "f", x);
    ellps.fromf(x);
    TryGetParameter(parameters, "rf", ellps.f_inv);

    return ellps;
  }

  static geom::GeoProjection CreateTransverseMercatorProjection(
    std::unordered_map<std::string, std::string> parameters,
    std::string proj_string,
    geom::Ellipsoid ellipsoid){

    geom::TransverseMercatorParams p;
    TryGetParameter(parameters, "lat_0", p.lat_0);
    TryGetParameter(parameters, "lon_0", p.lon_0);
    TryGetParameter(parameters, "k", p.k);
    TryGetParameter(parameters, "x_0", p.x_0);
    TryGetParameter(parameters, "y_0", p.y_0);
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
    if (!TryGetParameter(parameters, "zone", p.zone)) {
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
    TryGetParameter(parameters, "lon_0", p.lon_0);
    TryGetParameter(parameters, "lat_0", p.lat_0);
    if (!TryGetParameter(parameters, "lat_1", p.lat_1))
      log_warning("Missing 'lat_1' parameter for LCC projection. Using default value " + std::to_string(p.lat_1));
    if (!TryGetParameter(parameters, "lat_2", p.lat_2))
      log_warning("Missing 'lat_2' parameter for LCC projection. Using default value " + std::to_string(p.lat_2));
    TryGetParameter(parameters, "x_0", p.x_0);
    TryGetParameter(parameters, "y_0", p.y_0);
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
    TryGetParameter(parameters, "lat_0", result.latitude);
    TryGetParameter(parameters, "lon_0", result.longitude);
    return result;
  }

  static geom::GeoLocation CreateUniversalTransverseMercatorGeoReference(std::unordered_map<std::string, std::string> parameters){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    result.latitude = 0.0;
    if (TryGetParameter(parameters, "zone", result.longitude))
      result.longitude = geom::Math::ToRadians(6 * result.longitude - 183);
    return result;
  }

  static geom::GeoLocation CreateWebMercatorGeoReference(){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    return result;
  }

  static geom::GeoLocation CreateLamberConic2SPGeoReference(std::unordered_map<std::string, std::string> parameters){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    TryGetParameter(parameters, "lat_0", result.latitude);
    TryGetParameter(parameters, "lon_0", result.longitude);
    return result;
  }

  static geom::GeoLocation CreateDefaultGeoReference(){
    geom::GeoLocation result{0.0, 0.0, 0.0};
    return result;
  }

  static auto ParseGeoProjectionAndReference(const std::string& proj_string) {
    auto parameters = ParseProjectionParameters(proj_string);
    auto ellipsoid = CreateEllipsoid(parameters);

    // Get the projection type
    std::string proj;
    if (!TryGetParameter(parameters, "proj", proj)) {
      log_warning("cannot find the type of projection, using default transverse mercator");
      return std::make_pair(
        CreateDefaultProjection(ellipsoid),
        CreateDefaultGeoReference());
    }

    // Parse the parameters
    if (proj == "tmerc") {
      return std::make_pair(
        CreateTransverseMercatorProjection(parameters, proj_string, ellipsoid),
        CreateTransverseMercatorGeoReference(parameters));
    } else if (proj == "utm") {
      return std::make_pair(
        CreateUniversalTransverseMercatorProjection(parameters, proj_string, ellipsoid),
        CreateUniversalTransverseMercatorGeoReference(parameters));
    } else if (proj == "merc") {
      return std::make_pair(
        CreateWebMercatorProjection(proj_string, ellipsoid),
        CreateWebMercatorGeoReference());
    } else if (proj == "lcc") {
      return std::make_pair(
        CreateLambertConformalConicProjection(parameters, proj_string, ellipsoid),
        CreateLamberConic2SPGeoReference(parameters));
    } else {
      log_debug("projection '" + proj + "' is not supported, using default transverse mercator.");
      return std::make_pair(
        CreateDefaultProjection(ellipsoid),
        CreateDefaultGeoReference());
    }
  }

  void GeoReferenceParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {
    
    auto georeference_string =
      xml
      .child("OpenDRIVE")
      .child("header")
      .child_value("geoReference");
    
    auto reference_and_projection = ParseGeoProjectionAndReference(
      georeference_string);
    map_builder.SetGeoProjection(reference_and_projection.first);
    map_builder.SetGeoReference(reference_and_projection.second);
  }

} // parser
} // opendrive
} // carla
