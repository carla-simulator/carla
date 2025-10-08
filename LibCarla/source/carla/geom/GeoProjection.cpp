#include <cmath>
#include <cassert>

#include "GeoProjection.h"

#include "carla/geom/GeoLocation.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"

namespace carla {
namespace geom {

    static double DegreesToRadians(double degrees) {
    return degrees * Math::Pi<double>() / 180.0;
    }

    static double RadiansToDegrees(double radians) {
    return radians * 180.0 / Math::Pi<double>();
    }

    Location GeoProjection::GeoLocationToTransform(const GeoLocation& geolocation) const {
        switch (static_cast<ProjectionType>(params.index())) {

            case ProjectionType::TransverseMercator: {
                auto& p = boost::variant2::get<TransverseMercatorParams>(params);
                return GeoLocationToTransformTransverseMercator(geolocation, p);
            }

            case ProjectionType::UniversalTransverseMercator: {
                auto& p = boost::variant2::get<UniversalTransverseMercatorParams>(params);
                return GeoLocationToTransformUniversalTransverseMercator(geolocation, p);
            }

            case ProjectionType::WebMercator: {
                auto& p = boost::variant2::get<WebMercatorParams>(params);
                return GeoLocationToTransformWebMercator(geolocation, p);
            }

            case ProjectionType::LambertConformalConic: {
                auto& p = boost::variant2::get<LambertConformalConicParams>(params);
                return GeoLocationToTransformLambertConformalConic(geolocation, p);
            }

            default: {
                auto& p = boost::variant2::get<TransverseMercatorParams>(params);
                return GeoLocationToTransformTransverseMercator(geolocation, p);
            }
        }
    }

    GeoLocation GeoProjection::TransformToGeoLocation(const Location& location) const {
        switch (static_cast<ProjectionType>(params.index())) {

            case ProjectionType::TransverseMercator: {
                auto& p = boost::variant2::get<TransverseMercatorParams>(params);
                return TransformToGeoLocationTransverseMercator(location, p);
            }

            case ProjectionType::UniversalTransverseMercator: {
                auto& p = boost::variant2::get<UniversalTransverseMercatorParams>(params);
                return TransformToGeoLocationUniversalTransverseMercator(location, p);
            }

            case ProjectionType::WebMercator: {
                auto& p = boost::variant2::get<WebMercatorParams>(params);
                return TransformToGeoLocationWebMercator(location, p);
            }

            case ProjectionType::LambertConformalConic:{
                auto& p = boost::variant2::get<LambertConformalConicParams>(params);
                return TransformToGeoLocationLambertConformalConic(location, p);
            }

            default: {
                auto& p = boost::variant2::get<TransverseMercatorParams>(params);
                return TransformToGeoLocationTransverseMercator(location, p);
            }
        }
    }

    Location GeoProjection::GeoLocationToTransformTransverseMercator(
        const GeoLocation& geolocation, const TransverseMercatorParams p) const {

        // Using Snyder TM forward (ellipsoidal) to 6th order
        double lat  = DegreesToRadians(geolocation.latitude);
        double lon  = DegreesToRadians(geolocation.longitude);
        double lat_0 = DegreesToRadians(p.lat_0);
        double lon_0 = DegreesToRadians(p.lon_0);

        double dlon = std::atan2(std::sin(lon - lon_0), std::cos(lon - lon_0));

        double a = p.ellps.a;
        double e2 = p.ellps.e2();
        double ep2 = p.ellps.ep2();
        double e4 = e2 * e2;
        double e6 = e4 * e2;

        double N = a / std::sqrt(1.0 - e2 * std::sin(lat) * std::sin(lat));
        double T = std::tan(lat) * std::tan(lat);
        double C = ep2 * std::cos(lat) * std::cos(lat);
        double A = std::cos(lat) * dlon;

        auto meridional_arc = [&](double phi) {
            return p.ellps.a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * phi
                - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * std::sin(2.0 * phi)
                + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * std::sin(4.0 * phi)
                - (35.0 * e6 / 3072.0) * std::sin(6.0 * phi));
        };

        double M  = meridional_arc(lat);
        double M_0 = meridional_arc(lat_0);

