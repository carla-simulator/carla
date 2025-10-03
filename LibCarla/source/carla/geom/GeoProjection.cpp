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
                return GeoLocationToTransformTransverseMercator(geolocation);
            }

            case ProjectionType::UTM: {
                return GeoLocationToTransformUTM(geolocation);
            }

            case ProjectionType::WebMercator: {
                return GeoLocationToTransformWebMercator(geolocation);
            }

            case ProjectionType::LambertConic2SP: {
                return GeoLocationToTransformLambertConic2SP(geolocation);
            }

            default: {
                return GeoLocationToTransformTransverseMercator(geolocation);
            }
        }
    }

    GeoLocation GeoProjection::TransformToGeoLocation(const Location& location) const {
        switch (static_cast<ProjectionType>(params.index())) {

            case ProjectionType::TransverseMercator: {
                return TransformToGeoLocationTransverseMercator(location);
            }

            case ProjectionType::UTM: {
                return TransformToGeoLocationUTM(location);
            }

            case ProjectionType::WebMercator: {
                return TransformToGeoLocationWebMercator(location);
            }

            case ProjectionType::LambertConic2SP:{
                return TransformToGeoLocationLambertConic2SP(location);
            }

            default: {
                return TransformToGeoLocationTransverseMercator(location);
            }
        }
    }


















    Location GeoProjection::GeoLocationToTransformTransverseMercator(const GeoLocation& geolocation) const {
        const auto& p = boost::variant2::get<TransverseMercatorParams>(params);

        // Using Snyder TM forward (ellipsoidal) to 6th order
        const double lat  = DegreesToRadians(geolocation.latitude);
        const double lon  = DegreesToRadians(geolocation.longitude);
        const double lat_0 = DegreesToRadians(p.lat_0);
        const double lon_0 = DegreesToRadians(p.lon_0);

        double dlon = std::atan2(std::sin(lon - lon_0), std::cos(lon - lon_0));

        const double a = p.ellps.a;
        const double f = p.ellps.f();
        const double e2 = p.ellps.e2();
        const double ep2 = p.ellps.ep2();
        const double e4 = e2 * e2;
        const double e6 = e4 * e2;

        const double N = a / std::sqrt(1.0 - e2 * std::sin(lat) * std::sin(lat));
        const double T = std::tan(lat) * std::tan(lat);
        const double C = ep2 * std::cos(lat) * std::cos(lat);
        const double A = std::cos(lat) * dlon;

        auto meridional_arc = [&](double phi) {
            return p.ellps.a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * phi
                - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * std::sin(2.0 * phi)
                + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * std::sin(4.0 * phi)
                - (35.0 * e6 / 3072.0) * std::sin(6.0 * phi));
        };

        const double M  = meridional_arc(lat);
        const double M_0 = meridional_arc(lat_0);

        float x = p.x_0 + p.k * N * (A + (1.0 - T + C) * std::pow(A, 3) / 6.0
            + (5.0 - 18.0*T + T*T + 72.0*C - 58.0*ep2) * std::pow(A, 5) / 120.0);

        float y = p.y_0 + p.k * ((M - M_0) + N * std::tan(lat) * ((A*A) / 2.0
            + (5.0 - T + 9.0*C + 4.0*C*C) * std::pow(A, 4) / 24.0
            + (61.0 - 58.0*T + T*T + 600.0*C - 330.0*ep2) * std::pow(A, 6) / 720.0));

        return Location(x, y, geolocation.altitude);
    }

    Location GeoProjection::GeoLocationToTransformUTM(const GeoLocation& geolocation) const {
        const auto& p = boost::variant2::get<UTMParams>(params);

        // Using Snyder TM forward (ellipsoidal) to 6th order. Same formula as Transverse Mercator.
        const double lat  = DegreesToRadians(geolocation.latitude);
        const double lon  = DegreesToRadians(geolocation.longitude);
        const double lon_0 = DegreesToRadians(6 * p.zone - 183);
        const double lat_0 = 0;

        double dlon = std::atan2(std::sin(lon - lon_0), std::cos(lon - lon_0));

        const double a = p.ellps.a;
        const double f = p.ellps.f();
        const double e2 = p.ellps.e2();
        const double ep2 = p.ellps.ep2();
        const double e4 = e2 * e2;
        const double e6 = e4 * e2;

        const double N = a / std::sqrt(1.0 - e2 * std::sin(lat) * std::sin(lat));
        const double T = std::tan(lat) * std::tan(lat);
        const double C = ep2 * std::cos(lat) * std::cos(lat);
        const double A = std::cos(lat) * dlon;

        auto meridional_arc = [&](double phi) {
            return p.ellps.a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * phi
                - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * std::sin(2.0 * phi)
                + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * std::sin(4.0 * phi)
                - (35.0 * e6 / 3072.0) * std::sin(6.0 * phi));
        };

        const double M  = meridional_arc(lat);

        float x = p.x_0 + p.k * N * (A + (1.0 - T + C) * std::pow(A, 3) / 6.0
            + (5.0 - 18.0*T + T*T + 72.0*C - 58.0*ep2) * std::pow(A, 5) / 120.0);

        float y = p.y_0 + p.k * (M + N * std::tan(lat) * ((A*A) / 2.0
            + (5.0 - T + 9.0*C + 4.0*C*C) * std::pow(A, 4) / 24.0
            + (61.0 - 58.0*T + T*T + 600.0*C - 330.0*ep2) * std::pow(A, 6) / 720.0));

        return Location(x, y, geolocation.altitude);
    }

    Location GeoProjection::GeoLocationToTransformWebMercator(const GeoLocation& geolocation) const {
        const auto& p = boost::variant2::get<WebMercatorParams>(params);

        double lat = DegreesToRadians(geolocation.latitude);
        double lon = DegreesToRadians(geolocation.longitude);

        float x = p.ellps.a * lon;
        float y = p.ellps.a * std::log(std::tan(Math::Pi<double>() / 4.0 + lat / 2.0));

        return Location(x, y, geolocation.altitude);
    }

    Location GeoProjection::GeoLocationToTransformLambertConic2SP(const GeoLocation& geolocation) const {
        const auto& p = boost::variant2::get<LambertConic2SPParams>(params);

        const double lat = DegreesToRadians(geolocation.latitude);
        const double lon = DegreesToRadians(geolocation.longitude);
        const double lon_0 = DegreesToRadians(p.lon_0);
        const double lat_0 = DegreesToRadians(p.lat_0);
        const double lat_1 = DegreesToRadians(p.lat_1);
        const double lat_2 = DegreesToRadians(p.lat_2);

        const double a = p.ellps.a;
        const double e2 = p.ellps.e2();
        const double e = std::sqrt(e2);

        const double m1 = std::cos(lat_1) / std::sqrt(1.0 - e2 * std::sin(lat_1) * std::sin(lat_1));
        const double m2 = std::cos(lat_2) / std::sqrt(1.0 - e2 * std::sin(lat_2) * std::sin(lat_2));

        const double t0 = std::tan(Math::Pi<double>() / 4 - lat_0 / 2)
            / std::pow((1.0 - e * std::sin(lat_0)) / (1.0 + e * std::sin(lat_0)), e / 2);
        const double t1 = std::tan(Math::Pi<double>() / 4- lat_1 / 2)
            / std::pow((1.0 - e * std::sin(lat_1)) / (1.0 + e * std::sin(lat_1)), e / 2);
        const double t2 = std::tan(Math::Pi<double>() / 4 - lat_2 / 2)
            / std::pow((1.0 - e * std::sin(lat_2)) / (1.0 + e * std::sin(lat_2)), e / 2);
        const double t = std::tan(Math::Pi<double>() / 4 - lat / 2)
            / std::pow((1.0 - e * std::sin(lat)) / (1.0 + e * std::sin(lat)), e / 2);

        const double n = (std::log(m1) - std::log(m2)) / (std::log(t1) - std::log(t2));
        const double F = m1 / (n * std::pow(t1, n));
        const double rho = a * F * std::pow(t, n);
        const double rho0 = a * F * std::pow(t0, n);
        const double theta = n * std::atan2(std::sin(lon - lon_0), std::cos(lon - lon_0));

        float x = p.x_0 + rho * std::sin(theta);
        float y = p.y_0 + rho0 - rho * std::cos(theta);

        return Location(x, y, geolocation.altitude);
    }

    GeoLocation GeoProjection::TransformToGeoLocationTransverseMercator(const Location& location) const {
        const auto& p = boost::variant2::get<TransverseMercatorParams>(params);

        // Using Snyder TM inverse (ellipsoidal) to 6th order
        const double lat_0 = DegreesToRadians(p.lat_0);
        const double lon_0 = DegreesToRadians(p.lon_0);

        const double a = p.ellps.a;
        const double finv = p.ellps.f_inv;
        const double f = p.ellps.f();
        const double e2 = p.ellps.e2();
        const double ep2 = p.ellps.ep2();
        const double e4 = e2 * e2;
        const double e6 = e4 * e2;

        const double x = (location.x - p.x_0) / p.k;
        const double y = (location.y - p.y_0) / p.k;

        auto M_of = [&](double phi) {
            return a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * phi
            - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * std::sin(2.0 * phi)
            + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * std::sin(4.0 * phi)
            - (35.0 * e6 / 3072.0) * std::sin(6.0 * phi));
        };

        const double M = M_of(lat_0) + y;

        const double mu = M / (a * (1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0));
        const double e1 = (1.0 - std::sqrt(1.0 - e2)) / (1.0 + std::sqrt(1.0 - e2));
        const double e1_2 = e1 * e1;
        const double e1_3 = e1_2 * e1;
        const double e1_4 = e1_3 * e1;

        const double phi1 = mu + (3.0 * e1 / 2.0 - 27.0 * e1_3 / 32.0) * std::sin(2.0 * mu)
            + (21.0 * e1_2 / 16.0 - 55.0 * e1_4 / 32.0) * std::sin(4.0 * mu)
            + (151.0 * e1_3 / 96.0) * std::sin(6.0 * mu) + (1097.0 * e1_4 / 512.0) * std::sin(8.0 * mu);

        const double sin1 = std::sin(phi1), cos1 = std::cos(phi1), tan1 = std::tan(phi1);

        const double N1 = a / std::sqrt(1.0 - e2 * sin1 * sin1);
        const double R1 = a * (1.0 - e2) / std::pow(1.0 - e2 * sin1 * sin1, 1.5);
        const double T1 = tan1 * tan1;
        const double C1 = ep2 * cos1 * cos1;
        const double D  = x / N1;

        // Snyder TM inverse (to 6th order)
        const double lat = phi1 - (N1 * tan1 / R1) * ((D * D) / 2.0
            - (5.0 + 3.0 * T1 + 10.0 * C1 - 4.0 * C1 * C1 - 9.0 * ep2) * std::pow(D, 4) / 24.0
            + (61.0 + 90.0 * T1 + 298.0 * C1 + 45.0 * T1 * T1 - 252.0 * ep2 - 3.0 * C1 * C1) * std::pow(D, 6) / 720.0);

        double lon = lon_0 + (D - (1.0 + 2.0 * T1 + C1) * std::pow(D, 3) / 6.0
            + (5.0 - 2.0 * C1 + 28.0 * T1 + 3.0 * C1 * C1 + 8.0 * ep2 + 24.0 * T1 * T1) * std::pow(D, 5) / 120.0) / cos1;

        lon = std::atan2(std::sin(lon), std::cos(lon));

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);;
    }

    GeoLocation GeoProjection::TransformToGeoLocationUTM(const Location& location) const {
        const auto& p = boost::variant2::get<UTMParams>(params);

        // Using Snyder TM inverse (ellipsoidal) to 6th order. Same formula as Transverse Mercator.
        const double lon_0 = DegreesToRadians(6 * p.zone - 183); // central meridian

        const double a = p.ellps.a;
        const double finv = p.ellps.f_inv;
        const double f = p.ellps.f();
        const double e2 = p.ellps.e2();
        const double ep2 = p.ellps.ep2();
        const double e4 = e2 * e2;
        const double e6 = e4 * e2;

        const double x = (location.x - p.x_0) / p.k;
        const double y = (location.y - p.y_0) / p.k;

        const double mu = y / (a * (1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0));
        const double e1 = (1.0 - std::sqrt(1.0 - e2)) / (1.0 + std::sqrt(1.0 - e2));
        const double e1_2 = e1 * e1;
        const double e1_3 = e1_2 * e1;
        const double e1_4 = e1_3 * e1;

        const double phi1 = mu + (3.0 * e1 / 2.0 - 27.0 * e1_3 / 32.0) * std::sin(2.0 * mu)
            + (21.0 * e1_2 / 16.0 - 55.0 * e1_4 / 32.0) * std::sin(4.0 * mu)
            + (151.0 * e1_3 / 96.0) * std::sin(6.0 * mu) + (1097.0 * e1_4 / 512.0) * std::sin(8.0 * mu);

        const double sin1 = std::sin(phi1), cos1 = std::cos(phi1), tan1 = std::tan(phi1);

        const double N1 = a / std::sqrt(1.0 - e2 * sin1 * sin1);
        const double R1 = a * (1.0 - e2) / std::pow(1.0 - e2 * sin1 * sin1, 1.5);
        const double T1 = tan1 * tan1;
        const double C1 = ep2 * cos1 * cos1;
        const double D  = x / N1;

        const double lat = phi1 - (N1 * tan1 / R1) * ((D * D) / 2.0
            - (5.0 + 3.0 * T1 + 10.0 * C1 - 4.0 * C1 * C1 - 9.0 * ep2) * std::pow(D, 4) / 24.0
            + (61.0 + 90.0 * T1 + 298.0 * C1 + 45.0 * T1 * T1 - 252.0 * ep2 - 3.0 * C1 * C1) * std::pow(D, 6) / 720.0);

        double lon = lon_0 + (D - (1.0 + 2.0 * T1 + C1) * std::pow(D, 3) / 6.0
            + (5.0 - 2.0 * C1 + 28.0 * T1 + 3.0 * C1 * C1 + 8.0 * ep2 + 24.0 * T1 * T1) * std::pow(D, 5) / 120.0) / cos1;

        lon = std::atan2(std::sin(lon), std::cos(lon));

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);;
    }

    GeoLocation GeoProjection::TransformToGeoLocationWebMercator(const Location& location) const {
        const auto& p = boost::variant2::get<WebMercatorParams>(params);

        double lon = location.x / p.ellps.a;
        double lat = 2*std::atan(std::exp(location.y / p.ellps.a)) - Math::Pi<double>()/2;

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);
    }

    GeoLocation GeoProjection::TransformToGeoLocationLambertConic2SP(const Location& location) const {
        const auto& p = boost::variant2::get<LambertConic2SPParams>(params);

        double lon_0 = DegreesToRadians(p.lon_0);
        double lat_1 = DegreesToRadians(p.lat_1);
        double lat_2 = DegreesToRadians(p.lat_2);
        double lat_0 = DegreesToRadians(p.lat_0);

        const double a = p.ellps.a;
        const double e2 = p.ellps.e2();
        const double e = std::sqrt(e2);

        const double m1 = std::cos(lat_1) / std::sqrt(1.0 - e2 * std::sin(lat_1) * std::sin(lat_1));
        const double m2 = std::cos(lat_2) / std::sqrt(1.0 - e2 * std::sin(lat_2) * std::sin(lat_2));

        const double t0 = std::tan(Math::Pi<double>() / 4 - lat_0 / 2)
            / std::pow((1.0 - e * std::sin(lat_0)) / (1.0 + e * std::sin(lat_0)), e / 2);
        const double t1 = std::tan(Math::Pi<double>() / 4- lat_1 / 2)
            / std::pow((1.0 - e * std::sin(lat_1)) / (1.0 + e * std::sin(lat_1)), e / 2);
        const double t2 = std::tan(Math::Pi<double>() / 4 - lat_2 / 2)
            / std::pow((1.0 - e * std::sin(lat_2)) / (1.0 + e * std::sin(lat_2)), e / 2);

        const double n = (std::log(m1) - std::log(m2)) / (std::log(t1) - std::log(t2)); // cone constant
        const double F = m1 / (n * std::pow(t1, n));
        const double rho0 = a * F * std::pow(t0, n);

        const double dx = static_cast<double>(location.x) - p.x_0;
        const double dy = static_cast<double>(location.y) - p.y_0;

        const double sgn = (n >= 0.0) ? 1.0 : -1.0;
        const double Y = rho0 - dy;
        const double rho = sgn * std::hypot(dx, Y);
        const double theta = std::atan2(sgn * dx, sgn * Y);

        const double t = std::pow(rho / (a * F), 1.0 / n);

        auto lat_from_t = [&](double tval) {
            // initial spherical guess
            double lat = M_PI * 0.5 - 2.0 * std::atan(tval);
            for (int i = 0; i < 10; ++i) {
                const double lat_next = Math::Pi<double>() * 0.5
                    - 2.0 * std::atan(tval * std::pow((1.0 - e * std::sin(lat)) / (1.0 + e * std::sin(lat)), 0.5 * e));
                if (std::abs(lat_next - lat) < 1e-12){
                    lat = lat_next; break;
                }
                lat = lat_next;
            }
            return lat;
        };

        const double lat = lat_from_t(t);
        double lon = lon_0 + theta / n;
        lon = std::atan2(std::sin(lon), std::cos(lon));

        return GeoLocation(RadiansToDegrees(lat), RadiansToDegrees(lon), location.z);
    }
} // namespace geom
} // namespace carla
