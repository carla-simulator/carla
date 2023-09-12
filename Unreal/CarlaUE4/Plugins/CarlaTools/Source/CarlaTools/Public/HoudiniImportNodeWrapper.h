// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "HoudiniPublicAPIProcessHDANode.h"

#include "HoudiniImportNodeWrapper.generated.h"

// Delegate type for output pins on the node.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProcessImporterOutputPinDelegate, const bool, bCookSuccess, const bool, bBakeSuccess);

UCLASS()
class CARLATOOLS_API UHoudiniImportNodeWrapper : public UBlueprintAsyncActionBase
{
  GENERATED_BODY()

public:

  UHoudiniImportNodeWrapper(const FObjectInitializer& ObjectInitializer);

  UFUNCTION(BlueprintCallable, meta=(AdvancedDisplay=5,AutoCreateRefTerm="InInstantiateAt",BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Houdini Importer")
  static UHoudiniImportNodeWrapper* ImportBuildings(
      UObject* InHoudiniAsset,
      const FTransform& InInstantiateAt,
      UObject* InWorldContextObject,
      const FString& MapName, const FString& OSMFilePath,
      float Latitude, float Longitude,
      int ClusterSize, int CurrentCluster,
      bool bUseCOM);

  // Fires on task completed
  UPROPERTY(BlueprintAssignable, Category="Houdini|Public API")
	FOnProcessImporterOutputPinDelegate Completed;

	// Fires if the task fails
	UPROPERTY(BlueprintAssignable, Category="Houdini|Public API")
	FOnProcessImporterOutputPinDelegate Failed;

  virtual void Activate() override;

protected:

  UFUNCTION()
  void HandleCompleted(
      UHoudiniPublicAPIAssetWrapper* AssetWrapper, 
      bool bCookSuccess, 
      bool bBakeSuccess);

  UFUNCTION()
  void HandleFailed(
      UHoudiniPublicAPIAssetWrapper* AssetWrapper,
      bool bCookSuccess,
      bool bBakeSuccess);

private:
  UHoudiniPublicAPIProcessHDANode* HDANode;

};
