// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "Styling/SlateStyle.h"

class FSimReadyEditorStyle
    : public FSlateStyleSet
{
private:
    FSimReadyEditorStyle();
    ~FSimReadyEditorStyle();

    static FSimReadyEditorStyle SimReadyEditorStyle;
};