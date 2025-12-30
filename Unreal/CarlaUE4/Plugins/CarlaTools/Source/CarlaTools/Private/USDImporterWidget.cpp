// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "USDImporterWidget.h"
#ifdef WITH_SIMREADY
  #include "USDCARLAInterface.h"
#include "SimReadyPathHelper.h"
#endif
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "ReferenceSkeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "PackageHelperFunctions.h"
#include "EditorAssetLibrary.h"
#include <unordered_map>
#include <string>
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/BoxComponent.h"
#include "VehicleWheel.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditor.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "EditorStaticMeshLibrary.h"
#include "Carla/Util/BoundingBoxCalculator.h"


void UUSDImporterWidget::ImportUSDProp(
    const FString& USDPath, const FString& DestinationAssetPath, bool bAsBlueprint)
{
#ifdef WITH_SIMREADY
  UUSDCARLAInterface::ImportUSD(USDPath, DestinationAssetPath, false, bAsBlueprint, false);
#else
  UE_LOG(LogCarlaTools, Error, TEXT("SimReady Plugin is not enabled"));
#endif

}

void UUSDImporterWidget::ImportUSDVehicle(
    const FString& USDPath,
    const FString& DestinationAssetPath,
    FWheelTemplates BaseWheelData,
    TArray<FVehicleLight>& LightList,
    FWheelTemplates& WheelObjects,
    bool bAsBlueprint)
{
#ifdef WITH_SIMREADY
  if (USDPath.IsEmpty() || DestinationAssetPath.IsEmpty())
  {
    return;
  }
  // Import meshes
  UUSDCARLAInterface::ImportUSD(USDPath, DestinationAssetPath, false, bAsBlueprint, true);
  // Import Lights
  TArray<FUSDCARLALight> USDLights = UUSDCARLAInterface::GetUSDLights(USDPath);
  LightList.Empty();
  for (const FUSDCARLALight& USDLight : USDLights)
  {
    FVehicleLight Light {USDLight.Name, USDLight.Location, USDLight.Color};
    LightList.Add(Light);
  }
  // Import Wheel and suspension data
  TArray<FUSDCARLAWheelData> WheelsData = UUSDCARLAInterface::GetUSDWheelData(USDPath);
  if (WheelsData.Num() == 0)
  {
      return;
  }
  auto CreateVehicleWheel =
      [&](const FUSDCARLAWheelData& WheelData,
         TSubclassOf<UVehicleWheel> TemplateClass,
         const FString &PackagePathName)
      -> TSubclassOf<UVehicleWheel>
  {
    // Get a reference to the editor subsystem
    constexpr float MToCM = 100.f;
    constexpr float RadToDeg = 360.f/3.14159265359f;
    FString BlueprintName =  FPaths::GetBaseFilename(PackagePathName);
    FString BlueprintPath = FPaths::GetPath(PackagePathName);
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    // Create a new Blueprint factory
    UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
    // Set the parent class for the new Blueprint
    Factory->ParentClass = TemplateClass;
    // Create a new Blueprint asset with the given name
    UObject* NewAsset = AssetTools.CreateAsset(BlueprintName, BlueprintPath, UBlueprint::StaticClass(), Factory);
    if (!NewAsset)
    {
        UE_LOG(LogCarlaTools, Error, TEXT("Fail to create wheel blueprint %s at %s"), *BlueprintName, *BlueprintPath);
        return nullptr;
    }
    // Cast the new asset to a UBlueprint
    UBlueprint* NewBlueprint = Cast<UBlueprint>(NewAsset);
    // Modify the new Blueprint
    NewBlueprint->Modify();
    // Edit the default object for the new Blueprint
    UVehicleWheel* Result = Cast<UVehicleWheel>(NewBlueprint->GeneratedClass->ClassDefaultObject);
    Result->MaxBrakeTorque = MToCM*WheelData.MaxBrakeTorque;
    if (WheelData.MaxHandBrakeTorque != 0)
    {
      Result->MaxHandBrakeTorque = MToCM*WheelData.MaxHandBrakeTorque;
    }
    Result->SteerAngle = RadToDeg*WheelData.MaxSteerAngle;
    Result->SuspensionMaxDrop = MToCM*WheelData.MaxDroop;
    Result->LatStiffValue = WheelData.LateralStiffnessY;
    Result->LongStiffValue = WheelData.LongitudinalStiffness;
    return Result->GetClass();
  };
  // Save wheel objects
  FString AssetPath = DestinationAssetPath / FPaths::GetBaseFilename(USDPath);
#ifdef WITH_SIMREADY
  FSimReadyPathHelper::FixAssetName(AssetPath);
#endif
  FString PathWheelFL = AssetPath + "_Wheel_FLW";
  FString PathWheelFR = AssetPath + "_Wheel_FRW";
  FString PathWheelRL = AssetPath + "_Wheel_RLW";
  FString PathWheelRR = AssetPath + "_Wheel_RRW";
  WheelObjects.WheelFL = CreateVehicleWheel(
      WheelsData[0], BaseWheelData.WheelFL, PathWheelFL);
  WheelObjects.WheelFR = CreateVehicleWheel(
      WheelsData[1], BaseWheelData.WheelFR, PathWheelFR);
  WheelObjects.WheelRL = CreateVehicleWheel(
      WheelsData[2], BaseWheelData.WheelRL, PathWheelRL);
  WheelObjects.WheelRR = CreateVehicleWheel(
      WheelsData[3], BaseWheelData.WheelRR, PathWheelRR);

#else
  UE_LOG(LogCarlaTools, Error, TEXT("SimReady Plugin is not enabled"));
#endif
}

