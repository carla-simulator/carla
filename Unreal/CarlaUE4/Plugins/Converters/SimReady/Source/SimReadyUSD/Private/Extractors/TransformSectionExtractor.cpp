// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TransformSectionExtractor.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Channels/MovieSceneChannelProxy.h"


FTransformSectionExtractor::FTransformSectionExtractor(const UMovieSceneSection* Section)
{
    TransformSection = Cast<const UMovieScene3DTransformSection>(Section);
}

void FTransformSectionExtractor::Extract(const FExtractorInput& Input)
{
    TArrayView<FMovieSceneFloatChannel*> Channels = TransformSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
    FFrameRate FrameRate = TransformSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

    auto ExtractTimeSamples = [&](int32 TransformTypeIndex)
    {
        auto SaveTimeSamples = [&](int32 TransformTypeIndex, int32 TimeCode, const FVector& Value)
        {
            if (TransformTypeIndex == 0)
            {
                TransformTimeSamples.Translate.FindOrAdd(TimeCode, Value);
            }
            else if (TransformTypeIndex == 1)
            {
                TransformTimeSamples.Rotate.FindOrAdd(TimeCode, Value);
            }
            else
            {
                TransformTimeSamples.Scale.FindOrAdd(TimeCode, Value);
            }
        };

        int32 IndexX = TransformTypeIndex * 3;
        int32 IndexY = TransformTypeIndex * 3 + 1;
        int32 IndexZ = TransformTypeIndex * 3 + 2;

        double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
        for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
            FVector Value(0);
            bool SuccessX = Channels[IndexX]->Evaluate(FFrameTime(FrameNumber), Value.X);
            bool SuccessY = Channels[IndexY]->Evaluate(FFrameTime(FrameNumber), Value.Y);
            bool SuccessZ = Channels[IndexZ]->Evaluate(FFrameTime(FrameNumber), Value.Z);
            if (SuccessX || SuccessY || SuccessZ)
            {
                SaveTimeSamples(TransformTypeIndex, TimeCode + Input.StartTimeCode, Value);
            }
        }
    };

    for (int32 Index = 0; Index < 3; ++Index)
    {
        ExtractTimeSamples(Index);
    }
}

bool FTransformSectionExtractor::GetTransformTimeSamples(FTranslateRotateScaleTimeSamples& OutTransformTimeSamples)
{
    OutTransformTimeSamples = TransformTimeSamples;
    return true;
}