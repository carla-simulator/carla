// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/*
#include "HoudiniImportNodeWrapper.h"
#include "HoudiniAsset.h"

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
  UE_LOG(LogCarlaTools, Log, TEXT("Start building import"));
  UHoudiniAsset* InHoudiniAsset = Cast<UHoudiniAsset>(InHoudiniObject);
  if (!InHoudiniAsset)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("Houdini asset not valid"));
    return nullptr;
  }
  UHoudiniImportNodeWrapper* WrapperNode = NewObject<UHoudiniImportNodeWrapper>();

  TMap<FName, FHoudiniParameterTuple> InParameters = 
  { {"userMapName", FHoudiniParameterTuple(MapName)},
    {"osmPath", FHoudiniParameterTuple(OSMFilePath)},
    {"clusterSize", FHoudiniParameterTuple(ClusterSize)},
    {"displayedCluster", FHoudiniParameterTuple(CurrentCluster)},
    {"startCooking", FHoudiniParameterTuple(true)},
    {"lat", FHoudiniParameterTuple(Latitude)},
    {"lon", FHoudiniParameterTuple(Longitude)},
    {"centOfMass", FHoudiniParameterTuple(bUseCOM)}};
  
  WrapperNode->HDANode = 
      UHoudiniPublicAPIProcessHDANode::ProcessHDA(
      InHoudiniAsset, InInstantiateAt, InParameters, {}, {},
      InWorldContextObject, nullptr,
      true, true, "", EHoudiniEngineBakeOption::ToActor,
      true);
  WrapperNode->HDANode->Completed.AddDynamic(WrapperNode, &UHoudiniImportNodeWrapper::HandleCompleted);
  WrapperNode->HDANode->Failed.AddDynamic(WrapperNode, &UHoudiniImportNodeWrapper::HandleFailed);
  UE_LOG(LogCarlaTools, Log, TEXT("HDA node created"));
  return WrapperNode;
}

void UHoudiniImportNodeWrapper::Activate()
{
  HDANode->Activate();
}

void UHoudiniImportNodeWrapper::HandleCompleted(
    UHoudiniPublicAPIAssetWrapper* , 
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
    UHoudiniPublicAPIAssetWrapper* ,
    bool bCookSuccess, bool bBakeSuccess)
{
  UE_LOG(LogCarlaTools, Log, TEXT("Generation failed"));
  if (Failed.IsBound())
  {
    Failed.Broadcast(bCookSuccess, bBakeSuccess);
  }
  RemoveFromRoot();
}
*/