AActor* UUSDImporterWidget::GetGeneratedBlueprint(UWorld* World, const FString& USDPath)
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
  FString USDFileName = FPaths::GetBaseFilename(USDPath, true);
#ifdef WITH_SIMREADY
  FSimReadyPathHelper::FixAssetName(USDFileName);
#endif
  UE_LOG(LogCarlaTools, Log, TEXT("Searching for name %s"), *USDFileName);
  for (AActor* Actor : Actors)
  {
    if(Actor->GetName().Contains(USDFileName))
    {
      return Actor;
    }
  }
  return nullptr;
}

bool UUSDImporterWidget::MergeStaticMeshComponents(
    TArray<AActor*> Actors, const FString& DestMesh)
{
  if (Actors.Num() == 0)
  {
    UE_LOG(LogCarlaTools, Error, TEXT("No actors for merge"));
    return false;
  }
  UWorld* World = Actors[0]->GetWorld();
  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  TArray<UPrimitiveComponent*> ComponentsToMerge;
  for(AActor* Actor : Actors)
  {
    TArray<UPrimitiveComponent*> ActorComponents;
    Actor->GetComponents(ActorComponents, false);
    ComponentsToMerge.Append(ActorComponents);
  }
  FMeshMergingSettings MeshMergeSettings;
  TArray<UObject*> AssetsToSync;
  const float ScreenAreaSize = TNumericLimits<float>::Max();
  FVector NewLocation;
  MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, MeshMergeSettings, nullptr, nullptr, DestMesh, AssetsToSync, NewLocation, ScreenAreaSize, true);
  return true;
}

TArray<UObject*> UUSDImporterWidget::MergeMeshComponents(
    TArray<UPrimitiveComponent*> ComponentsToMerge,
    const FString& DestMesh)
{
  if(!ComponentsToMerge.Num())
  {
    return {};
  }
  UWorld* World = ComponentsToMerge[0]->GetWorld();
  const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
  FMeshMergingSettings MeshMergeSettings;
  MeshMergeSettings.bMergeEquivalentMaterials = false;
  TArray<UObject*> AssetsToSync;
  const float ScreenAreaSize = TNumericLimits<float>::Max();
  FVector NewLocation;
  MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, MeshMergeSettings, nullptr, nullptr, DestMesh, AssetsToSync, NewLocation, ScreenAreaSize, true);
  return AssetsToSync;
}

bool IsChildrenOf(USceneComponent* Component, FString StringInParent)
{
  USceneComponent* CurrentComponent = Component;
  while(CurrentComponent)
  {
    FString ComponentName = UKismetSystemLibrary::GetDisplayName(CurrentComponent);
    if(ComponentName.Contains(StringInParent))
    {
      return true;
    }
    CurrentComponent = CurrentComponent->GetAttachParent();
  }
  return false;
}

