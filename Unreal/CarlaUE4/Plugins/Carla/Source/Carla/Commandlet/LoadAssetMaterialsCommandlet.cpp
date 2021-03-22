// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LoadAssetMaterialsCommandlet.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RoadPainterTexRenderTarget(TEXT(
		"TextureRenderTarget2D'/Game/Carla/Blueprints/LevelDesign/RoadPainterAssets/RenderTexture.RenderTexture'"));

	RoadPainterSubclass = (UClass*)RoadPainterBlueprint.Object->GeneratedClass;
	RoadPainterTexture = RoadPainterTexRenderTarget.Object;
	
#endif
}

#if WITH_EDITORONLY_DATA

void ULoadAssetMaterialsCommandlet::ApplyRoadPainterMaterials()
{
	// ImportedMap path from import process
	const FString ImportedMap = TEXT("/Game/map_package/Maps/TestOSM");
	
	ARoadPainterWrapper *RoadPainterBp = World->SpawnActor<ARoadPainterWrapper>(RoadPainterSubclass);
	if (RoadPainterBp)
	{
		//Needed to call events in editor-mode
		FEditorScriptExecutionGuard ScriptGuard;
		RoadPainterBp->ZSizeEvent();
		RoadPainterBp->ClearAllEvent();
		RoadPainterBp->PaintAllRoadsEvent();
		
		//TArray<AActor*> FoundActors;
		//UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), FoundActors);
		//
		//AStaticMeshActor *RoadMeshActor = nullptr;
		//
		//bool FoundRoadActor = false;
		//for(int32 i = 0; i < FoundActors.Num() && FoundRoadActor == false; ++i)
		//{
		//	RoadMeshActor = Cast<AStaticMeshActor>(FoundActors[i]);
		//	if(RoadMeshActor)
		//	{
		//		if(RoadMeshActor->GetName().Equals("Roads_RoadNode") == true)
		//		{
		//			UE_LOG(LogTemp, Log, TEXT("Got it!"), *FoundActors[i]->GetName());
		//
		//			UMaterialInstanceDynamic *MI = UMaterialInstanceDynamic::Create(RoadNodeMaterialMaster, this, FName(TEXT("Road Painter Material Dynamic")));
		//			MI->CopyParameterOverrides((UMaterialInstance*)RoadMeshActor->GetStaticMeshComponent()->GetMaterial(0));
		//			RoadMeshActor->GetStaticMeshComponent()->SetMaterial(0, MI);
		//			MI->SetScalarParameterValue(TEXT("Map units (CM)"), RoadPainterBp->MapSize);
		//			MI->SetTextureParameterValue(TEXT("Texture Mask"), RoadPainterTexture);
		//			FoundRoadActor = true;
		//		}
		//	}
		//}

		//FTimerHandle Handle;
		//World->GetTimerManager().SetTimer(Handle, [=]() { RoadPainterBp->PaintAllRoadsEvent(); }, 5.0f, 1);
	}
}

void ULoadAssetMaterialsCommandlet::LoadImportedMapWorld(FAssetData &AssetData)
{
	// ImportedMap path from import process
	const FString ImportedMap = TEXT("/Game/map_package/Maps/TestOSM");

	// Load Map folder using object library
	MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
	MapObjectLibrary->AddToRoot();
	MapObjectLibrary->LoadAssetDataFromPath(*ImportedMap);
	MapObjectLibrary->LoadAssetsFromAssetData();
	MapObjectLibrary->GetAssetDataList(AssetDatas);

	if (AssetDatas.Num() > 0)
	{
		// Extract first asset found in folder path (i.e. the imported map)
		AssetData = AssetDatas.Pop();
		UE_LOG(LogTemp, Log, TEXT("The name of the asset : %s"), *AssetData.GetFullName());
	}
}

int32 ULoadAssetMaterialsCommandlet::Main(const FString &Params)
{
	FAssetData AssetData;
	LoadImportedMapWorld(AssetData);
	World = CastChecked<UWorld>(AssetData.GetAsset());
	World->InitWorld();
	ApplyRoadPainterMaterials();
	
#if WITH_EDITOR
	UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

	return 0;
}

#endif