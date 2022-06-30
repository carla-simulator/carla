// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PrepareAssetsForCookingCommandlet.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "HAL/PlatformFilemanager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Carla/MapGen/LargeMapManager.h"

static bool ValidateStaticMesh(UStaticMesh *Mesh)
{
  const FString AssetName = Mesh->GetName();

  if (AssetName.Contains(TEXT("light"), ESearchCase::IgnoreCase) ||
      AssetName.Contains(TEXT("sign"), ESearchCase::IgnoreCase))
  {
    return false;
  }

  for (int i = 0; i < Mesh->StaticMaterials.Num(); i++)
  {
    UMaterialInterface *Material = Mesh->GetMaterial(i);
    const FString MaterialName = Material->GetName();

    if (MaterialName.Contains(TEXT("light"), ESearchCase::IgnoreCase) ||
        MaterialName.Contains(TEXT("sign"), ESearchCase::IgnoreCase))
    {
      return false;
    }
  }

  return true;
}

UPrepareAssetsForCookingCommandlet::UPrepareAssetsForCookingCommandlet()
{
  // Set necessary flags to run commandlet
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;

#if WITH_EDITORONLY_DATA
  // Get Carla Default materials, which will be used at maps that require Carla materials.

  // Road materials.
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadRuralNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/MI_Road_Rural.MI_Road_Rural'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadBoulevardNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/MI_Road_Asphalt.MI_Road_Asphalt'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadAvenueNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/MI_Road_Asphalt.MI_Road_Asphalt'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadCityNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/MI_Road_Asphalt.MI_Road_Asphalt'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadElectricPoleNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/MI_Road_Asphalt.MI_Road_Asphalt'"));
  RoadRuralMaterial = (UMaterialInstance *) RoadRuralNode.Object;
  RoadBoulevardMaterial = (UMaterialInstance *) RoadBoulevardNode.Object;
  RoadAvenueMaterial = (UMaterialInstance *) RoadAvenueNode.Object;
  RoadCityMaterial = (UMaterialInstance *) RoadCityNode.Object;
  RoadElectricPoleMaterial = (UMaterialInstance *) RoadElectricPoleNode.Object;

  // Previous road CARLA material
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled_V2.M_Road_03_Tiled_V2'"));
  RoadMaterial = (UMaterialInstance *) RoadNode.Object;

  // Lane Markings.
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingYellowAsphaltNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarkingYellow_Asphalt.MI_LaneMarkingYellow_Asphalt'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingYellowResidentialNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarkingYellow_Residential.MI_LaneMarkingYellow_Residential'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingYellowRuralNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarkingYellow_Rural.MI_LaneMarkingYellow_Rural'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingAsphaltNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarking_Asphalt.MI_LaneMarking_Asphalt'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingResidentialNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarking_Residential.MI_LaneMarking_Residential'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingRuralNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarking_Rural.MI_LaneMarking_Rural'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingParkingNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LaneMarkings/MI_LaneMarking_Asphalt.MI_LaneMarking_Asphalt'"));
  MarkingYellowAsphaltMaterial = (UMaterialInstance *) MarkingYellowAsphaltNode.Object;
  MarkingYellowResidentialMaterial = (UMaterialInstance *) MarkingYellowResidentialNode.Object;
  MarkingYellowRuralMaterial = (UMaterialInstance *) MarkingYellowRuralNode.Object;
  MarkingAsphaltMaterial = (UMaterialInstance *) MarkingAsphaltNode.Object;
  MarkingResidentialMaterial = (UMaterialInstance *) MarkingResidentialNode.Object;
  MarkingRuralMaterial = (UMaterialInstance *) MarkingRuralNode.Object;
  MarkingParkingMaterial = (UMaterialInstance *) MarkingParkingNode.Object;

  // Previous lane marking CARLA material
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingYellowNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled_V3.M_Road_03_Tiled_V3'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingNode(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/M_Road_03_LMW.M_Road_03_LMW'"));
  MarkingYellowMaterial = (UMaterialInstance *) MarkingYellowNode.Object;
  MarkingMaterial = (UMaterialInstance *) MarkingNode.Object;

  // Curb and Gutter materials
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CurbNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_curb/MI_largeM_curb01.MI_largeM_curb01'"));
  CurbMaterial = (UMaterialInstance *) CurbNode.Object;

  // Gutter
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> GutterNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_gutter/MI_largeM_gutter01.MI_largeM_gutter01'"));
  GutterMaterial = (UMaterialInstance *) GutterNode.Object;

  // Sidewalk
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkResidentialNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Residential.MI_Sidewalk_Residential'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkResidentialGrassNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Residential_Grass.MI_Sidewalk_Residential_Grass'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkSkyscrapersNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Skyscrapper.MI_Sidewalk_Skyscrapper'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkApartmentNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkCommunityNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Community.MI_Sidewalk_Community'"));
  SidewalkResidentialMaterial = (UMaterialInstance *) SidewalkResidentialNode.Object;
  SidewalkResidentialGrassMaterial = (UMaterialInstance *) SidewalkResidentialGrassNode.Object;
  SidewalkSkyscrapersMaterial = (UMaterialInstance *) SidewalkSkyscrapersNode.Object;
  SidewalkApartmentMaterial = (UMaterialInstance *) SidewalkApartmentNode.Object;
  SidewalkCommunityMaterial = (UMaterialInstance *) SidewalkCommunityNode.Object;

  // Previous sidewalk CARLA material
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_sidewalk/tile01/MI_largeM_tile02.MI_largeM_tile02'"));
  SidewalkMaterial = (UMaterialInstance *) SidewalkNode.Object;

  // Terrain
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainHighwayNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Highway.MI_LargeLandscape_Highway'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainInterurbanNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Interurban.MI_LargeLandscape_Interurban'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainMountainNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Mountain.MI_LargeLandscape_Mountain'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainRuralNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Rural.MI_LargeLandscape_Rural'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainGrassNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainResidentialNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_Grass_Cutted.MI_Grass_Cutted'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainParkNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Ground/MI_Grass_Park.MI_Grass_Park'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainWaterLakeNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Water/MI_Water_Lake.MI_Water_Lake'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainSkyscrapersNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Skyscrapper.MI_Sidewalk_Skyscrapper'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainApartmentNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Apartment.MI_Sidewalk_Apartment'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainCommunityNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Sidewalk/MI_Sidewalk_Community.MI_Sidewalk_Community'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> BridgeWallNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Wall/MI_WallBridge.MI_WallBridge'"));
  TerrainHighwayMaterial = (UMaterialInstance *) TerrainHighwayNode.Object;
  TerrainInterurbanMaterial = (UMaterialInstance *) TerrainInterurbanNode.Object;
  TerrainMountainMaterial = (UMaterialInstance *) TerrainMountainNode.Object;
  TerrainRuralMaterial = (UMaterialInstance *) TerrainRuralNode.Object;
  TerrainGrassMaterial = (UMaterialInstance *) TerrainGrassNode.Object;
  TerrainWaterLakeMaterial = (UMaterialInstance *) TerrainWaterLakeNode.Object;
  TerrainParkMaterial = (UMaterialInstance *) TerrainParkNode.Object;
  TerrainSkyscrapersMaterial = (UMaterialInstance *) TerrainSkyscrapersNode.Object;
  TerrainApartmentMaterial = (UMaterialInstance *) TerrainApartmentNode.Object;
  TerrainCommunityMaterial = (UMaterialInstance *) TerrainCommunityNode.Object;
  TerrainResidentialMaterial = (UMaterialInstance *) TerrainResidentialNode.Object;
  BridgeWallMaterial = (UMaterialInstance *) BridgeWallNode.Object;

  // Previous terrain CARLA material
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Large_Maps/materials/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass'"));
  TerrainMaterial = (UMaterialInstance *) TerrainNode.Object;

  // Guardrail
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> GuardrailPostNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GuardRail/Materials/GuardRail/MI_GuardRailPost.MI_GuardRailPost'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> GuardrailMetalNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GuardRail/Materials/GuardRail/MI_GuardRail.MI_GuardRail'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> BridgeGuardrailPostNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GuardRail/Materials/GuardRail/MI_GuardRail.MI_GuardRail'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> BridgeGuardrailMetalNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GuardRail/Materials/GuardRail/MI_GuardRail.MI_GuardRail'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> BridgeGuardrailBaseNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Concrete/MI_BaseBridgeRailling.MI_BaseBridgeRailling'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SafetyWallHighwayNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/Wall/MI_SafetyWallHighWay.MI_SafetyWallHighWay'"));
  GuardrailPostMaterial = (UMaterialInstance *) GuardrailPostNode.Object;
  GuardrailMetalMaterial = (UMaterialInstance *) GuardrailMetalNode.Object;
  BridgeGuardrailPostMaterial = (UMaterialInstance *) BridgeGuardrailPostNode.Object;
  BridgeGuardrailMetalMaterial = (UMaterialInstance *) BridgeGuardrailMetalNode.Object;
  BridgeGuardrailBaseMaterial = (UMaterialInstance *) BridgeGuardrailBaseNode.Object;
  SafetyWallHighwayMaterial = (UMaterialInstance *) SafetyWallHighwayNode.Object;

#endif
}
#if WITH_EDITORONLY_DATA

FPackageParams UPrepareAssetsForCookingCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;
  TMap<FString, FString> ParamVals;

  ParseCommandLine(*InParams, Tokens, Params);

  FPackageParams PackageParams;

  // Parse and store Package name
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  // Parse and store flag for only preparing maps
  FParse::Bool(*InParams, TEXT("OnlyPrepareMaps="), PackageParams.bOnlyPrepareMaps);
  return PackageParams;
}

void UPrepareAssetsForCookingCommandlet::LoadWorld(FAssetData &AssetData)
{
  // BaseMap path inside Carla
  const FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");

  // Load Map folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // Extract first asset found in folder path (i.e. the BaseMap)
    AssetData = AssetDatas.Pop();
  }
}

void UPrepareAssetsForCookingCommandlet::LoadWorldTile(FAssetData &AssetData)
{
  // BaseTile path inside Carla
  const FString BaseTile = TEXT("/Game/Carla/Maps/TestMaps");

  // Load Map folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseTile);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // Extract first asset found in folder path (i.e. the BaseTile)
    AssetData = AssetDatas.Pop();
  }
}

void UPrepareAssetsForCookingCommandlet::LoadLargeMapWorld(FAssetData &AssetData)
{
  // BaseMap path inside Carla
  const FString BaseMap = TEXT("/Game/Carla/Maps/BaseLargeMap");

  // Load Map folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // Extract first asset found in folder path (i.e. the BaseMap)
    AssetData = AssetDatas.Pop();
  }
}

