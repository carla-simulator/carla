// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "ISimReadyEditorModule.h"

class FSimReadyEditorModule : public ISimReadyEditorModule
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TSharedPtr<class FSimReadyMainButton> MainButton;
    TFunction<void()> ShutdownViewportExtension;
};
