// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/ProceduralCustomMesh.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
#include <util/ue-header-guard-end.h>

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

  // This function will count instances of each static mesh in the level, if there are > MinNumOfInstancesToBeChanged they will be changed by instanced static meshes 
  // to reduce draw calls
  UFUNCTION(BlueprintCallable)
  static TArray<class AInstancedStaticMeshActor*> ChangeStaticMeshesInTheLevelForInstancedStaticMeshes(UWorld* World, TArray<UStaticMesh*> Filter, int MinNumOfInstancesToBeChanged = 20);

  // This function will removed instanced static meshes in the level and place static mesh actors instead
  UFUNCTION(BlueprintCallable)
  static TArray<AStaticMeshActor*> RevertStaticMeshesInTheLevelForInstancedStaticMeshes(UWorld* World, TArray<UStaticMesh*> Filter);
};