TArray<AStaticMeshActor *> UPrepareAssetsForCookingCommandlet::SpawnMeshesToWorld(
    const TArray<FString> &AssetsPaths,
    bool bUseCarlaMaterials,
    int i,
    int j)
{
  TArray<AStaticMeshActor *> SpawnedMeshes;

  // Create default Transform for all assets to spawn
  const FTransform ZeroTransform = FTransform();

  // Load assets specified in AssetsPaths by using an object library
  // for building map world
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;

  // name of current tile to cook
  FString TileName;
  if (i != -1)
  {
    TileName = FString::Printf(TEXT("_Tile_%d_%d"), i, j);
  }

  // try to get the name of the map that precedes all assets name
  FString AssetName;
  for (auto MapAsset : MapContents)
  {
    // Spawn Static Mesh
    MeshAsset = Cast<UStaticMesh>(MapAsset.GetAsset());
    if (MeshAsset && ValidateStaticMesh(MeshAsset))
    {
      // get asset name
      MapAsset.AssetName.ToString(AssetName);

      // check to ignore meshes from other tiles
      if (i == -1 || (i != -1 && (AssetName.EndsWith(TileName) || AssetName.Contains(TileName + "_"))))
      {
        MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ZeroTransform);
        UStaticMeshComponent *MeshComponent = MeshActor->GetStaticMeshComponent();
        MeshComponent->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));
        MeshActor->SetActorLabel(AssetName, true);

        // set complex collision as simple in asset
        UBodySetup *BodySetup = MeshAsset->BodySetup;
        if (BodySetup)
        {
          BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
          MeshAsset->MarkPackageDirty();
        }

        SpawnedMeshes.Add(MeshActor);

        if (bUseCarlaMaterials)
        {
          std::cout << "> Importing asset: " << TCHAR_TO_UTF8(*AssetName) << std::endl;
          // Set Carla Materials depending on RoadRunner's Semantic Segmentation tag and material name
          if (AssetName.Contains(SSTags::R_ROAD1) || AssetName.Contains(SSTags::R_ROAD2))
          {
            std::cout << ">> Asset is a Road" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName);
              if (MaterialName.Contains("RuralRoad_Road"))
              { // Worn out road with less asphalt and more gravel
                std::cout << " -> Rural Road" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, RoadRuralMaterial);
              }
              else if (MaterialName.Contains("Asphalt1_Road"))
              { // City road made of asphalt
                std::cout << " -> Asphalt Road" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, RoadCityMaterial);
              }
              else if (MaterialName.Contains("BoulevardStreet_Road"))
              { // City road made of asphalt. Used by the procedural decoration
                std::cout << " -> Boulevard Road" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, RoadBoulevardMaterial);
              }
              else if (MaterialName.Contains("AvenueStreet_Road"))
              { // City road made of asphalt. Used by the procedural decoration
                std::cout << " -> Avenue Road" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, RoadAvenueMaterial);
              }
              else if (MaterialName.Contains("ElectricPole_Road"))
              { // City road made of asphalt. Used by the procedural decoration
                std::cout << " -> Electric Pole Road" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, RoadElectricPoleMaterial);
              }
              else
              { // City road made of asphalt. Used by the procedural decoration
                std::cout << " -> Default Road???" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, RoadMaterial);
              }
            }
          }
          else if (AssetName.Contains(SSTags::R_MARKING1) || AssetName.Contains(SSTags::R_MARKING2))
          {
            std::cout << ">> Asset is a Lane Marking" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName);
              if (MaterialName.Contains("LaneMarking1_Marking"))
              { // White marking part of asphalt roads, clearly visible
                std::cout << " -> Asphalt Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingAsphaltMaterial);
              }
              else if (MaterialName.Contains("LaneMarking2_Marking"))
              { // White marking part at the residential roads, partially worn out
                std::cout << " -> Residential Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingResidentialMaterial);
              }
              else if (MaterialName.Contains("LaneMarking3_Marking"))
              { // White marking part at the rural roads, heavily worn out
                std::cout << " -> Rural Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingRuralMaterial);
              }
              else if (MaterialName.Contains("LaneMarkingYellow1_Marking"))
              { // Yellow marking part of asphalt roads, clearly visible
                std::cout << " -> Asphalt Yellow Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingYellowAsphaltMaterial);
              }
              else if (MaterialName.Contains("LaneMarkingYellow2_Marking"))
              { // Yellow marking part at the residential roads, partially worn out
                std::cout << " -> Residential Yellow Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingYellowResidentialMaterial);
              }
              else if (MaterialName.Contains("LaneMarkingYellow3_Marking"))
              { // Yellow marking part at the rural roads, heavily worn out
                std::cout << " -> Rural Yellow Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingYellowRuralMaterial);
              }
              else if (MaterialName.Contains("LaneMarkingParking_Marking"))
              { // Yellow marking part at the rural roads, heavily worn out
                std::cout << " -> Parking Lane Marking" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingParkingMaterial);
              }
              else if (MaterialName.Contains("Yellow"))
              {
                std::cout << " -> Default Yellow Lane Marking???" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingYellowMaterial);
              }
              else
              {
                std::cout << " -> Default White Lane Marking???" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, MarkingMaterial);
              }
            }
          }
          else if (AssetName.Contains(SSTags::R_CURB1) || AssetName.Contains(SSTags::R_CURB2))
          {
            std::cout << ">> Asset is a Curb" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName) << " -> Universal Curb" << std::endl;
              MeshActor->GetStaticMeshComponent()->SetMaterial(i, CurbMaterial);
            }
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_GUTTER1) || AssetName.Contains(SSTags::R_GUTTER2))
          {
            std::cout << ">> Asset is a Gutter" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName) << " -> Universal Gutter" << std::endl;
              MeshActor->GetStaticMeshComponent()->SetMaterial(i, GutterMaterial);
            }
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_SIDEWALK1) || AssetName.Contains(SSTags::R_SIDEWALK2))
          {
            std::cout << ">> Asset is a Sidewalk" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName);
              if (MaterialName.Contains("Sidewalk_Residential_Sidewalk"))
              { // Sidewalk with big squares of cement
                std::cout << " -> Residential Sidewalk" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkResidentialMaterial);
              }
              else if (MaterialName.Contains("Residential_Sidewalk"))
              {
                std::cout << " -> Residential Sidewalk" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkResidentialMaterial);
              }
              else if (MaterialName.Contains("Residential_path_Sidewalk"))
              {
                std::cout << " -> Residential Path Sidewalk" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkResidentialGrassMaterial);
              }
              else if (MaterialName.Contains("Sidewalk_Skyscraper_rrx_Sidewalk"))
              { // Sidewalk with more decorations
                std::cout << " -> Skyscraper Sidewalk" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkSkyscrapersMaterial);
              }
              else if (MaterialName.Contains("Sidewalk_Apartment_Sidewalk"))
              {
                std::cout << " -> Apartment Sidewalk" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkApartmentMaterial);
              }
              else if (MaterialName.Contains("Sidewalk_Comunity_Sidewalk"))
              { // Worn out sidewalk
                std::cout << " -> Community Sidewalk" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkCommunityMaterial);
              }
              else
              {
                std::cout << " -> Default Sidewalk???" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SidewalkMaterial);
              }
            }
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_TERRAIN))
          {
            std::cout << ">> Asset is a Terrain" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName);
              if (MaterialName.Contains("HighwayZone_Ground"))
              {  // Rocky ground with some grass
                std::cout << " -> Highway Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainHighwayMaterial);
              }
              else if (MaterialName.Contains("Interurban_Ground"))
              {  // Ground with a lot of grass and some ground
                std::cout << " -> Interurban Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainInterurbanMaterial);
              }
              else if (MaterialName.Contains("Mountain_Ground"))
              {  // A bit of rocky ground, a bit of grass, all mixed together
                std::cout << " -> Mountain Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainMountainMaterial);
              }
              else if (MaterialName.Contains("Rural_Ground"))
              {  // Ground with a more rural feel
                std::cout << " -> Rural Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainRuralMaterial);
              }
              else if (MaterialName.Contains("Grass2_rrx_Ground"))
              {  // Grass with other very small vegetation
                std::cout << " -> Grass Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainGrassMaterial);
              }
              else if (MaterialName.Contains("Water1_rrx_Ground"))
              {  // Water, part of a lake, moving on
                std::cout << " -> Water Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainWaterLakeMaterial);
              }
              else if (MaterialName.Contains("Park_Ground"))
              {  // Carefully gardened grass with some flora
                std::cout << " -> Park Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainParkMaterial);
              }
              else if (MaterialName.Contains("Skyscraper_rrx_Ground"))
              {  // Same as its sidewalk counterpart
                std::cout << " -> Skyscraper Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainSkyscrapersMaterial);
              }
              else if (MaterialName.Contains("Apartment_Ground"))
              {  // Same as its sidewalk counterpart
                std::cout << " -> Apartment Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainApartmentMaterial);
              }
              else if (MaterialName.Contains("Comunity_Ground"))
              {  // Same as its sidewalk counterpart
                std::cout << " -> Coomunity Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainCommunityMaterial);
              }
              else if (MaterialName.Contains("Residential_Ground"))
              {  // Same as its sidewalk counterpart
                std::cout << " -> Residential Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainResidentialMaterial);
              }
              else if (MaterialName.Contains("Concrete2"))
              {  // Same as its sidewalk counterpart
                std::cout << " -> Bridge Wall Terrain" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, BridgeWallMaterial);
              }
              else
              {
                std::cout << " -> Default Terrain???" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, TerrainMaterial);
              }
            }
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_PROP))
          {
            // This might missidentify some props if the materials are the same as the guardrail ones
            std::cout << ">> Asset is a Prop" << std::endl;
            for (int32 i = 0; i < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials.Num(); ++i)
            {
              auto MaterialName = MeshActor->GetStaticMeshComponent()->GetStaticMesh()->StaticMaterials[i].ImportedMaterialSlotName.ToString();
              std::cout << ">>> Material name: " << TCHAR_TO_UTF8(*MaterialName);
              if (MaterialName.Contains("M_GuardRail_Prop"))
              {  // Rocky ground with some grass
                std::cout << " -> Guardrail Post" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, GuardrailPostMaterial);
              }
              else if (MaterialName.Contains("Metal1_Marking"))
              {  // Ground with a lot of grass and some ground
                std::cout << " -> Guardrail Metal" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, GuardrailMetalMaterial);
              }
              else if (MaterialName.Contains("Metal_Road"))
              {  // Ground with a lot of grass and some ground
                std::cout << " -> Bridge Guardrail Post" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, BridgeGuardrailPostMaterial);
              }
              else if (MaterialName.Contains("Metal1_Prop"))
              {  // Ground with a lot of grass and some ground
                std::cout << " -> Bridge Guardrail Metal" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, BridgeGuardrailMetalMaterial);
              }
              else if (MaterialName.Contains("Concrete1_Prop") || MaterialName.Contains("Base_BridgeRalling_Prop"))
              {  // Ground with a lot of grass and some ground
                std::cout << " -> Bridge Guardrail Base" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, BridgeGuardrailBaseMaterial);
              }
              else if (MaterialName.Contains("Concrete1_Marking") || MaterialName.Contains("Concrete2_rrx_marking"))
              {  // Ground with a lot of grass and some ground
                std::cout << " -> Highway Wall" << std::endl;
                MeshActor->GetStaticMeshComponent()->SetMaterial(i, SafetyWallHighwayMaterial);
              }
              else
              {
                std::cout << " -> Unidentified Prop???" << std::endl;
              }
            }
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
        }
      }
    }
  }

  // Clear loaded assets in library
  AssetsObjectLibrary->ClearLoaded();

  // Mark package dirty
  World->MarkPackageDirty();

  return SpawnedMeshes;
}

