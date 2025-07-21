// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "VisibilityTrackExtractor.h"
#include "Tracks/MovieSceneVisibilityTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"

FVisibilityTrackExtractor::FVisibilityTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FVisibilityTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        SectionExtractor->GetBooleanTimeSamples(TimeSamplesData->VisibilityTimeSamples);
    }
}
