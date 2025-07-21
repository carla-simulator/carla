// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "TrackExtractor.h"

class FVectorTrackExtractor : public FTrackExtractor
{
public:
    FVectorTrackExtractor(const class UMovieSceneTrack*);

public:
    virtual void OnPostExtract(TSharedPtr<class ISectionExtractor> SectionExtractor, struct FTimeSamplesData* TimeSamplesData) override;
};