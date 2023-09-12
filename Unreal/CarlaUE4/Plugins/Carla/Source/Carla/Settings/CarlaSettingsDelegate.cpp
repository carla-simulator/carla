#include "Carla.h"
#include "Game/CarlaGameInstance.h"
#include "Carla/Settings/CarlaSettingsDelegate.h"

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Async.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/HUD.h"
#include "InstancedFoliageActor.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "Scalability.h"


static constexpr float CARLA_SETTINGS_MAX_SCALE_SIZE = 50.0f;

/// quality settings configuration between runs
EQualityLevel UCarlaSettingsDelegate::AppliedLowPostResetQualityLevel = EQualityLevel::Epic;

UCarlaSettingsDelegate::UCarlaSettingsDelegate()
  : ActorSpawnedDelegate(FOnActorSpawned::FDelegate::CreateUObject(
        this,
        &UCarlaSettingsDelegate::OnActorSpawned)) {}

void UCarlaSettingsDelegate::Reset()
{
  AppliedLowPostResetQualityLevel = EQualityLevel::Epic;
}

void UCarlaSettingsDelegate::RegisterSpawnHandler(UWorld *InWorld)
{
  CheckCarlaSettings(InWorld);
  InWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
}

void UCarlaSettingsDelegate::OnActorSpawned(AActor *InActor)
{
  check(CarlaSettings != nullptr);
  if (InActor != nullptr && IsValid(InActor) && !InActor->IsPendingKill() &&
      !InActor->IsA<AInstancedFoliageActor>() && // foliage culling is
                                                 // controlled per instance
      !InActor->IsA<ALandscape>() && // dont touch landscapes nor roads
      !InActor->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG) &&
      !InActor->ActorHasTag(UCarlaSettings::CARLA_SKY_TAG))
  {
    TArray<UPrimitiveComponent *> components;
    InActor->GetComponents(components);
    switch (CarlaSettings->GetQualityLevel())
    {
      case EQualityLevel::Low: {
        // apply settings for this actor for the current quality level
        float dist = CarlaSettings->LowStaticMeshMaxDrawDistance;
        const float maxscale = InActor->GetActorScale().GetMax();
        if (maxscale > CARLA_SETTINGS_MAX_SCALE_SIZE)
        {
          dist *= 100.0f;
        }
        SetActorComponentsDrawDistance(InActor, dist);
        break;
      }
      default: break;
    }
  }
}

void UCarlaSettingsDelegate::ApplyQualityLevelPostRestart()
{
  CheckCarlaSettings(nullptr);
  UWorld *InWorld = CarlaSettings->GetWorld();

  const EQualityLevel QualityLevel = CarlaSettings->GetQualityLevel();

  if (AppliedLowPostResetQualityLevel == QualityLevel)
  {
    return;
  }

  // enable temporal changes of quality (prevent saving last quality settings to file)
  Scalability::ToggleTemporaryQualityLevels(true);

  switch (QualityLevel)
  {
    case EQualityLevel::Low:
    {
      // execute tweaks for quality
      LaunchLowQualityCommands(InWorld);
      // iterate all directional lights, deactivate shadows
      SetAllLights(InWorld, CarlaSettings->LowLightFadeDistance, false, true);
      // Set all the roads the low quality materials
      SetAllRoads(InWorld, CarlaSettings->LowRoadPieceMeshMaxDrawDistance, CarlaSettings->LowRoadMaterials);
      // Set all actors with static meshes a max disntace configured in the
      // global settings for the low quality
      SetAllActorsDrawDistance(InWorld, CarlaSettings->LowStaticMeshMaxDrawDistance);
      // Disable all post process volumes
      SetPostProcessEffectsEnabled(InWorld, false);
      break;
    }
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unknown quality level, falling back to default."));
    case EQualityLevel::Epic:
    {
      LaunchEpicQualityCommands(InWorld);
      SetAllLights(InWorld, 0.0f, true, false);
      SetAllRoads(InWorld, 0, CarlaSettings->EpicRoadMaterials);
      SetAllActorsDrawDistance(InWorld, 0);
      SetPostProcessEffectsEnabled(InWorld, true);
      break;
    }
  }
  AppliedLowPostResetQualityLevel = QualityLevel;
}

