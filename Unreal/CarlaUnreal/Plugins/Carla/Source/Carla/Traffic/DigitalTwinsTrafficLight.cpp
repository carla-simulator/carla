// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DigitalTwinsTrafficLight.h"
#include "Carla.h"

ADigitalTwinsTrafficLight::ADigitalTwinsTrafficLight(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void ADigitalTwinsTrafficLight::BeginPlay()
{
  Super::BeginPlay();

  UTrafficLightComponent* TLComponent = GetTrafficLightComponent();
  if (TLComponent)
  {
    TLComponent->LightChangeDispatcher.AddDynamic(this, &ADigitalTwinsTrafficLight::OnLightStateChanged);
    UpdateToolRigLamps(TLComponent->GetLightState(), false);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("DigitalTwinsTrafficLight: No TrafficLightComponent found"));
  }
}

void ADigitalTwinsTrafficLight::OnLightStateChanged()
{
  UTrafficLightComponent* TLComponent = GetTrafficLightComponent();
  if (TLComponent)
  {
    // Every mesh of this actor is a rig mesh, tagged or not.
    UpdateToolRigLamps(TLComponent->GetLightState(), false);
  }
}
