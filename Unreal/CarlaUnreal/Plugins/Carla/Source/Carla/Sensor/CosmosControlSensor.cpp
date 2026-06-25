// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/CosmosControlSensor.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Cosmos/CosmosRoadGeometry.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/ImageUtil.h"
#include "Carla/Traffic/TrafficLightBase.h"

#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/road/Stencil.h>
#include <carla/geom/Transform.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "TextureResource.h"
#include "UObject/UObjectHash.h"
#include <util/ue-header-guard-end.h>

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
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(
      TEXT("cosmos_visualization"));
}

ACosmosControlSensor::ACosmosControlSensor(
    const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  Tags.Add(FName(TEXT("CosmosControlSensor")));
  added_persisted_stop_lines = false;
  added_persisted_route_lines = false;
  added_persisted_crosswalks = false;
  added_persisted_stencils = false;

  DynamicLines = CreateDefaultSubobject<ULineBatchComponent_CARLA>(
      FName(TEXT("CosmosDynamicLinesBatchComponent")));
  PersistentLines = CreateDefaultSubobject<ULineBatchComponent_CARLA>(
      FName(TEXT("CosmosPersistentLinesBatchComponent")));

  DynamicLines->bOnlyOwnerSee = true;
  PersistentLines->bOnlyOwnerSee = true;

  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/CosmosLens.CosmosLens'"));
}

void ACosmosControlSensor::SetUpSceneCaptureComponent(
    USceneCaptureComponent2D& SceneCapture)
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

  SceneCapture.PrimitiveRenderMode =
      ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  SceneCapture.ShowOnlyComponents.Empty();
  SceneCapture.ShowOnlyComponents.Emplace(DynamicLines);
  SceneCapture.ShowOnlyComponents.Emplace(PersistentLines);
}

FColor ACosmosControlSensor::GetColorByTag(
    carla::rpc::CityObjectLabel Tag, uint8 alpha)
{
  FColor vis_color = FColor::Black;

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
  default:
    break;
  }

  return vis_color.WithAlpha(alpha);
}

void ACosmosControlSensor::DrawRoadGeometry(
    const UWorld* World, int DepthPriority, uint8 DistAlpha)
{
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(World);
  if (GameMode == nullptr)
  {
    return;
  }

  const std::optional<carla::road::Map>& CarlaMap = GameMode->GetMap();
  if (!CarlaMap)
  {
    return;
  }

  // road::Map works in meters; the overlay is drawn in Unreal centimeters.
  constexpr float MetersToCm = 100.0f;

  auto DrawPolylines =
      [&](const TArray<TArray<FVector>>& Polylines, const FColor& Color)
  {
    const FColor LineColor = Color.WithAlpha(DistAlpha);
    for (const TArray<FVector>& Poly : Polylines)
    {
      for (int32 i = 0, Last = Poly.Num() - 1; i < Last; ++i)
      {
        DrawDebugLine(
            World, Poly[i] * MetersToCm, Poly[i + 1] * MetersToCm,
            LineColor, true, -1.f, DepthPriority, 5.0f);
      }
    }
  };

  // Painted lane lines (driving lanes, outer painted-mark records only).
  TArray<TArray<FVector>> LaneLinePolylines;
  CosmosRoadGeometry::BuildOuterBorderPolylines(
      *CarlaMap,
      carla::road::Lane::LaneType::Driving,
      /*bRequirePaintedMark=*/true,
      LaneLinePolylines);
  DrawPolylines(LaneLinePolylines, CosmosColors::LaneLines);

  // Road boundaries (Shoulder / Sidewalk / Median lane outer edges).
  TArray<TArray<FVector>> BoundaryPolylines;
  CosmosRoadGeometry::BuildOuterBorderPolylines(
      *CarlaMap,
      CosmosRoadGeometry::BoundaryLaneTypes(),
      /*bRequirePaintedMark=*/false,
      BoundaryPolylines);
  DrawPolylines(BoundaryPolylines, CosmosColors::RoadBoundaries);
}

