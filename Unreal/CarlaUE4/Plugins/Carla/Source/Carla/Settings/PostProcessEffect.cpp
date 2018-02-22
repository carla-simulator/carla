// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "PostProcessEffect.h"

#include "Package.h"

FString PostProcessEffect::ToString(EPostProcessEffect PostProcessEffect)
{
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPostProcessEffect"), true);
  if(!ptr)
    return FString("Invalid");
  return ptr->GetNameStringByIndex(static_cast<int32>(PostProcessEffect));
}
