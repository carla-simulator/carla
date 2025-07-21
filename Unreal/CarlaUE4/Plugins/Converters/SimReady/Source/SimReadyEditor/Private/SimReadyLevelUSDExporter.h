// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

//~=============================================================================
// LevelExporterUSD
//~=============================================================================

#pragma once

#include "Exporters/Exporter.h"
#include "SimReadyLevelUSDExporter.generated.h"

UCLASS()
class USimReadyLevelUSDExporter : public UExporter
{
public:
    GENERATED_BODY()

public:
    USimReadyLevelUSDExporter(const FObjectInitializer& ObjectInitializer = FObjectInitializer());

    //~ Begin UExporter Interface
    virtual bool ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, int32 FileIndex = 0, uint32 PortFlags = 0);
    bool Export(UWorld& World, FString& Path, bool bSaveAs, bool bReplace, bool ExportUE4Objects) const;
    //~ End UExporter Interface
};