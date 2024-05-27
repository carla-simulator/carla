#pragma once
#include "GameFramework/Actor.h"
#include "SkyBase.generated.h"

UCLASS(Abstract)
class CARLA_API ASkyBase :
	public AActor
{
	GENERATED_BODY()
public:

	ASkyBase(const FObjectInitializer& ObjectInitializer);

	APostProcessVolume* PostProcessVolume;

};