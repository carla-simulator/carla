// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "FloatSectionExtractor.h"
#include "Sections/MovieSceneFloatSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FFloatSectionExtractor::FFloatSectionExtractor(const UMovieSceneSection* Section)
{
    FloatSection = Cast<const UMovieSceneFloatSection>(Section);
}

void FFloatSectionExtractor::Extract(const FExtractorInput& Input)
{
    FMovieSceneFloatChannel* FloatChannel = FloatSection->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);
    FFrameRate FrameRate = FloatSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

    double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
    double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
    for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
        float ValueAtTime = 0.0f;
        bool Success = FloatChannel->Evaluate(FFrameTime(FrameNumber), ValueAtTime);
        if (Success)
        {
            FloatTimeSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
        }
    }
}

bool FFloatSectionExtractor::GetFloatTimeSamples(FFloatTimeSamples& OutValue)
{
    OutValue = FloatTimeSamples;
    return true;
}