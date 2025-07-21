// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "FloatTrackExtractor.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"


FFloatTrackExtractor::FFloatTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FFloatTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    auto FloatTrack = Cast<const UMovieSceneFloatTrack>(Track);

    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FFloatTimeSamples TimeSamples;
        if (SectionExtractor->GetFloatTimeSamples(TimeSamples))
        {
            TimeSamplesData->ParametersTimeSamples.ScalarTimeSamples.Add(FloatTrack->GetTrackName(), TimeSamples);
        }
    }
}