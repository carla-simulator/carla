// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "ParameterSectionExtractor.h"
#include "Sections/MovieSceneParameterSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FParameterSectionExtractor::FParameterSectionExtractor(const UMovieSceneSection* Section)
{
    ParameterSection = Cast<const UMovieSceneParameterSection>(Section);
}

void FParameterSectionExtractor::Extract(const FExtractorInput& Input)
{
    FFrameRate FrameRate = ParameterSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
    TArrayView<FMovieSceneFloatChannel*> Channels = ParameterSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
    
    int32 CurveIndex = 0;
    TArray<FScalarParameterNameAndCurve> ScalarParams = ParameterSection->GetScalarParameterNamesAndCurves();
    for (auto ScalarParam : ScalarParams)
    {
        FFloatTimeSamples ScalarSamples;

        double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
            
        for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
            float ValueAtTime = 0.0f;
            bool Success = Channels[CurveIndex]->Evaluate(FFrameTime(FrameNumber), ValueAtTime);
            if (Success)
            {
                ScalarSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
            }
        }

        ParameterTimeSamples.ScalarTimeSamples.FindOrAdd(ScalarParam.ParameterName, ScalarSamples);
        ++CurveIndex;
    }

    TArray<FVectorParameterNameAndCurves> VectorParams = ParameterSection->GetVectorParameterNamesAndCurves();

    for (auto VectorParam : VectorParams)
    {
        int32 NumXKeys = Channels[CurveIndex]->GetNumKeys();
        int32 NumYKeys = Channels[CurveIndex + 1]->GetNumKeys();
        int32 NumZKeys = Channels[CurveIndex + 2]->GetNumKeys();
        FVectorTimeSamples VectorSamples;
        VectorSamples.ChannelsUsed = 3;

        double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;

        for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
            FVector4 ValueAtTime(0);
            bool SuccessX = Channels[CurveIndex]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.X);
            bool SuccessY = Channels[CurveIndex + 1]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.Y);
            bool SuccessZ = Channels[CurveIndex + 2]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.Z);
            if (SuccessX || SuccessY || SuccessZ)
            {
                VectorSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
            }
        }

        ParameterTimeSamples.VectorTimeSamples.FindOrAdd(VectorParam.ParameterName, VectorSamples);

        CurveIndex += 3;
    }

    TArray<FColorParameterNameAndCurves> ColorParams = ParameterSection->GetColorParameterNamesAndCurves();

    for (auto ColorParam : ColorParams)
    {
    
        FColorTimeSamples ColorSamples;

        double TimeCodeCount = Input.EndTimeCode - Input.StartTimeCode;
        double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;

        for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));
            FLinearColor ValueAtTime(0, 0, 0);
            bool SuccessR = Channels[CurveIndex]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.R);
            bool SuccessG = Channels[CurveIndex + 1]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.G);
            bool SuccessB = Channels[CurveIndex + 2]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.B);
            bool SuccessA = Channels[CurveIndex + 3]->Evaluate(FFrameTime(FrameNumber), ValueAtTime.A);
            if (SuccessR || SuccessG || SuccessB || SuccessA)
            {
                ColorSamples.TimeSamples.FindOrAdd(TimeCode + Input.StartTimeCode, ValueAtTime);
            }
        }

        ParameterTimeSamples.ColorTimeSamples.FindOrAdd(ColorParam.ParameterName, ColorSamples);
        CurveIndex += 4;
    }
}

bool FParameterSectionExtractor::GetParameterTimeSamples(FNamedParameterTimeSamples& OutValue)
{
    OutValue = ParameterTimeSamples;
    return true;
}