bool UPrepareAssetsForCookingCommandlet::IsMapInTiles(const TArray<FString> &AssetsPaths)
{
  // Load assets specified in AssetsPaths by using an object library
  // for building map world
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;

  FString AssetName;
  bool Found = false;
  for (auto MapAsset : MapContents)
  {
    // Spawn Static Mesh
    MeshAsset = Cast<UStaticMesh>(MapAsset.GetAsset());
    if (MeshAsset && ValidateStaticMesh(MeshAsset))
    {
      // get asset name
      MapAsset.AssetName.ToString(AssetName);

      // check if the asset is a tile
      if (AssetName.Contains("_Tile_"))
      {
        Found = true;
        break;
      }
    }
  }

  // Clear loaded assets in library
  AssetsObjectLibrary->ClearLoaded();

  return Found;
}

void UPrepareAssetsForCookingCommandlet::DestroySpawnedActorsInWorld(
    TArray<AStaticMeshActor *> &SpawnedActors)
{
  // Destroy all spawned actors
  for (auto Actor : SpawnedActors)
  {
    Actor->Destroy();
  }

  // Mark package dirty
  World->MarkPackageDirty();
}

bool UPrepareAssetsForCookingCommandlet::SaveWorld(
    FAssetData &AssetData,
    const FString &PackageName,
    const FString &DestPath,
    const FString &WorldName,
    bool bGenerateSpawnPoints)
{
  // Create Package to save
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*WorldName);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Renaming map
  World->Rename(*WorldName, World->GetOuter());
  const FString PackagePath = DestPath + "/" + WorldName;
  FAssetRegistryModule::AssetRenamed(World, *PackagePath);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Check if OpenDrive file exists
  const FString PathXODR = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") +
      WorldName + TEXT("/OpenDrive/") + WorldName + TEXT(".xodr");

  bool bPackageSaved = false;
  if (FPaths::FileExists(PathXODR) && bGenerateSpawnPoints)
  {
    // We need to spawn OpenDrive assets before saving the map
    AOpenDriveActor *OpenWorldActor = CastChecked<AOpenDriveActor>(
        World->SpawnActor(AOpenDriveActor::StaticClass(),
        new FVector(),
        NULL));

    OpenWorldActor->BuildRoutes(WorldName);
    OpenWorldActor->AddSpawners();

    bPackageSaved = SavePackage(PackagePath, Package);

    // We need to destroy OpenDrive assets once saved the map
    OpenWorldActor->RemoveRoutes();
    OpenWorldActor->RemoveSpawners();
    OpenWorldActor->Destroy();
  }
  else
  {
    bPackageSaved = SavePackage(PackagePath, Package);
  }

  return bPackageSaved;
}

