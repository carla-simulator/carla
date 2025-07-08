// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyAnimationSequenceUSDExporter.h"
#include "SimReadyEditorPrivate.h"
#include "Animation/AnimSequence.h"
#include "SimReadyExporterDialog.h"


USimReadyAnimationSequenceUSDExporter::USimReadyAnimationSequenceUSDExporter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UAnimSequence::StaticClass();
    FormatExtension.Add(TEXT("usda"));
    FormatExtension.Add(TEXT("usd"));	// Must be lower case. Otherwise Stage_Create() will fail.
    FormatDescription.Add(TEXT("Universe Scene Description Text File"));
    FormatDescription.Add(TEXT("Universe Scene Description Binary File"));
}

bool USimReadyAnimationSequenceUSDExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    TArray<UObject*> UObjectsToExport;
    UObjectsToExport.Add(Object);
    SSimReadyExporterDialog::ShowDialog(EDialogType::Animation, UObjectsToExport, CurrentFilename);

    return true;
}