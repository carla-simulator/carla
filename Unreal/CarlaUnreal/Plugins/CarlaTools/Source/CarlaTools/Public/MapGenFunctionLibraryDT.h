// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine headers
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
// Carla C++ headers

// Carla plugin headers
//#include "Actor/ProceduralCustomMesh.h"

#include "MapGenFunctionLibraryDT.generated.h"

UCLASS(BlueprintType)
class CARLATOOLS_API UMapGenFunctionLibraryDT : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:

  UFUNCTION(BlueprintCallable)
  static UInstancedStaticMeshComponent* AddInstancedStaticMeshComponentToActor(AActor* TargetActor);

  UFUNCTION(BlueprintCallable)
  static UStaticMeshComponent* AddStaticMeshComponentToActor(AActor* TargetActor);

  UFUNCTION(BlueprintCallable)
  static USceneComponent* AddSceneComponentToActor(AActor* TargetActor);

  UFUNCTION(BlueprintCallable)
  static UStaticMeshComponent* SpawnMeshInsidePolygonWithRotation(
	  AActor* TargetActor,
	  UStaticMesh* Mesh,
	  const TArray<FVector2D>& Polygon,
	  float BuildingHeight
  );

private:
	UFUNCTION()
	static bool IsPointInPolygon(const FVector2D& Point, const TArray<FVector2D>& Polygon);
	UFUNCTION()
	static bool DoLinesIntersect(const FVector2D& A1, const FVector2D& A2, const FVector2D& B1, const FVector2D& B2);
	UFUNCTION()
	static bool GetRandomPointInPolygon(const TArray<FVector2D>& Polygon, FVector2D& OutPoint);
	UFUNCTION()
	static FVector2D RotatePoint(const FVector2D& Point, float AngleDeg);
	UFUNCTION()
	static bool DoesMeshFitWithRotation(UStaticMesh* Mesh, const FVector& SpawnLocation, const TArray<FVector2D>& Polygon, float& OutRotation);

};
