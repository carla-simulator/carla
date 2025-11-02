// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Exporters/Exporter.h"
#include "MDLMaterialExporter.generated.h"

UCLASS()
class UMDLMaterialExporter : public UExporter
{
    GENERATED_BODY()

    UMDLMaterialExporter();

    //~ Begin UExporter Interface
    virtual bool ExportText(const class FExportObjectInnerContext* Context, UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn, uint32 PortFlags = 0)override;
    //~ End UExporter Interface
};