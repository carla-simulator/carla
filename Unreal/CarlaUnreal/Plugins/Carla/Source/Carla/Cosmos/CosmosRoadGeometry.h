// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// CosmosRoadGeometry.h
//
// Shared OpenDRIVE road-geometry walk used by both the Cosmos static
// exporters (LaneLineExporter, RoadBoundaryExporter) and the in-engine
// ACosmosControlSensor overlay.  It reconstructs painted lane-line and
// road-boundary polylines from carla::road::Map.
//
// The ue4-dev implementation walked ARoadSpline boundary points.  ARoadSpline
// is intentionally not ported; instead the polyline is reconstructed by
// sampling lane waypoints along s and offsetting the lane center outward by
// half the lane width along the lane right vector (the lane's painted outer
// border / outer edge).  Keeping this single source of truth guarantees the
// exporters and the sensor render the same geometry.
//
// All emitted vertices are in METERS (road::Map is metric).  The static
// exporters emit meters directly; the in-engine sensor multiplies by 100 to
// convert to Unreal centimeters.

#pragma once

#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/road/Lane.h>
#include <carla/road/element/Waypoint.h>
#include <carla/road/element/RoadInfoMarkRecord.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector3D.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include <set>
#include <vector>

namespace CosmosRoadGeometry
{
  // Sampling step along s, in meters (road::Map is metric).
  inline constexpr double SampleStep = 1.0;
  inline constexpr double HalfLaneWidthEpsilon = 1e-3;

  // Reconstruct the outer-border polyline of a single lane by walking s and
  // offsetting the lane center outward by half the lane width along the lane
  // right vector.  When bRequirePaintedMark is true, a sample is only kept
  // where the lane carries an outer painted mark record (the ue4 lane-line
  // behaviour); when false every sample with a non-zero width is kept (the
  // ue4 road-boundary behaviour).  Vertices are appended in meters.
  inline void BuildLaneOuterBorder(
      const carla::road::Map& Map,
      const carla::road::element::Waypoint& Entry,
      bool bRequirePaintedMark,
      TArray<FVector>& OutPolyMeters)
  {
    const carla::road::Lane& Lane = Map.GetLane(Entry);
    const double LaneStart = Lane.GetDistance();
    const double LaneEnd = LaneStart + Lane.GetLength();
    const double Sign = Lane.IsPositiveDirection() ? 1.0 : -1.0;

    double S = LaneStart;
    bool bMore = true;
    while (bMore)
    {
      if (S >= LaneEnd)
      {
        S = LaneEnd;
        bMore = false;
      }

      carla::road::element::Waypoint Waypoint = Entry;
      Waypoint.s = S;

      bool bKeep = true;
      if (bRequirePaintedMark)
      {
        // Outer painted-mark record at this s.  A mark record is required for
        // the border to be a painted lane line (ue4 only rendered painted
        // driving-lane boundaries).
        const auto MarkRecords = Map.GetMarkRecord(Waypoint);
        const carla::road::element::RoadInfoMarkRecord* OuterMark =
            MarkRecords.second;
        bKeep = (OuterMark != nullptr);
      }

      if (bKeep)
      {
        const carla::geom::Transform Transform = Map.ComputeTransform(Waypoint);
        const double HalfWidth = Map.GetLaneWidth(Waypoint) * 0.5;
        if (HalfWidth > HalfLaneWidthEpsilon)
        {
          const carla::geom::Vector3D Right = Transform.GetRightVector();
          const carla::geom::Location& Center = Transform.location;
          const FVector Point(
              static_cast<float>(Center.x + Sign * HalfWidth * Right.x),
              static_cast<float>(Center.y + Sign * HalfWidth * Right.y),
              static_cast<float>(Center.z + Sign * HalfWidth * Right.z));
          OutPolyMeters.Add(Point);
        }
      }

      S += SampleStep;
    }
  }

  // Build the outer-border polylines (in meters) for every lane of the given
  // lane-type selection across the whole map.  The const Map does not expose
  // its road map publicly, so the road set is derived from the per-road-entry
  // waypoints of the requested lane type.
  inline void BuildOuterBorderPolylines(
      const carla::road::Map& Map,
      carla::road::Lane::LaneType LaneTypeSelection,
      bool bRequirePaintedMark,
      TArray<TArray<FVector>>& OutPolylinesMeters)
  {
    std::set<carla::road::RoadId> RoadIds;
    for (const auto& Waypoint :
         Map.GenerateWaypointsOnRoadEntries(LaneTypeSelection))
    {
      RoadIds.insert(Waypoint.road_id);
    }

    for (const carla::road::RoadId RoadId : RoadIds)
    {
      const std::vector<carla::road::element::Waypoint> LaneEntries =
          Map.GenerateWaypointsInRoad(RoadId, LaneTypeSelection);

      for (const carla::road::element::Waypoint& Entry : LaneEntries)
      {
        TArray<FVector> PolyMeters;
        BuildLaneOuterBorder(Map, Entry, bRequirePaintedMark, PolyMeters);
        if (PolyMeters.Num() > 0)
        {
          OutPolylinesMeters.Add(std::move(PolyMeters));
        }
      }
    }
  }

  // Lane-type selection that matches the ue4 road-boundary export
  // (Shoulder | Sidewalk | Median).
  inline carla::road::Lane::LaneType BoundaryLaneTypes()
  {
    const int32_t Mask =
        static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |
        static_cast<int32_t>(carla::road::Lane::LaneType::Sidewalk) |
        static_cast<int32_t>(carla::road::Lane::LaneType::Median);
    return static_cast<carla::road::Lane::LaneType>(Mask);
  }

} // namespace CosmosRoadGeometry
