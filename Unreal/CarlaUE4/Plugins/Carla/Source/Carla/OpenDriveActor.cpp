// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "OpenDriveActor.h"

#include "Util/OpenDrive.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/String.h>
#include <carla/geom/Math.h>
#include <compiler/enable-ue4-macros.h>

#include <functional>

TArray<FVector> DirectedPointArray2FVectorArray(
  const TArray<AOpenDriveActor::DirectedPoint> &DirectedPoints)
{
  TArray<FVector> Positions;
  Positions.Reserve(DirectedPoints.Num());
  for (int i = 0; i < DirectedPoints.Num(); ++i)
  {
    Positions.Add(DirectedPoints[i].location);
  }
  return Positions;
}

AOpenDriveActor::AOpenDriveActor(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  // Structure to hold one-time initialization
  static struct FConstructorStatics
  {
    // A helper class object we use to find target UTexture2D object in resource package
    ConstructorHelpers::FObjectFinderOptional<UTexture2D> TextureObject;
    FName Category;
    FText Name;
    FConstructorStatics()
      // Use helper class object to find the texture resource path
      : TextureObject(TEXT("/Carla/Icons/OpenDriveActorIcon"))
      , Category(TEXT("OpenDriveActor"))
      , Name(NSLOCTEXT("SpriteCategory", "OpenDriveActor", "OpenDriveActor"))
      {
      }
  } ConstructorStatics;

  // We need a scene component to attach Icon sprite
  USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
  RootComponent = SceneComponent;
  RootComponent->Mobility = EComponentMobility::Static;

#if WITH_EDITORONLY_DATA
  SpriteComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
  if (SpriteComponent)
  {
    SpriteComponent->Sprite = ConstructorStatics.TextureObject.Get();    // Get the sprite texture from helper class object
    SpriteComponent->SpriteInfo.Category = ConstructorStatics.Category;  // Assign sprite category name
    SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.Name;   // Assign sprite display name
    SpriteComponent->SetupAttachment(RootComponent); // Attach sprite to scene component
    SpriteComponent->Mobility = EComponentMobility::Static;
    SpriteComponent->SetEditorScale(1.5f);
  }
#endif // WITH_EDITORONLY_DATA
}

#if WITH_EDITOR
void AOpenDriveActor::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
  Super::PostEditChangeProperty(Event);

  const FName PropertyName = (Event.Property != NULL ? Event.Property->GetFName() : NAME_None);
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bGenerateRoutes))
  {
    if (bGenerateRoutes)
    {
      bGenerateRoutes = false;

      RemoveRoutes(); // Avoid OpenDrive overlapping
      RemoveSpawners(); // Restart the spawners in case OpenDrive has changed
      BuildRoutes();

      if (bAddSpawners)
      {
        AddSpawners();
      }
      if (bShowDebug)
      {
        DebugRoutes();
      }
    }
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bRemoveRoutes))
  {
    if (bRemoveRoutes)
    {
      bRemoveRoutes = false;

      RemoveDebugRoutes();
      RemoveSpawners();
      RemoveRoutes();
    }
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bShowDebug))
  {
    if (bShowDebug)
    {
      DebugRoutes();
    }
    else
    {
      RemoveDebugRoutes();
    }
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(AOpenDriveActor, bRemoveCurrentSpawners))
  {
    if (bRemoveCurrentSpawners)
    {
      bRemoveCurrentSpawners = false;

      RemoveSpawners();
    }
  }
}
#endif // WITH_EDITOR

ARoutePlanner *AOpenDriveActor::GenerateRoutePlanner(const TArray<DirectedPoint> &DirectedPoints)
{
  using CarlaMath = carla::geom::Math;

  TArray<FVector> Positions = DirectedPointArray2FVectorArray(DirectedPoints);
  ARoutePlanner *RoutePlanner = GetWorld()->SpawnActor<ARoutePlanner>();

  RoutePlanner->SetActorRotation(FRotator(0.0f, CarlaMath::to_degrees(DirectedPoints[0].tangent), 0.0f));
  RoutePlanner->SetActorLocation(DirectedPoints[0].location);
  RoutePlanner->SetBoxExtent(FVector(70.0f, 70.0f, 50.0f));
  RoutePlanner->AddRoute(1.0f, Positions);
  RoutePlanner->Init();
  RoutePlanners.Add(RoutePlanner);
  return RoutePlanner;
}