        double x = p.x_0 + p.k * N * (A + (1.0 - T + C) * std::pow(A, 3) / 6.0
            + (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * std::pow(A, 5) / 120.0);

        double  y = p.y_0 + p.k * ((M - M_0) + N * std::tan(lat) * ((A*A) / 2.0
            + (5.0 - T + 9.0 * C + 4.0 * C * C) * std::pow(A, 4) / 24.0
            + (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * std::pow(A, 6) / 720.0));

        return Location(static_cast<float>(x), static_cast<float>(y), static_cast<float>(geolocation.altitude));
    }

    Location GeoProjection::GeoLocationToTransformUniversalTransverseMercator(
        const GeoLocation& geolocation, const UniversalTransverseMercatorParams p) const {

        // Using Snyder TM forward (ellipsoidal) to 6th order. Same formula as Transverse Mercator.
        double lat = DegreesToRadians(geolocation.latitude);
        double lon = DegreesToRadians(geolocation.longitude);
        double lon_0 = DegreesToRadians(6 * p.zone - 183);
        double k = 0.9996;
        double x_0 = 500000.0;
        double y_0 = (p.north) ? 0.0 : 10000000.0;

        double a = p.ellps.a;
        double e2 = p.ellps.e2();
        double ep2 = p.ellps.ep2();
        double e4 = e2 * e2;
        double e6 = e4 * e2;

        double N = a / std::sqrt(1.0 - e2 * std::sin(lat) * std::sin(lat));
        double T = std::tan(lat) * std::tan(lat);
        double C = ep2 * std::cos(lat) * std::cos(lat);
        double A = std::cos(lat) * std::atan2(std::sin(lon - lon_0), std::cos(lon - lon_0));

        double M = p.ellps.a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * lat
            - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * std::sin(2.0 * lat)
            + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * std::sin(4.0 * lat)
            - (35.0 * e6 / 3072.0) * std::sin(6.0 * lat));

        double x = x_0 + k * N * (A + (1.0 - T + C) * std::pow(A, 3) / 6.0
            + (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * std::pow(A, 5) / 120.0);

        double y = y_0 + k * (M + N * std::tan(lat) * ((A * A) / 2.0
            + (5.0 - T + 9.0 * C + 4.0 * C * C) * std::pow(A, 4) / 24.0
            + (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * std::pow(A, 6) / 720.0));

        return Location(static_cast<float>(x), static_cast<float>(y), static_cast<float>(geolocation.altitude));
    }

    Location GeoProjection::GeoLocationToTransformWebMercator(
        const GeoLocation& geolocation, const WebMercatorParams p) const {

        double lat = DegreesToRadians(geolocation.latitude);
        double lon = DegreesToRadians(geolocation.longitude);

        double x = p.ellps.a * lon;
        double y = p.ellps.a * std::log(std::tan(Math::Pi<double>() / 4.0 + lat / 2.0));

        return Location(static_cast<float>(x), static_cast<float>(y), static_cast<float>(geolocation.altitude));
    }

    Location GeoProjection::GeoLocationToTransformLambertConformalConic(
        const GeoLocation& geolocation, const LambertConformalConicParams p) const {

        double lat = DegreesToRadians(geolocation.latitude);
        double lon = DegreesToRadians(geolocation.longitude);
        double lon_0 = DegreesToRadians(p.lon_0);
        double lat_0 = DegreesToRadians(p.lat_0);
        double lat_1 = DegreesToRadians(p.lat_1);
        double lat_2 = DegreesToRadians(p.lat_2);

        double a = p.ellps.a;
        double e2 = p.ellps.e2();
        double e = std::sqrt(e2);

        double m1 = std::cos(lat_1) / std::sqrt(1.0 - e2 * std::sin(lat_1) * std::sin(lat_1));
        double m2 = std::cos(lat_2) / std::sqrt(1.0 - e2 * std::sin(lat_2) * std::sin(lat_2));

        double t0 = std::tan(Math::Pi<double>() / 4 - lat_0 / 2)
            / std::pow((1.0 - e * std::sin(lat_0)) / (1.0 + e * std::sin(lat_0)), e / 2);
        double t1 = std::tan(Math::Pi<double>() / 4- lat_1 / 2)
            / std::pow((1.0 - e * std::sin(lat_1)) / (1.0 + e * std::sin(lat_1)), e / 2);
        double t2 = std::tan(Math::Pi<double>() / 4 - lat_2 / 2)
            / std::pow((1.0 - e * std::sin(lat_2)) / (1.0 + e * std::sin(lat_2)), e / 2);
        double t = std::tan(Math::Pi<double>() / 4 - lat / 2)
            / std::pow((1.0 - e * std::sin(lat)) / (1.0 + e * std::sin(lat)), e / 2);

        double n = (std::log(m1) - std::log(m2)) / (std::log(t1) - std::log(t2));
        double F = m1 / (n * std::pow(t1, n));
        double rho = a * F * std::pow(t, n);
        double rho0 = a * F * std::pow(t0, n);
        double theta = n * std::atan2(std::sin(lon - lon_0), std::cos(lon - lon_0));

        double x = p.x_0 + rho * std::sin(theta);
        double y = p.y_0 + rho0 - rho * std::cos(theta);

        return Location(static_cast<float>(x), static_cast<float>(y), static_cast<float>(geolocation.altitude));
    }

    GeoLocation GeoProjection::TransformToGeoLocationTransverseMercator(
        const Location& location, const TransverseMercatorParams p) const {

        // Using Snyder TM inverse (ellipsoidal) to 6th order
        double lat_0 = DegreesToRadians(p.lat_0);
        double lon_0 = DegreesToRadians(p.lon_0);

        double a = p.ellps.a;
        double e2 = p.ellps.e2();
        double ep2 = p.ellps.ep2();
        double e4 = e2 * e2;
        double e6 = e4 * e2;

        double x = (location.x - p.x_0) / p.k;
        double y = (location.y - p.y_0) / p.k;

        double M = a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * lat_0
            - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * std::sin(2.0 * lat_0)
            + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * std::sin(4.0 * lat_0)
            - (35.0 * e6 / 3072.0) * std::sin(6.0 * lat_0)) + y;

        double mu = M / (a * (1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0));
        double e1 = (1.0 - std::sqrt(1.0 - e2)) / (1.0 + std::sqrt(1.0 - e2));
        double e1_2 = e1 * e1;
        double e1_3 = e1_2 * e1;
        double e1_4 = e1_3 * e1;

        double phi1 = mu + (3.0 * e1 / 2.0 - 27.0 * e1_3 / 32.0) * std::sin(2.0 * mu)
            + (21.0 * e1_2 / 16.0 - 55.0 * e1_4 / 32.0) * std::sin(4.0 * mu)
            + (151.0 * e1_3 / 96.0) * std::sin(6.0 * mu) + (1097.0 * e1_4 / 512.0) * std::sin(8.0 * mu);

        double sin1 = std::sin(phi1), cos1 = std::cos(phi1), tan1 = std::tan(phi1);

        double N = a / std::sqrt(1.0 - e2 * sin1 * sin1);
        double R = a * (1.0 - e2) / std::pow(1.0 - e2 * sin1 * sin1, 1.5);
        double T = tan1 * tan1;
        double C = ep2 * cos1 * cos1;
        double D  = x / N;

        // Snyder TM inverse (to 6th order)
        double lat = phi1 - (N * tan1 / R) * ((D * D) / 2.0
            - (5.0 + 3.0 * T + 10.0 * C - 4.0 * C * C - 9.0 * ep2) * std::pow(D, 4) / 24.0
            + (61.0 + 90.0 * T + 298.0 * C + 45.0 * T * T - 252.0 * ep2 - 3.0 * C * C) * std::pow(D, 6) / 720.0);

        double lon = lon_0 + (D - (1.0 + 2.0 * T + C) * std::pow(D, 3) / 6.0
            + (5.0 - 2.0 * C + 28.0 * T + 3.0 * C * C + 8.0 * ep2 + 24.0 * T * T) * std::pow(D, 5) / 120.0) / cos1;

        lon = std::atan2(std::sin(lon), std::cos(lon));

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);;
    }

