// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UObject/WeakObjectPtr.h"
#include <util/ue-header-guard-end.h>

#include "SkyPreviewSubsystem.generated.h"

/// Lights the editor viewport of a map that carries no sun/sky rig, so an
/// imported map does not open to a black screen that reads as "still loading".
///
/// The rig the game mode spawns at BeginPlay only exists while playing, and
/// BaseMap saves none, so a stopped imported map has no light source at all.
/// This spawns a transient BP_Carla_Sky on map open: being transient it is
/// never serialized into the .umap, never becomes a World Partition actor and
/// never reaches a cook, so the map on disk is unchanged. It is a preview
/// only, and the weather API does not drive it.
UCLASS()
class CARLATOOLS_API USkyPreviewSubsystem : public UEditorSubsystem
{
  GENERATED_BODY()

public:

  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;

private:

  void OnMapOpened(const FString& Filename, bool bAsTemplate);

  /// Spawns the preview rig unless @a World already has a saved one.
  void SpawnPreviewRig(UWorld* World);

  TWeakObjectPtr<AActor> PreviewRig;

  FDelegateHandle MapOpenedHandle;
};
