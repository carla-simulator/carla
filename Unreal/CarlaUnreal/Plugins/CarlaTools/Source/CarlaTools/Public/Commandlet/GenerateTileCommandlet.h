// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "OpenDriveToMap.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/ObjectLibrary.h"
#include "Commandlets/Commandlet.h"
#include <util/ue-header-guard-end.h>

#include "GenerateTileCommandlet.generated.h"

// Each commandlet should generate only 1 Tile

DECLARE_LOG_CATEGORY_EXTERN(LogCarlaToolsMapGenerateTileCommandlet, Log, All);


UCLASS()
class CARLATOOLS_API UGenerateTileCommandlet
  : public UCommandlet
{
  GENERATED_BODY()

public:

  /// Default constructor.
  UGenerateTileCommandlet();
  UGenerateTileCommandlet(const FObjectInitializer &);

#if WITH_EDITORONLY_DATA

  virtual int32 Main(const FString &Params) override;

#endif // WITH_EDITORONLY_DATA
  UPROPERTY()
  UOpenDriveToMap* OpenDriveMap;
  UPROPERTY()
  UClass* OpenDriveClass;
};
