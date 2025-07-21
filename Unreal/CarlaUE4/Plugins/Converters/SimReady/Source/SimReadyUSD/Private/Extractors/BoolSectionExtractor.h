// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FBoolSectionExtractor : public ISectionExtractor
{
public:
    FBoolSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetBooleanTimeSamples(FBooleanTimeSamples&) override;

private:
    const class UMovieSceneBoolSection* BoolSection;
    FBooleanTimeSamples				BoolTimeSamples;
};