// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FSkeletalAnimationSectionExtractor : public ISectionExtractor
{
public:
    FSkeletalAnimationSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetSkeletalAnimationTimeSamples(struct FSkeletalAnimationTimeSamples& SkeletalAnimationTimeSamples);

private:
    const class UMovieSceneSkeletalAnimationSection* SkeletalAnimationSection;

    FSkeletalAnimationTimeSamples SkeletalAnimationTimeSamples;
};