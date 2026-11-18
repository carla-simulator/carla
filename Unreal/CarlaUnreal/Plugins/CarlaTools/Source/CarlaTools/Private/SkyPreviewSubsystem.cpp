// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SkyPreviewSubsystem.h"

#include "Carla/Weather/Sky.h"
#include "CarlaTools.h"

#include <util/ue-header-guard-begin.h>
#include "Editor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include <util/ue-header-guard-end.h>

namespace
{
  const TCHAR* SkyRigClassPath =
      TEXT("/Game/Carla/Blueprints/LevelDesign/BP_Carla_Sky.BP_Carla_Sky_C");
}

void USkyPreviewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  Super::Initialize(Collection);

  // Commandlets load maps too, and a spawn there would run against the world
  // a conversion or a cook is working on.
  if (IsRunningCommandlet())
  {
    return;
  }

  MapOpenedHandle = FEditorDelegates::OnMapOpened.AddUObject(
      this, &USkyPreviewSubsystem::OnMapOpened);
}

void USkyPreviewSubsystem::Deinitialize()
{
  if (MapOpenedHandle.IsValid())
  {
    FEditorDelegates::OnMapOpened.Remove(MapOpenedHandle);
    MapOpenedHandle.Reset();
  }

  Super::Deinitialize();
}

void USkyPreviewSubsystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
  SpawnPreviewRig(GEditor ? GEditor->GetEditorWorldContext().World() : nullptr);
}

void USkyPreviewSubsystem::SpawnPreviewRig(UWorld* World)
{
  PreviewRig.Reset();

  if (World == nullptr ||
      UGameplayStatics::GetActorOfClass(World, ASkyBase::StaticClass()) != nullptr)
  {
    return;
  }

  UClass* SkyClass = LoadClass<ASkyBase>(nullptr, SkyRigClassPath);
  if (SkyClass == nullptr)
  {
    UE_LOG(LogCarlaTools, Warning, TEXT(
        "Sky preview: could not load %s; the viewport stays unlit."), SkyRigClassPath);
    return;
  }

  // RF_Transient keeps the actor out of the .umap, and so out of the World
  // Partition actor descriptors and every cook.
  FActorSpawnParameters SpawnParameters;
  SpawnParameters.ObjectFlags = RF_Transient;
  SpawnParameters.bCreateActorPackage = false;
  SpawnParameters.Name = TEXT("CarlaSkyPreview");
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  AActor* Rig = World->SpawnActor<AActor>(SkyClass, FTransform::Identity, SpawnParameters);
  if (Rig == nullptr)
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("Sky preview: could not spawn %s."), SkyRigClassPath);
    return;
  }

  Rig->SetActorLabel(TEXT("CarlaSkyPreview (editor only, not saved)"));
  PreviewRig = Rig;

  UE_LOG(LogCarlaTools, Log, TEXT(
      "Sky preview: spawned a transient BP_Carla_Sky in %s."), *World->GetMapName());
}
