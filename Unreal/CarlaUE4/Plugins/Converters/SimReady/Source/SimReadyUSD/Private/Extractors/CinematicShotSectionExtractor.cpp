// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "CinematicShotSectionExtractor.h"
#include "Sections/MovieSceneCinematicShotSection.h"
#include "SequenceExtractor.h"

FCinematicShotSectionExtractor::FCinematicShotSectionExtractor(const UMovieSceneSection* Section)
    :FSubSectionExtractor(Section)
{
    CinematicShotSection = Cast<const UMovieSceneCinematicShotSection>(Section);
}

void FCinematicShotSectionExtractor::Extract(const FExtractorInput& Input)
{
    if (CinematicShotSection->GetSequence())
    {
        FFrameRate FrameRate = CinematicShotSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
        double StartTimeCode = FMath::RoundToDouble(CinematicShotSection->GetInclusiveStartFrame().Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond) - SectionStartTimeCode;
        double EndTimeCode = FMath::RoundToDouble(CinematicShotSection->GetExclusiveEndFrame().Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond) - 1.0 - SectionStartTimeCode;
        double StartTimeCodeOffset = FMath::RoundToDouble(CinematicShotSection->Parameters.StartFrameOffset.Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond);
        float TimeScale = CinematicShotSection->Parameters.TimeScale * Input.TimeScale;
        
        if (StartTimeCode > Input.EndTimeCode || EndTimeCode < Input.StartTimeCode)
        {
            return;
        }
        EndTimeCode = FMath::Min(EndTimeCode, Input.EndTimeCode);
        
        FTimeSampleRange EditRange = FSequenceExtractor::Get().GetRangeByPriority({ (int32)StartTimeCode, (int32)EndTimeCode }, CinematicShotSection->GetRowIndex(), CinematicShotSection->GetOverlapPriority());
        
        if (EditRange.StartTimeCode <= EditRange.EndTimeCode)
        {
            StartTimeCodeOffset += EditRange.StartTimeCode - StartTimeCode; // for update internal start time code
            FSequenceExtractor::Get().ClipCameraShots(EditRange);
            FSequenceExtractor::Get().Extract(CinematicShotSection->GetSequence(), EditRange.StartTimeCode, EditRange.EndTimeCode, StartTimeCodeOffset, TimeScale, ETrackBlendMode::Overwrite);
            FSequenceExtractor::Get().RestoreState(EditRange.StartTimeCode, EditRange.EndTimeCode);
        }
    }
}