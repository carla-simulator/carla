
#include "USDCARLAInterface.h"
#include "OmniversePxr.h"
#include "OmniverseUSDImporter.h"
#include "OmniverseUSDHelper.h"


bool UUSDCARLAInterface::ImportUSD(
    const FString& Path, const FString& Dest, 
    bool bImportUnusedReferences, bool bImportAsBlueprint)
{
  FOmniverseImportSettings Settings;
  Settings.bImportUnusedReferences = bImportUnusedReferences;
  Settings.bImportAsBlueprint = bImportAsBlueprint;
  return FOmniverseUSDImporter::LoadUSD(Path, Dest, Settings);
}

TArray<FUSDCARLALight> UUSDCARLAInterface::GetUSDLights(const FString& Path)
{
  TArray<FUSDCARLALight> Result;
  pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);
  std::vector<pxr::UsdPrim> LightPrims;
  const auto& PrimRange = Stage->Traverse();
  for (const auto& Prim : PrimRange) {
      if (Prim.IsA<pxr::UsdLuxLight>()) {
          LightPrims.push_back(Prim);
      }
  }
  for (pxr::UsdPrim& LightPrim : LightPrims)
  {
    pxr::UsdLuxLight Light(LightPrim);
    std::string StdName = LightPrim.GetName();
    
    pxr::GfVec3f PxColor;
    pxr::VtValue vtValue;
    if (Light.GetColorAttr().Get(&vtValue)) {
        PxColor = vtValue.Get<pxr::GfVec3f>();
    }
    pxr::UsdGeomXformCache Cache;
    pxr::GfMatrix4d Transform = Cache.GetLocalToWorldTransform(LightPrim);
    pxr::GfVec3d PxLocation = Transform.ExtractTranslation();
    
    FString Name(StdName.size(), UTF8_TO_TCHAR(StdName.c_str()));
    FLinearColor Color(PxColor[0], PxColor[1], PxColor[2]);
    FVector Location(PxLocation[0], -PxLocation[1], PxLocation[2]);
    Location *= 100.f;

    FUSDCARLALight LightParams {Name, Location, Color};
    Result.Add(LightParams);
  }
  return Result;
}