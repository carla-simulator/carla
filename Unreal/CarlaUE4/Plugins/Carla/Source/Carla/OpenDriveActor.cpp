// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "OpenDriveActor.h"

#include "Util/OpenDrive.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/road/WaypointGenerator.h>
#include <carla/rpc/String.h>
#include <carla/geom/Math.h>
#include <carla/road/element/Waypoint.h>
#include <compiler/enable-ue4-macros.h>

static TArray<FVector> WaypointVector2FVectorArray(
    const std::vector<carla::road::element::Waypoint> &Waypoints,
    const float TriggersHeight)
{
  TArray<FVector> Positions;
  Positions.Reserve(Waypoints.size());
  for (int i = 0; i < Waypoints.size(); ++i)
  {
    // Add the trigger height because the z position of the points does not
    // influence on the driver AI and is easy to visualize in the editor
    Positions.Add(Waypoints[i].ComputeTransform().location +
        FVector(0.f, 0.f, TriggersHeight));
  }
  return Positions;
}

AOpenDriveActor::AOpenDriveActor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficLightBP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/Streetlights_01/BP_TrafficLightPole.BP_TrafficLightPole'"));
  TrafficLightBlueprintClass = (UClass *) TrafficLightBP.Object->GeneratedClass;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficGroupBP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/Streetlights_01/BP_TrafficLightPoleGroup.BP_TrafficLightPoleGroup'"));
  TrafficGroupBlueprintClass = (UClass *) TrafficGroupBP.Object->GeneratedClass;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficSign30BP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/PostSigns/Round/SpeedLimiters/BP_SpeedLimit30.BP_SpeedLimit30'"));
  TrafficSign30BlueprintClass = (UClass *) TrafficSign30BP.Object->GeneratedClass;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficSign40BP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/PostSigns/Round/SpeedLimiters/BP_SpeedLimit40.BP_SpeedLimit40'"));
  TrafficSign40BlueprintClass = (UClass *) TrafficSign40BP.Object->GeneratedClass;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficSign60BP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/PostSigns/Round/SpeedLimiters/BP_SpeedLimit60.BP_SpeedLimit60'"));
  TrafficSign60BlueprintClass = (UClass *) TrafficSign60BP.Object->GeneratedClass;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficSign90BP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/PostSigns/Round/SpeedLimiters/BP_SpeedLimit90.BP_SpeedLimit90'"));
  TrafficSign90BlueprintClass = (UClass *) TrafficSign90BP.Object->GeneratedClass;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficSign100BP(TEXT(
      "Blueprint'/Game/Carla/Static/TrafficSigns/PostSigns/Round/SpeedLimiters/BP_SpeedLimit100.BP_SpeedLimit100'"));
  TrafficSign100BlueprintClass = (UClass *) TrafficSign100BP.Object->GeneratedClass;

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
  GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("hi")));

  using CarlaMath = carla::geom::Math;
  using IdType = carla::road::element::id_type;
  using Waypoint = carla::road::element::Waypoint;
  using WaypointGen = carla::road::WaypointGenerator;
  using TrafficGroup = carla::opendrive::types::TrafficLightGroup;
  using TrafficLight = carla::opendrive::types::TrafficLight;
  using TrafficBoxComponent = carla::opendrive::types::BoxComponent;
  using TrafficSign = carla::opendrive::types::TrafficSign;

  std::string ParseError;

  // As the OpenDrive file has the same name as level, build the path to the
  // xodr file using the lavel name and the game content directory.
  const FString XodrContent = FOpenDrive::Load(MapName);

  auto map_ptr = carla::opendrive::OpenDrive::Load(
      TCHAR_TO_UTF8(*XodrContent),
      XmlInputType::CONTENT,
      &ParseError);

  if (ParseError.size())
  {
    UE_LOG(LogCarla, Error, TEXT("OpenDrive parsing error: '%s'."), *carla::rpc::ToFString(ParseError));
    return;
  }

  const auto &map = map_ptr->GetData();

  // List with waypoints, each one at the end of each lane of the map
  const std::vector<Waypoint> MapLaneBeginWaypoint =
      WaypointGen::GenerateLaneEnd(*map_ptr);

  // Since we are going to iterate all the successors of all the lanes, we need
  // a vector to store the already visited lanes. Lanes can be successors of
  // multiple other lanes
  std::vector<std::pair<IdType, int>> AlreadyVisited;

  for (auto &&EndLaneWaypoint : MapLaneBeginWaypoint)
  {
    std::vector<Waypoint> Successors = WaypointGen::GetSuccessors(EndLaneWaypoint);

    // The RoutePlanner will be created only if some route must be added to it
    // so no one will be created unnecessarily
    ARoutePlanner *RoutePlanner = nullptr;

    // Fill the RoutePlanner with all the needed roads
    for (auto &&Successor : Successors)
    {
      const IdType RoadId = Successor.GetRoadId();
      const int LaneId = Successor.GetLaneId();

      // Create an identifier of the current lane
      const auto Identifier = std::make_pair(RoadId, LaneId);

      // If Identifier does not exist in AlreadyVisited we haven't visited the
      // lane
      if (!std::any_of(AlreadyVisited.begin(), AlreadyVisited.end(), [&Identifier](auto i) {
        return (i.first == Identifier.first && i.second == Identifier.second);
      }))
      {
        // Add the identifier as visited
        AlreadyVisited.emplace_back(Identifier);

        const float MaxDist = map.GetRoad(RoadId)->GetLength();

        std::vector<Waypoint> Waypoints;
        Waypoints.emplace_back(Successor);

        for (float Dist = RoadAccuracy; Dist < MaxDist; Dist += RoadAccuracy)
        {
          const auto NewWaypoint = WaypointGen::GetNext(Successor, Dist);

          check(Dist < MaxDist);
          check(NewWaypoint.size() == 1);

          Waypoints.emplace_back(NewWaypoint[0]);
        }

        // Merge with the first waypoint of the next lane if needed
        Waypoints.emplace_back(WaypointGen::GetNext(
            Successor, CarlaMath::clamp(MaxDist - 0.1f, 0.f, MaxDist))[0]);

        check(Waypoints.size() >= 2);

        TArray<FVector> Positions = WaypointVector2FVectorArray(Waypoints, TriggersHeight);

        // If the route planner does not exist, create it
        if (RoutePlanner == nullptr)
        {
          RoutePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
          RoutePlanner->bIsIntersection = std::any_of(Successors.begin(), Successors.end(), [](auto w) {
            return w.IsIntersection();
          });
          RoutePlanner->SetBoxExtent(FVector(70.f, 70.f, 50.f));
          RoutePlanner->SetActorRotation(EndLaneWaypoint.ComputeTransform().rotation);
          RoutePlanner->SetActorLocation(EndLaneWaypoint.ComputeTransform().location +
              FVector(0.f, 0.f, TriggersHeight));
        }

        RoutePlanner->AddRoute(1.f, Positions);
        RoutePlanners.Add(RoutePlanner);
      }
    }
  }

  const std::vector<TrafficGroup> TrafficLightGroup = map.GetTrafficGroups();
  for (TrafficGroup CurrentTrafficLightGroup : TrafficLightGroup)
  {
    double RedTime = CurrentTrafficLightGroup.red_time;
    double YellowTime = CurrentTrafficLightGroup.yellow_time;
    double GreenTime = CurrentTrafficLightGroup.green_time;
    FActorSpawnParameters SpawnParams;
    FOutputDeviceNull ar;
    AActor *SpawnedTrafficGroup = GetWorld()->SpawnActor<AActor>(TrafficGroupBlueprintClass,
        FVector(0, 0, 0),
        FRotator(0, 0, 0),
        SpawnParams);

    for (TrafficLight CurrentTrafficLight : CurrentTrafficLightGroup.traffic_lights)
    {
      FVector TLPos =
          FVector(CurrentTrafficLight.x_pos, CurrentTrafficLight.y_pos, CurrentTrafficLight.z_pos);
      FRotator TLRot = FRotator(CurrentTrafficLight.x_rot,
          CurrentTrafficLight.z_rot,
          CurrentTrafficLight.y_rot);
      AActor *SpawnedTrafficLight = GetWorld()->SpawnActor<AActor>(TrafficLightBlueprintClass,
          TLPos,
          TLRot,
          SpawnParams);
      FString AddTrafficLightCommand = FString::Printf(TEXT("AddTrafficLightPole %s"),
            *SpawnedTrafficLight->GetName());
      SpawnedTrafficGroup->CallFunctionByNameWithArguments(*AddTrafficLightCommand, ar, NULL, true);
      PersistentTrafficLights.Push(SpawnedTrafficGroup);
      SpawnedTrafficLight->CallFunctionByNameWithArguments(TEXT("InitData"), ar, NULL, true);
      for (TrafficBoxComponent TfBoxComponent : CurrentTrafficLight.box_areas)
      {
        FVector TLBoxPos = FVector(TfBoxComponent.x_pos,
            TfBoxComponent.y_pos,
            TfBoxComponent.z_pos);
        FRotator TLBoxRot = FRotator(TfBoxComponent.x_rot,
            TfBoxComponent.z_rot,
            TfBoxComponent.y_rot);

        FString BoxCommand = FString::Printf(TEXT("SetBoxLocationAndRotation %f %f %f %f %f %f"),
            TLBoxPos.X,
            TLBoxPos.Y,
            TLBoxPos.Z,
            TLBoxRot.Pitch,
            TLBoxRot.Roll,
            TLBoxRot.Yaw);
        SpawnedTrafficLight->CallFunctionByNameWithArguments(*BoxCommand, ar, NULL, true);
        PersistentTrafficLights.Push(SpawnedTrafficLight);
      }
    }
  }

  const std::vector<TrafficSign> TrafficSigns = map.GetTrafficSigns();
  for (TrafficSign CurrentTrafficSign : TrafficSigns) {
    //switch()
    AActor* SignActor;
    FOutputDeviceNull ar;

    FVector TSLoc = FVector(CurrentTrafficSign.x_pos, CurrentTrafficSign.y_pos, CurrentTrafficSign.z_pos);
    FRotator TSRot = FRotator(CurrentTrafficSign.x_rot, CurrentTrafficSign.z_rot, CurrentTrafficSign.y_rot);
    FActorSpawnParameters SpawnParams;
    switch(CurrentTrafficSign.speed) {
      case 30:
        SignActor = GetWorld()->SpawnActor<AActor>(TrafficSign30BlueprintClass,
          TSLoc,
          TSRot,
          SpawnParams);
        break;
      case 40:
        SignActor = GetWorld()->SpawnActor<AActor>(TrafficSign40BlueprintClass,
          TSLoc,
          TSRot,
          SpawnParams);
      break;
      case 60:
        SignActor = GetWorld()->SpawnActor<AActor>(TrafficSign60BlueprintClass,
          TSLoc,
          TSRot,
          SpawnParams);
        break;
      case 90:
        SignActor = GetWorld()->SpawnActor<AActor>(TrafficSign90BlueprintClass,
          TSLoc,
          TSRot,
          SpawnParams);
        break;
      case 100:
        SignActor = GetWorld()->SpawnActor<AActor>(TrafficSign100BlueprintClass,
          TSLoc,
          TSRot,
          SpawnParams);
      break;
    }
    PersistentTrafficSigns.Push(SignActor);
    for (TrafficBoxComponent TfBoxComponent : CurrentTrafficSign.box_areas)
      {
        FVector TLBoxPos = FVector(TfBoxComponent.x_pos,
            TfBoxComponent.y_pos,
            TfBoxComponent.z_pos);
        FRotator TLBoxRot = FRotator(TfBoxComponent.x_rot,
            TfBoxComponent.z_rot,
            TfBoxComponent.y_rot);

        FString BoxCommand = FString::Printf(TEXT("SetBoxLocationAndRotation %f %f %f %f %f %f"),
            TLBoxPos.X,
            TLBoxPos.Y,
            TLBoxPos.Z,
            TLBoxRot.Pitch,
            TLBoxRot.Roll,
            TLBoxRot.Yaw);
        SignActor->CallFunctionByNameWithArguments(*BoxCommand, ar, NULL, true);
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
  const int tl_num = PersistentTrafficLights.Num();
  for (int i = 0; i < tl_num; i++)
  {
    if(PersistentTrafficLights[i] != nullptr) {
      PersistentTrafficLights[i]->Destroy();
    }
  }
  PersistentTrafficLights.Empty();
  const int ts_num = PersistentTrafficSigns.Num();
  for (int i = 0; i < ts_num; i++)
  {
    if(PersistentTrafficSigns[i] != nullptr) {
      PersistentTrafficSigns[i]->Destroy();
    }
  }
  PersistentTrafficSigns.Empty();
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
#endif   // WITH_EDITOR
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
