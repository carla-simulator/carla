// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Map.h"

#include <boost/optional.hpp>

#include <map>

namespace carla {
namespace road {

  class MapBuilder {
  public:

    boost::optional<Map> Build();

  private:

    /// Set the total length of each road based on the geometries
    void SetTotalRoadSegmentLength();

    /// Create the pointers between RoadSegments based on the ids
    void CreatePointersBetweenRoadSegments();

  public:
    // called from road link parser
    void AddRoad(const uint32_t road_id,
                 const std::string name,
                 const double length,
                 const int32_t junction_id,
                 const int32_t predecessor,
                 const int32_t successor);
    void SetRoadTypeSpeed(const uint32_t road_id,
                          const double s,
                          const std::string type,
                          const double max,
                          const std::string unit);
    void AddRoadSection(const uint32_t road_id,
                        const uint32_t section_index,
                        const double s,
                        const double a,
                        const double b,
                        const double c,
                        const double d);
    void SetRoadLaneLink(const uint32_t road_id,
                          const int32_t section_index,
                          const int32_t lane_id,
                          const std::string lane_type,
                          const bool lane_level,
                          const int32_t predecessor,
                          const int32_t successor);

    // called from geometry parser
    void AddRoadGeometryLine(const int32_t road_id,
                             const double s,
                             const double x,
                             const double y,
                             const double hdg,
                             const double length);
    void AddRoadGeometryArc(const int32_t road_id,
                            const double s,
                            const double x,
                            const double y,
                            const double hdg,
                            const double length,
                            const double curvature);
    void AddRoadGeometrySpiral(const int32_t road_id,
                               const double s,
                               const double x,
                               const double y,
                               const double hdg,
                               const double length,
                               const double curvStart,
                               const double curvEnd);
    void AddRoadGeometryPoly3(const int32_t road_id,
                              const double s,
                              const double x,
                              const double y,
                              const double hdg,
                              const double length,
                              const double a,
                              const double b,
                              const double c,
                              const double d);
    void AddRoadGeometryParamPoly3(const int32_t road_id,
                                   const double s,
                                   const double x,
                                   const double y,
                                   const double hdg,
                                   const double length,
                                   const double aU,
                                   const double bU,
                                   const double cU,
                                   const double dU,
                                   const double aV,
                                   const double bV,
                                   const double cV,
                                   const double dV,
                                   const std::string p_range);

    // called from profiles parser
    void AddRoadElevationProfile(const int32_t road_id,
                                         const double s,
                                         const double a,
                                         const double b,
                                         const double c,
                                         const double d);
    void AddRoadLateralSuperelevation(const int32_t road_id,
                                      const double s,
                                      const double a,
                                      const double b,
                                      const double c,
                                      const double d);
    void AddRoadLateralCrossfall(const int32_t road_id,
                                 const double s,
                                 const double a,
                                 const double b,
                                 const double c,
                                 const double d,
                                 const std::string side);
    void AddRoadLateralShape(const int32_t road_id,
                             const double s,
                             const double a,
                             const double b,
                             const double c,
                             const double d,
                             const double t);


    // called from junction parser
    void CreateJunction(const int32_t id);

    void AddConnection(
        const int32_t junction_id,
        const int32_t connection_id,
        const int32_t incoming_road,
        const int32_t connecting_road);

    void AddLaneLink(
        const int32_t junction_id,
        const int32_t connection_id,
        const int32_t from,
        const int32_t to);

    MapData _map_data;
  };

} // namespace road
} // namespace carla
