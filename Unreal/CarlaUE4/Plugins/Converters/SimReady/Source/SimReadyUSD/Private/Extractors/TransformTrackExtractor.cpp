// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TransformTrackExtractor.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "TimeSamplesData.h"
#include "SectionExtractor.h"

FTransformTrackExtractor::FTransformTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FTransformTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{
    if (SectionExtractor.IsValid() && TimeSamplesData)
    {
        SectionExtractor->GetTransformTimeSamples(TimeSamplesData->TransformTimeSamples);
    }
}
