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
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Traffic/RoadSpline.h"
#include "carla/road/Map.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/UObjectGlobals.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Engine/Public/ConvexVolume.h"

const FColor ACosmosControlSensor::CosmosColors::LaneLines(98, 183, 249, 255);
const FColor ACosmosControlSensor::CosmosColors::Lanes(56, 103, 221, 255);
const FColor ACosmosControlSensor::CosmosColors::Poles(66, 40, 144, 255);
const FColor ACosmosControlSensor::CosmosColors::RoadBoundaries(200, 36, 35, 255);
const FColor ACosmosControlSensor::CosmosColors::WaitLines(185, 63, 34, 255);
const FColor ACosmosControlSensor::CosmosColors::Crosswalks(206, 131, 63, 255);
const FColor ACosmosControlSensor::CosmosColors::RoadMarkings(126, 204, 205, 255);
const FColor ACosmosControlSensor::CosmosColors::TrafficSigns(131, 175, 155, 255);
const FColor ACosmosControlSensor::CosmosColors::TrafficLights(252, 157, 155, 255);
const FColor ACosmosControlSensor::CosmosColors::Cars(255, 0, 0, 255);
const FColor ACosmosControlSensor::CosmosColors::Pedestrians(0, 255, 0, 255);


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
  added_persisted_stencils = false;

  DynamicLines = CreateDefaultSubobject<ULineBatchComponent_CARLA>(FName(TEXT("CosmosDynamicLinesBatchComponent")));
  PersistentLines = CreateDefaultSubobject<ULineBatchComponent_CARLA>(FName(TEXT("CosmosPersistentLinesBatchComponent")));

  DynamicLines->bOnlyOwnerSee = true;
  PersistentLines->bOnlyOwnerSee = true;

  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/CosmosLens.CosmosLens'"));
}

void ACosmosControlSensor::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  Super::SetUpSceneCaptureComponent(SceneCapture);

  SceneCapture.ShowFlags.SetAtmosphere(false);
  SceneCapture.ShowFlags.SetFog(false);
  SceneCapture.ShowFlags.SetVolumetricFog(false);
  SceneCapture.ShowFlags.SetMotionBlur(false);
  SceneCapture.ShowFlags.SetBloom(false);
  SceneCapture.ShowFlags.SetEyeAdaptation(false);
  SceneCapture.ShowFlags.SetTonemapper(false);
  SceneCapture.ShowFlags.SetColorGrading(false);
  SceneCapture.ShowFlags.SetDepthOfField(false);
  SceneCapture.ShowFlags.SetVignette(false);
  SceneCapture.ShowFlags.SetGrain(false);
  SceneCapture.ShowFlags.SetLensFlares(false);
  SceneCapture.ShowFlags.SetAntiAliasing(false);
  SceneCapture.ShowFlags.SetScreenSpaceReflections(false);
  SceneCapture.ShowFlags.SetAmbientOcclusion(false);
  SceneCapture.ShowFlags.SetDirectionalLights(false);
  SceneCapture.ShowFlags.SetPointLights(false);
  SceneCapture.ShowFlags.SetSpotLights(false);
  SceneCapture.ShowFlags.SetSkyLighting(false);
  SceneCapture.bCaptureEveryFrame = true;
  SceneCapture.PostProcessSettings.bOverride_ColorGamma = true;
  SceneCapture.PostProcessSettings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 1.0f);

  SceneCapture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  SceneCapture.ShowOnlyComponents.Empty();
  SceneCapture.ShowOnlyComponents.Emplace(DynamicLines);
  SceneCapture.ShowOnlyComponents.Emplace(PersistentLines);
}

