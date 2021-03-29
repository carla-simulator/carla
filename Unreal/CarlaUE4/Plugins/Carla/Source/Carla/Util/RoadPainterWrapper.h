// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "RoadPainterWrapper.generated.h"

UCLASS()
class CARLA_API ARoadPainterWrapper : public AActor
{
  GENERATED_BODY()

public:

  ARoadPainterWrapper();

  void BeginPlay() override;

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ZSizeEvent();

  //Paint events
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintByActorEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintOverSquareEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintOverCircleEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintAllRoadsEvent();

  //Spawn events
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void SpawnMeshesByActorEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void SpawnMeshesEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void SpawnDecalsEvent();

  //Clear events
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ClearMaterialEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ClearMaterialByActorEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ClearAllEvent();

  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
  void SetBlueprintVariables();

  UFUNCTION(BlueprintCallable, Category = "RoadPainter Wrapper")
  void GenerateDynamicAssets();

  UFUNCTION(Category = "RoadPainter Wrapper")
  void ModifyRoadMaterialParameters();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RoadPainter Wrapper")
  UMaterialInstanceDynamic *DynamicRoadMaterial;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RoadPainter Wrapper")
  UTextureRenderTarget2D *RoadTexture;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RoadPainter Wrapper")
  float MapSize;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RoadPainter Wrapper")
  bool bIsRenderedToTexture;

private:

  UFUNCTION(Category = "RoadPainter Wrapper")
  void GenerateTexture();

  UPROPERTY()
  UMaterial *RoadNodeMasterMaterial;

  UPROPERTY()
  UMaterialInstanceConstant *RoadNodePresetMaterial;
};
