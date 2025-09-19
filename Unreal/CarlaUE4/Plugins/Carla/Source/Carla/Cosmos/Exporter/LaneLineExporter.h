// LaneLineExporter.h
#pragma once

#include "CoreMinimal.h"
#include "CosmosStaticExporter.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include <carla/road/Map.h>
#include "LaneLineExporter.generated.h"

class ARoadSpline;

UCLASS()
class CARLA_API ULaneLineExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()

public:

  UFUNCTION(BlueprintCallable, Category = "Cosmos Static Exporter")
  static bool ExportCosmosLaneLines(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

private:

  static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);
  static FString MakeStableLabelId(const FString& Uuid, const TArray<FVector>& Vertices);
  static void AppendLaneLineLabel(
      TArray<TSharedPtr<FJsonValue>>& LabelsArr,
      const FString& Uuid,
      const FString& StartTs,
      const TArray<FVector>& PolylineMeters,
      const FString& SensorName,
      const FString& LabelClassNamespace,
      const FString& LabelClassIdentifier,
      const FString& LabelClassVersion,
      const FString& CoordinateFrame,
      const FString& LabelNameEnum,
      const FString& OptionalAssetRefUri,
      const FString& OptionalFeatureId,
      const FString& OptionalFeatureVer,
      const FString& OptionalClipVerId);

  static TArray<ARoadSpline*> GetLaneLineSplines(UWorld* World);
};