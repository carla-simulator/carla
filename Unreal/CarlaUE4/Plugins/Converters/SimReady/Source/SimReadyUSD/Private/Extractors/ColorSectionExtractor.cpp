// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "ColorSectionExtractor.h"
#include "Sections/MovieSceneColorSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FColorSectionExtractor::FColorSectionExtractor(const UMovieSceneSection* Section)
{
    ColorSection = Cast<const UMovieSceneColorSection>(Section);
}

void FColorSectionExtractor::Extract(const FExtractorInput& Input)
{
    TArrayView<FMovieSceneFloatChannel*> Channels = ColorSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
    FFrameRate FrameRate = ColorSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

    auto ExtractTimeSamples = [&]()
    {
        double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
        for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
            FLinearColor ValueAtTime(0, 0, 0);
            bool SuccessR = Channels[0]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.R);
            bool SuccessG = Channels[1]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.G);
            bool SuccessB = Channels[2]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.B);
            bool SuccessA = Channels[3]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.A);
            if (SuccessR || SuccessG || SuccessB || SuccessA)
            {
                ColorTimeSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
            }
        }
    };

    ExtractTimeSamples();
}

bool FColorSectionExtractor::GetColorTimeSamples(FColorTimeSamples& OutValue)
{
    OutValue = ColorTimeSamples;
    return true;
}