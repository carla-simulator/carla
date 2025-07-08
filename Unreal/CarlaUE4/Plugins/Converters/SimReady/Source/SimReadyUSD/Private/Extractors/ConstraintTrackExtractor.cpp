// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "ConstraintTrackExtractor.h"
#include "Tracks/MovieScene3DConstraintTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"

FConstraintTrackExtractor::FConstraintTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FConstraintTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FObjectBindingTimeSamples OutObjectBindingTimeSamples;
        if (SectionExtractor->GetObjectBindingTimeSamples(OutObjectBindingTimeSamples))
        {
            TimeSamplesData->ObjectBindingTimeSamples.Add(OutObjectBindingTimeSamples);
        }
    }
}
