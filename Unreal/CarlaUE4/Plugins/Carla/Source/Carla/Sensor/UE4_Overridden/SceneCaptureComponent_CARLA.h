// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/SceneCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneCaptureComponentCube.h"

#include "SceneCaptureComponent_CARLA.generated.h"

class AActor;
class FObjectInitializer;



UCLASS(abstract, hidecategories=(Collision, Object, Physics, SceneComponent, Mobility))
class CARLA_API USceneCaptureComponent_CARLA : public USceneCaptureComponent
{
	GENERATED_BODY()
public:

    USceneCaptureComponent_CARLA(const FObjectInitializer& = FObjectInitializer::Get());

    const AActor* ViewActor;

	inline virtual const AActor* GetViewOwner() const override { return ViewActor; }
};



UCLASS(abstract, hidecategories=(Collision, Object, Physics, SceneComponent, Mobility))
class CARLA_API USceneCaptureComponent2D_CARLA : public USceneCaptureComponent2D
{
	GENERATED_BODY()
public:

    USceneCaptureComponent2D_CARLA(const FObjectInitializer& = FObjectInitializer::Get());

    const AActor* ViewActor;

	inline virtual const AActor* GetViewOwner() const override { return ViewActor; }
};



UCLASS(abstract, hidecategories=(Collision, Object, Physics, SceneComponent, Mobility))
class CARLA_API USceneCaptureComponentCube_CARLA : public USceneCaptureComponentCube
{
	GENERATED_BODY()
public:

    USceneCaptureComponentCube_CARLA(const FObjectInitializer& = FObjectInitializer::Get());

    const AActor* ViewActor;

	inline virtual const AActor* GetViewOwner() const override { return ViewActor; }
};