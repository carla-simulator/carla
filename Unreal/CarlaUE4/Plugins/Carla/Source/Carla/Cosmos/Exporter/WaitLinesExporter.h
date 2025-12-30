// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// WaitLinesExporter.h

#pragma once

#include "CoreMinimal.h"
#include "CosmosStaticExporter.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "WaitLinesExporter.generated.h"

UCLASS()
class CARLA_API UWaitLinesExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()

public:

  static bool ExportCosmosWaitLines(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

private:

  static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);
  static void AppendWaitLineLabel(
      TArray<TSharedPtr<FJsonValue>>& LabelsArr,
      const FString& StartTs,
      const FVector& StartPoint,
      const FVector& EndPoint);
};

