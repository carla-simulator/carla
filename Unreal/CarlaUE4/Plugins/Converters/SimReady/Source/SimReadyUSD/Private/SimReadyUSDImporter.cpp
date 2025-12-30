// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUSDImporter.h"
#include "USDConverter.h"
#include "SimReadyUSDHelper.h"
#include "AssetRegistryModule.h"
#include "Extractors/TimeSamplesData.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/MessageDialog.h"
#include "Editor.h"
#include "SimReadyStageActor.h"
#include "SimReadyTexture.h"
#include "SimReadyPathHelper.h"
#include "SimReadySettings.h"
#include "UObject/GarbageCollection.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "USDHashGenerator.h"
#include "USDDerivedDataCache.h"
#include "SimReadyAssetImportHelper.h"
#include "SimReadyMDL.h"
#include "SimReadyUSDImporterHelper.h"
#include "SimReadyUSDTokens.h"
#include "SimReadyMaterialReparentUtility.h"
#include "ComponentRecreateRenderStateContext.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSimReadyImporter, Log, All);
DEFINE_LOG_CATEGORY(LogSimReadyImporter);
#define LOCTEXT_NAMESPACE "SimReadyEditor"

#define ASSET_FLAGS (EObjectFlags::RF_Standalone | EObjectFlags::RF_Public)

bool FSimReadyUSDImporter::LoadUSD(const FString& Path, const FString& DestinationPath, const FSimReadyImportSettings& ImportSettings)
{
    GEditor->CreateNewMapForEditing();

    static const auto CVarDistanceField = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.GenerateMeshDistanceFields"));
    int32 OriginalCVarDistanceFieldValue = CVarDistanceField->GetValueOnGameThread();
    IConsoleVariable* CVarDistanceFieldInterface = IConsoleManager::Get().FindConsoleVariable(TEXT("r.GenerateMeshDistanceFields"));
    //Avoid building the distance field when we import stages
    if (OriginalCVarDistanceFieldValue != 0 && CVarDistanceFieldInterface)
    {
        //Hack we change the distance field user console variable to control building of mesh distance fields
        CVarDistanceFieldInterface->SetWithCurrentPriority(0);
    }

    auto& World = *GEditor->GetEditorWorldContext().World();
    auto& StageActor = ASimReadyStageActor::Get(World);
    StageActor.InitializePreviewEnvironment();
    StageActor.ImportUSD(Path, DestinationPath, ImportSettings);
    World.DestroyActor(&StageActor, false, false);
    CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

    //Put back the distance field value
    if (OriginalCVarDistanceFieldValue != 0 && CVarDistanceFieldInterface)
    {
        CVarDistanceFieldInterface->SetWithCurrentPriority(OriginalCVarDistanceFieldValue);
    }

    return true;
}
#undef LOCTEXT_NAMESPACE