FColor ACosmosControlSensor::GetColorByTag(carla::rpc::CityObjectLabel Tag, uint8 alpha)
{
  FColor vis_color;

  switch (Tag) {
  case carla::rpc::CityObjectLabel::TrafficLight:
    vis_color = CosmosColors::TrafficLights;
    break;
  case carla::rpc::CityObjectLabel::TrafficSigns:
    vis_color = CosmosColors::TrafficSigns;
    break;
  case carla::rpc::CityObjectLabel::Poles:
    vis_color = CosmosColors::Poles;
    break;
  case carla::rpc::CityObjectLabel::Car:
  case carla::rpc::CityObjectLabel::Bicycle:
  case carla::rpc::CityObjectLabel::Bus:
  case carla::rpc::CityObjectLabel::Motorcycle:
  case carla::rpc::CityObjectLabel::Train:
  case carla::rpc::CityObjectLabel::Truck:
    vis_color = CosmosColors::Cars;
    break;
  case carla::rpc::CityObjectLabel::Pedestrians:
    vis_color = CosmosColors::Pedestrians;
    break;
  }

  return vis_color.WithAlpha(alpha);
}

void ACosmosControlSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACosmosControlSensor::PostPhysTick);

  DynamicLines->Flush();

  int depth_prio = ESceneDepthPriorityGroup::SDPG_World;
  int dist_alpha = CosmosColors::RoadBoundaries.A;
  float cutoff_dist = 3000.0f;
  ACarlaGameModeBase* carla_game_mode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
  auto* GameInstance = UCarlaStatics::GetGameInstance(World);

  //TODO: Finish Frustrum Culling
  //FMinimalViewInfo view_info;
  //FConvexVolume frustrum;
  //GetCaptureComponent2D()->GetCameraView(DeltaSeconds, view_info);
  //GetViewFrustumBounds(frustrum, view_info.CalculateProjectionMatrix(), false);

  AActor* player_actor = nullptr;
  for(TPair<FCarlaActor::IdType, TSharedPtr<FCarlaActor>> pair : GetEpisode().GetActorRegistry())
  {
    const FActorAttribute* Attribute = pair.Value->GetActorInfo()->Description.Variations.Find("role_name");
    if (Attribute && (Attribute->Value.Contains("hero") || Attribute->Value.Contains("ego_vehicle"))) {
      player_actor = pair.Value->GetActor();
      break;
    }
  }

  TArray<UObject*> CosmosRelevantComponents;
  GetObjectsOfClass(UMeshComponent::StaticClass(), CosmosRelevantComponents, true, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  for (UObject* Object : CosmosRelevantComponents) {


    UMeshComponent* mesh_component = Cast<UMeshComponent>(Object);
    if (!mesh_component->IsVisible()) continue;
    if (mesh_component->GetOwner() == nullptr) continue;
    if (mesh_component->GetOwner() == player_actor) continue;
    
    // Check if this is a vehicle and if it's in the ignored vehicles list for this sensor
    FCarlaActor* CarlaActor = GetEpisode().FindCarlaActor(mesh_component->GetOwner());
    if (CarlaActor && 
        CarlaActor->GetActorInfo()->Description.Id.Contains("vehicle") && 
        IgnoredVehicles.Contains(CarlaActor->GetActorId())) continue;

    //Assumed to be off the road (parkings, ceilings)
    //TODO: Better Occlusion techniques to root these out variable height maps)
    if (mesh_component->GetComponentLocation().Z > 10000.0f) continue;

    FVector box_origin, box_extent;
    FBoxSphereBounds bounds;
    UKismetSystemLibrary::GetActorBounds(mesh_component->GetOwner(), box_origin, box_extent);
    //TODO: Finish Frustrum Culling
    //if (!frustrum.IntersectBox(box_origin, box_extent)) continue;

    bounds = FBoxSphereBounds(box_origin, box_extent, 0.0f);

    const carla::rpc::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*mesh_component);
    UStaticMeshComponent* static_mesh_comp = Cast<UStaticMeshComponent>(mesh_component);
    USkeletalMeshComponent* skeletal_mesh_comp = Cast<USkeletalMeshComponent>(mesh_component);

    if (!static_mesh_comp && !skeletal_mesh_comp) continue;

    if (static_mesh_comp != nullptr)
    {
      if(static_mesh_comp->GetStaticMesh())
      {
        if (!static_mesh_comp->GetName().Contains("mesh") || static_mesh_comp->GetName().Contains("road")) continue;
        bounds = static_mesh_comp->GetStaticMesh()->GetBounds();
        bounds.Origin = box_origin;
      }
    }
    else if(skeletal_mesh_comp != nullptr)
    {
      if (skeletal_mesh_comp->SkeletalMesh)
      {
        //TODO: Get more precise pedestrian bounds
        bounds = skeletal_mesh_comp->SkeletalMesh->GetBounds();
        bounds.Origin = skeletal_mesh_comp->GetComponentLocation();
        bounds.Origin.Z += bounds.BoxExtent.Z;
      }
    }

    FColor vis_color = GetColorByTag(Tag, dist_alpha);

    if (Tag == carla::rpc::CityObjectLabel::TrafficLight || Tag == carla::rpc::CityObjectLabel::TrafficSigns)
    {
      DrawDebugSolidBox(World, mesh_component->GetComponentLocation(), bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color, false, -1, depth_prio);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Car ||
      Tag == carla::rpc::CityObjectLabel::Bicycle ||
      Tag == carla::rpc::CityObjectLabel::Bus ||
      Tag == carla::rpc::CityObjectLabel::Motorcycle ||
      Tag == carla::rpc::CityObjectLabel::Pedestrians ||
      Tag == carla::rpc::CityObjectLabel::Train ||
      Tag == carla::rpc::CityObjectLabel::Truck)
    {
      DrawDebugBox(World, bounds.Origin, bounds.BoxExtent, mesh_component->GetOwner()->GetActorRotation().Quaternion(), vis_color, false, -1, depth_prio, 10);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Poles)
    {
      //Filtering out horizontal poles
      //if (/*fmax(bounds.BoxExtent.X, bounds.BoxExtent.Y) > bounds.BoxExtent.Z && */!static_mesh_comp->GetStaticMesh()->GetFName().ToString().Contains("pole")) continue;
      
      float half_height = fmax(bounds.BoxExtent.Z, box_extent.Z);
      float distance_to_road = mesh_component->GetComponentLocation().Z;
      DrawDebugCapsule(World, mesh_component->GetComponentLocation() + FVector(0.0f, 0.0f, half_height), half_height + (distance_to_road > 250.0f ? 0.0f : distance_to_road), 0.1f, FQuat::Identity, vis_color, false, -1, depth_prio, 10);
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
      FVector ground_pos = FVector(stop_box_collider->GetComponentLocation().X, stop_box_collider->GetComponentLocation().Y, 0.0f);
      DrawDebugLine(World,
        ground_pos + -stop_box_collider->GetScaledBoxExtent().X * stop_box_collider->GetForwardVector() - 710.0f * stop_box_collider->GetRightVector(),
        ground_pos + stop_box_collider->GetScaledBoxExtent().X * stop_box_collider->GetForwardVector() - 710.0f * stop_box_collider->GetRightVector(),
        CosmosColors::WaitLines.WithAlpha(dist_alpha), true, -1, depth_prio, 10);
    }
  }

  if(!added_persisted_route_lines)
  {
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
      for (int i = 0, lenNumPoints = spline->SplineComponent->GetNumberOfSplinePoints() - 1; i < lenNumPoints; ++i)
      {
        const FVector p0 = spline->SplineComponent->GetLocationAtSplinePoint(i + 0, ESplineCoordinateSpace::World);
        const FVector p1 = spline->SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

        const float Dist = (float)i / (float)lenNumPoints;
        FColor debug_color = spline->BoundaryType != ERoadSplineBoundaryType::Driving ?
          CosmosColors::RoadBoundaries.WithAlpha(dist_alpha) : CosmosColors::LaneLines.WithAlpha(dist_alpha);

        DrawDebugLine(World, p0, p1, debug_color, true, -1.f, depth_prio, 5.0f);
      }
    };

    for (TPair<int32, TArray<ARoadSpline*>> splines_pair : SplinesByRoadId)
    {
      TArray<ARoadSpline*> splines = splines_pair.Value;

      for (ARoadSpline* spline : splines)
      {

        if (spline->BoundaryType != ERoadSplineBoundaryType::Driving &&
          spline->BoundaryType != ERoadSplineBoundaryType::Shoulder &&
          spline->BoundaryType != ERoadSplineBoundaryType::Sidewalk &&
          spline->BoundaryType != ERoadSplineBoundaryType::Median) continue;

        bool should_render = false;
        TArray<ARoadSpline*> found_splines = splines_pair.Value.FilterByPredicate([spline](ARoadSpline* in_spline) {
          return in_spline->LaneID == spline->LaneID +
            (spline->OrientationType == ERoadSplineOrientationType::Left ?
              (spline->LaneID == 1 ? -2 : -1) : (spline->LaneID == -1 ? 2 : 1));
        });

        if(spline->bIsJunction)
        {
          for (ARoadSpline* target_spline : found_splines)
          {
            ERoadSplineBoundaryType boundary = target_spline->BoundaryType;
            switch (boundary)
            {
            case ERoadSplineBoundaryType::Driving:
            case ERoadSplineBoundaryType::Shoulder:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = true;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Median) should_render = true;
              break;
            default:
              should_render = false;
              break;
            }
          }
        }
        else if (spline->OrientationType == ERoadSplineOrientationType::Left)
        {
          for (ARoadSpline* target_spline : found_splines)
          {
            ERoadSplineBoundaryType boundary = target_spline->BoundaryType;
            switch (boundary) 
            {
            case ERoadSplineBoundaryType::Driving:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = spline->LaneID > 0 && spline->LaneID * target_spline->LaneID > 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = spline->LaneID > 0 && spline->LaneID * target_spline->LaneID > 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Median) should_render = true;
              break;
            case ERoadSplineBoundaryType::Shoulder:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = spline->LaneID > 0 && spline->LaneID * target_spline->LaneID > 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Median) should_render = true;
              break;
            default:
              should_render = false;
              break;
            }
          }
        }
        else if (spline->OrientationType == ERoadSplineOrientationType::Right)
        {
          for (ARoadSpline* target_spline : found_splines)
          {
            ERoadSplineBoundaryType boundary = target_spline->BoundaryType;
            switch (boundary)
            {
            case ERoadSplineBoundaryType::Driving:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = spline->LaneID < 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = spline->LaneID < 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Median) should_render = true;
              break;
            case ERoadSplineBoundaryType::Shoulder:
              if (spline->BoundaryType == ERoadSplineBoundaryType::Driving) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Shoulder) should_render = false;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk) should_render = spline->LaneID < 0;
              else if (spline->BoundaryType == ERoadSplineBoundaryType::Median) should_render = true;
              break;
            default:
              should_render = false;
              break;
            }
          }
        }

        if (should_render) DrawSpline(spline);
      }
    }
  }

  // Crosswalks
  if (!added_persisted_crosswalks && carla_game_mode != nullptr)
  {
    added_persisted_crosswalks = true;

    std::vector<carla::geom::Location> crosswalks_points = carla_game_mode->GetMap()->GetAllCrosswalkZones();
    
    if (crosswalks_points.size() > 0)
    {
      TArray<FVector> current_polygon;
      carla::geom::Location first_in_loop = crosswalks_points[0];
      current_polygon.Add(first_in_loop.ToFVector() * 100.0f);

      for (int i = 1; i < crosswalks_points.size(); ++i)
      {
        if (crosswalks_points[i] == first_in_loop)
        {
          if (current_polygon.Num() >= 3)
          {
            TArray<FVector> mesh_vertices = current_polygon;
            TArray<int32> mesh_indices;
            
            // Simple triangulation
            for (int j = 1; j < current_polygon.Num() - 1; ++j)
            {
              mesh_indices.Add(0);
              mesh_indices.Add(j);
              mesh_indices.Add(j + 1);
            }
            
            DrawDebugMesh(World, mesh_vertices, mesh_indices, CosmosColors::Crosswalks.WithAlpha(dist_alpha), true, -1.0f, depth_prio);
            
            // for (int j = 0; j < current_polygon.Num(); ++j)
            // {
            //   int next_j = (j + 1) % current_polygon.Num();
            //   DrawDebugLine(World, current_polygon[j], current_polygon[next_j], 
            //               CosmosColors::Crosswalks.WithAlpha(255), true, -1.0f, depth_prio, 15.0f);
            // }
          }

          // Start new polygon if more points remain
          current_polygon.Empty();
          if (i < crosswalks_points.size() - 1)
          {
            first_in_loop = crosswalks_points[++i];
            current_polygon.Add(first_in_loop.ToFVector() * 100.0f);
          }
        }
        else
        {
          current_polygon.Add(crosswalks_points[i].ToFVector() * 100.0f);
        }
      }
    }
  }

  // Stencils
  if (!added_persisted_stencils && carla_game_mode != nullptr)
  {
    added_persisted_stencils = true;

    const auto& road_stencils = carla_game_mode->GetMap()->GetStencils();

    for (const auto& StencilPair : road_stencils)
    {
      const auto& Stencil = StencilPair.second;
      if (!Stencil)
      {
        continue;
      }

      const FTransform Transform = Stencil->GetTransform();
      const float StencilWidth = Stencil->GetWidth() * 100.0;
      const float StencilLength = Stencil->GetLength() * 100.0;
      FQuat StencilOrientation = Transform.GetRotation();

      TArray<FVector> mesh_vertices = {
        Transform.GetLocation() + StencilOrientation.RotateVector(FVector(-StencilLength/2, -StencilWidth/2, 0)),
        Transform.GetLocation() + StencilOrientation.RotateVector(FVector(StencilLength/2, -StencilWidth/2, 0)),
        Transform.GetLocation() + StencilOrientation.RotateVector(FVector(StencilLength/2, StencilWidth/2, 0)),
        Transform.GetLocation() + StencilOrientation.RotateVector(FVector(-StencilLength/2, StencilWidth/2, 0))
      };

      TArray<int32> mesh_indices = {
        0, 1, 2,
        0, 2, 3
      };

      DrawDebugMesh(World, mesh_vertices, mesh_indices, CosmosColors::RoadMarkings.WithAlpha(dist_alpha), true, -1.0f, depth_prio);
    }
  }

  USceneCaptureComponent2D* SceneCapture = GetCaptureComponent2D();
  FPixelReader::SendPixelsInRenderThread<ACosmosControlSensor, FColor>(*this);
}

