// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/Skeleton.h"
#include "ChaosVehicleWheel.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include <util/ue-header-guard-end.h>

#include "USDImporterWidget.generated.h"

USTRUCT(BlueprintType)
struct FWheelTemplates
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UChaosVehicleWheel> WheelFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UChaosVehicleWheel> WheelFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UChaosVehicleWheel> WheelRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UChaosVehicleWheel> WheelRL;
};

USTRUCT(BlueprintType)
struct FVehicleLight
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FString Name;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FVector Location = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FLinearColor Color = FLinearColor::Black;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVehicleMeshAnchorPoints
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorFR = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorFL = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorRR = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorRL = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelFR = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelFL = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelRR = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelRL = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector Hood = FVector::ZeroVector;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector Trunk = FVector::ZeroVector;

};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVehicleMeshParts
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> DoorRL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> Trunk;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> Hood;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> WheelRL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<UPrimitiveComponent*> Body;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVehicleMeshAnchorPoints Anchors;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<FVehicleLight> Lights;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMergedVehicleMeshParts
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> DoorFR = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> DoorFL = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> DoorRR = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> DoorRL = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> Trunk = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> Hood = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> WheelFR = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> WheelFL = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> WheelRR = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> WheelRL = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TObjectPtr<UStaticMesh> Body = nullptr;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVehicleMeshAnchorPoints Anchors;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  TArray<FVehicleLight> Lights;
};

UCLASS()
class CARLATOOLS_API UUSDImporterWidget : public UUserWidget
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintCallable, Category="USD Importer")
  void ImportUSDProp(const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint = true);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  void ImportUSDVehicle(
      const FString& USDPath,
      const FString& DestinationAssetPath,
      FWheelTemplates BaseWheelData,
      TArray<FVehicleLight>& LightList,
      FWheelTemplates& WheelObjects,
      bool bAsBlueprint = true);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static AActor* GetGeneratedBlueprint(UWorld* World, const FString& USDPath);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static bool MergeStaticMeshComponents(TArray<AActor*> Actors, const FString& DestMesh);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static TArray<UObject*> MergeMeshComponents(TArray<UPrimitiveComponent*> Components, const FString& DestMesh);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static FVehicleMeshParts SplitVehicleParts(
      AActor* BlueprintActor,
      const TArray<FVehicleLight>& LightList,
      UMaterialInterface* GlassMaterial);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static FMergedVehicleMeshParts GenerateVehicleMeshes(const FVehicleMeshParts& VehicleMeshParts, const FString& DestPath);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static AActor* GenerateNewVehicleBlueprint(
      UWorld* World,
      UClass* BaseClass,
      USkeletalMesh* NewSkeletalMesh,
      UPhysicsAsset* NewPhysicsAsset,
      const FString &DestPath,
      const FMergedVehicleMeshParts& VehicleMeshes,
      const FWheelTemplates& WheelTemplates);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static bool EditSkeletalMeshBones(
      USkeletalMesh* Skeleton,
      const TMap<FString, FTransform> &NewBoneTransforms);
  UFUNCTION(BlueprintCallable, Category="USD Importer")
  static void CopyCollisionToPhysicsAsset(
      UPhysicsAsset* PhysicsAssetToEdit, UStaticMesh* StaticMesh);

};
