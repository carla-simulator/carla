// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Dom/JsonObject.h"
#include <util/ue-header-guard-end.h>
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
  FVector DecalScale = FVector::ZeroVector;

  /// Min offset from one decal to another
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  FVector FixedDecalOffset = FVector::ZeroVector;

  /// Maximum scale to be applied to the decals
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalMaxScale = 1.0f;

  /// Min scale to be applied to the decals
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalMinScale = 1.0f;

  /// The decal yaw to be applied randomly
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float DecalRandomYaw = 90.0f;

  /// Random offset from one decal to another
  UPROPERTY(BlueprintReadOnly, Category = "Decals Properties")
  float RandomOffset = 3.0f;
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
  void ReadConfigFile(const FString &CurrentMapName, const TMap<FString, FString> &DecalNamesMap);

  /// Variable used for storing the JSON values of the decals
  /// so it can be later used by the blueprint (Road Painter Preset)
  UPROPERTY(BlueprintReadOnly, Category = "RoadPainter Wrapper")
  FDecalsProperties DecalPropertiesConfig;

private:

  /// Function to read 3D vectors from a JSON file
  FVector ReadVectorFromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};
