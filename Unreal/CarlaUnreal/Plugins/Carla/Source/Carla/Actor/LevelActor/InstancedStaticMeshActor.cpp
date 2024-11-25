// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Actor/LevelActor/InstancedStaticMeshActor.h"

#include <util/ue-header-guard-begin.h>
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include <util/ue-header-guard-end.h>

AInstancedStaticMeshActor::AInstancedStaticMeshActor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  InstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>("InstancedStaticMeshComponent");
  
}

