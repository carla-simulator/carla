// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// CrosswalksExporter.h
#pragma once

#include "CoreMinimal.h"
#include "CosmosStaticExporter.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
// libcarla (server-side OpenDRIVE)

#include "CrosswalksExporter.generated.h"

UCLASS()
class CARLA_API UCrosswalksExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()

public:
  static bool ExportCosmosCrosswalk(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

private:

  static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);
  static FString MakeStableLabelId(const FString& Uuid, const TArray<FVector>& Vertices);
  static void AppendCrosswalkLabel(
      TArray<TSharedPtr<FJsonValue>>& LabelsArr,
      const FString& Uuid,
      const FString& StartTs,
      const TArray<FVector>& PolyMeters,
      const FString& SensorName,
      const FString& LabelClassNamespace,
      const FString& LabelClassIdentifier,
      const FString& LabelClassVersion,
      const FString& CoordinateFrame,
      const FString& LabelNameEnum,
      const FString& TypeText,
      bool bIsImplicit,
      const FString& OptionalAssetRefUri,
      const FString& OptionalFeatureId,
      const FString& OptionalFeatureVer,
      const FString& OptionalClipVerId);


};