FString UPrepareAssetsForCookingCommandlet::GetFirstPackagePath(const FString &PackageName) const
{
  // Get all Package names
  TArray<FString> PackageList;
  IFileManager::Get().FindFilesRecursive(PackageList, *(FPaths::ProjectContentDir()),
      *(PackageName + TEXT(".Package.json")), true, false, false);

  if (PackageList.Num() == 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Package json file not found."));
    return {};
  }

  return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*PackageList[0]);
}

FAssetsPaths UPrepareAssetsForCookingCommandlet::GetAssetsPathFromPackage(const FString &PackageName) const
{
  const FString PackageJsonFilePath = GetFirstPackagePath(PackageName);

  FAssetsPaths AssetsPaths;

  // Get All Maps Path
  FString MapsFileJsonContent;
  if (FFileHelper::LoadFileToString(MapsFileJsonContent, *PackageJsonFilePath))
  {
    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(MapsFileJsonContent);
    if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
      // Add Maps Path
      auto MapsJsonArray = JsonParsed->GetArrayField(TEXT("maps"));

      for (auto &MapJsonValue : MapsJsonArray)
      {
        TSharedPtr<FJsonObject> MapJsonObject = MapJsonValue->AsObject();

        FMapData MapData;
        MapData.Name = MapJsonObject->GetStringField(TEXT("name"));
        MapData.Path = MapJsonObject->GetStringField(TEXT("path"));
        MapData.bUseCarlaMapMaterials = MapJsonObject->GetBoolField(TEXT("use_carla_materials"));

        AssetsPaths.MapsPaths.Add(std::move(MapData));
      }

      // Add Props Path
      auto PropJsonArray = JsonParsed->GetArrayField(TEXT("props"));

      for (auto &PropJsonValue : PropJsonArray)
      {
        TSharedPtr<FJsonObject> PropJsonObject = PropJsonValue->AsObject();

        const FString PropAssetPath = PropJsonObject->GetStringField(TEXT("path"));

        AssetsPaths.PropsPaths.Add(std::move(PropAssetPath));
      }
    }
  }
  return AssetsPaths;
}

