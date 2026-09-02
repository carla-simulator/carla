// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine headers
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
// Carla C++ headers

// Carla plugin headers

#include "DynamicMeshGeneration.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogCarlaDynamicMeshGeneration, Log, All);

UCLASS(BlueprintType)
class STREETMAPRUNTIME_API UDynamicMeshGeneration : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable)
  static UStaticMesh* CreateMeshFromPoints(
      TArray<FVector> Points3D,
      FName MeshName,
      const FString& AssetPath,
      bool bFlipped = true,
      FVector Offset = FVector::ZeroVector);

};
