// RoadMarkingExporter.h
#pragma once

#include "CoreMinimal.h"
#include "CosmosStaticExporter.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "RoadMarkingExporter.generated.h"

UCLASS()
class CARLA_API URoadMarkingExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()

public:

  static bool ExportCosmosRoadMarkings(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

private:

  static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);
  static void AppendRoadMarkingLabel(
      TArray<TSharedPtr<FJsonValue>>& LabelsArr,
      const FString& StartTs,
      const TArray<FVector>& SurfaceVertices);
};