FVehicleMeshParts UUSDImporterWidget::SplitVehicleParts(
    AActor* BlueprintActor,
    const TArray<FVehicleLight>& LightList,
    UMaterialInterface* GlassMaterial)
{
  FVehicleMeshParts Result;
  FMemory::Memzero(Result.Anchors);
  Result.Lights = LightList;
  TArray<UStaticMeshComponent*> MeshComponents;
  BlueprintActor->GetComponents(MeshComponents, false);
  TArray<USceneComponent*> SceneComponents;
  BlueprintActor->GetComponents(SceneComponents, false);
  FVector BodyLocation = FVector(0,0,0);
  TArray<UStaticMeshComponent*> GlassComponents;

  for (USceneComponent* Component : SceneComponents)
  {
      FString ComponentName = Component->GetName();
      if (ComponentName == "wheel_0_wheel_0")
      {
          Result.Anchors.WheelFL = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "wheel_1_wheel_1")
      {
          Result.Anchors.WheelFR = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "wheel_2_wheel_2")
      {
          Result.Anchors.WheelRL = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "wheel_3_wheel_3")
      {
          Result.Anchors.WheelRR = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "door_0_door_0")
      {
          Result.Anchors.DoorFL = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "door_1_door_1")
      {
          Result.Anchors.DoorFR = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "door_2_door_2")
      {
          Result.Anchors.DoorRL = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "door_3_door_3")
      {
          Result.Anchors.DoorRR = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "hood_hood")
      {
          Result.Anchors.Hood = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "trunk_trunk")
      {
          Result.Anchors.Trunk = Component->GetComponentTransform().GetLocation();
      }
      else if (ComponentName == "body_body")
      {
          Result.Anchors.Body = Component->GetComponentTransform().GetLocation();
      }
  }

  for (UStaticMeshComponent* Component : MeshComponents)
  {
    if (!Component->GetStaticMesh())
    {
      continue;
    }
    FString ComponentName = UKismetSystemLibrary::GetDisplayName(Component);
    Component->SetRelativeLocation(FVector::ZeroVector);

    if (IsChildrenOf(Component, "_door_0_"))
    {
	    Result.DoorFL.Add(Component);
    }
    else if (IsChildrenOf(Component, "_door_1_"))
    {
		Result.DoorFR.Add(Component);
    }
    else if (IsChildrenOf(Component, "_door_2_"))
    {
        Result.DoorRL.Add(Component);
    }
    else if (IsChildrenOf(Component, "_door_3_"))
    {
		Result.DoorRR.Add(Component);
    }
    else if (IsChildrenOf(Component, "_trunk_"))
    {
		Result.Trunk.Add(Component);
    }
    else if (IsChildrenOf(Component, "_hood_"))
    {
		Result.Hood.Add(Component);
    }
    else if (IsChildrenOf(Component, "_suspension_0_"))
    {    
        Result.WheelFL.Add(Component);
    }
    else if (IsChildrenOf(Component, "_suspension_1_"))
    {
	    Result.WheelFR.Add(Component);
    }
    else if (IsChildrenOf(Component, "_suspension_2"))
    { 
        Result.WheelRL.Add(Component);
    }
    else if (IsChildrenOf(Component, "_suspension_3_"))
    {     
	    Result.WheelRR.Add(Component);
    }
    else if (ComponentName.Contains("Collision"))
    {

    }
    else
    {
	    Result.Body.Add(Component);
    }

    if(ComponentName.Contains("_glass_") ||
       IsChildrenOf(Component, "_glass_"))
    {
      GlassComponents.Add(Component);
    }
  }

  // fix glass materials not being transparent
  for (UStaticMeshComponent* Compopnent : GlassComponents)
  {
    const TArray<UMaterialInterface*>& Materials = Compopnent->GetMaterials();
    for (int32 i = 0; i < Materials.Num(); i++)
    {
      UMaterialInterface* Material = Materials[i];
      if (Material)
      {
        FString MaterialName = Material->GetName();
        if (MaterialName == "DefaultMaterial")
        {
          Compopnent->SetMaterial(i, GlassMaterial);
        }
      }
    }
  }
  return Result;
}

