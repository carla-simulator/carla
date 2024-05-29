#include "Sky.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"


ASkyBase::ASkyBase(
	const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	PostProcessComponent(nullptr),
	ExponentialHeightFogComponent(nullptr),
	DirectionalLightComponentSun(nullptr),
	DirectionalLightComponentMoon(nullptr),
	SkyLightComponent(nullptr),
	VolumetricCloudComponent(nullptr),
	SkyAtmosphereComponent(nullptr)
{
  PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>("PostProcessComponent");
  ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>("ExponentialHeightFogComponent");
  DirectionalLightComponentSun = CreateDefaultSubobject<UDirectionalLightComponent>("DirectionalLightComponentSun");
  DirectionalLightComponentMoon = CreateDefaultSubobject<UDirectionalLightComponent>("DirectionalLightComponentMoon");
  SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>("SkyLightComponent");
  VolumetricCloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>("VolumetricCloudComponent");
  SkyAtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>("SkyAtmosphereComponent");
}
