// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "AttachSectionExtractor.h"
#include "Sections/MovieScene3DAttachSection.h"
#include "SequenceExtractor.h"
#include "Channels/MovieSceneChannelProxy.h"

FAttachSectionExtractor::FAttachSectionExtractor(const UMovieSceneSection* Section)
{
    AttachSection = Cast<const UMovieScene3DAttachSection>(Section);
}

void FAttachSectionExtractor::Extract(const FExtractorInput& Input)
{
    UMovieScene* MovieScene = AttachSection->GetTypedOuter<UMovieScene>();
    FFrameRate FrameRate = MovieScene->GetTickResolution();
    double SectionStartTimeCode = Input.InternalStartTimeCode + Input.StartTimeCodeOffset;
    
    TRange<FFrameNumber> Range = AttachSection->GetTrueRange();
    if (Range.GetLowerBound().IsOpen())
    {
        ObjectBindingTimeSamples.Range.StartTimeCode = Input.StartTimeCode;
    }
    else
    {
        ObjectBindingTimeSamples.Range.StartTimeCode = (FMath::CeilToDouble(Range.GetLowerBoundValue().Value * (FrameRate.AsInterval() * Input.TimeCodesPerSecond)) - SectionStartTimeCode) / Input.TimeScale + Input.StartTimeCode;
    }
    if (Range.GetUpperBound().IsOpen())
    {
        ObjectBindingTimeSamples.Range.EndTimeCode = Input.EndTimeCode;
    }
    else
    {
        ObjectBindingTimeSamples.Range.EndTimeCode = (FMath::FloorToDouble((Range.GetUpperBoundValue().Value - 1) * (FrameRate.AsInterval() * Input.TimeCodesPerSecond)) - SectionStartTimeCode) / Input.TimeScale + Input.StartTimeCode;
    }
    if (!ObjectBindingTimeSamples.Range.IsInteractWithRange(FTimeSampleRange(Input.StartTimeCode, Input.EndTimeCode)))
    {
        return;
    }
    ObjectBindingTimeSamples.Range.ClampToRange(FTimeSampleRange(Input.StartTimeCode, Input.EndTimeCode));

    FMovieSceneObjectBindingID ConstraintBindingID = AttachSection->GetConstraintBindingID();

    ObjectBindingTimeSamples.BindingObjectGuid = ConstraintBindingID.GetGuid();
}

bool FAttachSectionExtractor::GetObjectBindingTimeSamples(FObjectBindingTimeSamples& OutObjectBindingTimeSamples)
{
    if (!ObjectBindingTimeSamples.BindingObjectGuid.IsValid() || !ObjectBindingTimeSamples.Range.IsValid())
    {
        return false;
    }

    OutObjectBindingTimeSamples = ObjectBindingTimeSamples;
    return true;
}