void UCarlaSettingsDelegate::ApplyQualityLevelPreRestart()
{
  CheckCarlaSettings(nullptr);
  UWorld *InWorld = CarlaSettings->GetWorld();
  if (!IsValid(InWorld) || InWorld->IsPendingKill())
  {
    return;
  }
  // enable or disable world and hud rendering
  APlayerController *playercontroller = UGameplayStatics::GetPlayerController(InWorld, 0);
  if (playercontroller)
  {
    ULocalPlayer *player = playercontroller->GetLocalPlayer();
    if (player)
    {
      player->ViewportClient->bDisableWorldRendering = CarlaSettings->bDisableRendering;
    }
    // if we already have a hud class:
    AHUD *hud = playercontroller->GetHUD();
    if (hud)
    {
      hud->bShowHUD = !CarlaSettings->bDisableRendering;
    }
  }

}

UWorld *UCarlaSettingsDelegate::GetLocalWorld()
{
  return GEngine->GetWorldFromContextObjectChecked(this);
}

void UCarlaSettingsDelegate::CheckCarlaSettings(UWorld *world)
{
  if (IsValid(CarlaSettings))
  {
    return;
  }
  if (world == nullptr || !IsValid(world) || world->IsPendingKill())
  {
    world = GetLocalWorld();
  }
  check(world != nullptr);
  auto GameInstance  = Cast<UCarlaGameInstance>(world->GetGameInstance());
  check(GameInstance != nullptr);
  CarlaSettings = &GameInstance->GetCarlaSettings();
  check(CarlaSettings != nullptr);
}

void UCarlaSettingsDelegate::LaunchLowQualityCommands(UWorld *world) const
{
  if (!world)
  {
    return;
  }

  // launch commands to lower quality settings
  GEngine->Exec(world, TEXT("r.DefaultFeature.MotionBlur 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.Bloom 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AmbientOcclusion 0"));
  GEngine->Exec(world, TEXT("r.AmbientOcclusionLevels 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AmbientOcclusionStaticFraction 0"));
  GEngine->Exec(world, TEXT("r.RHICmdBypass 0"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AntiAliasing 1"));
  GEngine->Exec(world, TEXT("r.Streaming.PoolSize 2000"));
  GEngine->Exec(world, TEXT("r.HZBOcclusion 0"));
  GEngine->Exec(world, TEXT("r.MinScreenRadiusForLights 0.01"));
  GEngine->Exec(world, TEXT("r.SeparateTranslucency 0"));
  GEngine->Exec(world, TEXT("r.FinishCurrentFrame 0"));
  GEngine->Exec(world, TEXT("r.MotionBlurQuality 0"));
  GEngine->Exec(world, TEXT("r.PostProcessAAQuality 0"));
  GEngine->Exec(world, TEXT("r.BloomQuality 1"));
  GEngine->Exec(world, TEXT("r.SSR.Quality 0"));
  GEngine->Exec(world, TEXT("r.DepthOfFieldQuality 0"));
  GEngine->Exec(world, TEXT("r.SceneColorFormat 2"));
  GEngine->Exec(world, TEXT("r.TranslucencyVolumeBlur 0"));
  GEngine->Exec(world, TEXT("r.TranslucencyLightingVolumeDim 4"));
  GEngine->Exec(world, TEXT("r.MaxAnisotropy 8"));
  GEngine->Exec(world, TEXT("r.LensFlareQuality 0"));
  GEngine->Exec(world, TEXT("r.SceneColorFringeQuality 0"));
  GEngine->Exec(world, TEXT("r.FastBlurThreshold 0"));
  GEngine->Exec(world, TEXT("r.SSR.MaxRoughness 0.1"));
  GEngine->Exec(world, TEXT("r.AllowOcclusionQueries 1"));
  GEngine->Exec(world, TEXT("r.SSR 0"));
  // GEngine->Exec(world,TEXT("r.StencilForLODDither 1")); //readonly
  GEngine->Exec(world, TEXT("r.EarlyZPass 2")); // transparent before opaque
  GEngine->Exec(world, TEXT("r.EarlyZPassMovable 1"));
  GEngine->Exec(world, TEXT("Foliage.DitheredLOD 0"));
  // GEngine->Exec(world,TEXT("r.ForwardShading 0")); //readonly
  GEngine->Exec(world, TEXT("sg.PostProcessQuality 0"));
  // GEngine->Exec(world,TEXT("r.ViewDistanceScale 0.1")); //--> too extreme
  // (far clip too short)
  GEngine->Exec(world, TEXT("sg.ShadowQuality 0"));
  GEngine->Exec(world, TEXT("sg.TextureQuality 0"));
  GEngine->Exec(world, TEXT("sg.EffectsQuality 0"));
  GEngine->Exec(world, TEXT("sg.FoliageQuality 0"));
  GEngine->Exec(world, TEXT("foliage.DensityScale 0"));
  GEngine->Exec(world, TEXT("grass.DensityScale 0"));
  GEngine->Exec(world, TEXT("r.TranslucentLightingVolume 0"));
  GEngine->Exec(world, TEXT("r.LightShaftDownSampleFactor 4"));
  GEngine->Exec(world, TEXT("r.OcclusionQueryLocation 1"));
  // GEngine->Exec(world,TEXT("r.BasePassOutputsVelocity 0")); //--> readonly
  // GEngine->Exec(world,TEXT("r.DetailMode 0")); //-->will change to lods 0
  GEngine->Exec(world, TEXT("r.DefaultFeature.AutoExposure 1"));

}

void UCarlaSettingsDelegate::SetAllRoads(
    UWorld *world,
    const float max_draw_distance,
    const TArray<FStaticMaterial> &road_pieces_materials) const
{
  if (!world || !IsValid(world) || world->IsPendingKill())
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!world || !IsValid(world) || world->IsPendingKill())
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsWithTag(world, UCarlaSettings::CARLA_ROAD_TAG, actors);

    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor) || actor->IsPendingKill())
      {
        continue;
      }
      TArray<UStaticMeshComponent *> components;
      actor->GetComponents(components);
      for (int32 j = 0; j < components.Num(); j++)
      {
        UStaticMeshComponent *staticmeshcomponent = Cast<UStaticMeshComponent>(components[j]);
        if (staticmeshcomponent)
        {
          staticmeshcomponent->bAllowCullDistanceVolume = (max_draw_distance > 0);
          staticmeshcomponent->bUseAsOccluder = false;
          staticmeshcomponent->LDMaxDrawDistance = max_draw_distance;
          staticmeshcomponent->CastShadow = (max_draw_distance == 0);
          if (road_pieces_materials.Num() > 0)
          {
            TArray<FName> meshslotsnames = staticmeshcomponent->GetMaterialSlotNames();
            for (int32 k = 0; k < meshslotsnames.Num(); k++)
            {
              const FName &slotname = meshslotsnames[k];
              road_pieces_materials.ContainsByPredicate(
              [staticmeshcomponent, slotname](const FStaticMaterial &material)
              {
                if (material.MaterialSlotName.IsEqual(slotname))
                {
                  staticmeshcomponent->SetMaterial(
                  staticmeshcomponent->GetMaterialIndex(slotname),
                  material.MaterialInterface);
                  return true;
                }
                else
                {
                  return false;
                }
              });
            }
          }
        }
      }
    }
  }); // ,DELAY_TIME_TO_SET_ALL_ROADS, false);
}

