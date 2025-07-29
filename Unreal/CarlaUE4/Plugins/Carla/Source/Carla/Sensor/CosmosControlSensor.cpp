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

#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/RoutePlanner.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Traffic/RoadSpline.h"
#include "carla/road/Map.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/UObjectGlobals.h"


FActorDefinition ACosmosControlSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("cosmos_visualization"));
}

ACosmosControlSensor::ACosmosControlSensor(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Tags.Add(FName(TEXT("CosmosControlSensor")));
  added_persisted_stop_lines = false;
  added_persisted_route_lines = false;
  added_persisted_crosswalks = false;
  duplicated_persistent_comp = false;

  DynamicLines = CreateDefaultSubobject<ULineBatchComponent>(FName(TEXT("CosmosDynamicLinesBatchComponent")));
  PersistentLines = CreateDefaultSubobject<ULineBatchComponent>(FName(TEXT("CosmosPersistentLinesBatchComponent")));

  DynamicLines->bOnlyOwnerSee = true;
  PersistentLines->bOnlyOwnerSee = true;

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
    if (!mesh_component->IsVisible()) continue;
    if (mesh_component->GetOwner() == nullptr) continue;

    FVector box_origin, box_extent;
    FBoxSphereBounds bounds;
    UKismetSystemLibrary::GetActorBounds(mesh_component->GetOwner(), box_origin, box_extent);


    bounds = FBoxSphereBounds(box_origin, box_extent, 0.0f);

    const carla::rpc::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*mesh_component);
    UStaticMeshComponent* static_mesh_comp = Cast<UStaticMeshComponent>(mesh_component);
    USkeletalMeshComponent* skeletal_mesh_comp = Cast<USkeletalMeshComponent>(mesh_component);

    if (!static_mesh_comp && !skeletal_mesh_comp) continue;

    //TODO:Specialize for Skeletal
    if (static_mesh_comp != nullptr)
    {
      if(static_mesh_comp->GetStaticMesh())
      {
        //if (static_mesh_comp->GetAttachParent() != nullptr)
        //{
        //  if(static_mesh_comp->GetAttachParent()->GetName().Contains("VehicleMesh")) continue;
        //}
        if (!static_mesh_comp->GetName().Contains("mesh")) continue;
        bounds = static_mesh_comp->GetStaticMesh()->GetBounds();
        bounds.Origin = box_origin;
      }
    }
    else if(skeletal_mesh_comp != nullptr)
    {
      if (skeletal_mesh_comp->SkeletalMesh)
      {
        bounds = skeletal_mesh_comp->SkeletalMesh->GetBounds();
        bounds.Origin = skeletal_mesh_comp->GetComponentLocation();
        bounds.Origin.Z += bounds.BoxExtent.Z;
      }
    }

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
    case carla::rpc::CityObjectLabel::Train:
    case carla::rpc::CityObjectLabel::Truck:
      vis_color = FColor::Red;
      break;
    case carla::rpc::CityObjectLabel::Pedestrians:
      vis_color = FColor::Green;
      break;
    }

    if (Tag == carla::rpc::CityObjectLabel::TrafficLight || Tag == carla::rpc::CityObjectLabel::TrafficSigns)
    {
      DrawDebugSolidBox(World, mesh_component->GetComponentLocation(), bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color.WithAlpha(dist_alpha), false, -1, depth_prio);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Car ||
      Tag == carla::rpc::CityObjectLabel::Bicycle ||
      Tag == carla::rpc::CityObjectLabel::Bus ||
      Tag == carla::rpc::CityObjectLabel::Motorcycle ||
      Tag == carla::rpc::CityObjectLabel::Pedestrians ||
      Tag == carla::rpc::CityObjectLabel::Train ||
      Tag == carla::rpc::CityObjectLabel::Truck)
    {
      DrawDebugBox(World, bounds.Origin, bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color.WithAlpha(dist_alpha), false, -1, depth_prio, 10);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Poles)
    {
      //Filtering out horizontal poles
      if (fmax(bounds.BoxExtent.X, bounds.BoxExtent.Y) > bounds.BoxExtent.Z) continue;

      float half_height = bounds.BoxExtent.Z;
      DrawDebugCapsule(World, mesh_component->GetComponentLocation() + FVector(0.0f, 0.0f, half_height), half_height, 0.1f, FQuat::Identity, vis_color.WithAlpha(dist_alpha), false, -1, depth_prio, 20);
    }
  }


  if (!added_persisted_stop_lines)
  {
    added_persisted_stop_lines = true;

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

  if(!added_persisted_route_lines)
  {

    //TArray<AActor*> RouteSplines;
    //UGameplayStatics::GetAllActorsOfClass(World, ARoutePlanner::StaticClass(), RouteSplines);
    //for (AActor* RouteSpline : RouteSplines)
    //{
    //  ARoutePlanner* route_planner = Cast<ARoutePlanner>(RouteSpline);

    //  //route_planner->DrawRoutes();
    //  for (int i = 0, lenRoutes = route_planner->Routes.Num(); i < lenRoutes; ++i)
    //  {
    //    for (int j = 0, lenNumPoints = route_planner->Routes[i]->GetNumberOfSplinePoints() - 1; j < lenNumPoints; ++j)
    //    {
    //      const FVector p0 = route_planner->Routes[i]->GetLocationAtSplinePoint(j + 0, ESplineCoordinateSpace::World);
    //      const FVector p1 = route_planner->Routes[i]->GetLocationAtSplinePoint(j + 1, ESplineCoordinateSpace::World);

    //      static const float MinThickness = 3.f;
    //      static const float MaxThickness = 15.f;

    //      const float Dist = (float)j / (float)lenNumPoints;
    //      const float OneMinusDist = 1.f - Dist;
    //      //const float Thickness = OneMinusDist * MaxThickness + MinThickness;

    //      if (!route_planner->bIsIntersection)
    //      {
    //        DrawDebugLine(World, p0, p1, FColor(71, 134, 183).WithAlpha(dist_alpha), true, -1.f, depth_prio, 20.0f);
    //      }
    //    }
    //  }
    //}

    TArray<AActor*> RoadSplines;
    UGameplayStatics::GetAllActorsOfClass(World, ARoadSpline::StaticClass(), RoadSplines);
    if(RoadSplines.Num() > 0) added_persisted_route_lines = true;

    TMap<int32, TArray<ARoadSpline*>> SplinesByRoadId;

    TArray<ARoadSpline*> ShoulderRoadSplines;
    TArray<ARoadSpline*> DrivingRoadSplines;
    for (AActor* RoadSpline : RoadSplines)
    {
      ARoadSpline* spline = Cast<ARoadSpline>(RoadSpline);
      if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) ShoulderRoadSplines.Add(spline);
      else if(spline->BoundaryType == ERoadSplineBoundaryType::Driving) DrivingRoadSplines.Add(spline);

      if (!SplinesByRoadId.Contains(spline->RoadID)) SplinesByRoadId.Add(spline->RoadID);
      SplinesByRoadId[spline->RoadID].Add(spline);
    }

    auto DrawSpline = [&](ARoadSpline* spline)
    {

      UE_LOG(LogTemp, Log, TEXT("Drawing Road Spline"));

      for (int i = 0, lenNumPoints = spline->SplineComponent->GetNumberOfSplinePoints() - 1; i < lenNumPoints; ++i)
      {
        const FVector p0 = spline->SplineComponent->GetLocationAtSplinePoint(i + 0, ESplineCoordinateSpace::World) + (spline->OrientationType == ERoadSplineOrientationType::Left ? FVector(25.0f, 25.0f, 25.0f) : FVector::ZeroVector);
        const FVector p1 = spline->SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World) + (spline->OrientationType == ERoadSplineOrientationType::Left ? FVector(25.0f, 25.0f, 25.0f) : FVector::ZeroVector);

        static const float MinThickness = 3.f;
        static const float MaxThickness = 15.f;

        const float Dist = (float)i / (float)lenNumPoints;
        const float OneMinusDist = 1.f - Dist;
        //const float Thickness = OneMinusDist * MaxThickness + MinThickness;
        FColor debug_color = spline->BoundaryType != ERoadSplineBoundaryType::Driving ?
          FColor(206, 36, 35).WithAlpha(dist_alpha) :
          FColor(55, 103, 221).WithAlpha(dist_alpha);
        DrawDebugLine(World, p0, p1, debug_color, true, -1.f, depth_prio, 20.0f);
      }
    };

    //for (ARoadSpline* RoadSpline : DrivingRoadSplines)
    //{
    //  if (RoadSpline->bIsJunction || RoadSpline->OrientationType == ERoadSplineOrientationType::Right) continue;
    //  DrawSpline(RoadSpline);
    //}

    //for (ARoadSpline* RoadSpline : ShoulderRoadSplines)
    //{
    //  if (RoadSpline->bIsJunction ||
    //    (RoadSpline->LaneID > 0 && RoadSpline->OrientationType == ERoadSplineOrientationType::Right) ||
    //    (RoadSpline->LaneID < 0 && RoadSpline->OrientationType == ERoadSplineOrientationType::Left)) continue;
    //  DrawSpline(RoadSpline);
    //}
    

    for (TPair<int32, TArray<ARoadSpline*>> splines_pair : SplinesByRoadId)
    {
      TArray<ARoadSpline*> splines = splines_pair.Value;

      UE_LOG(LogTemp, Log, TEXT("Key %d has %d items"), splines_pair.Key, splines_pair.Value.Num());

      for (ARoadSpline* spline : splines)
      {

        UE_LOG(LogTemp, Log, TEXT("LaneID: %d RoadID: %d"), spline->LaneID, spline->RoadID);

        if (spline->BoundaryType != ERoadSplineBoundaryType::Driving &&
          spline->BoundaryType != ERoadSplineBoundaryType::Shoulder &&
          spline->BoundaryType != ERoadSplineBoundaryType::Sidewalk) continue;

        //TODO: Render rules for junction shoulders
        if(spline->bIsJunction)
        {
         
        }
        else if (spline->OrientationType == ERoadSplineOrientationType::Left)
        {

          UE_LOG(LogTemp, Log, TEXT("Spline is Left Orientated"));

          TArray<ARoadSpline*> found_splines = splines_pair.Value.FilterByPredicate([spline](ARoadSpline* in_spline) {
            return in_spline->LaneID == spline->LaneID + (spline->LaneID == 1 ? -2 : -1);
          });

          UE_LOG(LogTemp, Log, TEXT("Found Splines Num: %d"), found_splines.Num());

          for (ARoadSpline* target_spline : found_splines)
          {
            ERoadSplineBoundaryType boundary = target_spline->BoundaryType;
            bool should_render;
            switch (boundary) 
            {
            case ERoadSplineBoundaryType::Driving:
              //we only render right drivings
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = spline->LaneID > 0 && spline->LaneID * target_spline->LaneID > 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = true;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = true;
              break;
            case ERoadSplineBoundaryType::Shoulder:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = true;
              break;
            default:
              should_render = false;
              break;
            }
            if (should_render) DrawSpline(spline);
          }
        }
        else if (spline->OrientationType == ERoadSplineOrientationType::Right)
        {

          UE_LOG(LogTemp, Log, TEXT("Spline is Left Orientated"));

          TArray<ARoadSpline*> found_splines = splines_pair.Value.FilterByPredicate([spline](ARoadSpline* in_spline) {
            return in_spline->LaneID == spline->LaneID + (spline->LaneID == -1 ? 2 : 1);
          });

          UE_LOG(LogTemp, Log, TEXT("Found Splines Num: %d"), found_splines.Num());

          for (ARoadSpline* target_spline : found_splines)
          {
            ERoadSplineBoundaryType boundary = target_spline->BoundaryType;
            bool should_render;
            switch (boundary)
            {
            case ERoadSplineBoundaryType::Driving:
              //we only render right drivings
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = spline->LaneID < 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = true;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = true;
              break;
            case ERoadSplineBoundaryType::Shoulder:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = true;
              break;
            default:
              should_render = false;
              break;
            }
            if (should_render) DrawSpline(spline);
          }
        }
      }
    }
  }

  if(!added_persisted_crosswalks)
  {
    ACarlaGameModeBase* carla_game_mode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
    if (carla_game_mode != nullptr)
    {
      added_persisted_crosswalks = false;
      std::vector<carla::geom::Location> crosswalks_points = carla_game_mode->GetMap()->GetAllCrosswalkZones();
      carla::geom::Location first_in_loop = crosswalks_points[0];

      //FPlane crosswalk_plane(first_in_loop, FVector(0.0f, 0.0f, 1.0f));//(crosswalks_points[0], crosswalks_points[1], crosswalks_points[2]);
      FVector min, max;
      min = FVector(TNumericLimits<float>::Max());
      max = FVector(TNumericLimits<float>::Min());
      //TArray<FVector> Verts;
      //TArray<int32> Indices;
      //Indices.AddUninitialized(6);
      //Indices[0] = 0; Indices[1] = 2; Indices[2] = 1;
      //Indices[3] = 1; Indices[4] = 2; Indices[5] = 3;

      //Verts.Add(crosswalks_points[0]);
      //int32 vert_num = 1;
      for (int i = 1; i < crosswalks_points.size(); ++i)
      {
        DrawDebugLine(World, crosswalks_points[i - 1], crosswalks_points[i], FColor(202, 132, 58).WithAlpha(dist_alpha), false, -1.f, depth_prio, 20.0f);
        min = UKismetMathLibrary::Vector_ComponentMin(min, crosswalks_points[i]);
        max = UKismetMathLibrary::Vector_ComponentMax(max, crosswalks_points[i]);
        if (crosswalks_points[i] == first_in_loop)
        {
          //DrawDebugMesh(World, Verts, Indices, FColor(202, 132, 58).WithAlpha(dist_alpha), true, depth_prio);
          //Verts.Empty();
          //DrawDebugSolidBox(World, (min + max) * 0.5f, FVector(min - max).GetAbs() * 0.5f + FVector(0.0f,0.0f,10.0f), FColor(202, 132, 58).WithAlpha(dist_alpha), false, -1.0f, depth_prio);
          min = FVector(TNumericLimits<float>::Max());
          max = FVector(TNumericLimits<float>::Min());
          if (++i < crosswalks_points.size())
          {
            first_in_loop = crosswalks_points[i];
            //Verts.Add(crosswalks_points[i]);
          }
          //crosswalk_plane = FPlane(first_in_loop, FVector(0.0f, 0.0f, 1.0f));
        }
        else
        {
          //Verts.Add(crosswalks_points[i]);
        }
      }

    }
  }

  USceneCaptureComponent2D* SceneCapture = GetCaptureComponent2D();

  if (added_persisted_stop_lines && added_persisted_route_lines && added_persisted_crosswalks && !duplicated_persistent_comp)
  {
    duplicated_persistent_comp = true;
    //FObjectDuplicationParameters params(PersistentLines, GetWorld()->PersistentLineBatcher);
    //PersistentLines = Cast<ULineBatchComponent>(StaticDuplicateObjectEx(params));
    PersistentLines->BatchedLines = GetWorld()->PersistentLineBatcher->BatchedLines;
    PersistentLines->BatchedPoints = GetWorld()->PersistentLineBatcher->BatchedPoints;
    PersistentLines->DefaultLifeTime = GetWorld()->PersistentLineBatcher->DefaultLifeTime;
    PersistentLines->bCalculateAccurateBounds = GetWorld()->PersistentLineBatcher->bCalculateAccurateBounds;
    PersistentLines->MarkRenderStateDirty();
    GetWorld()->PersistentLineBatcher->Flush();

    SceneCapture->ShowOnlyComponents.Empty();
    SceneCapture->ShowOnlyComponents.Emplace(GetWorld()->LineBatcher);
    SceneCapture->ShowOnlyComponents.Emplace(PersistentLines);
  }

  FPixelReader::SendPixelsInRenderThread<ACosmosControlSensor, FColor>(*this);
}

void ACosmosControlSensor::BeginDestroy() {
  Super::BeginDestroy();
  if(GetWorld()) GetWorld()->PersistentLineBatcher->Flush();
}