bool SaveStringTextToFile(
    FString SaveDirectory,
    FString FileName,
    FString SaveText,
    bool bAllowOverWriting)
{
  IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  // CreateDirectoryTree returns true if the destination
  // directory existed prior to call or has been created
  // during the call.
  if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
  {
    // Get absolute file path
    const FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

    // Allow overwriting or file doesn't already exist
    if (bAllowOverWriting || !PlatformFile.FileExists(*AbsoluteFilePath))
    {
      FFileHelper::SaveStringToFile(SaveText, *AbsoluteFilePath);
    }
  }
  return true;
}

bool UPrepareAssetsForCookingCommandlet::SavePackage(const FString &PackagePath, UPackage *Package) const
{
  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackagePath,
      FPackageName::GetMapPackageExtension());

  if (FPaths::FileExists(*PackageFileName))
  {
    // Will not save package if it already exists
    return false;
  }

  return UPackage::SavePackage(
      Package,
      World,
      EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName,
      GError,
      nullptr,
      true,
      true,
      SAVE_NoError);
}

void UPrepareAssetsForCookingCommandlet::GenerateMapPathsFile(
    const FAssetsPaths &AssetsPaths,
    const FString &PropsMapPath)
{
  FString MapPathData;
  FString MapPathDataLinux;
  IFileManager &FileManager = IFileManager::Get();
  for (const auto &Map : AssetsPaths.MapsPaths)
  {
    MapPathData.Append(Map.Path + TEXT("/") + Map.Name + TEXT("\n"));
    MapPathDataLinux.Append(Map.Path + TEXT("/") + Map.Name + TEXT("+"));
    TArray<FAssetData> AssetsData;
    UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), true, true);
    ObjectLibrary->LoadAssetDataFromPath(Map.Path);
    ObjectLibrary->GetAssetDataList(AssetsData);
    int NumTiles = 0;
    for (FAssetData &AssetData : AssetsData)
    {
      FString AssetName = AssetData.AssetName.ToString();
      if (AssetName.Contains(Map.Name + "_Tile_"))
      {
        MapPathData.Append(Map.Path + TEXT("/") + AssetName + TEXT("\n"));
        MapPathDataLinux.Append(Map.Path + TEXT("/") + AssetName + TEXT("+"));
        NumTiles++;
      }
    }
    UE_LOG(LogTemp, Warning, TEXT("Found %d tiles"), NumTiles);
  }

  if (!PropsMapPath.IsEmpty())
  {
    MapPathData.Append(PropsMapPath + TEXT("/PropsMap"));
    MapPathDataLinux.Append(PropsMapPath + TEXT("/PropsMap"));
  }
  else
  {
    MapPathDataLinux.RemoveFromEnd(TEXT("+"));
  }

  const FString SaveDirectory = FPaths::ProjectContentDir();
  const FString FileName = FString("MapPaths.txt");
  const FString FileNameLinux = FString("MapPathsLinux.txt");
  SaveStringTextToFile(SaveDirectory, FileName, MapPathData, true);
  SaveStringTextToFile(SaveDirectory, FileNameLinux, MapPathDataLinux, true);
}

