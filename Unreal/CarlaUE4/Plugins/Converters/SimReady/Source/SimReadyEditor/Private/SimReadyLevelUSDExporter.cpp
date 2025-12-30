// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyLevelUSDExporter.h"
#include "EngineUtils.h"
#include "SimReadySettings.h"
#include "SimReadyStageActor.h"
#include "SimReadyUSDHelper.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyExporterDialog.h"
#include "SimReadyPathHelper.h"

USimReadyLevelUSDExporter::USimReadyLevelUSDExporter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UWorld::StaticClass();
    FormatExtension.Add(TEXT("usda"));
    FormatExtension.Add(TEXT("usd"));	// Must be lower case. Otherwise Stage_Create() will fail.
    FormatDescription.Add(TEXT("Universal Scene Description text SimReady assets for AV Sim."));
    FormatDescription.Add(TEXT("Universal Scene Description binary SimReady assets for AV Sim."));
}

bool USimReadyLevelUSDExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    return Export(*CastChecked<UWorld>(Object), CurrentFilename, true, false, true);
}

bool USimReadyLevelUSDExporter::Export(UWorld& World, FString& Path, bool bSaveAs, bool bReplace, bool ExportUE4Objects) const
{
    if (bSelectedOnly) // path to export selected
    {
        TArray<AActor*> ActorsToExport;
        TArray<ALevelSequenceActor*> LevelSequenceActors;
        FSimReadyUSDHelper::GetActorsToExportFromWorld(&World, bSelectedOnly, FSimReadyExportSettings(), ActorsToExport, LevelSequenceActors);
        SSimReadyExporterDialog::ShowDialog(ActorsToExport, Path);
    }
    else
    {
        //// Export to local
        TArray<UObject*> UObjectsToExport;
        UObjectsToExport.Add(&World);
        SSimReadyExporterDialog::ShowDialog(EDialogType::Level, UObjectsToExport, Path);
    }
    return true;
}