    GeoLocation GeoProjection::TransformToGeoLocationUniversalTransverseMercator(
        const Location& location, const UniversalTransverseMercatorParams p) const {

        // Using Snyder TM inverse (ellipsoidal) to 6th order. Same formula as Transverse Mercator.
        double lon_0 = DegreesToRadians(6 * p.zone - 183); // central meridian
        double k = 0.9996;
        double x_0 = 500000.0;
        double y_0 = (p.north) ? 0.0 : 10000000.0;

        double a = p.ellps.a;
        double e2 = p.ellps.e2();
        double ep2 = p.ellps.ep2();
        double e4 = e2 * e2;
        double e6 = e4 * e2;

        double x = (location.x - x_0) / k;
        double y = (location.y - y_0) / k;

        double mu = y / (a * (1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0));
        double e1 = (1.0 - std::sqrt(1.0 - e2)) / (1.0 + std::sqrt(1.0 - e2));
        double e1_2 = e1 * e1;
        double e1_3 = e1_2 * e1;
        double e1_4 = e1_3 * e1;

        double phi1 = mu + (3.0 * e1 / 2.0 - 27.0 * e1_3 / 32.0) * std::sin(2.0 * mu)
            + (21.0 * e1_2 / 16.0 - 55.0 * e1_4 / 32.0) * std::sin(4.0 * mu)
            + (151.0 * e1_3 / 96.0) * std::sin(6.0 * mu) + (1097.0 * e1_4 / 512.0) * std::sin(8.0 * mu);

        double sin1 = std::sin(phi1);
        double cos1 = std::cos(phi1);
        double tan1 = std::tan(phi1);

        double N = a / std::sqrt(1.0 - e2 * sin1 * sin1);
        double R = a * (1.0 - e2) / std::pow(1.0 - e2 * sin1 * sin1, 1.5);
        double T = tan1 * tan1;
        double C = ep2 * cos1 * cos1;
        double D  = x / N;

        double lat = phi1 - (N * tan1 / R) * ((D * D) / 2.0
            - (5.0 + 3.0 * T + 10.0 * C - 4.0 * C * C - 9.0 * ep2) * std::pow(D, 4) / 24.0
            + (61.0 + 90.0 * T + 298.0 * C + 45.0 * T * T - 252.0 * ep2 - 3.0 * C * C) * std::pow(D, 6) / 720.0);

        double lon = lon_0 + (D - (1.0 + 2.0 * T + C) * std::pow(D, 3) / 6.0
            + (5.0 - 2.0 * C + 28.0 * T + 3.0 * C * C + 8.0 * ep2 + 24.0 * T * T) * std::pow(D, 5) / 120.0) / cos1;

        lon = std::atan2(std::sin(lon), std::cos(lon));

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);;
    }

