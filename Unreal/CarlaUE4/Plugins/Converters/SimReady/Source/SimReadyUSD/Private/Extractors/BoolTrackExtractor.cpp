// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "BoolTrackExtractor.h"
#include "Tracks/MovieSceneBoolTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"


FBoolTrackExtractor::FBoolTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FBoolTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    auto BoolTrack = Cast<const UMovieSceneBoolTrack>(Track);

    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FBooleanTimeSamples TimeSamples;
        if (SectionExtractor->GetBooleanTimeSamples(TimeSamples))
        {
            TimeSamplesData->ParametersTimeSamples.BooleanTimeSamples.Add(BoolTrack->GetTrackName(), TimeSamples);
        }
    }
}