
#pragma once

#include "CoreMinimal.h"

class OMNIVERSEUSD_API FUSDCARLAInterface
{
public:
  static bool ImportUSD(const FString& Path, const FString& Dest, bool bImportUnusedReferences, bool bImportAsBlueprint);
};
