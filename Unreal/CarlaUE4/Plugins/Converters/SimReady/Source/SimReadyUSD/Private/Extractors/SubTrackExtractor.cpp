// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SubTrackExtractor.h"
#include "Tracks/MovieSceneSubTrack.h"
#include "TimeSamplesData.h"


FSubTrackExtractor::FSubTrackExtractor(const UMovieSceneTrack* Track)
    : FTrackExtractor(Track)
{
}

void FSubTrackExtractor::OnPostExtract(TSharedPtr<ISectionExtractor> SectionExtractor, FTimeSamplesData* TimeSamplesData)
{

}