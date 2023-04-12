
#pragma once

#include "UObject/Object.h"
#include "CoreMinimal.h"
#include "USDCARLAInterface.generated.h"

USTRUCT(BlueprintType)
struct FUSDCARLALight
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FString Name;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FVector Location;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FLinearColor Color;
};

UCLASS()
class OMNIVERSEUSD_API UUSDCARLAInterface : public UObject
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
  static bool ImportUSD(
      const FString& Path, const FString& Dest,
      bool bImportUnusedReferences, bool bImportAsBlueprint);
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
  static TArray<FUSDCARLALight> GetUSDLights(const FString& Path);
};
