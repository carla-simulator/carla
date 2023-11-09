
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

TArray<FUSDCARLAWheelData> UUSDCARLAInterface::GetUSDWheelData(const FString& Path)
{
  pxr::UsdStageRefPtr Stage = 
      FOmniverseUSDHelper::LoadUSDStageFromPath(Path);

  // Get the wheel data
  const std::string UsdPhysxWheelPath = "/vehicle/_physx/_physxWheels/";
  const std::string UsdPhysxSuspensionPath = "/vehicle/_physx/_physxSuspensions/";
  auto GetFloatAttributeValue = [](pxr::UsdPrim& Prim, const std::string& AttrName) -> float
  {
    pxr::UsdAttribute Attribute = 
        Prim.GetAttribute(pxr::TfToken(AttrName));
    if(!Attribute)
    {
      return 0.f;
    }
    float Value = 0.f;
    Attribute.Get(&Value);
    return Value;
  };
  auto GetPrimFromRelationship = [&](
      pxr::UsdRelationship& Relationship) -> pxr::UsdPrim
  {
    std::vector<pxr::SdfPath> Targets;
    Relationship.GetTargets(&Targets);
    if (!Targets.size())
    {
      return pxr::UsdPrim();
    }
    return Stage->GetPrimAtPath(Targets.front());
  };
  auto ParseWheelData = [&](
      const std::string& WheelName, 
      FUSDCARLAWheelData& OutWheelData) -> bool
  {
    pxr::SdfPath WheelPath(UsdPhysxWheelPath + WheelName);
    pxr::UsdPrim WheelPrim = Stage->GetPrimAtPath(WheelPath);
    if(!WheelPrim)
    {
      UE_LOG(LogOmniverseUsd, Warning, TEXT("Wheel prim fail"));
      return false;
    }
    pxr::UsdRelationship WheelRelationship;
    pxr::UsdRelationship TireRelationship;
    pxr::UsdRelationship SuspensionRelationship;
    for (pxr::UsdProperty& Property : WheelPrim.GetProperties())
    {
      FString Name (Property.GetBaseName().GetText());
      if(Name == "wheel")
      {
        WheelRelationship = Property.As<pxr::UsdRelationship>();
      }
      if(Name == "tire")
      {
        TireRelationship = Property.As<pxr::UsdRelationship>();
      }
      if(Name == "suspension")
      {
        SuspensionRelationship = 
            Property.As<pxr::UsdRelationship>();
      }
    }
    if(!WheelRelationship || !TireRelationship || !SuspensionRelationship)
    {
      UE_LOG(LogOmniverseUsd, Warning, TEXT("Relationships fail: %d %d %d"), 
          bool(WheelRelationship), bool(TireRelationship), bool(SuspensionRelationship));
      return false;
    }
    pxr::UsdPrim PhysxWheelPrim = GetPrimFromRelationship(WheelRelationship);
    pxr::UsdPrim PhysxTirePrim = GetPrimFromRelationship(TireRelationship);
    pxr::UsdPrim PhysxSuspensionlPrim = GetPrimFromRelationship(SuspensionRelationship);

    if (!PhysxWheelPrim || !PhysxTirePrim || !PhysxSuspensionlPrim)
    {
      UE_LOG(LogOmniverseUsd, Warning, TEXT("Prims fail: %d %d %d"), 
          bool(PhysxWheelPrim), bool(PhysxTirePrim), bool(PhysxSuspensionlPrim));
      return false;
    }
    OutWheelData.MaxBrakeTorque = 
        GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxBrakeTorque");
    OutWheelData.MaxHandBrakeTorque = 
        GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxHandBrakeTorque");
    OutWheelData.MaxSteerAngle = 
        GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxSteerAngle");
    OutWheelData.SpringStrength = 
        GetFloatAttributeValue(PhysxSuspensionlPrim, "physxVehicleSuspension:springStrength");
    OutWheelData.MaxCompression = 
        GetFloatAttributeValue(PhysxSuspensionlPrim, "physxVehicleSuspension:maxCompression");
    OutWheelData.MaxDroop = 
        GetFloatAttributeValue(PhysxSuspensionlPrim, "physxVehicleSuspension:maxDroop");
    OutWheelData.LateralStiffnessX = 
        GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:latStiffX");
    OutWheelData.LateralStiffnessY = 
        GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:latStiffY");
    OutWheelData.LongitudinalStiffness = 
        GetFloatAttributeValue(
        PhysxTirePrim, "physxVehicleTire:longitudinalStiffnessPerUnitGravity");

    UE_LOG(LogOmniverseUsd, Warning, TEXT("USD values: %f %f %f %f %f %f %f %f %f"), 
    OutWheelData.MaxBrakeTorque, OutWheelData.MaxHandBrakeTorque, OutWheelData.MaxSteerAngle,
    OutWheelData.SpringStrength, OutWheelData.MaxCompression, OutWheelData.MaxDroop,
    OutWheelData.LateralStiffnessX, OutWheelData.LateralStiffnessY, OutWheelData.LongitudinalStiffness);

    return true;
  };
  // default wheel values, overriden by ParseWheelData if physx data is present
  FUSDCARLAWheelData Wheel0 = 
      {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; // front left
  FUSDCARLAWheelData Wheel1 = 
      {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; // front right
  FUSDCARLAWheelData Wheel2 = 
      {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; // rear left
  FUSDCARLAWheelData Wheel3 = 
      {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; // rear right
  ParseWheelData("wheel_0", Wheel0);
  ParseWheelData("wheel_1", Wheel1);
  ParseWheelData("wheel_2", Wheel2);
  ParseWheelData("wheel_3", Wheel3);

  return {Wheel0, Wheel1, Wheel2, Wheel3};
}