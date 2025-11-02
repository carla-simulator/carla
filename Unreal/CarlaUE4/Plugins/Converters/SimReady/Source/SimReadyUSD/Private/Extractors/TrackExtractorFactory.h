// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

class FTrackExtractorFactory
{
public:
    FTrackExtractorFactory() {}
    ~FTrackExtractorFactory() {}

public:
    static FTrackExtractorFactory& Get();

public:
    class FTrackExtractor* CreateExtractor(const class UMovieSceneTrack*);

};