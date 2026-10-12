// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include <util/ue-header-guard-end.h>

#include "SceneCaptureComponent2D_CARLA.generated.h"



UCLASS(hidecategories=(Collision, Object, Physics, SceneComponent, Mobility))
class CARLA_API USceneCaptureComponent2D_CARLA : public USceneCaptureComponent2D
{
	GENERATED_BODY()
public:

  USceneCaptureComponent2D_CARLA(const FObjectInitializer& = FObjectInitializer::Get());

  UPROPERTY()
  TObjectPtr<AActor> ViewActor;

	virtual const AActor* GetViewOwner() const override;

  /// Free the persistent per-view render state (Lumen scene, TSR/TAA
  /// history, shadow caches...) right away. USceneCaptureComponent only
  /// releases ViewStates when the UObject is garbage collected, which in
  /// -game can be a minute after the sensor is destroyed; with
  /// bAlwaysPersistRenderingState that state is hundreds of MB per view.
  /// Game thread only; call after the render thread is done with the view
  /// (FlushRenderingCommands).
  void ReleaseViewStates()
  {
    ViewStates.Empty();
  }
};
