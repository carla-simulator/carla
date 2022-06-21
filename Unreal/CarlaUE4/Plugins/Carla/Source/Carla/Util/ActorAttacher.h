// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/AttachmentType.h>
#include <compiler/enable-ue4-macros.h>

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ActorAttacher.generated.h"

#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::AttachmentType:: e)

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  Rigid      = CARLA_ENUM_FROM_RPC(Rigid)      UMETA(DisplayName = "Rigid"),
  SpringArm  = CARLA_ENUM_FROM_RPC(SpringArm)  UMETA(DisplayName = "SpringArm"),

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
