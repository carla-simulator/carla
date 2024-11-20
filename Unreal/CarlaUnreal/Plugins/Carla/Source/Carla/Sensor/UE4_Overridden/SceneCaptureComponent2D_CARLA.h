// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  const AActor* ViewActor;

	virtual const AActor* GetViewOwner() const override;
};
