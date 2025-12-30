// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLMaterialExporter.h"
#include "MDLExporterUtility.h"
#include "Materials/Material.h"
#include "MDLOutputLogger.h"

UMDLMaterialExporter::UMDLMaterialExporter()
{
    bText = true;
    SupportedClass = UMaterialInterface::StaticClass();
    FormatExtension.Add(TEXT("mdl"));
    FormatDescription.Add(TEXT("Material Definition Language"));
}

bool UMDLMaterialExporter::ExportText(const class FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, uint32 PortFlags)
{
    auto Material = CastChecked<UMaterialInterface>(Object);

    if (Material)
    {
        FString OutputMDL;
        FString NewName = FPaths::GetBaseFilename(UMDLMaterialExporter::CurrentFilename);
        if (FMDLExporterUtility::IsLegalIdentifier(NewName))
        {
            FMDLExporterSetting Setting = {NewName, true, true};
            FMDLExporterUtility::ExportMDL(Material, Setting, OutputMDL);
        }
        else
        {
            UE_LOG(LogMDLOutput, Error, TEXT("MDL name %s was not legal"), *NewName);
        }

        Ar.Log(*OutputMDL);
    }

    return true;
}
