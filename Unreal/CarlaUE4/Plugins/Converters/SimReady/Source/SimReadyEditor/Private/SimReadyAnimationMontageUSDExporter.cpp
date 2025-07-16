// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyAnimationMontageUSDExporter.h"
#include "SimReadyEditorPrivate.h"
#include "Animation/AnimMontage.h"
#include "SimReadyExporterDialog.h"


USimReadyAnimationMontageUSDExporter::USimReadyAnimationMontageUSDExporter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UAnimMontage::StaticClass();
    FormatExtension.Add(TEXT("usda"));
    FormatExtension.Add(TEXT("usd"));	// Must be lower case. Otherwise Stage_Create() will fail.
    FormatDescription.Add(TEXT("Universe Scene Description Text File"));
    FormatDescription.Add(TEXT("Universe Scene Description Binary File"));
}

bool USimReadyAnimationMontageUSDExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    TArray<UObject*> UObjectsToExport;
    UObjectsToExport.Add(Object);
    SSimReadyExporterDialog::ShowDialog(EDialogType::Animation, UObjectsToExport, CurrentFilename);

    return true;
}