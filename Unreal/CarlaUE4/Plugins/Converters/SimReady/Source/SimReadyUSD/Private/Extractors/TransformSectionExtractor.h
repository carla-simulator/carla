// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FTransformSectionExtractor : public ISectionExtractor
{
public:
    FTransformSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetTransformTimeSamples(FTranslateRotateScaleTimeSamples&) override;

private:
    const class UMovieScene3DTransformSection* TransformSection;
    FTranslateRotateScaleTimeSamples	TransformTimeSamples;
};