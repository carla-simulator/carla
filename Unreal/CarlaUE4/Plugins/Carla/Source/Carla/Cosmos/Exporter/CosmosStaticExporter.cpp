#include "Carla.h"
#include "Carla/Cosmos/Exporter/CosmosStaticExporter.h"


bool UCosmosStaticExporter::ExportCosmosStaticExporter(class UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
	OutError = FString::Printf(TEXT("Export not implemented for this class"));
	return false;
}