void AOpenDriveActor::BuildRoutes()
{
  using IdType = carla::road::element::id_type;

  std::string ParseError;

  // NOTE(Andrei): As the OpenDrive file has the same name as level,
  // build the path to the xodr file using the lavel name and the
  // game content directory.
  FString MapName = GetWorld()->GetMapName();
  FString XodrContent = FOpenDrive::Load(MapName);

  auto map_ptr = carla::opendrive::OpenDrive::Load(TCHAR_TO_UTF8(*XodrContent),
      XmlInputType::CONTENT,
      &ParseError);

  if (ParseError.size())
  {
    UE_LOG(LogCarla, Error, TEXT("OpenDrive parsing error: '%s'."), *carla::rpc::ToFString(ParseError));
    return;
  }

  const auto &map = map_ptr->GetData();
  std::vector<carla::road::lane_junction_t> JunctionInfo = map.GetJunctionInformation();

  // NOTE(Andrei): Build the roads that are not junctions
  auto RoadIDsView = map.GetAllIds();
  std::vector<IdType> roadIDs(RoadIDsView.begin(), RoadIDsView.end());
  std::sort(roadIDs.begin(), roadIDs.end());

  for (auto &&id : roadIDs)
  {
    GenerateWaypointsRoad(map.GetRoad(id));
  }

  // NOTE(Andrei): Build the roads that are junctions as one RoutePlanner
  // can have more than one path that can be taken

  //       junctionId    roadID        laneID
  std::map<int, std::map<int, std::map<int, ARoutePlanner *>>> Junctions;

  for (auto && Junction : JunctionInfo)
  {
    TArray<TArray<DirectedPoint>> Waypoints;

    int FromRoadID = Junction.incomming_road;
    int ToRoadID = Junction.connection_road;
    int JunctonID = Junction.junction_id;

    GenerateWaypointsJunction(map.GetRoad(ToRoadID), Waypoints);
    ARoutePlanner *routePlanner = nullptr;

    std::sort(Junction.from_lane.begin(), Junction.from_lane.end(), std::greater<int>());

    if (Junction.from_lane[0] < 0)
    {
      std::reverse(Junction.from_lane.begin(), Junction.from_lane.end());
    }

    for (size_t n = 0; n < Junction.from_lane.size(); ++n)
    {
      int FromLaneID = Junction.from_lane[n];
      routePlanner = Junctions[JunctonID][FromRoadID][FromLaneID];

      if (routePlanner == nullptr)
      {
        routePlanner = GenerateRoutePlanner(Waypoints[n]);
        routePlanner->SetSplineColor(FColor::MakeRandomColor());
        Junctions[JunctonID][FromRoadID][FromLaneID] = routePlanner;
      }
      else
      {
        routePlanner->AddRoute(1.0, DirectedPointArray2FVectorArray(Waypoints[n]));
      }
    }
  }
}

/// Remove all the existing ARoutePlanner and VehicleSpawners previously
/// generated by this class to avoid overlapping
void AOpenDriveActor::RemoveRoutes()
{
  const int rp_num = RoutePlanners.Num();
  for (int i = 0; i < rp_num; i++)
  {
    if (RoutePlanners[i] != nullptr)
    {
      RoutePlanners[i]->Destroy();
    }
  }
  RoutePlanners.Empty();
}

TArray<AOpenDriveActor::DirectedPoint> AOpenDriveActor::GenerateLaneZeroPoints(
    const RoadSegment *road)
{
  size_t LanesOffsetIndex = 0;
  TArray<DirectedPoint> LaneZeroPoints;

  const RoadGeneralInfo *generalInfo =
      road->GetInfo<RoadGeneralInfo>(0.0);
  std::vector<std::pair<double, double>> LanesOffset = generalInfo->GetLanesOffset();

  for (float WaypointsOffset = 0.0f; WaypointsOffset < road->GetLength() + RoadAccuracy; WaypointsOffset += RoadAccuracy)
  {
    // NOTE(Andrei): Calculate the which laneOffset has to be used
    if (LanesOffsetIndex < LanesOffset.size() - 1 &&
        WaypointsOffset >= LanesOffset[LanesOffsetIndex + 1].first)
    {
      ++LanesOffsetIndex;
    }

    // NOTE(Andrei): Get waypoin at the offset, and invert the y axis
    DirectedPoint Waypoint = road->GetDirectedPointIn(WaypointsOffset);
    Waypoint.location.z = 1;

    // NOTE(Andrei): Applyed the laneOffset of the lane section
    Waypoint.ApplyLateralOffset(LanesOffset[LanesOffsetIndex].second);

    LaneZeroPoints.Add(Waypoint);
  }

  return LaneZeroPoints;
}

TArray<TArray<AOpenDriveActor::DirectedPoint>> AOpenDriveActor::GenerateRightLaneWaypoints(
    const RoadSegment *road,
    const TArray<DirectedPoint> &laneZeroPoints)
{
  const RoadInfoLane *lanesInfo =
      road->GetInfo<RoadInfoLane>(0.0);
  std::vector<int> rightLanes =
      lanesInfo->getLanesIDs(RoadInfoLane::which_lane_e::Right);

  TArray<TArray<DirectedPoint>> retWaypoints;
  double currentOffset = 0.0;

  for (size_t j = 0; j < rightLanes.size(); ++j)
  {
    const LaneInfo *laneInfo = lanesInfo->getLane(rightLanes[j]);
    const float HalfWidth = laneInfo->_width * 0.5;

    currentOffset += HalfWidth;
    if (laneInfo->_type == "driving")
    {
      TArray<DirectedPoint> roadWaypoints;
      for (int i = 0; i < laneZeroPoints.Num(); ++i)
      {
        DirectedPoint currentPoint = laneZeroPoints[i];
        currentPoint.ApplyLateralOffset(-currentOffset);
        roadWaypoints.Add(currentPoint);
      }
      if (roadWaypoints.Num() >= 2)
      {
        retWaypoints.Add(roadWaypoints);
      }
    }
    currentOffset += HalfWidth;
  }
  return retWaypoints;
}