void ACosmosControlSensor::PostPhysTick(
    UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACosmosControlSensor::PostPhysTick);

  DynamicLines->Flush();

  int depth_prio = ESceneDepthPriorityGroup::SDPG_World;
  int dist_alpha = CosmosColors::RoadBoundaries.A;
  float cutoff_dist = 3000.0f;
  ACarlaGameModeBase* carla_game_mode =
      Cast<ACarlaGameModeBase>(World->GetAuthGameMode());

  AActor* player_actor = nullptr;
  for (const TPair<FCarlaActor::IdType, TSharedPtr<FCarlaActor>>& pair :
       GetEpisode().GetActorRegistry())
  {
    const FActorAttribute* Attribute =
        pair.Value->GetActorInfo()->Description.Variations.Find("role_name");
    if (Attribute &&
        (Attribute->Value.Contains("hero") ||
         Attribute->Value.Contains("ego_vehicle")))
    {
      player_actor = pair.Value->GetActor();
      break;
    }
  }

  // ue4 passed EInternalObjectFlags::AllFlags here; that value no longer
  // exists in UE5.5 and the macro EInternalObjectFlags_AllFlags excludes
  // live objects (RefCounted / RootSet). Match the PR2 sibling Cosmos
  // exporters, which port the same ue4 call as EInternalObjectFlags::None.
  TArray<UObject*> CosmosRelevantComponents;
  GetObjectsOfClass(
      UMeshComponent::StaticClass(), CosmosRelevantComponents, true,
      EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::None);

  for (UObject* Object : CosmosRelevantComponents) {

    UMeshComponent* mesh_component = Cast<UMeshComponent>(Object);
    if (!mesh_component->IsVisible()) continue;
    if (mesh_component->GetOwner() == nullptr) continue;
    if (mesh_component->GetOwner() == player_actor) continue;

    // Assumed to be off the road (parkings, ceilings).
    // TODO: Better occlusion techniques to root these out (variable height
    // maps).
    if (mesh_component->GetComponentLocation().Z > 10000.0f) continue;

    FVector box_origin, box_extent;
    FBoxSphereBounds bounds;
    UKismetSystemLibrary::GetActorBounds(
        mesh_component->GetOwner(), box_origin, box_extent);

    bounds = FBoxSphereBounds(box_origin, box_extent, 0.0f);

    const carla::rpc::CityObjectLabel Tag =
        ATagger::GetTagOfTaggedComponent(*mesh_component);
    UStaticMeshComponent* static_mesh_comp =
        Cast<UStaticMeshComponent>(mesh_component);
    USkeletalMeshComponent* skeletal_mesh_comp =
        Cast<USkeletalMeshComponent>(mesh_component);

    if (!static_mesh_comp && !skeletal_mesh_comp) continue;

    if (static_mesh_comp != nullptr)
    {
      if (static_mesh_comp->GetStaticMesh())
      {
        if (!static_mesh_comp->GetName().Contains("mesh") ||
            static_mesh_comp->GetName().Contains("road")) continue;
        bounds = static_mesh_comp->GetStaticMesh()->GetBounds();
        bounds.Origin = box_origin;
      }
    }
    else if (skeletal_mesh_comp != nullptr)
    {
      if (skeletal_mesh_comp->GetSkeletalMeshAsset())
      {
        // TODO: Get more precise pedestrian bounds.
        bounds = skeletal_mesh_comp->GetSkeletalMeshAsset()->GetBounds();
        bounds.Origin = skeletal_mesh_comp->GetComponentLocation();
        bounds.Origin.Z += bounds.BoxExtent.Z;
      }
    }

    FColor vis_color = GetColorByTag(Tag, dist_alpha);

    if (Tag == carla::rpc::CityObjectLabel::TrafficLight ||
        Tag == carla::rpc::CityObjectLabel::TrafficSigns)
    {
      DrawDebugSolidBox(
          World, mesh_component->GetComponentLocation(), bounds.BoxExtent,
          mesh_component->GetOwner()->GetActorRotation().Quaternion(),
          vis_color, false, -1, depth_prio);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Car ||
      Tag == carla::rpc::CityObjectLabel::Bicycle ||
      Tag == carla::rpc::CityObjectLabel::Bus ||
      Tag == carla::rpc::CityObjectLabel::Motorcycle ||
      Tag == carla::rpc::CityObjectLabel::Pedestrians ||
      Tag == carla::rpc::CityObjectLabel::Train ||
      Tag == carla::rpc::CityObjectLabel::Truck)
    {
      DrawDebugBox(
          World, bounds.Origin, bounds.BoxExtent,
          mesh_component->GetOwner()->GetActorRotation().Quaternion(),
          vis_color, false, -1, depth_prio, 10);
    }
    else if (Tag == carla::rpc::CityObjectLabel::Poles)
    {
      float half_height = fmax(bounds.BoxExtent.Z, box_extent.Z);
      float distance_to_road = mesh_component->GetComponentLocation().Z;
      DrawDebugCapsule(
          World,
          mesh_component->GetComponentLocation() +
              FVector(0.0f, 0.0f, half_height),
          half_height +
              (distance_to_road > 250.0f ? 0.0f : distance_to_road),
          0.1f, FQuat::Identity, vis_color, false, -1, depth_prio, 10);
    }
  }

  if (!added_persisted_stop_lines)
  {
    added_persisted_stop_lines = true;

    TArray<AActor*> TrafficLights;
    UGameplayStatics::GetAllActorsOfClass(
        World, ATrafficLightBase::StaticClass(), TrafficLights);

    for (AActor* traffic_light : TrafficLights)
    {
      UBoxComponent* stop_box_collider = Cast<UBoxComponent>(
          traffic_light->GetComponentByClass(UBoxComponent::StaticClass()));
      if (stop_box_collider == nullptr) continue;
      FVector ground_pos = FVector(
          stop_box_collider->GetComponentLocation().X,
          stop_box_collider->GetComponentLocation().Y, 0.0f);
      DrawDebugLine(World,
        ground_pos +
            -stop_box_collider->GetScaledBoxExtent().X *
                stop_box_collider->GetForwardVector() -
            710.0f * stop_box_collider->GetRightVector(),
        ground_pos +
            stop_box_collider->GetScaledBoxExtent().X *
                stop_box_collider->GetForwardVector() -
            710.0f * stop_box_collider->GetRightVector(),
        CosmosColors::WaitLines.WithAlpha(dist_alpha), true, -1,
        depth_prio, 10);
    }
  }

  // Lane lines and road boundaries reconstructed from road::Map (replaces the
  // ue4 ARoadSpline walk; ARoadSpline is intentionally not ported).
  if (!added_persisted_route_lines && carla_game_mode != nullptr)
  {
    added_persisted_route_lines = true;
    DrawRoadGeometry(World, depth_prio, static_cast<uint8>(dist_alpha));
  }

  // Crosswalks.
  if (!added_persisted_crosswalks && carla_game_mode != nullptr)
  {
    const std::optional<carla::road::Map>& CarlaMap =
        carla_game_mode->GetMap();
    if (CarlaMap)
    {
      added_persisted_crosswalks = true;

      std::vector<carla::geom::Location> crosswalks_points =
          CarlaMap->GetAllCrosswalkZones();

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

              // Simple triangulation.
              for (int j = 1; j < current_polygon.Num() - 1; ++j)
              {
                mesh_indices.Add(0);
                mesh_indices.Add(j);
                mesh_indices.Add(j + 1);
              }

              DrawDebugMesh(
                  World, mesh_vertices, mesh_indices,
                  CosmosColors::Crosswalks.WithAlpha(dist_alpha), true, -1.0f,
                  depth_prio);
            }

            // Start new polygon if more points remain.
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
  }

  // Stencils.
  if (!added_persisted_stencils && carla_game_mode != nullptr)
  {
    const std::optional<carla::road::Map>& CarlaMap =
        carla_game_mode->GetMap();
    if (CarlaMap)
    {
      added_persisted_stencils = true;

      const auto& road_stencils = CarlaMap->GetStencils();

      for (const auto& StencilPair : road_stencils)
      {
        const auto& Stencil = StencilPair.second;
        if (!Stencil)
        {
          continue;
        }

        // road::Map is metric; convert to Unreal centimeters for the overlay.
        const FTransform Transform = Stencil->GetTransform();
        const FVector Location = Transform.GetLocation() * 100.0;
        const float StencilWidth = Stencil->GetWidth() * 100.0;
        const float StencilLength = Stencil->GetLength() * 100.0;
        const FQuat StencilOrientation = Transform.GetRotation();

        TArray<FVector> mesh_vertices = {
          Location + StencilOrientation.RotateVector(
              FVector(-StencilLength / 2, -StencilWidth / 2, 0)),
          Location + StencilOrientation.RotateVector(
              FVector(StencilLength / 2, -StencilWidth / 2, 0)),
          Location + StencilOrientation.RotateVector(
              FVector(StencilLength / 2, StencilWidth / 2, 0)),
          Location + StencilOrientation.RotateVector(
              FVector(-StencilLength / 2, StencilWidth / 2, 0))
        };

        TArray<int32> mesh_indices = {
          0, 1, 2,
          0, 2, 3
        };

        DrawDebugMesh(
            World, mesh_vertices, mesh_indices,
            CosmosColors::RoadMarkings.WithAlpha(dist_alpha), true, -1.0f,
            depth_prio);
      }
    }
  }

  if (!AreClientsListening())
    return;

  // The overlay scene capture leaves its render target as a sampled shader
  // resource (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL). The async readback
  // copies it with RHICopyTexture, which on UE5.5 requires the source in
  // CopySrc (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) and does not transition it
  // itself, so do it explicitly here before the readback below.
  if (UTextureRenderTarget2D* RenderTarget = GetCaptureRenderTarget())
  {
    ENQUEUE_RENDER_COMMAND(CosmosTransitionCaptureToCopySrc)(
        [RenderTarget](FRHICommandListImmediate& RHICmdList)
    {
      auto* Resource = static_cast<FTextureRenderTarget2DResource*>(
          RenderTarget->GetResource());
      if (Resource == nullptr)
        return;
      if (FRHITexture* Texture = Resource->GetRenderTargetTexture())
      {
        RHICmdList.Transition(FRHITransitionInfo(
            Texture, ERHIAccess::SRVMask, ERHIAccess::CopySrc));
      }
    });
  }

  auto FrameIndex = FCarlaEngine::GetFrameCounter();
  ImageUtil::ReadSensorImageDataAsyncFColor(*this, [this, FrameIndex](
    TArrayView<const FColor> Pixels,
    FIntPoint Size) -> bool
  {
    SendDataToClient(*this, Pixels, FrameIndex);
    return true;
  });
}

