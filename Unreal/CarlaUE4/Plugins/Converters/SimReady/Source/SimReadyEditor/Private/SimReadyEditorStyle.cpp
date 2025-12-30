// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

FSimReadyEditorStyle FSimReadyEditorStyle::SimReadyEditorStyle;


FSimReadyEditorStyle::FSimReadyEditorStyle()
    :FSlateStyleSet("SimReadyEditorStyle")
{
    SetContentRoot(IPluginManager::Get().FindPlugin("SimReady")->GetContentDir() / "Editor/Icons");

    const FVector2D Icon20x20(20.0f, 20.0f);
    const FVector2D Icon32x32(32.0f, 32.0f);
    const FVector2D Icon40x40(40.0f, 40.0f);
    const FVector2D Icon64x64(64.0f, 64.0f);

    Set("MainButton.SimReady", new FSlateImageBrush(RootToContentDir(TEXT("Connected.png")), Icon40x40));
    FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FSimReadyEditorStyle::~FSimReadyEditorStyle()
{
    FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}
