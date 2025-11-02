// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "Exporters/Exporter.h"
#include "SimReadyAnimationMontageUSDExporter.generated.h"

UCLASS()
class USimReadyAnimationMontageUSDExporter : public UExporter
{
    GENERATED_UCLASS_BODY()

    //~ Begin UExporter Interface
    virtual bool ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex = 0, uint32 PortFlags = 0) override;
    //~ End UExporter Interface
};
