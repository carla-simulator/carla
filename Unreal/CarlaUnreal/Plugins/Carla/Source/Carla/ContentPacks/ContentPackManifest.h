// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

/// One map declared by a content pack manifest.
struct CARLA_API FCarlaContentPackMap
{
  /// Map name as exposed by get_available_maps / load_world.
  FString Name;

  /// Long package name of the level, e.g. "/NewPack/Maps/NewTown".
  FString Package;

  /// OpenDRIVE file relative to the pack Content dir, e.g. "Maps/OpenDrive/NewTown.xodr".
  FString Xodr;

  bool bWorldPartition = false;
};

/// Parsed carla-pack.json. Authoritative for map discovery: nothing walks the
/// pack directories looking for .umap files.
///
///   {
///     "name": "NewPack", "version": "1.0.0",
///     "carla_version": "0.10.2", "base_release": "carla-0.10.2-Linux",
///     "engine": { "version": "5.8.0", "commit": "abcdef0" }, "platform": "Linux",
///     "maps": [ { "name", "package", "xodr", "world_partition" } ],
///     "catalogs": [ "Config/Vehicles.json", ... ],
///     "files": { "<relpath>": "sha256:..." }
///   }
struct CARLA_API FCarlaContentPackManifest
{
  FString Name;
  FString Version;
  FString CarlaVersion;
  FString BaseRelease;
  FString EngineVersion;
  FString EngineCommit;
  FString Platform;
  TArray<FCarlaContentPackMap> Maps;
  TArray<FString> Catalogs;
  TMap<FString, FString> Files;

  /// Absolute path of the manifest file this was parsed from.
  FString ManifestPath;

  /// Absolute path of the pack directory (parent of the manifest).
  FString PackDir;

  /// Parse a manifest from a JSON string. OutError explains a false return.
  static bool Parse(
      const FString &JsonText,
      FCarlaContentPackManifest &OutManifest,
      FString &OutError);

  /// Read and parse <ManifestPath>. Fills ManifestPath/PackDir.
  static bool LoadFromFile(
      const FString &InManifestPath,
      FCarlaContentPackManifest &OutManifest,
      FString &OutError);

  /// Name must be usable as a plugin name and mount point ("/<Name>/").
  static bool IsValidPackName(const FString &InName);

  TArray<FString> GetMapNames() const;
};