FMergedVehicleMeshParts UUSDImporterWidget::GenerateVehicleMeshes(
    const FVehicleMeshParts& VehicleMeshParts, const FString& DestPath)
{
  FMergedVehicleMeshParts Result;
  auto MergePart =
      [](TArray<UPrimitiveComponent*> Components, const FString& DestMeshPath)
      -> UStaticMesh*
      {
        if (!Components.Num())
        {
          return nullptr;
        }

        FString FixedPath = DestMeshPath;
#ifdef WITH_SIMREADY
        FSimReadyPathHelper::FixAssetName(FixedPath);
#endif
        TArray<UObject*> Output = MergeMeshComponents(Components, FixedPath);
        if (Output.Num())
        {
          return Cast<UStaticMesh>(Output[0]);
        }
        else
        {
          return nullptr;
        }
      };
  Result.DoorFR = MergePart(VehicleMeshParts.DoorFR, DestPath + "_door_fr");
  Result.DoorFL = MergePart(VehicleMeshParts.DoorFL, DestPath + "_door_fl");
  Result.DoorRR = MergePart(VehicleMeshParts.DoorRR, DestPath + "_door_rr");
  Result.DoorRL = MergePart(VehicleMeshParts.DoorRL, DestPath + "_door_rl");
  Result.Trunk = MergePart(VehicleMeshParts.Trunk, DestPath + "_trunk");
  Result.Hood = MergePart(VehicleMeshParts.Hood, DestPath + "_hood");
  Result.WheelFR = MergePart(VehicleMeshParts.WheelFR, DestPath + "_wheel_fr");
  Result.WheelFL = MergePart(VehicleMeshParts.WheelFL, DestPath + "_wheel_fl");
  Result.WheelRR = MergePart(VehicleMeshParts.WheelRR, DestPath + "_wheel_rr");
  Result.WheelRL = MergePart(VehicleMeshParts.WheelRL, DestPath + "_wheel_rl");
  Result.Body = MergePart(VehicleMeshParts.Body, DestPath + "_body");

  auto GenerateLODs = [](UStaticMesh* StaticMesh, int32 LODCount, float* LODScreenSize)
  {
      if (StaticMesh)
      {
          StaticMesh->Modify();
          StaticMesh->SetNumSourceModels(LODCount);

          StaticMesh->bAutoComputeLODScreenSize = false;
          for (int32 LODIndex = 0; LODIndex < LODCount; ++LODIndex)
          {
              FStaticMeshSourceModel& SrcModel = StaticMesh->GetSourceModel(LODIndex);
              SrcModel.ScreenSize.Default = LODScreenSize[LODIndex];
          }

          StaticMesh->PostEditChange();
      }
  };

  const int32 BodyLODCount = 5;
  float BodyLODScreenSize[BodyLODCount] =
  {
      1.0f,
      0.5f,
      0.3f,
      0.15f,
      0.05f,
  };

  const int32 PartLODCount = 3;
  float PartLODScreenSize[PartLODCount] =
  {
      1.0f,
      0.3f,
      0.05f,
  };

  GenerateLODs(Result.DoorFR, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.DoorFL, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.DoorRR, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.DoorRL, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.Trunk, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.Hood, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.WheelFR, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.WheelFL, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.WheelRR, PartLODCount, PartLODScreenSize);
  GenerateLODs(Result.WheelRL, PartLODCount, PartLODScreenSize);

  GenerateLODs(Result.Body, BodyLODCount, BodyLODScreenSize);

  Result.Anchors = VehicleMeshParts.Anchors;
  Result.Lights = VehicleMeshParts.Lights;
  return Result;
}

FString GetCarlaLightName(const FString &USDName)
{
  FString LowerCaseUSDName = USDName.ToLower();
  FString LightType = "";
  if (LowerCaseUSDName.Contains("headlight"))
  {
    LightType = "low_beam";
  }
  else if (LowerCaseUSDName.Contains("brakelight"))
  {
    LightType = "brake";
  }
  else if (LowerCaseUSDName.Contains("blinker"))
  {
    LightType = "blinker";
  }
  else if (LowerCaseUSDName.Contains("night"))
  {
    LightType = "high_beam";
  }
  else if (LowerCaseUSDName.Contains("reverse"))
  {
    LightType = "reverse";
  }
  else if (LowerCaseUSDName.Contains("highbeam"))
  {
    LightType = "high_beam";
  }
  else if (LowerCaseUSDName.Contains("foglight"))
  {
    LightType = "fog";
  }
  else if (LowerCaseUSDName.Contains("tailLight"))
  {
    LightType = "position";
  }
  else if (LowerCaseUSDName.Contains("emergency"))
  {
    LightType = "special1";
    return LightType + "-" + USDName;
  }
  else
  {
    return USDName;
  }

  FString FinalName = "-" + LightType + "-";
  if (LowerCaseUSDName.EndsWith("_fr"))
  {
    FinalName = "front" + FinalName + "r-";
  }
  else if (LowerCaseUSDName.EndsWith("_fl"))
  {
    FinalName = "front" + FinalName + "l-";
  }
  else if (LowerCaseUSDName.EndsWith("_rr"))
  {
    FinalName = "back" + FinalName + "r-";
  }
  else if (LowerCaseUSDName.EndsWith("_rl"))
  {
    FinalName = "back" + FinalName + "l-";
  }

  return FinalName;
}

