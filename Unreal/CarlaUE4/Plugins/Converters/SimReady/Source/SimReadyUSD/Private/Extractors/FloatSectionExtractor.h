// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FFloatSectionExtractor : public ISectionExtractor
{
public:
    FFloatSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetFloatTimeSamples(FFloatTimeSamples&) override;

private:
    const class UMovieSceneFloatSection* FloatSection;
    FFloatTimeSamples					FloatTimeSamples;
};