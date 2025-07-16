// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TrackExtractor.h"
#include "TimeSamplesData.h"
#include "MovieSceneTrack.h"
#include "SectionExtractor.h"
#include "SectionExtractorFactory.h"

FTrackExtractor::FTrackExtractor(const UMovieSceneTrack* InTrack)
    : Track(InTrack)
{

}

void FTrackExtractor::Extract(const FExtractorInput& Input, FTimeSamplesData* TimeSamplesData)
{
    for (const UMovieSceneSection* Section : Track->GetAllSections())
    {
        TSharedPtr<ISectionExtractor> Extractor = MakeShareable<ISectionExtractor>(FSectionExtractorFactory::Get().CreateExtractor(Section));
        if (Extractor)
        {
            Extractor->Extract(Input);

            OnPostExtract(Extractor, TimeSamplesData);
        }
    }
}
