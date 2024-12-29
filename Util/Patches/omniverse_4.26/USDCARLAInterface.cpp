#include "USDCARLAInterface.h"
#include "OmniversePxr.h"
#include "OmniverseUSDImporter.h"
#include "OmniverseUSDHelper.h"

// Import a USD file with the specified settings.
bool UUSDCARLAInterface::ImportUSD(
    const FString& Path, const FString& Dest, 
    bool bImportUnusedReferences, bool bImportAsBlueprint)
{
  // Configure the import settings.
  FOmniverseImportSettings Settings;
  Settings.bImportUnusedReferences = bImportUnusedReferences;
  Settings.bImportAsBlueprint = bImportAsBlueprint;

  // Perform the USD import.
  return FOmniverseUSDImporter::LoadUSD(Path, Dest, Settings);
}

// Retrieve all light data from a USD file at the specified path.
TArray<FUSDCARLALight> UUSDCARLAInterface::GetUSDLights(const FString& Path)
{
  TArray<FUSDCARLALight> Result;

  // Load the USD stage from the file path.
  pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);

  // Traverse the stage to find all light primitives.
  std::vector<pxr::UsdPrim> LightPrims;
  const auto& PrimRange = Stage->Traverse();
  for (const auto& Prim : PrimRange) {
      if (Prim.IsA<pxr::UsdLuxLight>()) {
          LightPrims.push_back(Prim);
      }
  }

  // Extract and convert light attributes into a readable format.
  for (pxr::UsdPrim& LightPrim : LightPrims)
  {
    pxr::UsdLuxLight Light(LightPrim);

    // Extract light name and attributes.
    std::string StdName = LightPrim.GetName();
    pxr::GfVec3f PxColor;
    pxr::VtValue vtValue;
    if (Light.GetColorAttr().Get(&vtValue)) {
        PxColor = vtValue.Get<pxr::GfVec3f>();
    }

    // Extract the world transform of the light.
    pxr::UsdGeomXformCache Cache;
    pxr::GfMatrix4d Transform = Cache.GetLocalToWorldTransform(LightPrim);
    pxr::GfVec3d PxLocation = Transform.ExtractTranslation();

    // Convert extracted attributes to Unreal's formats.
    FString Name(StdName.size(), UTF8_TO_TCHAR(StdName.c_str()));
    FLinearColor Color(PxColor[0], PxColor[1], PxColor[2]);
    FVector Location(PxLocation[0], -PxLocation[1], PxLocation[2]); // Adjust for Unreal's coordinate system.
    Location *= 100.f; // Convert from meters to centimeters.

    FUSDCARLALight LightParams {Name, Location, Color};
    Result.Add(LightParams);
  }

  return Result;
}

