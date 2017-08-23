// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

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
