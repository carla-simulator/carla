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

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void GenerateImpostor();

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void CookProceduralBuildingToMesh(const FString& DestinationPath, const FString& FileName);

private:
  void RenderImpostorView(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View);
  void MoveCameraToViewPosition(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View);

};
