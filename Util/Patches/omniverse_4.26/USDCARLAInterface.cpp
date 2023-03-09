
#include "USDCARLAInterface.h"
#include "OmniverseUSDImporter.h"


bool FUSDCARLAInterface::ImportUSD(
    const FString& Path, const FString& Dest, 
    bool bImportUnusedReferences, bool bImportAsBlueprint)
{
  FOmniverseImportSettings Settings;
  Settings.bImportUnusedReferences = bImportUnusedReferences;
  Settings.bImportAsBlueprint = bImportAsBlueprint;
  return FOmniverseUSDImporter::LoadUSD(Path, Dest, Settings);
}
