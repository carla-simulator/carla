// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "InstancedStaticMeshActor.generated.h"


class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class CARLA_API AInstancedStaticMeshActor : public AActor
{
  GENERATED_BODY()

public:
  AInstancedStaticMeshActor(const FObjectInitializer &ObjectInitializer);

  UFUNCTION(BlueprintPure, Category="Getter")
  UHierarchicalInstancedStaticMeshComponent* GetInstancedStaticMeshComponent(){
    return InstancedStaticMeshComponent;
  }

protected:
  UPROPERTY(VisibleAnywhere, Category="Components")
  UHierarchicalInstancedStaticMeshComponent* InstancedStaticMeshComponent;
};
