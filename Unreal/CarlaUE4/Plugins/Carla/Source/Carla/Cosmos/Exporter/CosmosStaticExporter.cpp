// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#include "Carla.h"
#include "Carla/Cosmos/Exporter/CosmosStaticExporter.h"


bool UCosmosStaticExporter::ExportCosmosStaticExporter(class UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
	OutError = FString::Printf(TEXT("Export not implemented for this class"));
	return false;
}
