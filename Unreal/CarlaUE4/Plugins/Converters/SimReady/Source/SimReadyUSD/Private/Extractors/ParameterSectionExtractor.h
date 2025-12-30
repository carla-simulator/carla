// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FParameterSectionExtractor : public ISectionExtractor
{
public:
    FParameterSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetParameterTimeSamples(struct FNamedParameterTimeSamples& OutValue) override;

private:
    const class UMovieSceneParameterSection* ParameterSection;
    FNamedParameterTimeSamples				ParameterTimeSamples;
};