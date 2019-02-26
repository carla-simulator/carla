// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/GnssSensor.h"

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/detail/Simulator.h"
#include "carla/geom/Math.h"
#include "carla/sensor/data/GnssEvent.h"
#include "carla/StringUtil.h"

#include <exception>
#include <cmath>
#include <sstream>
#include <limits>

#if defined(_WIN32) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#include <math.h> // cmath is not enough for MSVC
#endif

namespace carla {
namespace client {

  const double EARTH_RADIUS_EQUA = 6378137.0;// earth radius at equator [m]

  // inlined functions to avoid multiple definitions

  /// @brief convert latitude to scale, which is needed by mercator transformations
  /// @param lat latitude in degrees (DEG)
  /// @return scale factor
  /// @note when converting from lat/lon -> mercator and back again,
  ///        or vice versa, use the same scale in both transformations!
  inline double LatToScale (double lat) {
    return cos(lat * geom::Math::pi() / 180.0);
  }

  /// @brief converts lat/lon/scale to mx/my (mx/my in meters if correct scale is given)
  ///
  template<class float_type>
  inline void LatLonToMercator (double lat, double lon, double scale, float_type &mx, float_type &my) {
    mx = scale * lon * geom::Math::pi() * EARTH_RADIUS_EQUA / 180.0;
    my = scale * EARTH_RADIUS_EQUA * log( tan((90.0+lat) * geom::Math::pi() / 360.0) );
  }

  /// @brief converts mx/my/scale to lat/lon (mx/my in meters if correct scale is given)
  inline void MercatorToLatLon (double mx, double my, double scale, double &lat, double &lon) {
    lon = mx * 180.0 / (geom::Math::pi() * EARTH_RADIUS_EQUA * scale);
    lat = 360.0 * atan( exp(my/(EARTH_RADIUS_EQUA * scale)) ) / geom::Math::pi() - 90.0;
  }

  /// @brief adds meters dx/dy to given lat/lon and returns new lat/lon
  inline void LatLonAddMeters (double lat_start, double lon_start, double dx, double dy, double &lat_end, double &lon_end) {
    double scale = LatToScale (lat_start);
    double mx,my;
    LatLonToMercator (lat_start, lon_start, scale, mx, my);
    mx += dx;
    my += dy;
    MercatorToLatLon (mx, my, scale, lat_end, lon_end);
  }

  GnssSensor::~GnssSensor() = default;

  void GnssSensor::Listen(CallbackFunctionType callback) {
    if (_is_listening) {
      log_error(GetDisplayId(), ": already listening");
      return;
    }

    if (GetParent() == nullptr) {
      throw_exception(std::runtime_error(GetDisplayId() + ": not attached to vehicle"));
      return;
    }

    SharedPtr<Map> map = GetWorld().GetMap();

    DEBUG_ASSERT(map != nullptr);

    auto self = boost::static_pointer_cast<GnssSensor>(shared_from_this());

    //parse geo reference string
    _map_latitude = std::numeric_limits<double>::quiet_NaN();
    _map_longitude = std::numeric_limits<double>::quiet_NaN();
    std::vector<std::string> geo_ref_splitted;
    StringUtil::Split(geo_ref_splitted, map->GetGeoReference(), " ");

    for (auto element: geo_ref_splitted) {
      std::vector<std::string> geo_ref_key_value;
      StringUtil::Split(geo_ref_key_value, element, "=");
      if (geo_ref_key_value.size() != 2u) {
        continue;
      }

      if (geo_ref_key_value[0] == "+lat_0") {
        _map_latitude = ParseDouble(geo_ref_key_value[1]);
      } else if (geo_ref_key_value[0] == "+lon_0") {
        _map_longitude = ParseDouble(geo_ref_key_value[1]);
      }
    }


    if (std::isnan(_map_latitude) || std::isnan(_map_longitude)) {
      log_warning(GetDisplayId(), ": cannot parse georeference: '" + map->GetGeoReference() + "'. Using default values.");
      _map_latitude = 42.0;
      _map_longitude = 2.0;
    }

    log_debug(GetDisplayId(), ": map geo reference: latitude ", _map_latitude, ", longitude ", _map_longitude);

    log_debug(GetDisplayId(), ": subscribing to tick event");
    GetEpisode().Lock()->RegisterOnTickEvent([
        cb=std::move(callback),
        weak_self=WeakPtr<GnssSensor>(self)](const auto &timestamp) {
      auto self = weak_self.lock();
      if (self != nullptr) {
        auto data = self->TickGnssSensor(timestamp);
        if (data != nullptr) {
          cb(std::move(data));
        }
      }
    });
    _is_listening = true;
  }

  double GnssSensor::ParseDouble(std::string const &stringValue) const {
    double value;
    std::istringstream istr(stringValue);
    istr.imbue(std::locale("C"));
    istr >> value;
    if (istr.fail() || !istr.eof()) {
      value = std::numeric_limits<double>::quiet_NaN();
    }
    return value;
  }

  SharedPtr<sensor::SensorData> GnssSensor::TickGnssSensor(
      const Timestamp &timestamp) {
    try {
      const auto location = GetLocation();
      double current_lat, current_lon;

      LatLonAddMeters(_map_latitude, _map_longitude, location.x, location.y, current_lat, current_lon);

      return MakeShared<sensor::data::GnssEvent>(
               timestamp.frame_count,
               GetTransform(),
               current_lat,
               current_lon,
               location.z);
    } catch (const std::exception &e) {
      /// @todo We need to unsubscribe the sensor.
      // log_error("LaneDetector:", e.what());
      return nullptr;
    }

  }

  void GnssSensor::Stop() {
    _is_listening = false;
  }

} // namespace client
} // namespace carla
