// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SkeletalAnimationTrackExtractor.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"

FSkeletalAnimationTrackExtractor::FSkeletalAnimationTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FSkeletalAnimationTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FSkeletalAnimationTimeSamples OutSkeletalAnimationTimeSamples;
        if (SectionExtractor->GetSkeletalAnimationTimeSamples(OutSkeletalAnimationTimeSamples))
        {
            TimeSamplesData->SkeletalAnimationTimeSamples.Add(OutSkeletalAnimationTimeSamples);
        }
    }
}
