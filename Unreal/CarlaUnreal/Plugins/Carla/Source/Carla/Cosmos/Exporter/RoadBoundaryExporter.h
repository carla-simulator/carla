// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// RoadBoundaryExporter.h
#pragma once

#include "CosmosStaticExporter.h"
// libcarla (server-side OpenDRIVE road network)

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

#include "RoadBoundaryExporter.generated.h"

UCLASS()
class CARLA_API URoadBoundaryExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()

public:

  static bool ExportCosmosRoadBoundaries(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

private:

  static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);
  static void AppendRoadBoundaryLabel(
      TArray<TSharedPtr<FJsonValue>>& LabelsArr,
      const FString& StartTs,
      const TArray<FVector>& PolyMeters);
};
