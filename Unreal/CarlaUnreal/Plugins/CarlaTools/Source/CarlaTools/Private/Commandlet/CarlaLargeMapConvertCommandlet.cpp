// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Commandlet/CarlaLargeMapConvertCommandlet.h"

#include "Carla/MapGen/LargeMapManager.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/LevelStreaming.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "Engine/WorldComposition.h"
#include "EngineUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

DEFINE_LOG_CATEGORY_STATIC(LogCarlaLargeMapConvert, Log, All);

void UCarlaLargeMapConvertCommandlet::OnWorldLoaded(UWorld* World)
{
  // Registers the WorldComposition tiles as the world's streaming levels and
  // applies any FWorldTileInfo positions (zero for CARLA maps).
  Super::OnWorldLoaded(World);

  // Drop the legacy sun/sky rig instead of converting it. Converted actors are
  // spatially loaded, so these would stream in with their cell as static
  // orphan lights that fight the weather-driven BP_Carla_Sky rig the game
  // mode spawns on World Partition maps (the sun would be stuck at whatever
  // angle the legacy map was saved with).
  for (TActorIterator<AActor> It(World); It; ++It)
  {
    AActor* Actor = *It;
    if (Actor->IsA<ADirectionalLight>() ||
        Actor->IsA<ASkyLight>() ||
        Actor->IsA<AExponentialHeightFog>())
    {
      UE_LOG(LogCarlaLargeMapConvert, Display, TEXT(
          "Dropping legacy lighting actor %s (%s); the runtime sky rig replaces it."),
          *Actor->GetName(), *Actor->GetClass()->GetName());
      World->DestroyActor(Actor);
    }
    // The legacy tile manager destroys itself at BeginPlay on World Partition
    // worlds, but its saved MapTiles property still holds object references
    // to the ULevelStreamingDynamic tiles this conversion deletes -- leaving
    // it in the map produces a failed-import load error per tile on every
    // subsequent load (and forces commandlets into exit code 1).
    else if (Actor->IsA<ALargeMapManager>())
    {
      UE_LOG(LogCarlaLargeMapConvert, Display, TEXT(
          "Dropping legacy ALargeMapManager %s; World Partition streams natively."),
          *Actor->GetName());
      World->DestroyActor(Actor);
    }
  }

  UWorldComposition* WorldComposition = World->WorldComposition;
  if (!WorldComposition)
  {
    return;
  }

  // TilesInfo.txt sits next to the map package:
  // "<first tile center X (m)>,<first tile center Y (m)>,<tile size (m)>"
  const FString MapPackage = World->GetPackage()->GetName();
  const FString MapDir = FPaths::GetPath(FPackageName::LongPackageNameToFilename(MapPackage));
  const FString TilesInfoPath = MapDir / TEXT("TilesInfo.txt");

  FString TilesInfoContent;
  if (!FFileHelper::LoadFileToString(TilesInfoContent, *TilesInfoPath))
  {
    UE_LOG(LogCarlaLargeMapConvert, Warning, TEXT(
        "No TilesInfo.txt at %s; tiles keep the transforms the stock commandlet assigned."),
        *TilesInfoPath);
    return;
  }

  TArray<FString> Values;
  TilesInfoContent.ParseIntoArray(Values, TEXT(","), true);
  if (Values.Num() < 3)
  {
    UE_LOG(LogCarlaLargeMapConvert, Error, TEXT(
        "Malformed TilesInfo.txt (%s): expected 'centerX,centerY,size' in meters."),
        *TilesInfoContent);
    return;
  }

  constexpr double MetersToCm = 100.0;
  const double FirstTileCenterX = FCString::Atod(*Values[0]) * MetersToCm;
  const double FirstTileCenterY = FCString::Atod(*Values[1]) * MetersToCm;
  const double TileSide = FCString::Atod(*Values[2]) * MetersToCm;

  UWorldComposition::FTilesList& Tiles = WorldComposition->GetTilesList();
  for (int32 TileIdx = 0; TileIdx < Tiles.Num(); ++TileIdx)
  {
    FWorldCompositionTile& Tile = Tiles[TileIdx];
    ULevelStreaming* StreamingLevel = WorldComposition->TilesStreaming[TileIdx];
    if (!StreamingLevel)
    {
      continue;
    }

    // Parse "<Map>_Tile_<X>_<Y>" (same convention as ALargeMapManager).
    const FString TileName = FPackageName::GetShortName(Tile.PackageName);
    TArray<FString> Parts;
    TileName.ParseIntoArray(Parts, TEXT("_"), false);
    if (Parts.Num() < 3 || !Parts[Parts.Num() - 3].Equals(TEXT("Tile")))
    {
      UE_LOG(LogCarlaLargeMapConvert, Warning, TEXT(
          "Tile %s does not follow the <Map>_Tile_<X>_<Y> convention; skipped."),
          *TileName);
      continue;
    }
    const int32 TileX = FCString::Atoi(*Parts[Parts.Num() - 2]);
    const int32 TileY = FCString::Atoi(*Parts[Parts.Num() - 1]);

    // ALargeMapManager::GetTileLocation: (X, -Y, 0) * TileSide + Tile0Offset,
    // where the cooked Tile0Offset equals the TilesInfo.txt first tile center.
    const FVector TileLocation(
        FirstTileCenterX + TileSide * TileX,
        FirstTileCenterY - TileSide * TileY,
        0.0);

    StreamingLevel->LevelTransform = FTransform(TileLocation);
    UE_LOG(LogCarlaLargeMapConvert, Display, TEXT(
        "Tile %s -> grid (%d, %d) -> world location %s"),
        *TileName, TileX, TileY, *TileLocation.ToCompactString());
  }
}