// Retrieve wheel data for a vehicle from a USD file.
TArray<FUSDCARLAWheelData> UUSDCARLAInterface::GetUSDWheelData(const FString& Path)
{
  // Load the USD stage from the file path.
  pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);

  // Paths for wheel and suspension data within the USD file.
  const std::string UsdPhysxWheelPath = "/vehicle/_physx/_physxWheels/";
  const std::string UsdPhysxSuspensionPath = "/vehicle/_physx/_physxSuspensions/";

  // Helper function to get a float attribute value from a USD primitive.
  auto GetFloatAttributeValue = [](pxr::UsdPrim& Prim, const std::string& AttrName) -> float
  {
    pxr::UsdAttribute Attribute = Prim.GetAttribute(pxr::TfToken(AttrName));
    if(!Attribute) return 0.f;

    float Value = 0.f;
    Attribute.Get(&Value);
    return Value;
  };

  // Helper function to get a USD primitive from a relationship.
  auto GetPrimFromRelationship = [&](
      pxr::UsdRelationship& Relationship) -> pxr::UsdPrim
  {
    std::vector<pxr::SdfPath> Targets;
    Relationship.GetTargets(&Targets);
    return Targets.empty() ? pxr::UsdPrim() : Stage->GetPrimAtPath(Targets.front());
  };

  // Helper function to parse wheel data for a specific wheel.
  auto ParseWheelData = [&](
      const std::string& WheelName, 
      FUSDCARLAWheelData& OutWheelData) -> bool
  {
    // Retrieve the wheel USD primitive.
    pxr::SdfPath WheelPath(UsdPhysxWheelPath + WheelName);
    pxr::UsdPrim WheelPrim = Stage->GetPrimAtPath(WheelPath);
    if(!WheelPrim) {
      UE_LOG(LogOmniverseUsd, Warning, TEXT("Wheel prim not found: %s"), *FString(WheelName.c_str()));
      return false;
    }

    // Retrieve relationships for wheel, tire, and suspension data.
    pxr::UsdRelationship WheelRelationship, TireRelationship, SuspensionRelationship;
    for (pxr::UsdProperty& Property : WheelPrim.GetProperties()) {
      FString Name (Property.GetBaseName().GetText());
      if(Name == "wheel") WheelRelationship = Property.As<pxr::UsdRelationship>();
      if(Name == "tire") TireRelationship = Property.As<pxr::UsdRelationship>();
      if(Name == "suspension") SuspensionRelationship = Property.As<pxr::UsdRelationship>();
    }

    if(!WheelRelationship || !TireRelationship || !SuspensionRelationship) {
      UE_LOG(LogOmniverseUsd, Warning, TEXT("Failed to find required relationships for wheel: %s"), *FString(WheelName.c_str()));
      return false;
    }

    // Retrieve the primitives for the relationships.
    pxr::UsdPrim PhysxWheelPrim = GetPrimFromRelationship(WheelRelationship);
    pxr::UsdPrim PhysxTirePrim = GetPrimFromRelationship(TireRelationship);
    pxr::UsdPrim PhysxSuspensionPrim = GetPrimFromRelationship(SuspensionRelationship);

    if (!PhysxWheelPrim || !PhysxTirePrim || !PhysxSuspensionPrim) {
      UE_LOG(LogOmniverseUsd, Warning, TEXT("Failed to find primitives for wheel: %s"), *FString(WheelName.c_str()));
      return false;
    }

    // Extract and set wheel parameters.
    OutWheelData.MaxBrakeTorque = GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxBrakeTorque");
    OutWheelData.MaxHandBrakeTorque = GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxHandBrakeTorque");
    OutWheelData.MaxSteerAngle = GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxSteerAngle");
    OutWheelData.SpringStrength = GetFloatAttributeValue(PhysxSuspensionPrim, "physxVehicleSuspension:springStrength");
    OutWheelData.MaxCompression = GetFloatAttributeValue(PhysxSuspensionPrim, "physxVehicleSuspension:maxCompression");
    OutWheelData.MaxDroop = GetFloatAttributeValue(PhysxSuspensionPrim, "physxVehicleSuspension:maxDroop");
    OutWheelData.LateralStiffnessX = GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:latStiffX");
    OutWheelData.LateralStiffnessY = GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:latStiffY");
    OutWheelData.LongitudinalStiffness = GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:longitudinalStiffnessPerUnitGravity");

    return true;
  };

  // Define default wheel data, which will be overridden if parsed successfully.
  FUSDCARLAWheelData Wheel0 = {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; // front left
  FUSDCARLAWheelData Wheel1 = {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; // front right
  FUSDCARLAWheelData Wheel2 = {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; // rear left
  FUSDCARLAWheelData Wheel3 = {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; // rear right

  // Parse the data for each wheel.
  ParseWheelData("wheel_0", Wheel0);
  ParseWheelData("wheel_1", Wheel1);
  ParseWheelData("wheel_2", Wheel2);
  ParseWheelData("wheel_3", Wheel3);

  return {Wheel0, Wheel1, Wheel2, Wheel3};
}