ULineBatchComponent_CARLA* ACosmosControlSensor::GetDebugLineBatcher(
    bool bPersistentLines)
{
  return (bPersistentLines ? PersistentLines : DynamicLines);
}

void ACosmosControlSensor::DrawDebugBox(
    const UWorld* InWorld, FVector const& Center, FVector const& Box,
    const FQuat& Rotation, FColor const& Color, bool bPersistentLines,
    float LifeTime, uint8 DepthPriority, float Thickness)
{
  // No debug line drawing on dedicated server.
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    // This means foreground lines can't be persistent.
    if (ULineBatchComponent_CARLA* const LineBatcher =
            GetDebugLineBatcher(bPersistentLines))
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

void ACosmosControlSensor::DrawDebugSolidBox(
    const UWorld* InWorld, FVector const& Center, FVector const& Extent,
    FQuat const& Rotation, FColor const& Color, bool bPersistent,
    float LifeTime, uint8 DepthPriority)
{
  // No debug line drawing on dedicated server.
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    if (ULineBatchComponent_CARLA* const LineBatcher =
            GetDebugLineBatcher(bPersistent))
    {
      // Build transform from Rotation, Center with uniform scale of 1.0.
      FTransform Transform(Rotation, Center, FVector(1.0f, 1.0f, 1.0f));
      // The Transform handles the Center location, so this box needs to be
      // centered on origin.
      FBox Box = FBox::BuildAABB(FVector::ZeroVector, Extent);
      LineBatcher->DrawSolidBox(Box, Transform, Color, DepthPriority, 0.0f);
    }
  }
}