void UCarlaSettingsDelegate::SetActorComponentsDrawDistance(
    AActor *actor,
    const float max_draw_distance) const
{
  if (!actor)
  {
    return;
  }
  TArray<UPrimitiveComponent *> components;
  actor->GetComponents(components, false);
  float dist = max_draw_distance;
  const float maxscale = actor->GetActorScale().GetMax();
  if (maxscale > CARLA_SETTINGS_MAX_SCALE_SIZE)
  {
    dist *= 100.0f;
  }
  for (int32 j = 0; j < components.Num(); j++)
  {
    UPrimitiveComponent *primitivecomponent = Cast<UPrimitiveComponent>(components[j]);
    if (IsValid(primitivecomponent))
    {
      primitivecomponent->SetCullDistance(dist);
      primitivecomponent->bAllowCullDistanceVolume = dist > 0;
    }
  }
}

void UCarlaSettingsDelegate::SetAllActorsDrawDistance(UWorld *world, const float max_draw_distance) const
{
  /// @TODO: use semantics to grab all actors by type
  /// (vehicles,ground,people,props) and set different distances configured in
  /// the global properties
  if (!world || !IsValid(world) || world->IsPendingKill())
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!world || !IsValid(world) || world->IsPendingKill())
    {
      return;
    }
    TArray<AActor *> actors;
    // set the lower quality - max draw distance
    UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor) || actor->IsPendingKill() ||
      actor->IsA<AInstancedFoliageActor>() ||   // foliage culling is controlled
                                                // per instance
      actor->IsA<ALandscape>() ||   // dont touch landscapes nor roads
      actor->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG) ||
      actor->ActorHasTag(UCarlaSettings::CARLA_SKY_TAG))
      {
        continue;
      }
      SetActorComponentsDrawDistance(actor, max_draw_distance);
    }
  });
}

