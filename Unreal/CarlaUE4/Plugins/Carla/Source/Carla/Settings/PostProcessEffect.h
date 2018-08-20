// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "PostProcessEffect.generated.h"

UENUM(BlueprintType)
enum class EPostProcessEffect : uint8
{
  None                  UMETA(DisplayName = "RGB without any post-processing"),
  SceneFinal            UMETA(DisplayName = "RGB with post-processing present at the scene"),
  Depth                 UMETA(DisplayName = "Depth Map"),
  SemanticSegmentation  UMETA(DisplayName = "Semantic Segmentation"),

  SIZE                  UMETA(Hidden),
  INVALID               UMETA(Hidden),
};

/// Helper class for working with EPostProcessEffect.
class CARLA_API PostProcessEffect {
public:

  using uint_type = typename std::underlying_type<EPostProcessEffect>::type;

  static constexpr uint_type ToUInt(EPostProcessEffect PostProcessEffect)
  {
    return static_cast<uint_type>(PostProcessEffect);
  }

  static constexpr EPostProcessEffect FromUInt(uint_type Index)
  {
    return static_cast<EPostProcessEffect>(Index);
  }

  static FString ToString(EPostProcessEffect PostProcessEffect);

  static FString ToString(uint_type Index)
  {
    return ToString(FromUInt(Index));
  }

  static EPostProcessEffect FromString(const FString &String);
};
