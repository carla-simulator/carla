// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMaterialReparentSettings.h"



#if WITH_EDITOR
void USimReadyMaterialReparentSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Save to config
    auto Settings = GetMutableDefault<USimReadyMaterialReparentSettings>();
    Settings->UpdateDefaultConfigFile();
}
#endif
