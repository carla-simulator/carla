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
#include "Carla/Util/ProceduralCustomMesh.h"

#include "MapGenFunctionLibrary.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogCarlaMapGenFunctionLibrary, Log, All);

UCLASS(BlueprintType)
class CARLA_API UMapGenFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable)
  static UStaticMesh* CreateMesh(
      const FProceduralCustomMesh& Data,
      const TArray<FProcMeshTangent>& ParamTangents,
      UMaterialInstance* MaterialInstance,
      FString MapName,
      FString FolderName,
      FName MeshName);

  static FMeshDescription BuildMeshDescriptionFromData(
      const FProceduralCustomMesh& Data,
      const TArray<FProcMeshTangent>& ParamTangents,
      UMaterialInstance* MaterialInstance );

  UFUNCTION(BlueprintCallable)
  static FVector2D GetTransversemercProjection(float lat, float lon, float lat0, float lon0);

  UFUNCTION(BlueprintCallable)
  static void SetThreadToSleep(float seconds);

  UFUNCTION(BlueprintCallable)
  static void FlushRenderingCommandsInBlueprint();

  UFUNCTION(BlueprintCallable)
  static void CleanupGEngine();
};