void UPrepareAssetsForCookingCommandlet::GeneratePackagePathFile(const FString &PackageName)
{
  FString SaveDirectory = FPaths::ProjectContentDir();
  FString FileName = FString("PackagePath.txt");
  FString PackageJsonFilePath = GetFirstPackagePath(PackageName);
  SaveStringTextToFile(SaveDirectory, FileName, PackageJsonFilePath, true);
}

void UPrepareAssetsForCookingCommandlet::PrepareMapsForCooking(
    const FString &PackageName,
    const TArray<FMapData> &MapsPaths)
{
  FString BasePath = TEXT("/Game/") + PackageName + TEXT("/Static/");

  for (const auto &Map : MapsPaths)
  {
    const FString MapPath = TEXT("/") + Map.Name;

    const FString DefaultPath   = TEXT("/Game/") + PackageName + TEXT("/Maps/") + Map.Name;
    const FString RoadsPath     = BasePath + SSTags::ROAD      + MapPath;
    const FString RoadLinesPath = BasePath + SSTags::ROADLINE  + MapPath;
    const FString TerrainPath   = BasePath + SSTags::TERRAIN   + MapPath;
    const FString SidewalkPath  = BasePath + SSTags::SIDEWALK  + MapPath;

    // Spawn assets located in semantic segmentation folders
    TArray<FString> DataPath = {DefaultPath, RoadsPath, RoadLinesPath, TerrainPath, SidewalkPath};

    // check whether we have a single map or a map in tiles
    if (!IsMapInTiles(DataPath))
    {
      UE_LOG(LogTemp, Log, TEXT("Cooking map"));
      // Load World
      FAssetData AssetData;
      LoadWorld(AssetData);
      UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }
      // try to cook the whole map (no tiles)
      TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials, -1, -1);
      // Save the World in specified path
      SaveWorld(AssetData, PackageName, Map.Path, Map.Name);
      // Remove spawned actors from world to keep equal as BaseMap
      DestroySpawnedActorsInWorld(SpawnedActors);
    }
    else
    {
      TArray<TPair<FString, FIntVector>> MapPathsIds;

      FVector PositionTile0 = FVector();
      float TileSize = 200000.f;
      FString TxtFile;
      FString TilesInfoPath = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") + Map.Name + "/TilesInfo.txt";
      UE_LOG(LogTemp, Warning, TEXT("Loading %s ..."), *TilesInfoPath);
      if (FFileHelper::LoadFileToString(TxtFile, *(TilesInfoPath)) == true) {

        TArray<FString> Out;
        TxtFile.ParseIntoArray(Out, TEXT(","), true);
        if (Out.Num() >= 3)
        {
          const float METERSTOCM = 100.f;
          PositionTile0.X = METERSTOCM * FCString::Atof(*Out[0]);
          PositionTile0.Y = METERSTOCM * FCString::Atof(*Out[1]);
          TileSize = METERSTOCM * FCString::Atof(*Out[2]);
        }
        else
        {
          UE_LOG(LogTemp, Warning, TEXT("TilesInfo.txt format is invalid file"));
        }
      }
      else {
        UE_LOG(LogTemp, Warning, TEXT("Could not find TilesInfo.txt file"));
      }

      UE_LOG(LogTemp, Log, TEXT("Cooking tiles:"));
      // Load World
      FAssetData AssetData;
      LoadWorldTile(AssetData);
      UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }
      // try to create each possible tile of the map
      int  i, j;
      bool Res;
      j = 0;
      do
      {
        i = 0;
        do
        {
          // Spawn
          TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials, i, j);
          Res = SpawnedActors.Num() > 0;
          if (Res)
          {
            UE_LOG(LogTemp, Log, TEXT(" Tile %d,%d found"), i, j);
            FString TileName;
            TileName = FString::Printf(TEXT("%s_Tile_%d_%d"), *Map.Name, i, j);
            // Save the World in specified path
            // UE_LOG(LogTemp, Log, TEXT("Saving as %s to %s"), *TileName, *Map.Path);
            SaveWorld(AssetData, PackageName, Map.Path, TileName);
            MapPathsIds.Add(
                TPair<FString, FIntVector>(
                  Map.Path + "/" + TileName, FIntVector(i, j, 0)));
            // Remove spawned actors from world to keep equal as BaseMap
            DestroySpawnedActorsInWorld(SpawnedActors);
            ++i;
          }
        }
        while (Res);
        ++j;
      }
      while (i > 0);

      #if WITH_EDITOR
        UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
      #endif
      // Load base map for tiled maps
      LoadLargeMapWorld(AssetData);
      BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }

      // Generate Large Map Manager
      ALargeMapManager* LargeMapManager = World->SpawnActor<ALargeMapManager>(
          ALargeMapManager::StaticClass(), FTransform());
      LargeMapManager->LargeMapTilePath = Map.Path;
      LargeMapManager->LargeMapName = Map.Name;
      LargeMapManager->SetTile0Offset(PositionTile0);
      LargeMapManager->SetTileSize(TileSize);
      LargeMapManager->GenerateMap(MapPathsIds);

      SaveWorld(AssetData, PackageName, Map.Path, Map.Name, false);

      UE_LOG(LogTemp, Log, TEXT("End cooking tiles"));
    }
  }
}

