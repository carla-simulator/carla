// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MaterialTrackExtractor.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"


FMaterialTrackExtractor::FMaterialTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FMaterialTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    auto MaterialTrack = Cast<const UMovieSceneComponentMaterialTrack>(Track);

    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        FNamedParameterTimeSamples TimeSamples;
        if (SectionExtractor->GetParameterTimeSamples(TimeSamples))
        {
            TimeSamplesData->MaterialsTimeSamples.Add(MaterialTrack ? MaterialTrack->GetMaterialIndex() : 0, TimeSamples);
        }
    }
}