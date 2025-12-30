// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "StaticMeshExporterUSD.h"


// Return false so that Engine/Plugins/Importers/USDImporter/Content/Python/init_unreal.py doesn't throw errors
bool UStaticMeshExporterUsd::IsUsdAvailable()
{
    return false;
}

UStaticMeshExporterUsd::UStaticMeshExporterUsd()
{
}
