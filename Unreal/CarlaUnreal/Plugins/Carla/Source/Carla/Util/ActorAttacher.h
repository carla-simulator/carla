// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Kismet/BlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <carla/rpc/AttachmentType.h>
#include <util/enable-ue4-macros.h>

#include "ActorAttacher.generated.h"

#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::AttachmentType:: e)

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  Rigid           = CARLA_ENUM_FROM_RPC(Rigid)            UMETA(DisplayName = "Rigid"),
  SpringArm       = CARLA_ENUM_FROM_RPC(SpringArm)        UMETA(DisplayName = "SpringArm"),
  SpringArmGhost  = CARLA_ENUM_FROM_RPC(SpringArmGhost)   UMETA(DisplayName = "SpringArmGhost"),

  SIZE      UMETA(Hidden),
  INVALID   UMETA(Hidden)
};

static_assert(
    static_cast<uint8>(EAttachmentType::SIZE) == static_cast<uint8>(carla::rpc::AttachmentType::SIZE),
    "Please keep these two enums in sync.");

#undef CARLA_ENUM_FROM_RPC

UCLASS()
class CARLA_API UActorAttacher : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintCallable, Category="CARLA|Actor Attacher")
  static void AttachActors(AActor *Child, AActor *Parent, EAttachmentType AttachmentType);
};