void UCarlaSettingsDelegate::SetPostProcessEffectsEnabled(UWorld *world, const bool enabled) const
{
  TArray<AActor *> actors;
  UGameplayStatics::GetAllActorsOfClass(world, APostProcessVolume::StaticClass(), actors);
  for (int32 i = 0; i < actors.Num(); i++)
  {
    AActor *actor = actors[i];
    if (!IsValid(actor) || actor->IsPendingKill())
    {
      continue;
    }
    APostProcessVolume *postprocessvolume = Cast<APostProcessVolume>(actor);
    if (postprocessvolume)
    {
      postprocessvolume->bEnabled = enabled;
    }
  }
}

void UCarlaSettingsDelegate::LaunchEpicQualityCommands(UWorld *world) const
{
  if (!world)
  {
    return;
  }

  GEngine->Exec(world, TEXT("r.AmbientOcclusionLevels -1"));
  GEngine->Exec(world, TEXT("r.RHICmdBypass 1"));
  GEngine->Exec(world, TEXT("r.DefaultFeature.AntiAliasing 1"));
  GEngine->Exec(world, TEXT("r.Streaming.PoolSize 2000"));
  GEngine->Exec(world, TEXT("r.MinScreenRadiusForLights 0.03"));
  GEngine->Exec(world, TEXT("r.SeparateTranslucency 1"));
  GEngine->Exec(world, TEXT("r.PostProcessAAQuality 4"));
  GEngine->Exec(world, TEXT("r.BloomQuality 5"));
  GEngine->Exec(world, TEXT("r.SSR.Quality 3"));
  GEngine->Exec(world, TEXT("r.DepthOfFieldQuality 2"));
  GEngine->Exec(world, TEXT("r.SceneColorFormat 4"));
  GEngine->Exec(world, TEXT("r.TranslucencyVolumeBlur 1"));
  GEngine->Exec(world, TEXT("r.TranslucencyLightingVolumeDim 64"));
  GEngine->Exec(world, TEXT("r.MaxAnisotropy 8"));
  GEngine->Exec(world, TEXT("r.LensFlareQuality 2"));
  GEngine->Exec(world, TEXT("r.SceneColorFringeQuality 1"));
  GEngine->Exec(world, TEXT("r.FastBlurThreshold 100"));
  GEngine->Exec(world, TEXT("r.SSR.MaxRoughness -1"));
  // GEngine->Exec(world,TEXT("r.StencilForLODDither 0")); //readonly
  GEngine->Exec(world, TEXT("r.EarlyZPass 3"));
  GEngine->Exec(world, TEXT("r.EarlyZPassMovable 1"));
  GEngine->Exec(world, TEXT("Foliage.DitheredLOD 1"));
  GEngine->Exec(world, TEXT("sg.PostProcessQuality 3"));
  GEngine->Exec(world, TEXT("r.ViewDistanceScale 1")); // --> too extreme (far
                                                       // clip too short)
  GEngine->Exec(world, TEXT("sg.ShadowQuality 3"));
  GEngine->Exec(world, TEXT("sg.TextureQuality 3"));
  GEngine->Exec(world, TEXT("sg.EffectsQuality 3"));
  GEngine->Exec(world, TEXT("sg.FoliageQuality 3"));
  GEngine->Exec(world, TEXT("foliage.DensityScale 1"));
  GEngine->Exec(world, TEXT("grass.DensityScale 1"));
  GEngine->Exec(world, TEXT("r.TranslucentLightingVolume 1"));
  GEngine->Exec(world, TEXT("r.LightShaftDownSampleFactor 2"));
  // GEngine->Exec(world,TEXT("r.OcclusionQueryLocation 0"));
  // GEngine->Exec(world,TEXT("r.BasePassOutputsVelocity 0")); //readonly
  GEngine->Exec(world, TEXT("r.DetailMode 2"));
}

void UCarlaSettingsDelegate::SetAllLights(
    UWorld *world,
    const float max_distance_fade,
    const bool cast_shadows,
    const bool hide_non_directional) const
{
  if (!world || !IsValid(world) || world->IsPendingKill())
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!world || !IsValid(world) || world->IsPendingKill())
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsOfClass(world, ALight::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      if (!IsValid(actors[i]) || actors[i]->IsPendingKill())
      {
        continue;
      }
      // tweak directional lights
      ADirectionalLight *directionallight = Cast<ADirectionalLight>(actors[i]);
      if (directionallight)
      {
        directionallight->SetCastShadows(cast_shadows);
        directionallight->SetLightFunctionFadeDistance(max_distance_fade);
        continue;
      }
      // disable any other type of light
      actors[i]->SetActorHiddenInGame(hide_non_directional);
    }
  });

}