AActor* UUSDImporterWidget::GenerateNewVehicleBlueprint(
    UWorld* World,
    UClass* BaseClass,
    USkeletalMesh* NewSkeletalMesh,
    UPhysicsAsset* NewPhysicsAsset,
    const FString &DestPath, 
    const FMergedVehicleMeshParts& VehicleMeshes,
    const FWheelTemplates& WheelTemplates)
{
  std::unordered_map<std::string, std::pair<UStaticMesh*, FVector>> MeshMap = {
    {"SM_DoorFR", {VehicleMeshes.DoorFR, VehicleMeshes.Anchors.DoorFR}},
    {"SM_DoorFL", {VehicleMeshes.DoorFL, VehicleMeshes.Anchors.DoorFL}},
    {"SM_DoorRR", {VehicleMeshes.DoorRR, VehicleMeshes.Anchors.DoorRR}},
    {"SM_DoorRL", {VehicleMeshes.DoorRL, VehicleMeshes.Anchors.DoorRL}},
    {"Trunk", {VehicleMeshes.Trunk, VehicleMeshes.Anchors.Trunk}},
    {"Hood", {VehicleMeshes.Hood, VehicleMeshes.Anchors.Hood}},
    {"Wheel_FR", {VehicleMeshes.WheelFR, FVector(0,0,0)}},
    {"Wheel_FL", {VehicleMeshes.WheelFL, FVector(0,0,0)}},
    {"Wheel_RR", {VehicleMeshes.WheelRR, FVector(0,0,0)}},
    {"Wheel_RL", {VehicleMeshes.WheelRL, FVector(0,0,0)}},
    {"Body", {VehicleMeshes.Body, VehicleMeshes.Anchors.Body}}
  };

  AActor* TemplateActor = World->SpawnActor<AActor>(BaseClass);
  // Get an replace all static meshes with the appropiate mesh
  TArray<UStaticMeshComponent*> MeshComponents;
  TemplateActor->GetComponents(MeshComponents);
  for (UStaticMeshComponent* Component : MeshComponents)
  {
    std::string ComponentName = TCHAR_TO_UTF8(*Component->GetName());
    auto &MapElement = MeshMap[ComponentName];
    UStaticMesh* ComponentMesh = MapElement.first;
    FVector MeshLocation = MapElement.second;
    if(ComponentMesh)
    {
      Component->SetStaticMesh(ComponentMesh);
      Component->SetRelativeLocation(MeshLocation);
    }
    UE_LOG(LogCarlaTools, Log, TEXT("Component name %s, name %s"),
    *UKismetSystemLibrary::GetDisplayName(Component), *Component->GetName());
  }

  // Get the skeletal mesh and modify it to match the vehicle parameters
  USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(
      TemplateActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
  if(!SkeletalMeshComponent)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Skeletal mesh component not found"));
    return nullptr;
  }
  USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
  TMap<FString, FTransform> NewBoneTransform = {
    {"Wheel_Front_Left", FTransform(VehicleMeshes.Anchors.WheelFL)},
    {"Wheel_Front_Right", FTransform(VehicleMeshes.Anchors.WheelFR)},
    {"Wheel_Rear_Right", FTransform(VehicleMeshes.Anchors.WheelRR)},
    {"Wheel_Rear_Left", FTransform(VehicleMeshes.Anchors.WheelRL)}
  };
  if(!SkeletalMesh)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Mesh not generated, skeletal mesh missing"));
    return nullptr;
  }
  bool bSuccess = EditSkeletalMeshBones(NewSkeletalMesh, NewBoneTransform);
  if (!NewSkeletalMesh || !bSuccess)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("EditSkeletalMeshes failure"));
    return nullptr;
  }
  SkeletalMeshComponent->SetSkeletalMesh(NewSkeletalMesh);

  UMaterial* Indicator = LoadObject<UMaterial>(nullptr, TEXT("/Game/Carla/Static/GenericMaterials/Vehicles/Lights/MF_Indicator.MF_Indicator"));
  UMaterial* BlueSiren = LoadObject<UMaterial>(nullptr, TEXT("/Game/Carla/Static/Car/4Wheeled/DodgeCharge/Materials/MF_Blue_Siren.MF_Blue_Siren"));
  UMaterial* RedSiren = LoadObject<UMaterial>(nullptr, TEXT("/Game/Carla/Static/Car/4Wheeled/DodgeCharge/Materials/MF_Red_Siren.MF_Red_Siren"));
  UTextureLightProfile* Defined = LoadObject<UTextureLightProfile>(nullptr, TEXT("/Game/Carla/Static/Car/IES_Profiles/Defined.Defined"));
  UTextureLightProfile* XArrowDiffuse = LoadObject<UTextureLightProfile>(nullptr, TEXT("/Game/Carla/Static/Car/IES_Profiles/XArrowDiffuse.XArrowDiffuse"));
  UTextureLightProfile* Vee = LoadObject<UTextureLightProfile>(nullptr, TEXT("/Game/Carla/Static/Car/IES_Profiles/Vee.Vee"));
  

  UE_LOG(LogCarlaTools, Log, TEXT("Num Lights %d"), VehicleMeshes.Lights.Num());
  for (const FVehicleLight& Light : VehicleMeshes.Lights)
  {
    FString FixedLightName = GetCarlaLightName(Light.Name);
    UClass * LightClass = USpotLightComponent::StaticClass();
    if (FixedLightName.Contains("special"))
    {
      LightClass = UPointLightComponent::StaticClass();
    }
    ULocalLightComponent * LightComponent = NewObject<ULocalLightComponent>(TemplateActor, LightClass, FName(*FixedLightName));
    LightComponent->RegisterComponent();
    LightComponent->AttachToComponent(
        TemplateActor->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    LightComponent->SetRelativeLocation(Light.Location); // Set the position of the light relative to the actor
    LightComponent->SetIntensityUnits(ELightUnits::Lumens);
    LightComponent->SetIntensity(1550.0f); // Set the brightness of the light
    LightComponent->SetVolumetricScatteringIntensity(0.f);
    LightComponent->SetAttenuationRadius(200.0f);
    LightComponent->SetCastShadows(false);

    if (FixedLightName.Contains("high_beam"))
    {
      USpotLightComponent* SpotLight =
          Cast<USpotLightComponent>(LightComponent);
      if (FixedLightName.Contains("-r-"))
      {
          SpotLight->SetRelativeRotation(FRotator(5.0f, 7.0f, 1.0f));
      }
      else
      {
          SpotLight->SetRelativeRotation(FRotator(5.0f, -7.0f, -1.0f));
      }
      SpotLight->SetAttenuationRadius(4000.0f);
      SpotLight->SetInnerConeAngle(20.f);
      SpotLight->SetOuterConeAngle(30.f);
      SpotLight->SetIntensity(105000.0f); // Set the brightness of the light
      SpotLight->SetVolumetricScatteringIntensity(0.025f);
      SpotLight->SetCastShadows(true);
      SpotLight->SetIESTexture(Vee);
    }
    else if (FixedLightName.Contains("low_beam"))
    {
      USpotLightComponent* SpotLight =
          Cast<USpotLightComponent>(LightComponent);
      SpotLight->SetRelativeRotation(FRotator(10.0f, 0, 0));
      SpotLight->SetAttenuationRadius(2000.0f);
      SpotLight->SetInnerConeAngle(50.f);
      SpotLight->SetOuterConeAngle(70.f);
      SpotLight->SetIntensity(35500.f); // Set the brightness of the light
      SpotLight->SetVolumetricScatteringIntensity(0.025f);
      SpotLight->SetCastShadows(true);
      SpotLight->SetIESTexture(Vee);
    }
    else if (FixedLightName.Contains("blinker"))
    {
        USpotLightComponent* SpotLight =
            Cast<USpotLightComponent>(LightComponent);
        if (FixedLightName.Contains("-r-"))
        {
            if (FixedLightName.Contains("front-"))
            {
                SpotLight->SetRelativeRotation(FRotator(-20.0f, 50.0f, 33.0f));
            }
            else
            {
                SpotLight->SetRelativeRotation(FRotator(-33.0f, -210.0f, 40.0f));
            }
        }
        else
        {
            if (FixedLightName.Contains("front-"))
            {
                SpotLight->SetRelativeRotation(FRotator(-20.0f, -50.0f, 33.0f));
            }
            else
            {
                SpotLight->SetRelativeRotation(FRotator(-33.0f, 210.0f, 40.0f));
            }
        }

        //SpotLight->SetRelativeRotation(FRotator(-3.f, 0, 0));
        SpotLight->SetInnerConeAngle(35.f);
        SpotLight->SetOuterConeAngle(70.f);
        // Setup light function material
        SpotLight->SetLightFunctionMaterial(Indicator);
        SpotLight->SetIESTexture(Defined);
    }
    else if (FixedLightName.Contains("brake"))
    {
        USpotLightComponent* SpotLight =
            Cast<USpotLightComponent>(LightComponent);
        if (FixedLightName.Contains("-r-"))
        {
            SpotLight->SetRelativeRotation(FRotator(320.0f, -191.0f, -191.0f));
        }
        else
        {
            SpotLight->SetRelativeRotation(FRotator(320.0f, 191.0f, -191.0f));
        }
        SpotLight->SetInnerConeAngle(50);
        SpotLight->SetOuterConeAngle(70.f);
        SpotLight->SetAttenuationRadius(1000.0f);
        SpotLight->SetIESTexture(XArrowDiffuse);
    }
    else if (FixedLightName.Contains("special1"))
    {
        LightComponent->SetAttenuationRadius(1000.0f);
        if (Light.Color.B == 1.0f)
        {
            LightComponent->SetLightFunctionMaterial(BlueSiren);
        }
        else if (Light.Color.R == 1.0f)
        {
            LightComponent->SetLightFunctionMaterial(RedSiren);
        }
        LightComponent->SetIntensity(50000.0f);
    }
    LightComponent->SetLightColor(Light.Color);
    TemplateActor->AddInstanceComponent(LightComponent);
    UE_LOG(LogCarlaTools, Log, TEXT("Spawn Light %s, %s, %s"), *Light.Name, *Light.Location.ToString(), *Light.Color.ToString());
  }

  TMap<FString, UStaticMesh*> ConstraintMap = {
  {"PhysConst_FR", VehicleMeshes.DoorFR},
  {"PhysConst_FL", VehicleMeshes.DoorFL},
  {"PhysConst_RR", VehicleMeshes.DoorRR},
  {"PhysConst_RL", VehicleMeshes.DoorRL},
  };

  TArray<UPhysicsConstraintComponent*> ConstraintComponents;
  TemplateActor->GetComponents(ConstraintComponents);
  for (UPhysicsConstraintComponent* Component : ConstraintComponents)
  {
	  auto ConstraintMesh = ConstraintMap.Find(Component->GetName());
	  if (ConstraintMesh && *ConstraintMesh == nullptr) // No valid static mesh, cleanup constraint
	  {
		  Component->ComponentName1.ComponentName = NAME_None;
	  }
  }

  // set the wheel radius
  UVehicleWheel* WheelDefault;
  WheelDefault = WheelTemplates.WheelFL->GetDefaultObject<UVehicleWheel>();
  if (VehicleMeshes.WheelFL)
  {
    WheelDefault->ShapeRadius = VehicleMeshes.WheelFL->GetBounds().SphereRadius;
  }
  WheelDefault = WheelTemplates.WheelFR->GetDefaultObject<UVehicleWheel>();
  if (VehicleMeshes.WheelFR)
  {
    WheelDefault->ShapeRadius = VehicleMeshes.WheelFR->GetBounds().SphereRadius;
  }
  WheelDefault = WheelTemplates.WheelRL->GetDefaultObject<UVehicleWheel>();
  if (VehicleMeshes.WheelRL)
  {
    WheelDefault->ShapeRadius = VehicleMeshes.WheelRL->GetBounds().SphereRadius;
  }
  WheelDefault = WheelTemplates.WheelRR->GetDefaultObject<UVehicleWheel>();
  if (VehicleMeshes.WheelRR)
  {
    WheelDefault->ShapeRadius = VehicleMeshes.WheelRR->GetBounds().SphereRadius;
  }
  // assign generated wheel types
  TArray<FWheelSetup> WheelSetups;
  FWheelSetup Setup;
  Setup.WheelClass = WheelTemplates.WheelFL;
  Setup.BoneName = "Wheel_Front_Left";
  WheelSetups.Add(Setup);
  Setup.WheelClass = WheelTemplates.WheelFR;
  Setup.BoneName = "Wheel_Front_Right";
  WheelSetups.Add(Setup);
  Setup.WheelClass = WheelTemplates.WheelRL;
  Setup.BoneName = "Wheel_Rear_Left";
  WheelSetups.Add(Setup);
  Setup.WheelClass = WheelTemplates.WheelRR;
  Setup.BoneName = "Wheel_Rear_Right";
  WheelSetups.Add(Setup);
  ACarlaWheeledVehicle* CarlaVehicle =
      Cast<ACarlaWheeledVehicle>(TemplateActor);
  if(CarlaVehicle)
  {
    UWheeledVehicleMovementComponent4W* MovementComponent =
        Cast<UWheeledVehicleMovementComponent4W>(
            CarlaVehicle->GetVehicleMovementComponent());
    MovementComponent->WheelSetups = WheelSetups;
  }
  else
  {
    UE_LOG(LogCarlaTools, Error, TEXT("Null CarlaVehicle"));
  }

  UBoxComponent* BoxComponent = Cast<UBoxComponent>(
      TemplateActor->GetComponentByClass(UBoxComponent::StaticClass()));
  if (BoxComponent)
  {
      TArray<FBoundingBox> Result;
      TArray<uint8> Tags;
      UBoundingBoxCalculator::GetBBsOfStaticMeshComponents(MeshComponents, Result, Tags);
      FBoundingBox BoundingBox = UBoundingBoxCalculator::CombineBBs(Result);

      const FTransform& CompToWorldTransform = CarlaVehicle->GetRootComponent()->GetComponentTransform();
      const FRotator Rotation = CompToWorldTransform.GetRotation().Rotator();
      const FVector Translation = CompToWorldTransform.GetLocation();
      const FVector Scale = CompToWorldTransform.GetScale3D();

      // Invert BB origin to local space
      BoundingBox.Origin -= Translation;
      BoundingBox.Origin = Rotation.UnrotateVector(BoundingBox.Origin);
      BoundingBox.Origin /= Scale;

      // Prepare Box Collisions
      FTransform Transform;
      Transform.SetTranslation(BoundingBox.Origin);
      BoxComponent->SetRelativeTransform(Transform);
      BoxComponent->SetBoxExtent(BoundingBox.Extent);
  }

  // Set the vehicle collision in the new physicsasset object
  UEditorStaticMeshLibrary::AddSimpleCollisions(
      VehicleMeshes.Body, EScriptingCollisionShapeType::NDOP26);
  CopyCollisionToPhysicsAsset(NewPhysicsAsset, VehicleMeshes.Body);
  // assign the physics asset to the skeletal mesh
  NewSkeletalMesh->PhysicsAsset = NewPhysicsAsset;
  // Create the new blueprint vehicle
  FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
  Params.bReplaceActor = false;
  Params.bKeepMobility = true;
  Params.bDeferCompilation = false;
  Params.bOpenBlueprint = false;
  Params.ParentClassOverride = BaseClass;
  FKismetEditorUtilities::CreateBlueprintFromActor(
      DestPath,
      TemplateActor,
      Params);
  return nullptr;
}

