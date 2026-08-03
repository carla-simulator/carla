// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once

#include "Carla/Vehicle/VehicleSpawnPoint.h"

#include <util/disable-ue4-macros.h>
#include "carla/road/Map.h"
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include <util/ue-header-guard-end.h>

#include <optional>

#include "OpenDriveGenerator.generated.h"

class UMaterialInterface;

UCLASS()
class CARLA_API AProceduralMeshActor : public AActor
{
  GENERATED_BODY()
public:
  AProceduralMeshActor();

  UPROPERTY(Category = "Procedural Mesh Actor", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TObjectPtr<UProceduralMeshComponent> MeshComponent;
};

UCLASS()
class CARLA_API AOpenDriveGenerator : public AActor
{
  GENERATED_BODY()

public:

  AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer);

  /// Set the OpenDRIVE information as string and generates the
  /// queryable map structure.
  bool LoadOpenDrive(const FString &OpenDrive);

  /// Get the OpenDRIVE information as string.
  const FString &GetOpenDrive() const;

  /// Checks if the OpenDrive has been loaded and it's valid.
  bool IsOpenDriveValid() const;

  /// Generates the road and sidewalk mesh based on the OpenDRIVE information.
  void GenerateRoadMesh();

  /// Generates the crosswalk mesh based on the OpenDRIVE information.
  void GenerateCrosswalkMesh();

  /// Generates the lane-marking meshes (white/yellow) based on the
  /// OpenDRIVE information.
  void GenerateLaneMarkings();

  /// Generates pole meshes based on the OpenDRIVE information.
  void GeneratePoles();

  /// Generates spawn points along the road.
  void GenerateSpawnPoints();

  void GenerateAll();

protected:

  virtual void BeginPlay() override;

  /// Determine the height where the spawners will be placed, relative to each
  /// RoutePlanner
  UPROPERTY(Category = "Spawners", EditAnywhere)
  float SpawnersHeight = 300.f;

  UPROPERTY(Category = "Spawners", EditAnywhere)
  TArray<TObjectPtr<AVehicleSpawnPoint>> VehicleSpawners;

  UPROPERTY(EditAnywhere)
  FString OpenDriveData;

  UPROPERTY(EditAnywhere)
  TArray<TObjectPtr<AActor>> ActorMeshList;

  /// Material applied to driving-lane mesh sections.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> RoadMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A.MI_Road_Rural_A")));

  /// Material applied to sidewalk mesh sections.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> SidewalkMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment")));

  /// Material applied to the crosswalk mesh. Content/Carla does not ship a
  /// dedicated flat "zebra crossing" ground material yet, so this defaults
  /// to a light lane-marking asphalt material as a visible placeholder.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> CrosswalkMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Asphalt_B_LaneMarkingWhite.MI_Road_Asphalt_B_LaneMarkingWhite")));

  /// Material applied to white lane-marking meshes.
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> LaneMarkingWhiteMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A_LaneMarkingWhite.MI_Road_Rural_A_LaneMarkingWhite")));

  /// Material applied to yellow lane-marking meshes (centre line).
  UPROPERTY(Category = "Materials", EditAnywhere)
  TSoftObjectPtr<UMaterialInterface> LaneMarkingYellowMaterial = TSoftObjectPtr<UMaterialInterface>(
      FSoftObjectPath(TEXT("/Game/Carla/Static/GenericMaterials/Roads/MI_Road_Rural_A_LaneMarkingYellow.MI_Road_Rural_A_LaneMarkingYellow")));

  /// Vertical offset (cm) applied to crosswalk and lane-marking meshes above
  /// the road surface, to avoid z-fighting with the road mesh section.
  UPROPERTY(Category = "Materials", EditAnywhere)
  float DecalZOffset = 1.5f;

};
