#pragma once
#include <string>
#include <limits>
#include <unordered_map>
#include <boost/optional.hpp>

#include "carla/geom/Math.h"
#include "carla/geom/Location.h"

namespace carla {
namespace geom {

    // Must be all lower case to support both cases when parsing
    static const std::unordered_map<std::string, std::pair<double,double>> custom_ellipsoids = {
        {"wgs84",  {6378137.0, 298.257223563}},
        {"grs80",  {6378137.0, 298.257222101}},
        {"intl",   {6378388.0, 297.0}},
        {"bessel", {6377397.155, 299.1528128}},
        {"clrk66", {6378206.4, 294.9786982138}},
        {"airy",   {6377563.396, 299.3249646}},
        {"wgs72",  {6378135.0, 298.26}},
        {"wgs66",  {6378145.0, 298.25}},
        {"sphere", {6370997.0, std::numeric_limits<double>::infinity()}}
    };

    class Ellipsoid {
    public:

        Ellipsoid() = default;

        Ellipsoid(double a, double f_inv):
            a(a), f_inv(f_inv) {}

        double a = 6378137.0;
        double f_inv = std::numeric_limits<double>::infinity();  // Default to sphere

        double f() const {
            return 1.0 / f_inv;
        }
        double b() const {
            return a * (1.0 - f());
        }

        double e2() const {
            return f() * (2.0 - f());
        }
        double ep2() const{
            return e2() / (1.0 - e2());
        }

        void fromb(double b) {
            f_inv = 1.0 / (1.0 - b/a);
        }
        void fromf(double f) {
            f_inv = 1.0 / f;
        }

        bool operator==(const Ellipsoid &rhs) const {
            return (a == rhs.a) && (f_inv == rhs.f_inv);
        }

        bool operator!=(const Ellipsoid &rhs) const {
            return (a != rhs.a) || (f_inv != rhs.f_inv);
        }
    };

    class OffsetTransform {
    public:

        OffsetTransform() = default;
        
        OffsetTransform(double offset_x, double offset_y, double offset_z, double offset_hdg): 
            offset_x(offset_x), offset_y(offset_y), offset_z(offset_z), offset_cos_h(std::cos(-offset_hdg)), offset_sin_h(std::sin(-offset_hdg)) {}

        double offset_x = 0.0;
        double offset_y = 0.0;
        double offset_z = 0.0;
        double offset_cos_h = 1.0;
        double offset_sin_h = 0.0;

        Location ApplyTransformation(const Location& location) const{

            double tx = location.x + offset_x;
            double ty = -location.y + offset_y;

            double x_rot = (tx * offset_cos_h) - (ty * offset_sin_h);
            double y_rot = (tx * offset_sin_h) + (ty * offset_cos_h);

            return Location{static_cast<float>(x_rot), static_cast<float>(y_rot), static_cast<float>(location.z + offset_z)};
        }

        bool operator==(const OffsetTransform& rhs) const {
            return (offset_x == rhs.offset_x) && (offset_y == rhs.offset_y) && (offset_z == rhs.offset_z) && (offset_cos_h == rhs.offset_cos_h) && (offset_sin_h == rhs.offset_sin_h);
        }
    };

    class TransverseMercatorParams {
    public:

        TransverseMercatorParams() = default;

        TransverseMercatorParams(double lat_0, double lon_0, double k, double x_0, double y_0, Ellipsoid ellps):
            lat_0(lat_0), lon_0(lon_0), k(k), x_0(x_0), y_0(y_0), ellps(ellps) {}

        double lat_0 = 0.0f;
        double lon_0 = 0.0f;
        double k = 1.0f;
        double x_0 = 0.0f;
        double y_0 = 0.0f;
        Ellipsoid ellps = Ellipsoid();

        bool operator==(const TransverseMercatorParams &rhs) const {
            return (lat_0 == rhs.lat_0) && (lon_0 == rhs.lon_0) && (k == rhs.k)
                && (x_0 == rhs.x_0) && (y_0 == rhs.y_0) && (ellps == rhs.ellps);
        }

        bool operator!=(const TransverseMercatorParams &rhs) const {
            return (lat_0 != rhs.lat_0) || (lon_0 != rhs.lon_0) || (k != rhs.k)
                || (x_0 != rhs.x_0) || (y_0 != rhs.y_0) || (ellps != rhs.ellps);
        }
    };

    class UniversalTransverseMercatorParams {
    public:

        UniversalTransverseMercatorParams() = default;

        UniversalTransverseMercatorParams(int zone, bool north, Ellipsoid ellps, boost::optional<OffsetTransform> offset = boost::none):
            zone(zone), north(north), ellps(ellps), offset(offset) {}

        int zone = 31;
        bool north = true;
        Ellipsoid ellps = Ellipsoid();
        boost::optional<OffsetTransform> offset;

        bool operator==(const UniversalTransverseMercatorParams &rhs) const {
            return (zone == rhs.zone) && (north == rhs.north) && (ellps == rhs.ellps) && (offset == rhs.offset);
        }

        bool operator!=(const UniversalTransverseMercatorParams &rhs) const {
            return (zone != rhs.zone) || (north != rhs.north) || (ellps != rhs.ellps) || (offset == rhs.offset);
        }
    };


    class WebMercatorParams {
    public:

        WebMercatorParams() = default;
    
        WebMercatorParams(Ellipsoid ellps):
            ellps(ellps) {}

        Ellipsoid ellps = Ellipsoid();

        bool operator==(const WebMercatorParams &rhs) const {
            return (ellps == rhs.ellps);
        }

        bool operator!=(const WebMercatorParams &rhs) const {
            return (ellps != rhs.ellps);
        }
    };

    class LambertConformalConicParams {
    public:

        LambertConformalConicParams() = default;
    
        LambertConformalConicParams(
            double lat_0, double lat_1, double lat_2, double lon_0, double x_0, double y_0, Ellipsoid ellps):
            lat_0(lat_0), lat_1(lat_1), lat_2(lat_2), lon_0(lon_0), x_0(x_0), y_0(y_0), ellps(ellps) {}

        double lat_0 = 0.0;
        double lat_1 = -5.0;
        double lat_2 = 5.0;
        double lon_0 = 0.0;
        double x_0 = 0.0;
        double y_0 = 0.0;
        Ellipsoid ellps = Ellipsoid();

        bool operator==(const LambertConformalConicParams &rhs) const {
            return (lat_0 == rhs.lat_0) && (lat_1 == rhs.lat_1) && (lat_2 == rhs.lat_2) && (lon_0 == rhs.lon_0)
                && (x_0 == rhs.x_0) && (y_0 == rhs.y_0) && (ellps == rhs.ellps);
        }

        bool operator!=(const LambertConformalConicParams &rhs) const {
            return (lat_0 != rhs.lat_0) || (lat_1 != rhs.lat_1) || (lat_2 != rhs.lat_2) || (lon_0 != rhs.lon_0)
                || (x_0 != rhs.x_0) || (y_0 != rhs.y_0) || (ellps != rhs.ellps);
        }
    };

} // namespace geom
} // namespace carla
