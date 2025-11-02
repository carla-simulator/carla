// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "VectorTrackExtractor.h"
#include "Tracks/MovieSceneVectorTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"


FVectorTrackExtractor::FVectorTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FVectorTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    auto VectorTrack = Cast<const UMovieSceneVectorTrack>(Track);

    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FVectorTimeSamples TimeSamples;
        if (SectionExtractor->GetVectorTimeSamples(TimeSamples))
        {
            TimeSamplesData->ParametersTimeSamples.VectorTimeSamples.Add(VectorTrack->GetTrackName(), TimeSamples);
        }
    }
}