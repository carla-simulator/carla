// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LoadAssetMaterialsCommandlet.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"

ULoadAssetMaterialsCommandlet::ULoadAssetMaterialsCommandlet()
{
	// Set necessary flags to run commandlet
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

#if WITH_EDITORONLY_DATA

	static ConstructorHelpers::FObjectFinder<UBlueprint> RoadPainterBlueprint(TEXT(
		"Blueprint'/Game/Carla/Blueprints/LevelDesign/RoadPainterPreset.RoadPainterPreset'"));

	RoadPainterSubclass = (UClass*)RoadPainterBlueprint.Object->GeneratedClass;
	
#endif
}

#if WITH_EDITORONLY_DATA

void ULoadAssetMaterialsCommandlet::ApplyRoadPainterMaterials()
{

	ARoadPainterWrapper *RoadPainterBp = World->SpawnActor<ARoadPainterWrapper>(RoadPainterSubclass);
	if (RoadPainterBp)
	{
		//Needed to call events in editor-mode
		FEditorScriptExecutionGuard ScriptGuard;
    RoadPainterBp->ClearAllEvent();
		RoadPainterBp->ZSizeEvent();
    RoadPainterBp->ModifyRoadMaterialParameters();
    RoadPainterBp->SetBlueprintVariables();
	}
}

FString ULoadAssetMaterialsCommandlet::GetFirstPackagePath(const FString &PackageName) const
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

void ULoadAssetMaterialsCommandlet::LoadAssetsMaterials(const FString &PackageName, const TArray<FMapData> &MapsPaths)
{
  // Load World
  FAssetData AssetData;
  for (const auto &Map : MapsPaths)
  {
  
    const FString DefaultPath = TEXT("/Game/") + PackageName + TEXT("/Maps/") + Map.Name;
    MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
    MapObjectLibrary->AddToRoot();
    MapObjectLibrary->LoadAssetDataFromPath(*DefaultPath);
    MapObjectLibrary->LoadAssetsFromAssetData();
    MapObjectLibrary->GetAssetDataList(AssetDatas);
    if (AssetDatas.Num() > 0)
    {
        // Extract first asset found in folder path (i.e. the imported map)
        AssetData = AssetDatas.Pop();
        UE_LOG(LogTemp, Log, TEXT("The name of the asset : %s"), *AssetData.GetFullName());
    }
    
    World = CastChecked<UWorld>(AssetData.GetAsset());
    World->InitWorld();
    ApplyRoadPainterMaterials();
  }
}

FPackageParams ULoadAssetMaterialsCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;
  TMap<FString, FString> ParamVals;
  
  ParseCommandLine(*InParams, Tokens, Params);
  
  FPackageParams PackageParams;
  
  // Parse and store Package name
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  return PackageParams;
}

FAssetsPaths ULoadAssetMaterialsCommandlet::GetAssetsPathFromPackage(const FString &PackageName) const
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

int32 ULoadAssetMaterialsCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // Get Props and Maps Path
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  LoadAssetsMaterials(PackageParams.Name, AssetsPaths.MapsPaths);
	
#if WITH_EDITOR
	UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

	return 0;
}

#endif