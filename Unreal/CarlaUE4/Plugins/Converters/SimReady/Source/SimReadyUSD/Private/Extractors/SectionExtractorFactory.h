// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

class FSectionExtractorFactory
{
public:
    FSectionExtractorFactory() {}
    ~FSectionExtractorFactory() {}

public:
    static FSectionExtractorFactory& Get();

public:
    class ISectionExtractor* CreateExtractor(const class UMovieSceneSection*);

};