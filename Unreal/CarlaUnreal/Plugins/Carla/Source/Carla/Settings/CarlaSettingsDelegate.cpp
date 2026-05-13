// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Settings/CarlaSettingsDelegate.h"
#include "Carla.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Settings/CarlaSettings.h"

#include <util/ue-header-guard-begin.h>
#include "Game/CarlaGameInstance.h"
#include "Async/Async.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/HUD.h"
#include "InstancedFoliageActor.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "Scalability.h"
#include <util/ue-header-guard-end.h>

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
  if (IsValid(InActor) &&
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
      LaunchLowQualityCommands(InWorld);
      SetAllRoads(InWorld, CarlaSettings->LowRoadPieceMeshMaxDrawDistance, CarlaSettings->LowRoadMaterials);
      ApplyPerActorQualitySettings(
          InWorld,
          CarlaSettings->LowLightFadeDistance,
          false,
          true,
          CarlaSettings->LowStaticMeshMaxDrawDistance);
      SetPostProcessEffectsEnabled(InWorld, false);
      break;
    }
    case EQualityLevel::Medium:
    {
      LaunchMediumQualityCommands(InWorld);
      SetAllRoads(InWorld, 0, CarlaSettings->EpicRoadMaterials);
      ApplyPerActorQualitySettings(InWorld, 0.0f, true, false, 0);
      SetPostProcessEffectsEnabled(InWorld, true);
      break;
    }
    case EQualityLevel::High:
    {
      LaunchHighQualityCommands(InWorld);
      SetAllRoads(InWorld, 0, CarlaSettings->EpicRoadMaterials);
      ApplyPerActorQualitySettings(InWorld, 0.0f, true, false, 0);
      SetPostProcessEffectsEnabled(InWorld, true);
      break;
    }
    default:
      UE_LOG(LogCarla, Warning, TEXT("Unknown quality level, falling back to default."));
    case EQualityLevel::Epic:
    {
      LaunchEpicQualityCommands(InWorld);
      SetAllRoads(InWorld, 0, CarlaSettings->EpicRoadMaterials);
      ApplyPerActorQualitySettings(InWorld, 0.0f, true, false, 0);
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
  if (!IsValid(InWorld))
  {
    return;
  }
  // enable or disable world and hud rendering
  APlayerController *playercontroller = UGameplayStatics::GetPlayerController(InWorld, 0);
  if (playercontroller)
  {
    ULocalPlayer *player = playercontroller->GetLocalPlayer();
    if (player)
      player->ViewportClient->bDisableWorldRendering = CarlaSettings->bDisableRendering;
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
  if (!IsValid(world))
  {
    world = GetLocalWorld();
  }
  check(world != nullptr);
  auto GameInstance  = Cast<UCarlaGameInstance>(world->GetGameInstance());
  check(GameInstance != nullptr);
  CarlaSettings = &GameInstance->GetCarlaSettings();
  check(CarlaSettings != nullptr);
}

// LaunchLowQualityCommands, LaunchMediumQualityCommands,
// LaunchHighQualityCommands, and LaunchEpicQualityCommands are retained as
// no-op stubs for the legacy UCarlaSettingsDelegate interface. The active
// CarlaQuality_<Tier> configuration -- memory pools, scalability bucket
// selection, per-tier r.* overrides -- is applied at engine init by
// CarlaDeviceProfileSelectorModule::StartupModule
// (Unreal/CarlaUnreal/Source/CarlaDeviceProfileSelector). There is no
// runtime CVar burst from this delegate.

void UCarlaSettingsDelegate::LaunchLowQualityCommands(UWorld *world) const
{
  (void)world;
}

void UCarlaSettingsDelegate::LaunchMediumQualityCommands(UWorld *world) const
{
  (void)world;
}

void UCarlaSettingsDelegate::LaunchHighQualityCommands(UWorld *world) const
{
  (void)world;
}

void UCarlaSettingsDelegate::SetAllRoads(
    UWorld *world,
    const float max_draw_distance,
    const TArray<FStaticMaterial> &road_pieces_materials) const
{
  if (!IsValid(world))
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!IsValid(world))
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsWithTag(world, UCarlaSettings::CARLA_ROAD_TAG, actors);

    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor))
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
  if (!IsValid(world))
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=, this]() {
    if (!IsValid(world))
    {
      return;
    }
    TArray<AActor *> actors;
    // set the lower quality - max draw distance
    UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor) ||
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
    if (!IsValid(actor))
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
  (void)world;
}

void UCarlaSettingsDelegate::SetAllLights(
    UWorld *world,
    const float max_distance_fade,
    const bool cast_shadows,
    const bool hide_non_directional) const
{
  if (!IsValid(world))
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=]() {
    if (!IsValid(world))
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsOfClass(world, ALight::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      if (!IsValid(actors[i]))
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

void UCarlaSettingsDelegate::ApplyPerActorQualitySettings(
    UWorld *world,
    const float light_fade_distance,
    const bool cast_directional_shadows,
    const bool hide_non_directional_lights,
    const float draw_distance) const
{
  if (!IsValid(world))
  {
    return;
  }
  AsyncTask(ENamedThreads::GameThread, [=, this]() {
    if (!IsValid(world))
    {
      return;
    }
    TArray<AActor *> actors;
    UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(), actors);
    for (int32 i = 0; i < actors.Num(); i++)
    {
      AActor *actor = actors[i];
      if (!IsValid(actor))
      {
        continue;
      }

      if (ADirectionalLight *directional = Cast<ADirectionalLight>(actor))
      {
        directional->SetCastShadows(cast_directional_shadows);
        directional->SetLightFunctionFadeDistance(light_fade_distance);
        continue;
      }
      if (actor->IsA<ALight>())
      {
        actor->SetActorHiddenInGame(hide_non_directional_lights);
        continue;
      }

      if (actor->IsA<AInstancedFoliageActor>() ||
          actor->IsA<ALandscape>() ||
          actor->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG) ||
          actor->ActorHasTag(UCarlaSettings::CARLA_SKY_TAG))
      {
        continue;
      }

      SetActorComponentsDrawDistance(actor, draw_distance);
    }
  });
}
