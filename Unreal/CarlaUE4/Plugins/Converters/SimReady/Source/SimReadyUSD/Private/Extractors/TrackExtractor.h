// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

class FTrackExtractor
{
public:
    FTrackExtractor(const class UMovieSceneTrack*);
    virtual ~FTrackExtractor() {}

public:
    void Extract(const struct FExtractorInput& Input, struct FTimeSamplesData* TimeSamplesData = nullptr);

public:
    virtual void OnPostExtract(TSharedPtr<class ISectionExtractor> SectionExtractor, struct FTimeSamplesData* TimeSamplesData) = 0;

protected:
    const class UMovieSceneTrack* Track;
};