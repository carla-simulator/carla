// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLight.h"

UCarlaLight::UCarlaLight()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void UCarlaLight::SetLightIntensity(float Intensity)
{
  LightIntensity = Intensity;
  UpdateLights();
}

float UCarlaLight::GetLightIntensity()
{
  return LightIntensity;
}

void UCarlaLight::SetLightColor(FLinearColor Color)
{
  LightColor = Color;
  UpdateLights();
}

FLinearColor UCarlaLight::GetLightColor()
{
  return LightColor;
}

void UCarlaLight::SetLightOn(bool bOn)
{
  bLightOn = bOn;
  UpdateLights();
}

bool UCarlaLight::GetLightOn()
{
  return bLightOn;
}

void UCarlaLight::SetLightType(ELightType Type)
{
  LightType = Type;
}

ELightType UCarlaLight::GetLightType()
{
  return LightType;
}
