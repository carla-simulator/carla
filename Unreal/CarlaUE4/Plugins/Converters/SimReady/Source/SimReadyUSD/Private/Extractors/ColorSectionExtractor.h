// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FColorSectionExtractor : public ISectionExtractor
{
public:
    FColorSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetColorTimeSamples(FColorTimeSamples&) override;

private:
    const class UMovieSceneColorSection* ColorSection;
    FColorTimeSamples						ColorTimeSamples;
};