TArray<TArray<AOpenDriveActor::DirectedPoint>> AOpenDriveActor::GenerateLeftLaneWaypoints(
    const RoadSegment *road,
    const TArray<DirectedPoint> &laneZeroPoints)
{
  using CarlaMath = carla::geom::Math;

  const RoadInfoLane *lanesInfo =
      road->GetInfo<RoadInfoLane>(0.0);
  std::vector<int> leftLanes = lanesInfo->getLanesIDs(RoadInfoLane::which_lane_e::Left);

  TArray<TArray<DirectedPoint>> retWaypoints;
  double currentOffset = 0.0;

  for (size_t j = 0; j < leftLanes.size(); ++j)
  {
    const LaneInfo *laneInfo = lanesInfo->getLane(leftLanes[j]);
    const float HalfWidth = laneInfo->_width * 0.5;

    currentOffset += HalfWidth;
    if (laneInfo->_type == "driving")
    {
      TArray<DirectedPoint> roadWaypoints;
      for (int i = 0; i < laneZeroPoints.Num(); ++i)
      {
        DirectedPoint currentPoint = laneZeroPoints[i];
        currentPoint.ApplyLateralOffset(currentOffset);
        if (currentPoint.tangent + CarlaMath::pi() < CarlaMath::pi_double())
        {
          currentPoint.tangent += CarlaMath::pi();
        }
        else
        {
          currentPoint.tangent -= CarlaMath::pi();
        }
        roadWaypoints.Add(currentPoint);
      }
      if (roadWaypoints.Num() >= 2)
      {
        Algo::Reverse(roadWaypoints);
        retWaypoints.Add(roadWaypoints);
      }
    }
    currentOffset += HalfWidth;
  }
  return retWaypoints;
}

void AOpenDriveActor::GenerateWaypointsRoad(const RoadSegment *road)
{
  const RoadGeneralInfo *generalInfo =
      road->GetInfo<RoadGeneralInfo>(0.0);
  if (generalInfo->GetJunctionId() > -1)
  {
    return;
  }

  TArray<DirectedPoint> laneZeroPoints = GenerateLaneZeroPoints(road);

  TArray<TArray<DirectedPoint>> rightLaneWaypoints = GenerateRightLaneWaypoints(road, laneZeroPoints);
  TArray<TArray<DirectedPoint>> leftLaneWaypoints = GenerateLeftLaneWaypoints(road, laneZeroPoints);

  for (int i = 0; i < rightLaneWaypoints.Num(); ++i)
  {
    GenerateRoutePlanner(rightLaneWaypoints[i]);
  }

  for (int i = 0; i < leftLaneWaypoints.Num(); ++i)
  {
    GenerateRoutePlanner(leftLaneWaypoints[i]);
  }
}

void AOpenDriveActor::GenerateWaypointsJunction(
    const RoadSegment *road,
    TArray<TArray<DirectedPoint>> &out_waypoints)
{
  const RoadGeneralInfo *generalInfo =
      road->GetInfo<RoadGeneralInfo>(0.0);
  if (generalInfo->GetJunctionId() == -1)
  {
    return;
  }

  TArray<DirectedPoint> laneZeroPoints = GenerateLaneZeroPoints(road);
  out_waypoints = GenerateRightLaneWaypoints(road, laneZeroPoints);

  if (out_waypoints.Num() == 0)
  {
    out_waypoints = GenerateLeftLaneWaypoints(road, laneZeroPoints);
  }
}

void AOpenDriveActor::DebugRoutes() const
{
  for (int i = 0; i < RoutePlanners.Num(); ++i)
  {
    if (RoutePlanners[i] != nullptr)
    {
      RoutePlanners[i]->DrawRoutes();
    }
  }
}

void AOpenDriveActor::RemoveDebugRoutes() const
{
  #if WITH_EDITOR
  FlushPersistentDebugLines(GetWorld());
  #endif // WITH_EDITOR
}

void AOpenDriveActor::AddSpawners()
{
  for (int i = 0; i < RoutePlanners.Num(); ++i)
  {
    if (RoutePlanners[i] != nullptr)
    {
      FTransform Trans = RoutePlanners[i]->GetActorTransform();
      AVehicleSpawnPoint *Spawner = GetWorld()->SpawnActor<AVehicleSpawnPoint>();
      Spawner->SetActorRotation(Trans.GetRotation());
      Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.0f, 0.0f, SpawnersHeight));
      VehicleSpawners.Add(Spawner);
    }
  }
}

void AOpenDriveActor::RemoveSpawners()
{
  const int vs_num = VehicleSpawners.Num();
  for (int i = 0; i < vs_num; i++)
  {
    if (VehicleSpawners[i] != nullptr)
    {
      VehicleSpawners[i]->Destroy();
    }
  }
  VehicleSpawners.Empty();
}
