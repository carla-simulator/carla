#pragma once

#include "CoreMinimal.h"

#include "TaggedMaterials.generated.h"


UCLASS(BlueprintType)
class CARLA_API UTaggedMaterialsRegistry : public UObject
{
  GENERATED_BODY()

public:
  static UTaggedMaterialsRegistry* Create(const FString& RegistryName);
  static UTaggedMaterialsRegistry* Get();
  static UTaggedMaterialsRegistry* Load(const FString& RegistryName, bool bIsSuffix = true);

  UMaterialInstanceDynamic* GetTaggedMaterial();
  UMaterialInstanceDynamic* GetTaggedMaterial(UMaterialInterface* UsedMaterial);
  FORCEINLINE int32 Num() const {return TaggedMaskedMaterials.Num();}

#if WITH_EDITOR
  virtual void PostInitProperties() override;
  virtual void BeginDestroy() override;

  void Save();
#endif // WITH_EDITOR

  static const FString TaggedMaterialsRootDir;
protected:
  UTaggedMaterialsRegistry();

#if WITH_EDITOR
  void InjectTag(UMaterialInterface* MaterialInterface);
  void InjectTagIntoMaterial(UMaterial* Material);
  void InjectTagIntoMaterialInstance(UMaterialInstance* MaterialInstance);

  void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources);
#endif // WITH_EDITOR

private:
  UPROPERTY(VisibleAnywhere)
  UMaterial* TaggedOpaqueMaterial;

  UPROPERTY(VisibleAnywhere)
  TMap<FString, UMaterialInterface*> TaggedMaskedMaterials;

  bool bPendingChanges = false;

  static UTaggedMaterialsRegistry* Registry;
};

// The actor is only required for cooking a UTaggedMaterialsRegistry as part of a UWorld
UCLASS()
class ATaggedMaterialsRegistryActor : public AActor
{
  GENERATED_BODY()

public:
  UPROPERTY(VisibleAnywhere)
  UTaggedMaterialsRegistry* TaggedMaterialsRegistry;
};
