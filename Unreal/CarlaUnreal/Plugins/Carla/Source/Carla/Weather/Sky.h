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

	UPROPERTY(EditAnywhere, Category = "Components")
	APostProcessVolume* PostProcessVolume;

	UPROPERTY(EditAnywhere, Category = "Components")
	AExponentialHeightFog* ExponentialHeightFog;

	UPROPERTY(EditAnywhere, Category = "Components")
	UDirectionalLightComponent* DirectionalLightComponentSun;

	UPROPERTY(EditAnywhere, Category = "Components")
	UDirectionalLightComponent* DirectionalLightComponentMoon;

	UPROPERTY(EditAnywhere, Category = "Components")
	USkyLightComponent* SkyLightComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	UVolumetricCloudComponent* VolumetricCloudComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	USkyAtmosphereComponent* SkyAtmosphereComponent;


};