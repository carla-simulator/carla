// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "ProceduralBuildingUtilities.generated.h"

class USceneCaptureComponent2D;
class UTexture2D;

UENUM(BlueprintType)
enum EBuildingCameraView
{
  FRONT,
  LEFT,
  BACK,
  RIGHT
};

/**
 * 
 */
UCLASS()
class CARLATOOLS_API AProceduralBuildingUtilities : public AEditorUtilityActor
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintImplementableEvent, Category="Procedural Building Utilities")
  void BakeSceneCaptureRTToTextureAtlas(UTexture2D* SceneCaptureRT, EBuildingCameraView TargetView);

  UFUNCTION(BlueprintImplementableEvent, Category="Procedural Building Utilities")
  void SetTargetTextureToSceneCaptureComponent(USceneCaptureComponent2D* SceneCaptureComponent);

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void GenerateImpostorTexture(const FVector& BuildingSize);

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  class UProceduralMeshComponent* GenerateImpostorGeometry(const FVector& BuildingSize);

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void CookProceduralBuildingToMesh(const FString& DestinationPath, const FString& FileName);

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void CookProceduralMeshToMesh(class UProceduralMeshComponent* Mesh, const FString& DestinationPath, const FString& FileName);

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  class UMaterialInstanceConstant* GenerateBuildingMaterialAsset(
      const FString& DuplicateParentPath,
      const FString& DestinationPath,
      const FString& FileName);

private:
  void RenderImpostorView(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View);
  void MoveCameraToViewPosition(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View);

  void CreateBuildingImpostorGeometryForView(
      class UProceduralMeshComponent* Mesh,
      const FVector& BuildingSize,
      const EBuildingCameraView View);

  void GetWidthDepthFromView(
      const FVector& BuildingSize,
      const EBuildingCameraView View,
      float& OutWidth, float& OutDepth,
      float& OutViewAngle);

  void CalculateViewGeometryUVs(
      const float BuildingWidth,
      const float BuildingHeight,
      const EBuildingCameraView View,
      TArray<FVector2D>& OutUVs);

};
