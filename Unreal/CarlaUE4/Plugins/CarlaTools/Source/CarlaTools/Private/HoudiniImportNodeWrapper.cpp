// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "HoudiniImportNodeWrapper.h"

UHoudiniImportNodeWrapper::UHoudiniImportNodeWrapper(const FObjectInitializer& ObjectInitializer)
{
  if ( HasAnyFlags(RF_ClassDefaultObject) == false )
	{
		AddToRoot();
	}
}

UHoudiniImportNodeWrapper* UHoudiniImportNodeWrapper::ImportBuildings(
    UObject* InHoudiniObject,
    const FTransform& InInstantiateAt,
    UObject* InWorldContextObject,
    const FString& MapName, const FString& OSMFilePath,
    float Latitude, float Longitude,
    int ClusterSize, int CurrentCluster,
    bool bUseCOM)
{
  UE_LOG(LogCarlaTools, Error, TEXT("Houdini asset not valid"));
  return nullptr;
}

void UHoudiniImportNodeWrapper::Activate()
{
}

void UHoudiniImportNodeWrapper::HandleCompleted(
    bool bCookSuccess, bool bBakeSuccess)
{
  UE_LOG(LogCarlaTools, Log, TEXT("Generation Finished"));
  if (Completed.IsBound())
  {
    Completed.Broadcast(bCookSuccess, bBakeSuccess);
  }
  RemoveFromRoot();
}

void UHoudiniImportNodeWrapper::HandleFailed(
    bool bCookSuccess, bool bBakeSuccess)
{
  UE_LOG(LogCarlaTools, Log, TEXT("Generation failed"));
  if (Failed.IsBound())
  {
    Failed.Broadcast(bCookSuccess, bBakeSuccess);
  }
  RemoveFromRoot();
}
