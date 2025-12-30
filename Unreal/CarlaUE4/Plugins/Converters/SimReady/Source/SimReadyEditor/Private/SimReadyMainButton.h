// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Textures/SlateIcon.h"
#include "Types/SlateEnums.h"

class FSimReadyMainButton:public TSharedFromThis<FSimReadyMainButton>
{
public:
    void Initialize();

    static TSharedPtr<SHorizontalBox> GetUserBox();

protected:
    FText GetTooltip();
    FText GetButtonText();
    FSlateIcon GetButtonIcon();
    TSharedRef<SWidget> GetMenu();
    void ShowMaterialReparent();
};