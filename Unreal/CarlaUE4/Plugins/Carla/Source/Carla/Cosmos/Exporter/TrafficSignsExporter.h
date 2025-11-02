// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// TrafficSignsExporter.h
#pragma once

#include "CoreMinimal.h"
#include "CosmosStaticExporter.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "TrafficSignsExporter.generated.h"

UCLASS()
class CARLA_API UTrafficSignsExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()

public:
  static bool ExportCosmosTrafficSigns(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

private:
	static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);

	// Simplified export functions for each type
	static void AppendPoleLabel(
		TArray<TSharedPtr<FJsonValue>>& Labels,
		const FString& StartTs,
		const FVector& TopM,
		const FVector& BaseM);

	static void AppendTrafficSignLabel(
		TArray<TSharedPtr<FJsonValue>>& Labels,
		const FString& StartTs,
		const TArray<FVector>& V8Meters);

	static void AppendTrafficLightLabel(
		TArray<TSharedPtr<FJsonValue>>& Labels,
		const FString& StartTs,
		const TArray<FVector>& V8Meters);

	// Helper functions
	static void BuildPolePolylineMeters(const UStaticMeshComponent* Comp, FVector& OutTopM, FVector& OutBaseM);
	static void BuildComponentCuboidMeters(const UStaticMeshComponent* Comp, TArray<FVector>& Out8VertsMeters);
};
