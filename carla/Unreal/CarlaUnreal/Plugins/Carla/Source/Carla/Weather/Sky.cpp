#include "Sky.h"

#include <util/ue-header-guard-begin.h>
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include <util/ue-header-guard-end.h>


ASkyBase::ASkyBase(
	const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
  PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>("PostProcessComponent");
  RootComponent = PostProcessComponent;

  ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>("ExponentialHeightFogComponent");
  ExponentialHeightFogComponent->SetupAttachment(RootComponent);

  DirectionalLightComponentSun = CreateDefaultSubobject<UDirectionalLightComponent>("DirectionalLightComponentSun");
  DirectionalLightComponentSun->SetupAttachment(RootComponent);

  DirectionalLightComponentMoon = CreateDefaultSubobject<UDirectionalLightComponent>("DirectionalLightComponentMoon");
  DirectionalLightComponentMoon->SetupAttachment(RootComponent);

  SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>("SkyLightComponent");
  SkyLightComponent->SetupAttachment(RootComponent);

  VolumetricCloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>("VolumetricCloudComponent");
  VolumetricCloudComponent->SetupAttachment(RootComponent);

  SkyAtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>("SkyAtmosphereComponent");
  SkyAtmosphereComponent->SetupAttachment(RootComponent);

}
