#pragma once
#include <string>
#include <limits>
#include <unordered_map>

namespace carla {
namespace geom {

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
    };

    class UniversalTransverseMercatorParams {
    public:

        UniversalTransverseMercatorParams() = default;

        UniversalTransverseMercatorParams(int zone, bool north, Ellipsoid ellps):
            zone(zone), north(north), ellps(ellps) {}

        int zone = 31;
        bool north = true;
        Ellipsoid ellps = Ellipsoid();
    };

    class WebMercatorParams {
    public:

        WebMercatorParams() = default;
    
        WebMercatorParams(Ellipsoid ellps):
            ellps(ellps) {}

        Ellipsoid ellps = Ellipsoid();
    };

    class LambertConformalConicParams {
    public:

        LambertConformalConicParams() = default;
    
        LambertConformalConicParams(
            double lat_0, double lat_1, double lat_2, double lon_0, double x_0, double y_0, Ellipsoid ellps):
            lat_0(lat_0), lat_1(lat_1), lat_2(lat_2), lon_0(lon_0), x_0(x_0), y_0(y_0), ellps(ellps) {}

        double lat_0 = 0.0;
        double lat_1 = 0.0;
        double lat_2 = 0.0;
        double lon_0 = 0.0;
        double x_0 = 0.0;
        double y_0 = 0.0;
        Ellipsoid ellps = Ellipsoid();
    };

} // namespace geom
} // namespace carla
