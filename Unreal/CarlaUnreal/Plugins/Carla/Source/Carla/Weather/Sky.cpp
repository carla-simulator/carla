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
	PostProcessComponent(CreateDefaultSubobject<UPostProcessComponent>("ASkyBase::PostProcessComponent")),
	ExponentialHeightFogComponent(CreateDefaultSubobject<UExponentialHeightFogComponent>("ASkyBase::ExponentialHeightFogComponent")),
	DirectionalLightComponentSun(CreateDefaultSubobject<UDirectionalLightComponent>("ASkyBase::DirectionalLightComponentSun")),
	DirectionalLightComponentMoon(CreateDefaultSubobject<UDirectionalLightComponent>("ASkyBase::DirectionalLightComponentMoon")),
	SkyLightComponent(CreateDefaultSubobject<USkyLightComponent>("ASkyBase::SkyLightComponent")),
	VolumetricCloudComponent(CreateDefaultSubobject<UVolumetricCloudComponent>("ASkyBase::VolumetricCloudComponent")),
	SkyAtmosphereComponent(CreateDefaultSubobject<USkyAtmosphereComponent>("ASkyBase::SkyAtmosphereComponent"))
{
}
