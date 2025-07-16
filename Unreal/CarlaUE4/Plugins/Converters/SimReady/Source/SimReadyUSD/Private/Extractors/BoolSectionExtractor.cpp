// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "BoolSectionExtractor.h"
#include "Sections/MovieSceneBoolSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FBoolSectionExtractor::FBoolSectionExtractor(const UMovieSceneSection* Section)
{
    BoolSection = Cast<const UMovieSceneBoolSection>(Section);
}

void FBoolSectionExtractor::Extract(const FExtractorInput& Input)
{
    FMovieSceneBoolChannel* BoolChannel = BoolSection->GetChannelProxy().GetChannel<FMovieSceneBoolChannel>(0);
    FFrameRate FrameRate = BoolSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

    TOptional<bool> ValueAtLastTime;
    double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
    double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
    for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
        
        // check if out of the range of bool section 
        if (BoolSection->SectionRange.GetLowerBound().IsClosed())
        {
            const FFrameNumber LowFrameNumber = BoolSection->SectionRange.GetLowerBound().IsExclusive() ? (BoolSection->SectionRange.GetLowerBound().GetValue() + 1) : BoolSection->SectionRange.GetLowerBound().GetValue();
            if (LowFrameNumber > FrameNumber)
            {
                continue;
            }
        }

        bool ValueAtTime = false;
        BoolChannel->Evaluate(FFrameTime(FrameNumber), ValueAtTime);
        if(!ValueAtLastTime.IsSet() || (ValueAtLastTime.IsSet() && ValueAtLastTime.GetValue() != ValueAtTime))
        {
            BoolTimeSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
            ValueAtLastTime = ValueAtTime;
        }
    }

    if (ValueAtLastTime.IsSet())
    {
        BoolTimeSamples.TimeSamples.FindOrAdd(Input.EndTimeCode, ValueAtLastTime.GetValue());
    }
}

bool FBoolSectionExtractor::GetBooleanTimeSamples(FBooleanTimeSamples& OutBoolTimeSamples)
{
    OutBoolTimeSamples = BoolTimeSamples;
    return true;
}