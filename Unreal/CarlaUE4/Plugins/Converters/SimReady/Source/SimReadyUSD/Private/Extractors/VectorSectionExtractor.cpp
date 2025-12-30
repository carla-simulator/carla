// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "VectorSectionExtractor.h"
#include "Sections/MovieSceneVectorSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FVectorSectionExtractor::FVectorSectionExtractor(const UMovieSceneSection* Section)
{
    VectorSection = Cast<const UMovieSceneVectorSection>(Section);
}

void FVectorSectionExtractor::Extract(const FExtractorInput& Input)
{
    TArrayView<FMovieSceneFloatChannel*> Channels = VectorSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
    FFrameRate FrameRate = VectorSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

    auto ExtractTimeSamples = [&](int32 UsedChannels)
    {
        VectorTimeSamples.ChannelsUsed = UsedChannels;

        double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
        for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
            FVector4 ValueAtTime(0);
            bool SuccessX = false;
            bool SuccessY = false;
            bool SuccessZ = false;
            bool SuccessW = false;
            for (int32 Index = 0; Index < UsedChannels; ++Index)
            {
                if (Index == 0)
                {
                    SuccessX = Channels[Index]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.X);
                }
                else if (Index == 1)
                {
                    SuccessY = Channels[Index]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.Y);
                }
                else if (Index == 2)
                {
                    SuccessZ = Channels[Index]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.Z);
                }
                else
                {
                    SuccessW = Channels[Index]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.W);
                }
            }

            if (SuccessX || SuccessY || SuccessZ || SuccessW)
            {
                VectorTimeSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
            }
        }
    };

    ExtractTimeSamples(VectorSection->GetChannelsUsed());
}

bool FVectorSectionExtractor::GetVectorTimeSamples(FVectorTimeSamples& OutValue)
{
    OutValue = VectorTimeSamples;
    return true;
}
