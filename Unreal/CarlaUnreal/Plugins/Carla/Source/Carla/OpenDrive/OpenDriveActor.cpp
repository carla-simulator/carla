// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/OpenDrive/OpenDriveActor.h"
#include "Carla.h"
#include "Carla/OpenDrive/OpenDrive.h"

#include <util/disable-ue4-macros.h>
#include <carla/geom/Math.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/element/Waypoint.h>
#include <carla/rpc/String.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "DrawDebugHelpers.h"
#include <util/ue-header-guard-end.h>

#include <algorithm>
#include <unordered_set>

AOpenDriveActor::AOpenDriveActor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  // Structure to hold one-time initialization
  static struct FConstructorStatics
  {
    // A helper class object we use to find target UTexture2D object in resource
    // package
    ConstructorHelpers::FObjectFinderOptional<UTexture2D> TextureObject;
    FName Category;
    FText Name;
    FConstructorStatics()
    // Use helper class object to find the texture resource path
      : TextureObject(TEXT("/Carla/Icons/OpenDriveActorIcon")),
        Category(TEXT("OpenDriveActor")),
        Name(NSLOCTEXT("SpriteCategory", "OpenDriveActor", "OpenDriveActor"))
    {}
  } ConstructorStatics;

  // We need a scene component to attach Icon sprite
  USceneComponent *SceneComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
  RootComponent = SceneComponent;
  RootComponent->Mobility = EComponentMobility::Static;

#if WITH_EDITORONLY_DATA
  SpriteComponent =
      ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
  if (SpriteComponent)
  {
    // Get the sprite texture from helper class object
    SpriteComponent->Sprite = ConstructorStatics.TextureObject.Get();
    // Assign sprite category name
    SpriteComponent->SpriteInfo.Category = ConstructorStatics.Category;
    // Assign sprite display name
    SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.Name;
    // Attach sprite to scene component
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->Mobility = EComponentMobility::Static;
    SpriteComponent->SetEditorScale(1.f);
  }
#endif // WITH_EDITORONLY_DATA
}

#if WITH_EDITOR
void AOpenDriveActor::PostEditChangeProperty(struct FPropertyChangedEvent &Event)
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

void AOpenDriveActor::BuildRoutes()
{
  BuildRoutes(GetWorld()->GetMapName());
}

void AOpenDriveActor::BuildRoutes(FString MapName)
{
  using Waypoint = carla::road::element::Waypoint;

  // As the OpenDrive file has the same name as level, build the path to the
  // xodr file using the lavel name and the game content directory.
  static FString XodrContent;
  if (CustomPath.IsEmpty()){
    XodrContent = UOpenDrive::LoadXODR(MapName);
  } else {
    XodrContent = UOpenDrive::LoadXODRFullPath(CustomPath);
  }

  auto map = carla::opendrive::OpenDriveParser::Load(carla::rpc::FromLongFString(XodrContent));

  if (!map.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("Failed to parse OpenDrive file."));
    return;
  }

  // List with waypoints, each one at the end of each lane of the map
  const std::vector<Waypoint> LaneWaypoints =
      map->GenerateWaypointsOnRoadEntries();

  std::unordered_map<Waypoint, std::vector<Waypoint>> PredecessorMap;

  for (auto &Wp : LaneWaypoints)
  {
    const auto PredecessorsList = map->GetPredecessors(Wp);
    if (PredecessorsList.empty())
    {
      continue;
    }
    const auto MinRoadId = *std::min_element(
        PredecessorsList.begin(),
        PredecessorsList.end(),
        [](const auto &WaypointA, const auto &WaypointB) {
          return WaypointA.road_id < WaypointB.road_id;
        });
    PredecessorMap[MinRoadId].emplace_back(Wp);
  }

  for (auto &&PredecessorWp : PredecessorMap)
  {
    ARoutePlanner *RoutePlanner = nullptr;

    for (auto &&Wp : PredecessorWp.second)
    {
      std::vector<Waypoint> Waypoints;
      auto CurrentWp = Wp;

      do
      {
        Waypoints.emplace_back(CurrentWp);
        const auto Successors = map->GetNext(CurrentWp, RoadAccuracy);
        if (Successors.empty())
        {
          break;
        }
        if (Successors.front().road_id != Wp.road_id)
        {
          break;
        }
        CurrentWp = Successors.front();
      } while (CurrentWp.road_id == Wp.road_id);

      // connect the last wp of the current road to the first wp of the following road
      const auto FollowingWp = map->GetSuccessors(CurrentWp);
      if (!FollowingWp.empty())
      {
        Waypoints.emplace_back(FollowingWp.front());
      }

      if (Waypoints.size() >= 2)
      {
        TArray<FVector> Positions;
        Positions.Reserve(Waypoints.size());
        for (int i = 0; i < Waypoints.size(); ++i)
        {
          // Add the trigger height because the z position of the points does not
          // influence on the driver AI and is easy to visualize in the editor
          Positions.Add(map->ComputeTransform(Waypoints[i]).location +
              FVector(0.f, 0.f, TriggersHeight));
        }

        // If the route planner does not exist, create it
        if (RoutePlanner == nullptr )
        {
          const auto WpTransform = map->ComputeTransform(Wp);
          RoutePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
          RoutePlanner->bIsIntersection = map->IsJunction(Wp.road_id);
          RoutePlanner->SetBoxExtent(FVector(70.f, 70.f, 50.f));
          RoutePlanner->SetActorRotation(WpTransform.rotation);
          RoutePlanner->SetActorLocation(WpTransform.location +
              FVector(0.f, 0.f, TriggersHeight));
        }

        if (RoutePlanner != nullptr)
        {
          RoutePlanner->AddRoute(1.f, Positions);
          RoutePlanners.Add(RoutePlanner);
        }
      }
    }
  }
}

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
      if (!bOnIntersections && RoutePlanners[i]->bIsIntersection)
      {
        continue;
      }
      else
      {
        FTransform Trans = RoutePlanners[i]->GetActorTransform();
        AVehicleSpawnPoint *Spawner = GetWorld()->SpawnActor<AVehicleSpawnPoint>();
        Spawner->SetActorRotation(Trans.GetRotation());
        Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight));
        VehicleSpawners.Add(Spawner);
      }
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
