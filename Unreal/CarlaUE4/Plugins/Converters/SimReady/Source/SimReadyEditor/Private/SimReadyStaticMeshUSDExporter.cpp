// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyStaticMeshUSDExporter.h"
#include "SimReadyEditorPrivate.h"
#include "Engine/StaticMesh.h"
#include "SimReadyExporterDialog.h"


USimReadyStaticMeshUSDExporter::USimReadyStaticMeshUSDExporter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UStaticMesh::StaticClass();
    FormatExtension.Add(TEXT("usda"));
    FormatExtension.Add(TEXT("usd"));	// Must be lower case. Otherwise Stage_Create() will fail.
    FormatDescription.Add(TEXT("Universal Scene Description Text File"));
    FormatDescription.Add(TEXT("Universal Scene Description Binary File"));
}

bool USimReadyStaticMeshUSDExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    TArray<UObject*> UObjectsToExport;
    UObjectsToExport.Add(Object);
    SSimReadyExporterDialog::ShowDialog(EDialogType::Object, UObjectsToExport, CurrentFilename);

    return true;
}