void ACosmosControlSensor::DrawDebugLine(
    const UWorld* InWorld, FVector const& LineStart, FVector const& LineEnd,
    FColor const& Color, bool bPersistentLines, float LifeTime,
    uint8 DepthPriority, float Thickness)
{
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    // This means foreground lines can't be persistent.
    if (ULineBatchComponent_CARLA* const LineBatcher =
            GetDebugLineBatcher(bPersistentLines))
    {
      float rlinear = ((float)Color.R) / 255.0f;
      float glinear = ((float)Color.G) / 255.0f;
      float blinear = ((float)Color.B) / 255.0f;

      LineBatcher->DrawLine(
          LineStart, LineEnd, FLinearColor(rlinear, glinear, blinear),
          DepthPriority, Thickness, 0.0f);
    }
  }
}

void ACosmosControlSensor::DrawDebugMesh(
    const UWorld* InWorld, const TArray<FVector>& Vertices,
    const TArray<int32>& Indices, const FColor& Color, bool bPersistentLines,
    float LifeTime, uint8 DepthPriority)
{
  // No debug mesh drawing on dedicated server.
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    if (ULineBatchComponent_CARLA* const LineBatcher =
            GetDebugLineBatcher(bPersistentLines))
    {
      LineBatcher->DrawMesh(Vertices, Indices, Color, DepthPriority, 0.0f);
    }
  }
}

