// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "PostProcessEffect.h"

FString PostProcessEffect::ToString(EPostProcessEffect PostProcessEffect)
{
  static_assert(
      static_cast<uint8>(EPostProcessEffect::SIZE) == 4u,
      "If you add a new post-process effect, please update these functions.");

  switch (PostProcessEffect)
  {
    case EPostProcessEffect::None:                  return TEXT("None");
    case EPostProcessEffect::SceneFinal:            return TEXT("SceneFinal");
    case EPostProcessEffect::Depth:                 return TEXT("Depth");
    case EPostProcessEffect::SemanticSegmentation:  return TEXT("SemanticSegmentation");
    default:
      UE_LOG(LogCarla, Error, TEXT("Invalid post-processing effect \"%d\""), ToUInt(PostProcessEffect));
      return TEXT("INVALID");
  }
}

EPostProcessEffect PostProcessEffect::FromString(const FString &String)
{
  auto Str = String.ToLower();
  if (Str == "none") {
    return EPostProcessEffect::None;
  } else if (Str == "scenefinal") {
    return EPostProcessEffect::SceneFinal;
  } else if (Str == "depth") {
    return EPostProcessEffect::Depth;
  } else if (Str == "semanticsegmentation") {
    return EPostProcessEffect::SemanticSegmentation;
  } else {
    UE_LOG(LogCarla, Error, TEXT("Invalid post-processing effect \"%s\""), *String);
    return EPostProcessEffect::INVALID;
  }
}
