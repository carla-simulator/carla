#pragma once
#include <string>
#include <boost/variant2/variant.hpp>

namespace carla {
namespace geom {

    class Location;
    class Transform;
    class GeoLocation;

    struct Ellipsoid {
        double a     = 6378137.0;           // semi-major
        double f_inv = 298.257223563;       // inverse flattening
        double f() const {return 1.0 / f_inv;}
        double b() const {return a * (1.0 - f());}
        double e2() const {return f() * (2.0 - f());}
        double ep2() const{return e2() / (1.0 - e2());}

        void fromb(double b) {f_inv = 1.0 / (1.0 - b/a);}
        void fromf(double f) {f_inv = 1.0 / f;}
    };

    /// All the supported projection types. To add new ones, add them to this enum,
    /// create the parameters structure and the parser from geo to location (and viceversa).
    enum class ProjectionType {
        TransverseMercator,  // custom TM
        UTM,                 // Universal Transverse Mercator (WGS84)
        WebMercator,         // EPSG:3857
        LambertConic2SP,     // EPSG:… (common in EU)
    };

    // All are lacking the datum and ellipsoid, so suppose spherical Earth. 

    struct TransverseMercatorParams {
        double lat_0    = 0.0;              // latitude of origin
        double lon_0    = 0.0;              // longitude of origin
        double k        = 1.0;              // scale factor at origin
        double x_0      = 0.0;              // false easting
        double y_0      = 0.0;              // false northing
        Ellipsoid ellps = Ellipsoid();      // Earth0's ellipsoidal shape
    };

    struct UTMParams {
        int    zone     = 31;               // 1 to 60
        bool   north    = true;             // hemisphere
        double k        = 0.9996;           // scale factor at origin. This should be fixed
        double x_0      = 5000000.0;        // false easting
        double y_0      = 0.0;              // false northing
        Ellipsoid ellps = Ellipsoid();      // Earth0's ellipsoidal shape
    };

    struct WebMercatorParams {
        Ellipsoid ellps = Ellipsoid();      // Earth0's ellipsoidal shape. A sphere in this case.
    };

    struct LambertConic2SPParams {
        double lon_0    = 0.0;              // central meridian
        double lat_0    = 0.0;              // latitude of origin
        double lat_1    = 0.0;              // 1st standard parallel
        double lat_2    = 0.0;              // 2nd standard parallel
        double x_0      = 0.0;              // false easting
        double y_0      = 0.0;              // false northing
        double e        = 0.0818191908426;  // eccentricity
        Ellipsoid ellps = Ellipsoid();      // Earth0's ellipsoidal shape
    };

    using ProjectionParams = boost::variant2::variant<
        TransverseMercatorParams,
        UTMParams,
        WebMercatorParams,
        LambertConic2SPParams>;

    struct GeoProjection {

        template <typename T>
        static auto Make(T&& args)
        {
            GeoProjection r = { };
            r.params = ProjectionParams(std::forward<T>(args));
            return r;
        }

        /// Get the type of projection.
        ProjectionType getType() const {
            return static_cast<ProjectionType>(params.index());
        }

        /// Get the paramaters.
        const ProjectionParams& getParams() const {
            return params;
        }

        /// Set the Proj raw string.
        void setRawReference(std::string s) {
            raw_reference = std::move(s);
        }

        /// Get the Proj raw string.
        const std::string& getRawReference() const {
            return raw_reference;
        }

        // Projection parameters.
        ProjectionParams params;

        /// Proj string reference.
        std::string raw_reference;

        /// Transform the given location to a geo location.
        Location GeoLocationToTransform(const GeoLocation& geolocation) const;

        /// Transform the given geo location to a location using TransverseMercator.
        Location GeoLocationToTransformTransverseMercator(const GeoLocation& geolocation) const;

        /// Transform the given geo location to a location using UTM.
        Location GeoLocationToTransformUTM(const GeoLocation& geolocation) const;

        /// Transform the given geo location to a location using WebMercator.
        Location GeoLocationToTransformWebMercator(const GeoLocation& geolocation) const;

        /// Transform the given geo location to a location using LambertConic2SP.
        Location GeoLocationToTransformLambertConic2SP(const GeoLocation& geolocation) const;

        /// Transform the given geo location to a location.
        GeoLocation TransformToGeoLocation(const Location& location) const;

        /// Transform the given location to a geo location using TransverseMercator.
        GeoLocation TransformToGeoLocationTransverseMercator(const Location& location) const;

        /// Transform the given location to a geo location using UTM.
        GeoLocation TransformToGeoLocationUTM(const Location& location) const;

        /// Transform the given location to a geo location using WebMercator.
        GeoLocation TransformToGeoLocationWebMercator(const Location& location) const;

        /// Transform the given location to a geo location using LambertConic2SP.
        GeoLocation TransformToGeoLocationLambertConic2SP(const Location& location) const;

    };
} // namespace geom
} // namespace carla
