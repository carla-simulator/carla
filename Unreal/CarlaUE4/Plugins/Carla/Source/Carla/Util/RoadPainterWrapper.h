// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "RoadPainterWrapper.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FDecalsProperties
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  TArray<UMaterialInstance*> DecalMaterials;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  TArray<int32> DecalNumToSpawn;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  FVector DecalScale;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  FVector FixedDecalOffset;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalMaxScale;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalMinScale;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalRandomYaw;

  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float RandomOffset;
};

UCLASS()
class CARLA_API ARoadPainterWrapper : public AActor
{
  GENERATED_BODY()

public:

  ARoadPainterWrapper();

  void BeginPlay() override;

  /// Event used for setting the size of the map via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ZSizeEvent();

  /// Event used for painting by actor points
  /// generated on the road (created during the import process) via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintByActorEvent();

  /// Event used for painting as a whole square via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintOverSquareEvent();

  /// Event used for painting as a whole circle via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintOverCircleEvent();

  /// Event used for painting the road map to the selected texture via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void PaintAllRoadsEvent();

  /// Event used for spawning meshes by actor points
  /// generated on the road (created during the import process) via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void SpawnMeshesByActorEvent();

  /// Event used for spawning meshes via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void SpawnMeshesEvent();

  /// Event used for spawning decals via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void SpawnDecalsEvent();

  /// Event for clearing the materials on the road via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ClearMaterialEvent();

  /// Event for clearing the materials close to the actor points
  /// generated on the road (created during the import process) via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ClearMaterialByActorEvent();

  /// Event for clearing all the materials on the road via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
	void ClearAllEvent();

  /// Event for setting the necessary variables in blueprint in order to paint the roads 
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
  void SetBlueprintVariables();

  UFUNCTION(Category = "RoadPainter Wrapper")
  void ReadConfigFile();

  /// Generate the assets necessary for painting the roads.
  /// This function MUST only be called right after the construction script
  /// in blueprint has finished.
  UFUNCTION(BlueprintCallable, Category = "RoadPainter Wrapper")
  void GenerateDynamicAssets();

  /// The size of the whole map
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RoadPainter Wrapper")
  float MapSize;

  /// Texture used for painting the whole road map into it.
  /// This is later used for spawning the different decals and meshes.
  UPROPERTY(BlueprintReadOnly, Category = "RoadPainter Wrapper")
  UTextureRenderTarget2D *RoadTexture; 

  UPROPERTY(BlueprintReadOnly, Category = "RoadPainter Wrapper")
  FDecalsProperties DecalPropertiesConfig;

private:

  /// Create a procedural texture for painting the road maps
  /// and save it to disk.
  UFUNCTION(Category = "RoadPainter Wrapper")
  void GenerateTexture();

  FVector ReadVectorFromJsonObject(TSharedPtr<FJsonObject> JsonObject);

  UPROPERTY()
  TMap<FString, UMaterialInstance*> DecalNamesMap;

  /// Master material of the roads (necessary for creating material instances)
  UPROPERTY()
  UMaterial *RoadNodeMasterMaterial;

  /// Material instance that copies 
  UPROPERTY()
  UMaterialInstanceConstant *RoadNodePresetMaterial;

  /// Variable to know if the map has already being rendered
  /// to our generated texture
  UPROPERTY()
  bool bIsRenderedToTexture;
};