bool UUSDImporterWidget::EditSkeletalMeshBones(
    USkeletalMesh* NewSkeletalMesh,
    const TMap<FString, FTransform> &NewBoneTransforms)
{
  if(!NewSkeletalMesh)
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Skeletal mesh invalid"));
    return false;
  }
  FReferenceSkeleton& ReferenceSkeleton = NewSkeletalMesh->RefSkeleton;
  FReferenceSkeletonModifier SkeletonModifier(ReferenceSkeleton, NewSkeletalMesh->Skeleton);
  for (auto& Element : NewBoneTransforms)
  {
    const FString& BoneName = Element.Key;
    const FTransform& BoneTransform = Element.Value;
    int32 BoneIdx = SkeletonModifier.FindBoneIndex(FName(*BoneName));
    if (BoneIdx == INDEX_NONE)
    {
      UE_LOG(LogCarlaTools, Log, TEXT("Bone %s not found"), *BoneName);
    }
    UE_LOG(LogCarlaTools, Log, TEXT("Bone %s corresponds to index %d"), *BoneName, BoneIdx);
    SkeletonModifier.UpdateRefPoseTransform(BoneIdx, BoneTransform);
  }

  NewSkeletalMesh->MarkPackageDirty();
  UPackage* Package = NewSkeletalMesh->GetOutermost();

  const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

  UE_LOG(LogCarlaTools, Log, TEXT("Saving vehicle skeletal mesh to: %s "), *PackageFileName);

  return UPackage::SavePackage(
    Package, 
    NewSkeletalMesh, 
    EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, 
    *PackageFileName, 
    GError, 
    nullptr, 
    true, 
    true, 
    SAVE_NoError
    );
}

void UUSDImporterWidget::CopyCollisionToPhysicsAsset(
    UPhysicsAsset* PhysicsAssetToEdit, UStaticMesh* StaticMesh)
{
  UE_LOG(LogCarlaTools, Log, TEXT("Num bodysetups %d"), PhysicsAssetToEdit->SkeletalBodySetups.Num());
  UBodySetup* BodySetupPhysicsAsset = 
      PhysicsAssetToEdit->SkeletalBodySetups[
          PhysicsAssetToEdit->FindBodyIndex(FName("Vehicle_Base"))];
  UBodySetup* BodySetupStaticMesh = StaticMesh->BodySetup;
  // MUST clear existing simple collision, or bCreatedPhysicsMeshes is still enabled and won't get the valid ConvexMesh
  BodySetupPhysicsAsset->RemoveSimpleCollision();
  BodySetupPhysicsAsset->AggGeom = BodySetupStaticMesh->AggGeom;
  // rebuild physics meshes
  BodySetupPhysicsAsset->CreatePhysicsMeshes();
}