void ACosmosControlSensor::DrawDebugCapsule(
    const UWorld* InWorld, FVector const& Center, float HalfHeight,
    float Radius, const FQuat& Rotation, FColor const& Color,
    bool bPersistentLines, float LifeTime, uint8 DepthPriority,
    float Thickness)
{
  // No debug line drawing on dedicated server.
  if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
  {
    const int32 DrawCollisionSides = 16;

    FVector Origin = Center;
    FMatrix Axes = FQuatRotationTranslationMatrix(Rotation, FVector::ZeroVector);
    FVector XAxis = Axes.GetScaledAxis(EAxis::X);
    FVector YAxis = Axes.GetScaledAxis(EAxis::Y);
    FVector ZAxis = Axes.GetScaledAxis(EAxis::Z);

    // Draw top and bottom circles.
    float HalfAxis = FMath::Max<float>(HalfHeight - Radius, 1.f);
    FVector TopEnd = Origin + HalfAxis * ZAxis;
    FVector BottomEnd = Origin - HalfAxis * ZAxis;

    DrawCircle(InWorld, TopEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawCircle(InWorld, BottomEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

    // Draw domed caps.
    DrawHalfCircle(InWorld, TopEnd, YAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawHalfCircle(InWorld, TopEnd, XAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

    FVector NegZAxis = -ZAxis;

    DrawHalfCircle(InWorld, BottomEnd, YAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawHalfCircle(InWorld, BottomEnd, XAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

    // Draw connected lines.
    DrawDebugLine(InWorld, TopEnd + Radius * XAxis, BottomEnd + Radius * XAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugLine(InWorld, TopEnd - Radius * XAxis, BottomEnd - Radius * XAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugLine(InWorld, TopEnd + Radius * YAxis, BottomEnd + Radius * YAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    DrawDebugLine(InWorld, TopEnd - Radius * YAxis, BottomEnd - Radius * YAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
  }
}

void ACosmosControlSensor::DrawHalfCircle(
    const UWorld* InWorld, const FVector& Base, const FVector& X,
    const FVector& Y, const FColor& Color, float Radius, int32 NumSides,
    bool bPersistentLines, float LifeTime, uint8 DepthPriority,
    float Thickness)
{
  float AngleDelta = 2.0f * (float)PI / ((float)NumSides);
  FVector LastVertex = Base + X * Radius;

  for (int32 SideIndex = 0; SideIndex < (NumSides / 2); SideIndex++)
  {
    FVector Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
    DrawDebugLine(InWorld, LastVertex, Vertex, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    LastVertex = Vertex;
  }
}

void ACosmosControlSensor::DrawCircle(
    const UWorld* InWorld, const FVector& Base, const FVector& X,
    const FVector& Y, const FColor& Color, float Radius, int32 NumSides,
    bool bPersistentLines, float LifeTime, uint8 DepthPriority,
    float Thickness)
{
  const float AngleDelta = 2.0f * PI / NumSides;
  FVector LastVertex = Base + X * Radius;

  for (int32 SideIndex = 0; SideIndex < NumSides; SideIndex++)
  {
    const FVector Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
    DrawDebugLine(InWorld, LastVertex, Vertex, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
    LastVertex = Vertex;
  }
}
