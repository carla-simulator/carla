// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "TimeSamplesData.h"

class FAttachSectionExtractor : public ISectionExtractor
{
public:
    FAttachSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;
    virtual bool GetObjectBindingTimeSamples(struct FObjectBindingTimeSamples& ObjectBindingTimeSamples);

private:
    const class UMovieScene3DAttachSection* AttachSection;
    FObjectBindingTimeSamples	ObjectBindingTimeSamples;
};