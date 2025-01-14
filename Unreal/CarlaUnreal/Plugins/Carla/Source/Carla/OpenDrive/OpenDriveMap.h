// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <util/enable-ue4-macros.h>

#include "OpenDriveMap.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWaypoint
{
  GENERATED_BODY()

  carla::road::element::Waypoint Waypoint;
};

USTRUCT(BlueprintType)
struct CARLA_API FWaypointConnection
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite)
  FWaypoint Start;

  UPROPERTY(BlueprintReadWrite)
  FWaypoint End;
};

/// Helper class for exposing CARLA OpenDrive API to blueprints.
UCLASS(BlueprintType, Blueprintable)
class CARLA_API UOpenDriveMap : public UObject
{
  GENERATED_BODY()

public:

  UOpenDriveMap(const FObjectInitializer &ObjectInitializer);

  /// Return whether this map has been initialized.
  UFUNCTION(BlueprintCallable)
  bool HasMap() const
  {
    return Map.IsSet();
  }

  /// Load this map with an OpenDrive (XODR) file.
  UFUNCTION(BlueprintCallable)
  bool Load(const FString &XODRContent);

  /// Given a location, return the closest point on the centre of a lane.
  UFUNCTION(BlueprintCallable)
  FWaypoint GetClosestWaypointOnRoad(FVector Location, bool &Success) const;

  /// Generate waypoints all over the map at an approximated distance.
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GenerateWaypoints(float ApproxDistance = 100.0f) const;

  /// Generate the minimum set of waypoints that define the topology of this
  /// map. The waypoints are placed at the entrance of each lane.
  UFUNCTION(BlueprintCallable)
  TArray<FWaypointConnection> GenerateTopology() const;

  /// Generate waypoints on each lane at the start of each road.
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GenerateWaypointsOnRoadEntries() const;

  /// Compute the location of a waypoint.
  UFUNCTION(BlueprintCallable)
  FVector ComputeLocation(FWaypoint Waypoint) const;

  /// Compute the locations of an array of waypoints.
  UFUNCTION(BlueprintCallable)
  TArray<FVector> ComputeLocations(const TArray<FWaypoint> &Waypoints) const;

  /// Compute the transform of a waypoint. The X-axis is directed towards the
  /// direction of the road at that waypoint.
  UFUNCTION(BlueprintCallable)
  FTransform ComputeTransform(FWaypoint Waypoint) const;

  /// Compute the transforms of an array of waypoints.
  UFUNCTION(BlueprintCallable)
  TArray<FTransform> ComputeTransforms(const TArray<FWaypoint> &Waypoints) const;

  /// Return the list of waypoints at a given distance such that a vehicle at
  /// waypoint could drive to.
  UFUNCTION(BlueprintCallable)
  TArray<FWaypoint> GetNext(FWaypoint Waypoint, float Distance = 100.0f) const;

private:

  TOptional<carla::road::Map> Map;
};
