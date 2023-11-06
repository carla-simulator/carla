// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/Skeleton.h"
#include "VehicleWheel.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include "USDImporterWidget.generated.h"

USTRUCT(BlueprintType)
struct FWheelTemplates
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheel")
  TSubclassOf<UVehicleWheel> WheelRL;
};

USTRUCT(BlueprintType)
struct FVehicleLight
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FString Name;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FVector Location;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FLinearColor Color;
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FVehicleMeshAnchorPoints
{
  GENERATED_BODY();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector DoorRL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector WheelRL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector Hood;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  FVector Trunk;

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
  UStaticMesh* DoorFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* DoorRL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* Trunk;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* Hood;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelFR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelFL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelRR;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* WheelRL;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Importer")
  UStaticMesh* Body;
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
