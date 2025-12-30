// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SkeletalAnimationSectionExtractor.h"
#include "Sections/MovieSceneSkeletalAnimationSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FSkeletalAnimationSectionExtractor::FSkeletalAnimationSectionExtractor(const UMovieSceneSection* Section)
{
    SkeletalAnimationSection = Cast<const UMovieSceneSkeletalAnimationSection>(Section);
}

void FSkeletalAnimationSectionExtractor::Extract(const FExtractorInput& Input)
{
    UMovieScene* MovieScene = SkeletalAnimationSection->GetTypedOuter<UMovieScene>();
    FFrameRate FrameRate = MovieScene->GetTickResolution();
    double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;

    TRange<FFrameNumber> Range = SkeletalAnimationSection->GetTrueRange();
    int32 AnimeTimeCodes = (int)((Range.GetUpperBoundValue().Value - 1 - Range.GetLowerBoundValue().Value) * FrameRate.AsInterval() * Input.TimeCodesPerSecond + 0.5f);	
    SkeletalAnimationTimeSamples.Range.StartTimeCode = (FMath::CeilToDouble(Range.GetLowerBoundValue().Value * (FrameRate.AsInterval() * Input.TimeCodesPerSecond)) - SectionStartTimeCode) / Input.TimeScale + Input.StartTimeCode;
    SkeletalAnimationTimeSamples.Range.EndTimeCode = SkeletalAnimationTimeSamples.Range.StartTimeCode + AnimeTimeCodes - 1;
    if (!SkeletalAnimationTimeSamples.Range.IsInteractWithRange(FTimeSampleRange(Input.StartTimeCode, Input.EndTimeCode)))
    {
        return;
    }
    SkeletalAnimationTimeSamples.Range.ClampToRange(FTimeSampleRange(Input.StartTimeCode, Input.EndTimeCode));

    SkeletalAnimationTimeSamples.Animation = SkeletalAnimationSection->Params.Animation;
    SkeletalAnimationTimeSamples.FirstLoopStartFrameOffset = FMath::RoundToDouble(SkeletalAnimationSection->Params.FirstLoopStartFrameOffset.Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond);
    SkeletalAnimationTimeSamples.StartFrameOffset = FMath::RoundToDouble(SkeletalAnimationSection->Params.StartFrameOffset.Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond);
    SkeletalAnimationTimeSamples.EndFrameOffset = FMath::RoundToDouble(SkeletalAnimationSection->Params.EndFrameOffset.Value * FrameRate.AsInterval() * Input.TimeCodesPerSecond);
    SkeletalAnimationTimeSamples.PlayRate = SkeletalAnimationSection->Params.PlayRate;
    SkeletalAnimationTimeSamples.bReverse = SkeletalAnimationSection->Params.bReverse;

    double TimeCodeCount = SkeletalAnimationTimeSamples.Range.EndTimeCode - SkeletalAnimationTimeSamples.Range.StartTimeCode;
    for (double TimeCode = 0.0; TimeCode <= TimeCodeCount; TimeCode += 1.0)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)((TimeCode * Input.TimeScale + SectionStartTimeCode + (SkeletalAnimationTimeSamples.Range.StartTimeCode - Input.StartTimeCode)) / (FrameRate.AsInterval() * Input.TimeCodesPerSecond)));

        float ValueAtTime = 1.0f;
        SkeletalAnimationSection->Params.Weight.Evaluate(FFrameTime(FrameNumber), ValueAtTime);
        ValueAtTime = ValueAtTime * SkeletalAnimationSection->EvaluateEasing(FFrameTime(FrameNumber));
        SkeletalAnimationTimeSamples.Weights.TimeSamples.FindOrAdd(TimeCode + SkeletalAnimationTimeSamples.Range.StartTimeCode, ValueAtTime);
    }
}

bool FSkeletalAnimationSectionExtractor::GetSkeletalAnimationTimeSamples(FSkeletalAnimationTimeSamples& OutSkeletalAnimationTimeSamples)
{
    if (SkeletalAnimationTimeSamples.Animation == nullptr || !SkeletalAnimationTimeSamples.Range.IsValid())
    {
        return false;
    }

    OutSkeletalAnimationTimeSamples = SkeletalAnimationTimeSamples;
    return true;
}