    GeoLocation GeoProjection::TransformToGeoLocationWebMercator(
        const Location& location, const WebMercatorParams p) const {

        double lon = location.x / p.ellps.a;
        double lat = 2*std::atan(std::exp(location.y / p.ellps.a)) - Math::Pi<double>()/2;

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);
    }

    GeoLocation GeoProjection::TransformToGeoLocationLambertConformalConic(
        const Location& location, const LambertConformalConicParams p) const {

        double lon_0 = DegreesToRadians(p.lon_0);
        double lat_1 = DegreesToRadians(p.lat_1);
        double lat_2 = DegreesToRadians(p.lat_2);
        double lat_0 = DegreesToRadians(p.lat_0);

        double a = p.ellps.a;
        double e2 = p.ellps.e2();
        double e = std::sqrt(e2);

        double m1 = std::cos(lat_1) / std::sqrt(1.0 - e2 * std::sin(lat_1) * std::sin(lat_1));
        double m2 = std::cos(lat_2) / std::sqrt(1.0 - e2 * std::sin(lat_2) * std::sin(lat_2));

        double t0 = std::tan(Math::Pi<double>() / 4 - lat_0 / 2)
            / std::pow((1.0 - e * std::sin(lat_0)) / (1.0 + e * std::sin(lat_0)), e / 2);
        double t1 = std::tan(Math::Pi<double>() / 4- lat_1 / 2)
            / std::pow((1.0 - e * std::sin(lat_1)) / (1.0 + e * std::sin(lat_1)), e / 2);
        double t2 = std::tan(Math::Pi<double>() / 4 - lat_2 / 2)
            / std::pow((1.0 - e * std::sin(lat_2)) / (1.0 + e * std::sin(lat_2)), e / 2);

        double n = (std::log(m1) - std::log(m2)) / (std::log(t1) - std::log(t2)); // cone constant
        double F = m1 / (n * std::pow(t1, n));
        double rho0 = a * F * std::pow(t0, n);

        double x = static_cast<double>(location.x) - p.x_0;
        double y = static_cast<double>(location.y) - p.y_0;

        double sgn = (n >= 0.0) ? 1.0 : -1.0;
        double Y = rho0 - y;
        double rho = sgn * std::hypot(x, Y);
        double theta = std::atan2(sgn * x, sgn * Y);

        double t = std::pow(rho / (a * F), 1.0 / n);

        // initial spherical guess
        double lat = Math::Pi<double>() * 0.5 - 2.0 * std::atan(t);
        for (int i = 0; i < 10; ++i) {
            const double lat_next = Math::Pi<double>() * 0.5
                - 2.0 * std::atan(t * std::pow((1.0 - e * std::sin(lat)) / (1.0 + e * std::sin(lat)), 0.5 * e));
            if (std::abs(lat_next - lat) < 1e-12){
                lat = lat_next; break;
            }
            lat = lat_next;
        }
        double lon = lon_0 + theta / n;
        lon = std::atan2(std::sin(lon), std::cos(lon));

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);
    }
} // namespace geom
} // namespace carla
