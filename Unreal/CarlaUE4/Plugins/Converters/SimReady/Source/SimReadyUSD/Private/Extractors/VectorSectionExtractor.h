// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FVectorSectionExtractor : public ISectionExtractor
{
public:
    FVectorSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetVectorTimeSamples(FVectorTimeSamples&) override;

private:
    const class UMovieSceneVectorSection* VectorSection;
    FVectorTimeSamples						VectorTimeSamples;
};