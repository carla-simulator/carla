#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "Carla/Game/TaggedMaterials.h"
#include "GenerateTaggedMaterialsRegistry.generated.h"

UCLASS()
class CARLA_API UGenerateTaggedMaterialsRegistryCommandlet : public UCommandlet
{
  GENERATED_BODY()

  UGenerateTaggedMaterialsRegistryCommandlet();

#if WITH_EDITOR
public:

  virtual int32 Main(const FString& Params) override;

private:
  class IAssetRegistry& GetAssetRegistry() const;
  TSet<UMaterialInterface*> FindMaskedMaterialInterfaces(const FName& PackageName);
  void CreateMapPackage(const FString& PackageName, UTaggedMaterialsRegistry* TaggedMaterialsRegistry);

  static TArray<FString> GetPackagePaths(const FString& PackageName);
  
  TSet<FName> ScannedAssets;
  mutable class IAssetRegistry* CachedAssetRegistry;
#endif // WITH_EDITOR
};
