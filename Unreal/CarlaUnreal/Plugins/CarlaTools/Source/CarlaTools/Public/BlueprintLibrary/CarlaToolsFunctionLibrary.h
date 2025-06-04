// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once


#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
#include <util/ue-header-guard-end.h>

#include "CarlaToolsFunctionLibrary.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsFunctionLibrary, Log, All);

UCLASS(BlueprintType)
class CARLATOOLS_API UCarlaToolsFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

  struct FTriangle
  {
    FVector3f V0, V1, V2;
  };

public:
  UFUNCTION(BlueprintCallable)
  static void ChunkAndSubdivideStaticMesh(UStaticMesh* SourceMesh, FString MeshName, int ChunkSize = 100, FString OutputFolder = TEXT("/Game/Result") );

  static void SubdivideTriangles(const TArray<UCarlaToolsFunctionLibrary::FTriangle>& InTriangles, int Factor, TArray<UCarlaToolsFunctionLibrary::FTriangle>& OutTriangles);

  UFUNCTION(BlueprintCallable)
  static void SplitStaticMeshByMaterial(UStaticMesh* SourceMesh, FString BaseName, FString OutputFolder, int SubdivisionFactor);

};