ULineBatchComponent_CARLA* ACosmosControlSensor::GetDebugLineBatcher(bool bPersistentLines)
{
  return (bPersistentLines ? PersistentLines : DynamicLines);
}

void ACosmosControlSensor::DrawDebugBox(const UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
  // no debug line drawing on dedicated server
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    // this means foreground lines can't be persistent 
    if (ULineBatchComponent_CARLA* const LineBatcher = GetDebugLineBatcher(bPersistentLines))
    {
      float const LineLifeTime = 0.0f;
      TArray<struct FBatchedLine> Lines;

      FTransform const Transform(Rotation);
      FVector Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
      FVector End = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
      End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
      End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
      End = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
      End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
      new(Lines)FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
      End = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
      new(Lines) FBatchedLine(Center + Start, Center + End, Color, LineLifeTime, Thickness, DepthPriority);

      LineBatcher->DrawLines(Lines);
    }
  }
}

void ACosmosControlSensor::DrawDebugSolidBox(const UWorld* InWorld, FVector const& Center, FVector const& Extent, FQuat const& Rotation, FColor const& Color, bool bPersistent, float LifeTime, uint8 DepthPriority)
{
  // no debug line drawing on dedicated server
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    if (ULineBatchComponent_CARLA* const LineBatcher = GetDebugLineBatcher(bPersistent))
    {
      FTransform Transform(Rotation, Center, FVector(1.0f, 1.0f, 1.0f));	// Build transform from Rotation, Center with uniform scale of 1.0.
      FBox Box = FBox::BuildAABB(FVector::ZeroVector, Extent);	// The Transform handles the Center location, so this box needs to be centered on origin.
      LineBatcher->DrawSolidBox(Box, Transform, Color, DepthPriority, 0.0f);
    }
  }
}

