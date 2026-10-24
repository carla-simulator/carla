// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <carla/rpc/CustomMeshDescription.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <carla/rpc/String.h>
#include <util/enable-ue4-macros.h>

class ACarlaGameModeBase;

#pragma pack(push, 1)
struct FCMeshHeader
{
  uint32 Magic;        // 0x53454D43 ('CMES' in little-endian)
  uint32 Version;      // 1
  uint32 VertexCount;
  uint32 IndexCount;
  uint32 Flags;        // Bit 0: HasNormals, Bit 1: HasUVs, Bit 2: HasColors
};
#pragma pack(pop)

constexpr uint32 CMESH_MAGIC = 0x53454D43; // 'C' | ('M' << 8) | ('E' << 16) | ('S' << 24)
constexpr uint32 CMESH_VERSION_1 = 1;
constexpr uint32 CMESH_FLAG_HAS_NORMALS = 1 << 0;
constexpr uint32 CMESH_FLAG_HAS_UVS     = 1 << 1;
constexpr uint32 CMESH_FLAG_HAS_COLORS  = 1 << 2;

struct CARLA_API FCarlaMapLayer
{
  FString Name;
  FString MeshFile; // Relative to bundle root, e.g. "Meshes/roads.cmesh"
  carla::rpc::CityObjectLabel SemanticLabel = carla::rpc::CityObjectLabel::None;
  FString MaterialPreset;
  bool bEnableCollision = true;
  FTransform Transform = FTransform::Identity;
  carla::rpc::CustomMaterialParameters MaterialParams;
};

struct CARLA_API FCarlaMapPlacement
{
  FString AssetPath;
  TArray<FTransform> Transforms;
};

struct CARLA_API FCarlaMapBundle
{
  FString MapName;
  FString BundleDir;
  FString XodrFile;
  FString NavFile;
  carla::rpc::OpendriveGenerationParameters GenerationParams;
  TArray<FCarlaMapLayer> Layers;
  TArray<FCarlaMapPlacement> Placements;
  TArray<FTransform> CustomSpawnPoints;

  bool IsValid() const { return !MapName.IsEmpty() && !BundleDir.IsEmpty(); }

  /// Discovers all available .carlamap bundles in project Maps/, Saved/Maps/,
  /// -carla-maps= command line and CARLA_MAPS_PATH environment variable.
  /// Returns a map of MapName -> BundleDirectory.
  static TMap<FString, FString> FindAllBundles();

  /// Loads a bundle definition from carla-map.json in the specified directory.
  static bool LoadBundleFromDir(const FString &Dir, FCarlaMapBundle &OutBundle, FString &OutError);

  /// Reads a binary .cmesh file from disk.
  static bool LoadCMesh(
      const FString &FilePath,
      TArray<FVector> &OutVertices,
      TArray<int32> &OutTriangles,
      TArray<FVector> &OutNormals,
      TArray<FVector2D> &OutUV0,
      TArray<FLinearColor> &OutColors,
      FString &OutError);

  /// Reads a binary .cmesh file from raw bytes in memory.
  static bool LoadCMeshFromMemory(
      const uint8 *Data,
      int64 DataLen,
      TArray<FVector> &OutVertices,
      TArray<int32> &OutTriangles,
      TArray<FVector> &OutNormals,
      TArray<FVector2D> &OutUV0,
      TArray<FLinearColor> &OutColors,
      FString &OutError);

  /// Resolves a material preset or custom PBR parameters to a UMaterialInterface.
  static UMaterialInterface *ResolveMaterial(
      UObject *Outer,
      const FString &Preset,
      const carla::rpc::CustomMaterialParameters &Params);

  /// Converts a string name (e.g. "Roads", "Sidewalks", "Buildings") to CityObjectLabel.
  static carla::rpc::CityObjectLabel ParseSemanticLabel(const FString &LabelStr);

  /// Stages bundle OpenDRIVE and pre-compiled NavMesh files into Saved/OpenDrive and Saved/Nav.
  bool StageBundle(FString &OutError) const;

  /// Instantiates all .cmesh layers, HISM placements, and spawn points in the loaded world.
  int32 InstantiateInWorld(UWorld *World, ACarlaGameModeBase *GameMode, FString &OutError) const;
};
