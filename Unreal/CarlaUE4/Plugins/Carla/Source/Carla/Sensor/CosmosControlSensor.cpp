// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/CosmosControlSensor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/LineBatchComponent.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "carla/Traffic/RoutePlanner.h"
#include "carla/Game/Tagger.h"
#include "carla/Game/CarlaGameModeBase.h"
#include "carla/road/Map.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"


FActorDefinition ACosmosControlSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("cosmos_visualization"));
}

ACosmosControlSensor::ACosmosControlSensor(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Tags.Add(FName(TEXT("CosmosControlSensor")));
  added_persisted_stop_lines = true;
  added_persisted_route_lines = true;
  added_persisted_crosswalks = true;

  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  //AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  // TODO: Setup OnActorSpawnHandler so we can refresh components
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &ACosmosControlSensor::OnActorSpawned));
}

void ACosmosControlSensor::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  Super::SetUpSceneCaptureComponent(SceneCapture);

  ApplyViewMode(VMI_Unlit, true, SceneCapture.ShowFlags);

  SceneCapture.ShowFlags.SetAtmosphere(false);

  SceneCapture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  SceneCapture.ShowOnlyComponents.Emplace(GetWorld()->LineBatcher);
  SceneCapture.ShowOnlyComponents.Emplace(GetWorld()->PersistentLineBatcher);
  GetWorld()->PersistentLineBatcher->Flush();
}


void ACosmosControlSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACosmosControlSensor::PostPhysTick);

  int depth_prio = 0;

  int dist_alpha = 255;
  float cutoff_dist = 3000.0f;

  //TODO: Move to sensor once it's able to be spawned
  TArray<UObject*> CosmosRelevantComponents;
  GetObjectsOfClass(UMeshComponent::StaticClass(), CosmosRelevantComponents, true, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  for (UObject* Object : CosmosRelevantComponents) {
    UMeshComponent* mesh_component = Cast<UMeshComponent>(Object);
    if (mesh_component->GetOwner() == nullptr) continue;

    FVector box_origin, box_extent;
    FBoxSphereBounds bounds;
    UKismetSystemLibrary::GetActorBounds(mesh_component->GetOwner(), box_origin, box_extent);


    bounds = FBoxSphereBounds(box_origin, box_extent, 0.0f);

    UStaticMeshComponent* static_mesh_comp = Cast<UStaticMeshComponent>(mesh_component);
    USkeletalMeshComponent* skeletal_mesh_comp = Cast<USkeletalMeshComponent>(mesh_component);
    //TODO:Specialize for Skeletal
    if (static_mesh_comp != nullptr)
    {
      if(static_mesh_comp->GetStaticMesh())
      {
        bounds = static_mesh_comp->GetStaticMesh()->GetBounds();
      }
    }
    else if(skeletal_mesh_comp != nullptr)
    {
      if (skeletal_mesh_comp->SkeletalMesh)
      {
        //bounds = skeletal_mesh_comp->SkeletalMesh->GetBounds();
      }
    }

    const carla::rpc::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*mesh_component);
    FColor vis_color;
    switch (Tag) {
    case carla::rpc::CityObjectLabel::TrafficLight:
      vis_color = FColor(109, 144, 136);
      break;
    case carla::rpc::CityObjectLabel::TrafficSigns:
      vis_color = FColor(214, 144, 137);
      break;
    case carla::rpc::CityObjectLabel::Poles:
      vis_color = FColor(58, 4, 124);
      break;
    case carla::rpc::CityObjectLabel::Car:
    case carla::rpc::CityObjectLabel::Bicycle:
    case carla::rpc::CityObjectLabel::Bus:
    case carla::rpc::CityObjectLabel::Motorcycle:
      vis_color = FColor::Red;
      break;
    case carla::rpc::CityObjectLabel::Pedestrians:
      vis_color = FColor::Green;
      break;
    }

    if (Tag == carla::rpc::CityObjectLabel::TrafficLight || Tag == carla::rpc::CityObjectLabel::TrafficSigns)
    {
      //DrawDebugSolidBox(World, box_origin, box_extent, mesh_component->GetComponentRotation().Quaternion(), FColor::Cyan);
      DrawDebugBox(World, mesh_component->GetComponentLocation(), bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color.WithAlpha(dist_alpha), false, -1, depth_prio, 10);
      //DrawDebugSolidBox(World, box_origin, bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color, false, -1.0f, 10U);
      //DrawDebugSolidPlane(World, FPlane(FVector(0.0f, 1.0f, 0.0f), 1.0f), box_origin, FVector2D(bounds.BoxExtent.X, bounds.BoxExtent.Z), vis_color);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Car ||
      Tag == carla::rpc::CityObjectLabel::Bicycle ||
      Tag == carla::rpc::CityObjectLabel::Bus ||
      Tag == carla::rpc::CityObjectLabel::Motorcycle ||
      Tag == carla::rpc::CityObjectLabel::Pedestrians ||
      Tag == carla::rpc::CityObjectLabel::Train ||
      Tag == carla::rpc::CityObjectLabel::Truck)
    {
      DrawDebugBox(World, box_origin, bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color.WithAlpha(dist_alpha), false, -1, depth_prio, 10);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Poles)
    {
      //Filtering out horizontal poles
      if (fmax(bounds.BoxExtent.X, bounds.BoxExtent.Y) > bounds.BoxExtent.Z) continue;

      float half_height = bounds.BoxExtent.Z;
      DrawDebugCapsule(World, mesh_component->GetComponentLocation() + FVector(0.0f, 0.0f, half_height), half_height, 0.1f, FQuat::Identity, vis_color.WithAlpha(dist_alpha), false, -1, depth_prio, 20);
    }
  }


  if (added_persisted_stop_lines)
  {
    added_persisted_stop_lines = false;

    TArray<AActor*> TrafficLights;
    UGameplayStatics::GetAllActorsOfClass(World, ATrafficLightBase::StaticClass(), TrafficLights);

    for (AActor* traffic_light : TrafficLights)
    {
      UBoxComponent* stop_box_collider = Cast<UBoxComponent>(traffic_light->GetComponentByClass(UBoxComponent::StaticClass()));
      //DrawDebugCapsule(World, stop_box_collider->GetComponentLocation() - FVector(0.0f, 0.0f, stop_box_collider->GetScaledBoxExtent().Z), stop_box_collider->GetScaledBoxExtent().X, 0.1f, stop_box_collider->GetRightVector()., FColor::Red, false, -1, 0, 20);
      //DrawDebugLine(const UWorld * InWorld, FVector const& LineStart, FVector const& LineEnd, FColor const& Color, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0, float Thickness = 0.f) {}
      DrawDebugLine(World,
        stop_box_collider->GetComponentLocation() - FVector(0.0f, 0.0f, stop_box_collider->GetScaledBoxExtent().Z) + -stop_box_collider->GetScaledBoxExtent().X * stop_box_collider->GetForwardVector() - 710.0f * stop_box_collider->GetRightVector(),
        stop_box_collider->GetComponentLocation() - FVector(0.0f, 0.0f, stop_box_collider->GetScaledBoxExtent().Z) + stop_box_collider->GetScaledBoxExtent().X * stop_box_collider->GetForwardVector() - 710.0f * stop_box_collider->GetRightVector(),
        FColor::Red.WithAlpha(dist_alpha), true, -1, depth_prio, 20);
    }
  }

  if(added_persisted_route_lines)
  {
    added_persisted_route_lines = false;

    TArray<AActor*> RouteSplines;
    UGameplayStatics::GetAllActorsOfClass(World, ARoutePlanner::StaticClass(), RouteSplines);
    for (AActor* RouteSpline : RouteSplines)
    {
      ARoutePlanner* route_planner = Cast<ARoutePlanner>(RouteSpline);

      //route_planner->DrawRoutes();
      for (int i = 0, lenRoutes = route_planner->Routes.Num(); i < lenRoutes; ++i)
      {
        for (int j = 0, lenNumPoints = route_planner->Routes[i]->GetNumberOfSplinePoints() - 1; j < lenNumPoints; ++j)
        {
          const FVector p0 = route_planner->Routes[i]->GetLocationAtSplinePoint(j + 0, ESplineCoordinateSpace::World);
          const FVector p1 = route_planner->Routes[i]->GetLocationAtSplinePoint(j + 1, ESplineCoordinateSpace::World);

          static const float MinThickness = 3.f;
          static const float MaxThickness = 15.f;

          const float Dist = (float)j / (float)lenNumPoints;
          const float OneMinusDist = 1.f - Dist;
          //const float Thickness = OneMinusDist * MaxThickness + MinThickness;

          if (!route_planner->bIsIntersection)
          {
            DrawDebugLine(World, p0, p1, FColor(71, 134, 183).WithAlpha(dist_alpha), true, -1.f, depth_prio, 20.0f);
          }
        }
      }
    }
  }

  if(added_persisted_crosswalks)
  {
    ACarlaGameModeBase* carla_game_mode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
    if (carla_game_mode != nullptr)
    {
      added_persisted_crosswalks = false;
      std::vector<carla::geom::Location> crosswalks_points = carla_game_mode->GetMap()->GetAllCrosswalkZones();
      carla::geom::Location first_in_loop = crosswalks_points[0];
      for (int i = 1; i < crosswalks_points.size(); ++i)
      {
        DrawDebugLine(World, crosswalks_points[i - 1], crosswalks_points[i], FColor(202, 132, 58).WithAlpha(dist_alpha), true, -1.f, depth_prio, 20.0f);

        if (crosswalks_points[i] == first_in_loop)
        {
          ++i;
          if (i < crosswalks_points.size()) first_in_loop = crosswalks_points[i];
        }
      }
    }
  }

  USceneCaptureComponent2D* SceneCapture = GetCaptureComponent2D();
  FPixelReader::SendPixelsInRenderThread<ACosmosControlSensor, FColor>(*this);
}

void ACosmosControlSensor::BeginDestroy() {
  Super::BeginDestroy();
  if(GetWorld()) GetWorld()->PersistentLineBatcher->Flush();
}
