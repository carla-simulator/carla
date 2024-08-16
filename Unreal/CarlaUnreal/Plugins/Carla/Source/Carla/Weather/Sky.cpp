#include "Sky.h"
#include "WeatherParameters.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"


ASkyBase::ASkyBase(
	const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
  PostProcess =
	CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));

  ExponentialHeightFog =
	CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"));

  DirectionalLight =
	CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));

  SkyLight =
	CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));

  VolumetricCloud =
	CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"));

  SkyAtmosphere =
	CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
}

UPostProcessComponent* ASkyBase::GetPostProcess()
{
	check(PostProcess != nullptr);
	return PostProcess;
}

UExponentialHeightFogComponent* ASkyBase::GetExponentialHeightFog()
{
	check(ExponentialHeightFog != nullptr);
	return ExponentialHeightFog;
}

UDirectionalLightComponent* ASkyBase::GetDirectionalLight()
{
	check(DirectionalLight != nullptr);
	return DirectionalLight;
}

USkyLightComponent* ASkyBase::GetSkyLight()
{
	check(SkyLight != nullptr);
	return SkyLight;
}

UVolumetricCloudComponent* ASkyBase::GetVolumetricCloud()
{
	check(VolumetricCloud != nullptr);
	return VolumetricCloud;
}

USkyAtmosphereComponent* ASkyBase::GetSkyAtmosphere()
{
	check(SkyAtmosphere != nullptr);
	return SkyAtmosphere;
}

bool ASkyBase::IsNight() const
{
	return false;
}

void ASkyBase::UpdateSun()
{
	DirectionalLight->SetMobility(EComponentMobility::Movable);

	auto Pitch = -WeatherParameters.SunAltitudeAngle;
	auto Yaw = WeatherParameters.SunAzimuthAngle;
	DirectionalLight->SetWorldRotation(FRotator(Pitch, Yaw, 0.0));
}

void ASkyBase::UpdateMoon()
{
}

void ASkyBase::UpdateNight()
{
	if (!IsNight())
		return;

}
