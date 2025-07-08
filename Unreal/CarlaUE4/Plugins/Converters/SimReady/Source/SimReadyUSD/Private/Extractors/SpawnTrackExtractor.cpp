// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SpawnTrackExtractor.h"
#include "Tracks/MovieSceneSpawnTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"

FSpawnTrackExtractor::FSpawnTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FSpawnTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        SectionExtractor->GetBooleanTimeSamples(TimeSamplesData->SpawnTimeSamples);
    }
}
