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
