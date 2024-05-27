#include "Sky.h"

ASkyBase::ASkyBase(
	const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	PostProcessComponent(nullptr),
	ExponentialHeightFogComponent(nullptr),
	DirectionalLightComponentSun(nullptr),
	DirectionalLightComponentMoon(nullptr),
	SkyLightComponent(nullptr),
	VolumetricCloudComponent(nullptr),
	SkyAtmosphereComponent(nullptr),
{
}
