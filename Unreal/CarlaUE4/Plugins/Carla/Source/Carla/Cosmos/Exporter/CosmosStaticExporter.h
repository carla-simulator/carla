// MapJsonExporter.h
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

	virtual FString GetObjectTypeName() const
    {
        return TEXT("object");
    }

	/**
	 * Perform the export for this object type.
	 * @param World        The world/context to query (may be null if your data source is external).
	 * @param SessionId    The full session ID string, e.g. "uuid_start_end".
	 * @param OutFilePath  Absolute file path to write (the coordinator pre-creates dirs).
	 * @param OutError     On failure, set a human-readable reason.
	 * @return             true on success.
	 */
    virtual	bool Export(class UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
    {
        OutError = FString::Printf(TEXT("Export not implemented for %s"), *GetObjectTypeName());
        return false;
    }

};
