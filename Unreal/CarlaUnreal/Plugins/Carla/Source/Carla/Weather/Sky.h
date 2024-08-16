#pragma once
#include "GameFramework/Actor.h"
#include "Sky.generated.h"

class UPostProcessComponent;
class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class USkyAtmosphereComponent;

struct CARLA_API FWeatherParameters;

UCLASS(Abstract)
class CARLA_API ASkyBase :
	public AActor
{
	GENERATED_BODY()

public:

	ASkyBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPostProcessComponent* GetPostProcess();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UExponentialHeightFogComponent* GetExponentialHeightFog();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDirectionalLightComponent* GetDirectionalLight();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkyLightComponent* GetSkyLight();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UVolumetricCloudComponent* GetVolumetricCloud();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkyAtmosphereComponent* GetSkyAtmosphere();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsNight() const;

	UFUNCTION(BlueprintCallable)
	void UpdateSun();

	UFUNCTION(BlueprintCallable)
	void UpdateMoon();

	UFUNCTION(BlueprintCallable)
	void UpdateNight();

protected:

	UPROPERTY(EditAnywhere, Category = "Components")
	UPostProcessComponent* PostProcess;

	UPROPERTY(EditAnywhere, Category = "Components")
	UExponentialHeightFogComponent* ExponentialHeightFog;

	UPROPERTY(EditAnywhere, Category = "Components")
	UDirectionalLightComponent* DirectionalLight;

	UPROPERTY(EditAnywhere, Category = "Components")
	USkyLightComponent* SkyLight;

	UPROPERTY(EditAnywhere, Category = "Components")
	UVolumetricCloudComponent* VolumetricCloud;

	UPROPERTY(EditAnywhere, Category = "Components")
	USkyAtmosphereComponent* SkyAtmosphere;

	UPROPERTY(EditAnywhere, Category = "Weather")
	FWeatherParameters WeatherParameters;


};
