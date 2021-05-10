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

  /// The decals used on the road
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  TArray<UMaterialInstance*> DecalMaterials;

  /// How many decals (or material instances) of each, should be applied to the road
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  TArray<int32> DecalNumToSpawn;

  /// Scale of each decal on the road
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  FVector DecalScale;

  /// Min offset from one decal to another
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  FVector FixedDecalOffset;

  /// Maximum scale to be applied to the decals
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalMaxScale;

  /// Min scale to be applied to the decals
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalMinScale;

  /// The decal yaw to be applied randomly
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalRandomYaw;

  /// Random offset from one decal to another
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

  /// Event used for spawning meshes via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
  void SpawnMeshesEvent();

  /// Event used for spawning decals via blueprint
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
  void SpawnDecalsEvent();

  /// Event for setting the necessary variables in blueprint in order to paint the roads 
  UFUNCTION(BlueprintImplementableEvent, Category = "RoadPainter Wrapper")
  void SetBlueprintVariables();

  /// Function for reading the decals configuration file (in JSON format)
  UFUNCTION(Category = "RoadPainter Wrapper")
  void ReadConfigFile(const FString &CurrentMapName);

  /// Generate the assets necessary for painting the roads.
  /// This function MUST only be called right after the construction script (in blueprint) has finished.
  UFUNCTION(BlueprintCallable, Category = "RoadPainter Wrapper")
  void GenerateDynamicAssets();

  UPROPERTY(BlueprintReadOnly, Category = "RoadPainter Wrapper")
  UTexture2D *RoadBrushTexture;

  /// Variable used for storing the JSON values of the decals
  /// so it can be later used by the blueprint (Road Painter Preset)
  UPROPERTY(BlueprintReadOnly, Category = "RoadPainter Wrapper")
  FDecalsProperties DecalPropertiesConfig;

private:

  /// Function to read 3D vectors from a JSON file
  FVector ReadVectorFromJsonObject(TSharedPtr<FJsonObject> JsonObject);

  /// Dictionary for translating the JSON file "decal_names" array
  /// to already loaded Material Instances, which are used to apply on the roads
  UPROPERTY()
  TMap<FString, FString> DecalNamesMap;

  UPROPERTY()
  TArray<FString> RoadTextureBrushes;

  /// Master material of the roads (necessary for creating material instances)
  UPROPERTY()
  FString RoadMasterMaterialName;

  /// Material instance that copies 
  UPROPERTY()
  FString RoadInstanceMaterialName;
};
