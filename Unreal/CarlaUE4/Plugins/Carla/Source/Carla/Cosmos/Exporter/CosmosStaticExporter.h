// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// UCosmosStaticExporter.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CosmosStaticExporter.generated.h"

/**
 * Base class for per-object-type JSON exporters (e.g., crosswalks, waitlines).
 * Implement Export() to gather data and write the JSON payload to OutFilePath.
 */
UCLASS()
class CARLA_API UCosmosStaticExporter : public UObject
{
	GENERATED_BODY()
public:

	static FString GenerateSesionId(float TimeElapsed)
	{
		// Generate a GUID with hyphens
		FString Uuid = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

		// Example: current UTC time in seconds since epoch
		FDateTime Now = FDateTime::UtcNow();
		int64 UnixTime = Now.ToUnixTimestamp();           // seconds
		int64 StartTs = UnixTime * 1000000;               // microseconds
		int64 EndTs = StartTs + static_cast<int64>(TimeElapsed * 1000000); // microseconds

		// Build the session ID
		return FString::Printf(TEXT("%s_%lld_%lld"),
			*Uuid,
			StartTs,
			EndTs);
	}

	/**
	 * Perform the export for this object type.
	 * @param World        The world/context to query (may be null if your data source is external).
	 * @param SessionId    The full session ID string, e.g. "uuid_start_end".
	 * @param OutFilePath  Absolute file path to write (the coordinator pre-creates dirs).
	 * @param OutError     On failure, set a human-readable reason.
	 * @return             true on success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cosmos Static Exporter")
    static bool ExportCosmosStaticExporter(class UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError);

	static bool ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
	{
		// Expect: uuid_start_end (all as strings)
		int32 A = INDEX_NONE, B = INDEX_NONE;
		if (!In.FindChar(TEXT('_'), A)) return false;
		if (!In.FindLastChar(TEXT('_'), B)) return false;
		if (A <= 0 || B <= A+1 || B >= In.Len()-1) return false;
		OutUuid  = In.Left(A);
		OutStart = In.Mid(A+1, B-(A+1));
		OutEnd   = In.Mid(B+1);
		return true;
	}
};
