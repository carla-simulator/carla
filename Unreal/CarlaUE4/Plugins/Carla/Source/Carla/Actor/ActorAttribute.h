// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/ActorAttributeType.h>
#include <compiler/enable-ue4-macros.h>

#include "ActorAttribute.generated.h"

#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::ActorAttributeType:: e)

/// List of valid types for actor attributes.
UENUM(BlueprintType)
enum class EActorAttributeType : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  Bool      = CARLA_ENUM_FROM_RPC(Bool)      UMETA(DisplayName = "Bool"),
  Int       = CARLA_ENUM_FROM_RPC(Int)       UMETA(DisplayName = "Integer"),
  Float     = CARLA_ENUM_FROM_RPC(Float)     UMETA(DisplayName = "Float"),
  String    = CARLA_ENUM_FROM_RPC(String)    UMETA(DisplayName = "String"),
  RGBColor  = CARLA_ENUM_FROM_RPC(RGBColor)  UMETA(DisplayName = "RGB Color (comma separated)"),

  SIZE        UMETA(Hidden),
  INVALID     UMETA(Hidden)
};

#undef CARLA_ENUM_FROM_RPC

/// Definition of an actor variation. Variations represent attributes of the
/// actor that the user can modify in order to generate variations of the same
/// actor.
///
/// A list of recommended values is given. If bRestrictToRecommended is true,
/// only recommended values are accepted as valid.
USTRUCT(BlueprintType)
struct CARLA_API FActorVariation
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EActorAttributeType Type = EActorAttributeType::String;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FString> RecommendedValues;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bRestrictToRecommended = false;
};

/// An actor attribute, may be an intrinsic (non-modifiable) attribute of the
/// actor or an user-defined actor variation.
USTRUCT(BlueprintType)
struct CARLA_API FActorAttribute
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EActorAttributeType Type = EActorAttributeType::String;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Value;
};
