// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SectionExtractor.h"
#include "SubSectionExtractor.h"
#include "SequenceExtractor.h"

class FCinematicShotSectionExtractor : public FSubSectionExtractor
{
public:
    FCinematicShotSectionExtractor(const class UMovieSceneSection*);

public:
    virtual void Extract(const FExtractorInput& Input) override;

private:
    const class UMovieSceneCinematicShotSection* CinematicShotSection;

    friend FSequenceExtractor;
};