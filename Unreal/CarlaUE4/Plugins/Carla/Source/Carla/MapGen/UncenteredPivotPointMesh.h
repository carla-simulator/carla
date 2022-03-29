// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UncenteredPivotPointMesh.generated.h"


UCLASS()
class CARLA_API AUncenteredPivotPointMesh : public AActor
{
  GENERATED_BODY()

public:
  AUncenteredPivotPointMesh(const FObjectInitializer& ObjectInitializer);

  UStaticMeshComponent* GetMeshComp()
  {
    return SMComp;
  }

private:
  UPROPERTY(EditAnywhere, Category="Uncenceterd PP Mesh")
  UStaticMeshComponent* SMComp = nullptr;

};