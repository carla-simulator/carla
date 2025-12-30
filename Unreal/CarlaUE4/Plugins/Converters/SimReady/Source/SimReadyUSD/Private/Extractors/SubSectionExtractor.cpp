// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SubSectionExtractor.h"
#include "Sections/MovieSceneSubSection.h"
#include "SequenceExtractor.h"

FSubSectionExtractor::FSubSectionExtractor(const UMovieSceneSection* Section)
{
    SubSection = Cast<const UMovieSceneSubSection>(Section);
}

void FSubSectionExtractor::Extract(const FExtractorInput& Input)
{
    if (SubSection->GetSequence())
    {
        FFrameRate FrameRate = SubSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
        double StartTimeCode = FMath::RoundToDouble(SubSection->GetInclusiveStartFrame().Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond) - SectionStartTimeCode;
        double EndTimeCode = FMath::RoundToDouble(SubSection->GetExclusiveEndFrame().Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond) - 1.0 - SectionStartTimeCode;
        double StartTimeCodeOffset = FMath::RoundToDouble(SubSection->Parameters.StartFrameOffset.Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond);
        float TimeScale = SubSection->Parameters.TimeScale * Input.TimeScale;
        
        if (StartTimeCode > Input.EndTimeCode || EndTimeCode < Input.StartTimeCode)
        {
            return;
        }
        EndTimeCode = FMath::Min(EndTimeCode, Input.EndTimeCode);

        FSequenceExtractor::Get().Extract(SubSection->GetSequence(), StartTimeCode, EndTimeCode, StartTimeCodeOffset, TimeScale, ETrackBlendMode::Accumulate);
        FSequenceExtractor::Get().RestoreState(StartTimeCode, EndTimeCode);
    }
}