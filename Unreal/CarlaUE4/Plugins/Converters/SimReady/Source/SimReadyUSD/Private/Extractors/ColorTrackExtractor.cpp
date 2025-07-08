// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "ColorTrackExtractor.h"
#include "Tracks/MovieSceneColorTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"

FColorTrackExtractor::FColorTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FColorTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    auto ColorTrack = Cast<const UMovieSceneColorTrack>(Track);

    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FColorTimeSamples TimeSamples;
        if (SectionExtractor->GetColorTimeSamples(TimeSamples))
        {
            TimeSamplesData->ParametersTimeSamples.ColorTimeSamples.Add(ColorTrack->GetTrackName(), TimeSamples);
        }
    }
}