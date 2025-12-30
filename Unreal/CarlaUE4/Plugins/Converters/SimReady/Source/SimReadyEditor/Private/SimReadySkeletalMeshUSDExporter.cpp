// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadySkeletalMeshUSDExporter.h"
#include "SimReadyEditorPrivate.h"
#include "Engine/SkeletalMesh.h"
#include "SimReadyExporterDialog.h"


USimReadySkeletalMeshUSDExporter::USimReadySkeletalMeshUSDExporter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = USkeletalMesh::StaticClass();
    FormatExtension.Add(TEXT("usda"));
    FormatExtension.Add(TEXT("usd"));	// Must be lower case. Otherwise Stage_Create() will fail.
    FormatDescription.Add(TEXT("Universe Scene Description Text File"));
    FormatDescription.Add(TEXT("Universe Scene Description Binary File"));
}

bool USimReadySkeletalMeshUSDExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    TArray<UObject*> UObjectsToExport;
    UObjectsToExport.Add(Object);
    SSimReadyExporterDialog::ShowDialog(EDialogType::Object, UObjectsToExport, CurrentFilename);

    return true;
}