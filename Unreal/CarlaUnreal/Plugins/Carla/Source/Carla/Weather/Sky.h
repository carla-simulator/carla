#pragma once
#include "GameFramework/Actor.h"
#include "Sky.generated.h"

class APostProcessVolume;
class AExponentialHeightFog;
class UDirectionalLightComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class USkyAtmosphereComponent;

UCLASS(Abstract)
class CARLA_API ASkyBase :
	public AActor
{
	GENERATED_BODY()
public:

	ASkyBase(const FObjectInitializer& ObjectInitializer);

	APostProcessVolume* PostProcessVolume;
	AExponentialHeightFog* ExponentialHeightFog;
	UDirectionalLightComponent* DirectionalLightComponentSun;
	UDirectionalLightComponent* DirectionalLightComponentMoon;
	USkyLightComponent* SkyLightComponent;
	UVolumetricCloudComponent* VolumetricCloudComponent;
	USkyAtmosphereComponent* SkyAtmosphereComponent;

};