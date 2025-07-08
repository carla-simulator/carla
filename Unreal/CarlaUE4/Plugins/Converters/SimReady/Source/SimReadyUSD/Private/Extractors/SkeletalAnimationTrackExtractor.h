// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "TrackExtractor.h"

class FSkeletalAnimationTrackExtractor : public FTrackExtractor
{
public:
    FSkeletalAnimationTrackExtractor(const class UMovieSceneTrack*);

public:
    virtual void OnPostExtract(TSharedPtr<class ISectionExtractor> SectionExtractor, struct FTimeSamplesData* TimeSamplesData) override;
};