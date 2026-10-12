// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Commandlets/WorldPartitionConvertCommandlet.h"
#include <util/ue-header-guard-end.h>

#include "CarlaLargeMapConvertCommandlet.generated.h"

/// World Partition conversion for CARLA's legacy tiled large maps
/// (Town11/12/13). These maps register their tiles in WorldComposition but
/// never author FWorldTileInfo positions: at runtime ALargeMapManager parses
/// the tile grid coordinates out of the "<Map>_Tile_<X>_<Y>" package name and
/// places each tile at FirstTileCenter + TileSize * (X, -Y), with both values
/// coming from the map's TilesInfo.txt. The stock commandlet therefore merges
/// every tile at the origin. This subclass reproduces the CARLA placement
/// convention by baking that transform into each tile's streaming level before
/// the tiles are loaded for conversion.
///
/// Usage (same arguments as the stock commandlet):
///   UnrealEditor <uproject> -run=CarlaLargeMapConvert \
///       /Game/Carla/Maps/Town12/Town12 -ConversionSuffix \
///       -AllowCommandletRendering -unattended -nosound
UCLASS(Config = Engine)
class CARLATOOLS_API UCarlaLargeMapConvertCommandlet
  : public UWorldPartitionConvertCommandlet
{
  GENERATED_BODY()

public:

  virtual void OnWorldLoaded(UWorld* World) override;
};
