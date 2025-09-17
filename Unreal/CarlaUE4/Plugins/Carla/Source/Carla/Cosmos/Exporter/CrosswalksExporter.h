
// CrosswalksExporter.h
#pragma once

#include "CoreMinimal.h"
#include "CosmosStaticExporter.h"
#include "CrosswalksExporter.generated.h"

UCLASS()
class CARLA_API UCrosswalksExporter : public UCosmosStaticExporter
{
	GENERATED_BODY()
public:
	virtual FString GetObjectTypeName() const override { return TEXT("crosswalks"); }
	virtual bool Export(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError) override;
};