void ACosmosControlSensor::DrawDebugLine(const UWorld* InWorld, FVector const& LineStart, FVector const& LineEnd, FColor const& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    // this means foreground lines can't be persistent 
    if (ULineBatchComponent_CARLA* const LineBatcher = GetDebugLineBatcher(bPersistentLines))
    {
      float rlinear = ((float)Color.R) / 255.0f;
      float glinear = ((float)Color.G) / 255.0f;
      float blinear = ((float)Color.B) / 255.0f;

      LineBatcher->DrawLine(LineStart, LineEnd, FLinearColor(rlinear, glinear, blinear), DepthPriority, Thickness, 0.0f);
    }
  }
}

void ACosmosControlSensor::DrawDebugMesh(const UWorld* InWorld, const TArray<FVector>& Vertices, const TArray<int32>& Indices, const FColor& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority)
{
  // no debug mesh drawing on dedicated server
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    if (ULineBatchComponent_CARLA* const LineBatcher = GetDebugLineBatcher(bPersistentLines))
    {
      LineBatcher->DrawMesh(Vertices, Indices, Color, DepthPriority, 0.0f);
    }
  }
}

void ACosmosControlSensor::DrawDebugCapsule(const UWorld* InWorld, FVector const& Center, float HalfHeight, float Radius, const FQuat& Rotation, FColor const& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
  // no debug line drawing on dedicated server
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    const int32 DrawCollisionSides = 16;

    FVector Origin = Center;
    FMatrix Axes = FQuatRotationTranslationMatrix(Rotation, FVector::ZeroVector);
    FVector XAxis = Axes.GetScaledAxis(EAxis::X);
    FVector YAxis = Axes.GetScaledAxis(EAxis::Y);
    FVector ZAxis = Axes.GetScaledAxis(EAxis::Z);

    // Draw top and bottom circles
    float HalfAxis = FMath::Max<float>(HalfHeight - Radius, 1.f);
    FVector TopEnd = Origin + HalfAxis * ZAxis;
    FVector BottomEnd = Origin - HalfAxis * ZAxis;

    DrawCircle(InWorld, TopEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawCircle(InWorld, BottomEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

    // Draw domed caps
    DrawHalfCircle(InWorld, TopEnd, YAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawHalfCircle(InWorld, TopEnd, XAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

    FVector NegZAxis = -ZAxis;

    DrawHalfCircle(InWorld, BottomEnd, YAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawHalfCircle(InWorld, BottomEnd, XAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

    // Draw connected lines
    DrawDebugLine(InWorld, TopEnd + Radius * XAxis, BottomEnd + Radius * XAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugLine(InWorld, TopEnd - Radius * XAxis, BottomEnd - Radius * XAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugLine(InWorld, TopEnd + Radius * YAxis, BottomEnd + Radius * YAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugLine(InWorld, TopEnd - Radius * YAxis, BottomEnd - Radius * YAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
  }
}

void ACosmosControlSensor::DrawHalfCircle(const UWorld* InWorld, const FVector& Base, const FVector& X, const FVector& Y, const FColor& Color, float Radius, int32 NumSides, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
  float	AngleDelta = 2.0f * (float)PI / ((float)NumSides);
  FVector	LastVertex = Base + X * Radius;

  for (int32 SideIndex = 0; SideIndex < (NumSides / 2); SideIndex++)
  {
    FVector	Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
    DrawDebugLine(InWorld, LastVertex, Vertex, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    LastVertex = Vertex;
  }
}

void ACosmosControlSensor::DrawCircle(const UWorld* InWorld, const FVector& Base, const FVector& X, const FVector& Y, const FColor& Color, float Radius, int32 NumSides, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
  const float	AngleDelta = 2.0f * PI / NumSides;
  FVector	LastVertex = Base + X * Radius;

  for (int32 SideIndex = 0; SideIndex < NumSides; SideIndex++)
  {
    const FVector Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
    DrawDebugLine(InWorld, LastVertex, Vertex, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    LastVertex = Vertex;
  }
}
