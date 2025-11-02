// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadyPxr.h"
#include "SimReadyExportSettings.h"

class FSimReadyEditorExportUtils
{
public:
    static bool AreObjectsFromTheSameClass(const TArray<class UObject*>& Objects);
};