void UPrepareAssetsForCookingCommandlet::PreparePropsForCooking(
    FString &PackageName,
    const TArray<FString> &PropsPaths,
    FString &MapDestPath)
{
  // Load World
  FAssetData AssetData;
  // Loads the BaseMap
  LoadWorld(AssetData);
  UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
  if (BaseMapRedirector != nullptr) {
    World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
  }
  else {
    World = CastChecked<UWorld>(AssetData.GetAsset());
  }

  // Remove the meshes names from the original path for props, so we can load
  // props inside folder
  TArray<FString> PropPathDirs = PropsPaths;

  for (auto &PropPath : PropPathDirs)
  {
    PropPath.Split(TEXT("/"), &PropPath, nullptr,
        ESearchCase::Type::IgnoreCase, ESearchDir::Type::FromEnd);
  }

  // Add props in a single Base Map
  TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(PropPathDirs, false);

  const FString MapName("PropsMap");
  SaveWorld(AssetData, PackageName, MapDestPath, MapName);

  DestroySpawnedActorsInWorld(SpawnedActors);
  MapObjectLibrary->ClearLoaded();
}

int32 UPrepareAssetsForCookingCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // Get Props and Maps Path
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  if (PackageParams.bOnlyPrepareMaps)
  {
    PrepareMapsForCooking(PackageParams.Name, AssetsPaths.MapsPaths);
  }
  else
  {
    FString PropsMapPath("");

    if (AssetsPaths.PropsPaths.Num() > 0)
    {
      PropsMapPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Maps/PropsMap");
      PreparePropsForCooking(PackageParams.Name, AssetsPaths.PropsPaths, PropsMapPath);
    }

    // Save Map Path File for further use
    GenerateMapPathsFile(AssetsPaths, PropsMapPath);

    // Saves Package path for further use
    GeneratePackagePathFile(PackageParams.Name);
  }

#if WITH_EDITOR
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

  return 0;
}
#endif