
// CrosswalksExporter.h
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
	// --- Helpers (no default args here to avoid UE4 default-arg issues) ---
	static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd);
	static FString MakeStableLabelId(const FString& Uuid, const FVector& P0Meters, const FVector& P1Meters);
	static void BuildPolePolylineMeters(const UStaticMeshComponent* Comp, FVector& OutTopM, FVector& OutBaseM);
	static FString ClassifyPoleType(const UStaticMeshComponent* Comp);
	static void AppendPoleLabelJson(
		TArray<TSharedPtr<FJsonValue>>& Labels,
		const FString& Uuid,
		const FString& StartTs,
		const FVector& TopM,
		const FVector& BaseM,
		const FString& PoleTypeText);


  // --- Traffic Lights helpers (new) ---
	/** Compute 8 world-space corners (meters) of the component’s oriented bounding box. */
	static void BuildComponentCuboidMeters(const UStaticMeshComponent* Comp, TArray<FVector>& Out8VertsMeters);

	static void AppendTrafficSignLabelJson(
		TArray<TSharedPtr<FJsonValue>>& Labels,
		const FString& Uuid,
		const FString& StartTs,
		const TArray<FVector>& V8Meters,    // 8 vertices in meters
		const FString& LightClassText       // e.g., "RED","GREEN" or "TRAFFIC_LIGHT"
	);

};
