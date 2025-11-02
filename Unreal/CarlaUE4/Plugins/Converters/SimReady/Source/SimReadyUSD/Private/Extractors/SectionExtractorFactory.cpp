// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SectionExtractorFactory.h"
#include "SectionExtractor.h"
#include "TransformSectionExtractor.h"
#include "SubSectionExtractor.h"
#include "BoolSectionExtractor.h"
#include "FloatSectionExtractor.h"
#include "ColorSectionExtractor.h"
#include "VectorSectionExtractor.h"
#include "ParameterSectionExtractor.h"
#include "AttachSectionExtractor.h"
#include "SkeletalAnimationSectionExtractor.h"
#include "CinematicShotSectionExtractor.h"

#include "MovieSceneSection.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Sections/MovieSceneSubSection.h"
#include "Sections/MovieSceneBoolSection.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieSceneColorSection.h"
#include "Sections/MovieSceneVectorSection.h"
#include "Sections/MovieSceneParameterSection.h"
#include "Sections/MovieScene3DAttachSection.h"
#include "Sections/MovieSceneSkeletalAnimationSection.h"
#include "Sections/MovieSceneCinematicShotSection.h"

FSectionExtractorFactory& FSectionExtractorFactory::Get()
{
    static FSectionExtractorFactory Singleton;
    return Singleton;
}

ISectionExtractor* FSectionExtractorFactory::CreateExtractor(const UMovieSceneSection* MovieSceneSection)
{
    if (MovieSceneSection == nullptr)
    {
        return nullptr;
    }

    if (!MovieSceneSection->IsActive())
    {
        return nullptr;
    }

    if (MovieSceneSection->IsA<UMovieScene3DTransformSection>())
    {
        return new FTransformSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneCinematicShotSection>())
    {
        return new FCinematicShotSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneBoolSection>())
    {
        return new FBoolSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneFloatSection>())
    {
        return new FFloatSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneColorSection>())
    {
        return new FColorSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneVectorSection>())
    {
        return new FVectorSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneParameterSection>())
    {
        return new FParameterSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieScene3DAttachSection>())
    {
        return new FAttachSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneSkeletalAnimationSection>())
    {
        return new FSkeletalAnimationSectionExtractor(MovieSceneSection);
    }
    else if (MovieSceneSection->IsA<UMovieSceneSubSection>())
    {
        return new FSubSectionExtractor(MovieSceneSection);
    }

    return nullptr;
}