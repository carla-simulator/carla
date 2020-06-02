// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/OpenDrive/OpenDriveMap.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

namespace UOpenDriveMap_Private {

  template <typename RangeT>
  static auto GetSize(const RangeT &Range)
  {
    return Range.size();
  }

  template <typename T>
  static auto GetSize(const TArray<T> &Array)
  {
    return Array.Num();
  }


  template <typename T, typename RangeT, typename FuncT>
  static TArray<T> TransformToTArray(RangeT &&Range, FuncT &&TransformFunction)
  {
    TArray<T> Result;
    Result.Reserve(GetSize(Range));
    for (auto &&Item : Range)
    {
      Result.Emplace(TransformFunction(Item));
    }
    return Result;
  }

  template <typename T, typename RangeT>
  static TArray<T> TransformToTArray(RangeT &&Range)
  {
    return TransformToTArray<T>(
        std::forward<RangeT>(Range),
        [](auto &&Item) { return T{Item}; });
  }

} // namespace UOpenDriveMap_Private

UOpenDriveMap::UOpenDriveMap(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

bool UOpenDriveMap::Load(const FString &XODRContent)
{
  auto ResultMap = carla::opendrive::OpenDriveParser::Load(
      carla::rpc::FromLongFString(XODRContent));
  if (ResultMap)
  {
    Map = std::move(*ResultMap);
  }
  return HasMap();
}

FWaypoint UOpenDriveMap::GetClosestWaypointOnRoad(FVector Location, bool &Success) const
{
  check(HasMap());
  auto Result = Map->GetClosestWaypointOnRoad(Location);
  Success = Result.has_value();
  return Result.has_value() ? FWaypoint{*Result} : FWaypoint{};
}

TArray<FWaypoint> UOpenDriveMap::GenerateWaypoints(float ApproxDistance) const
{
  if (ApproxDistance < 1.0f)
  {
    UE_LOG(LogCarla, Error, TEXT("GenerateWaypoints: Please provide an ApproxDistance greater than 1 centimetre."));
    return {};
  }
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GenerateWaypoints(ApproxDistance / 1e2f));
}

TArray<FWaypointConnection> UOpenDriveMap::GenerateTopology() const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypointConnection>(Map->GenerateTopology(), [](auto &&Item) {
    return FWaypointConnection{FWaypoint{Item.first}, FWaypoint{Item.second}};
  });
}

TArray<FWaypoint> UOpenDriveMap::GenerateWaypointsOnRoadEntries() const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GenerateWaypointsOnRoadEntries());
}

FVector UOpenDriveMap::ComputeLocation(FWaypoint Waypoint) const
{
  return ComputeTransform(Waypoint).GetLocation();
}

TArray<FVector> UOpenDriveMap::ComputeLocations(const TArray<FWaypoint> &Waypoints) const
{
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FVector>(Waypoints, [this](auto &&Waypoint) {
    return ComputeLocation(Waypoint);
  });
}

FTransform UOpenDriveMap::ComputeTransform(FWaypoint Waypoint) const
{
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return Map->ComputeTransform(Waypoint.Waypoint);
}

TArray<FTransform> UOpenDriveMap::ComputeTransforms(const TArray<FWaypoint> &Waypoints) const
{
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FTransform>(Waypoints, [this](auto &&Waypoint) {
    return ComputeTransform(Waypoint);
  });
}

TArray<FWaypoint> UOpenDriveMap::GetNext(FWaypoint Waypoint, float Distance) const
{
  if (Distance < 1.0f)
  {
    UE_LOG(LogCarla, Error, TEXT("GetNext: Please provide a Distance greater than 1 centimetre."));
    return {};
  }
  check(HasMap());
  using namespace UOpenDriveMap_Private;
  return TransformToTArray<FWaypoint>(Map->GetNext(Waypoint.Waypoint, Distance / 1e2f));
}
