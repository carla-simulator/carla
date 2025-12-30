#pragma once
#include <string>
#include "carla/geom/GeoProjectionsParams.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>
#pragma warning(pop)
#else
#include <boost/variant2/variant.hpp>
#endif

namespace carla {
namespace geom {

    class Location;
    class Transform;
    class GeoLocation;

    /// All the supported projection types. To add new ones, add them to this enum,
    /// create the parameters structure and the parser from geo to location (and viceversa).
    enum class ProjectionType {
        TransverseMercator,
        UniversalTransverseMercator,
        WebMercator,
        LambertConformalConic,
    };

    using ProjectionParams = boost::variant2::variant<
        TransverseMercatorParams,
        UniversalTransverseMercatorParams,
        WebMercatorParams,
        LambertConformalConicParams>;

    struct GeoProjection {

        template <typename T>
        static GeoProjection Make(T&& args)
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
        void setPROJString(std::string s) {
            proj_string = std::move(s);
        }

        /// Get the Proj raw string.
        const std::string& getPROJString() const {
            return proj_string;
        }

        // Projection parameters.
        ProjectionParams params;

        /// Proj string reference.
        std::string proj_string;

        /// Transform the given location to a geo location.
        Location GeoLocationToTransform(const GeoLocation& geolocation) const;

        /// Transform the given geo location to a location using TransverseMercator.
        Location GeoLocationToTransformTransverseMercator(
            const GeoLocation& geolocation, const TransverseMercatorParams params) const;

        /// Transform the given geo location to a location using UniversalTransverseMercator.
        Location GeoLocationToTransformUniversalTransverseMercator(
            const GeoLocation& geolocation, const UniversalTransverseMercatorParams params) const;

        /// Transform the given geo location to a location using WebMercator.
        Location GeoLocationToTransformWebMercator(
            const GeoLocation& geolocation, const WebMercatorParams params) const;

        /// Transform the given geo location to a location using LambertConformalConic.
        Location GeoLocationToTransformLambertConformalConic(
            const GeoLocation& geolocation, const LambertConformalConicParams params) const;

        /// Transform the given geo location to a location.
        GeoLocation TransformToGeoLocation(const Location& location) const;

        /// Transform the given location to a geo location using TransverseMercator.
        GeoLocation TransformToGeoLocationTransverseMercator(
            const Location& location, const TransverseMercatorParams params) const;

        /// Transform the given location to a geo location using UniversalTransverseMercator.
        GeoLocation TransformToGeoLocationUniversalTransverseMercator(
            const Location& location, const UniversalTransverseMercatorParams) const;

        /// Transform the given location to a geo location using WebMercator.
        GeoLocation TransformToGeoLocationWebMercator(
            const Location& location, const WebMercatorParams params) const;

        /// Transform the given location to a geo location using LambertConformalConic.
        GeoLocation TransformToGeoLocationLambertConformalConic(
            const Location& location, const LambertConformalConicParams params) const;

    };
} // namespace geom
} // namespace carla
