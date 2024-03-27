// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Commandlets/Commandlet.h"

#include <compiler/disable-ue4-macros.h>

#include <compiler/enable-ue4-macros.h>

#include "OpenDriveToMap.h"

#include "SetProperPositionForWorldPartitionCommandlet.generated.h"

// Each commandlet should generate only 1 Tile

DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Log, All);


UCLASS()
class CARLATOOLS_API USetProperPositionForWorldPartitionCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /// Default constructor.
  USetProperPositionForWorldPartitionCommandlet();
  USetProperPositionForWorldPartitionCommandlet(const FObjectInitializer &);

#if WITH_EDITORONLY_DATA

  virtual int32 Main(const FString &Params) override;

  void ProcessTile(const FIntVector CurrentTilesInXY, const float TileSize);

#endif // WITH_EDITORONLY_DATA
};
