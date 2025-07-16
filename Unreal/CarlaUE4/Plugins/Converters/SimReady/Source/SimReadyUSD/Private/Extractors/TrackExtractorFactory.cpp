// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TrackExtractorFactory.h"
#include "TrackExtractor.h"
#include "TransformTrackExtractor.h"
#include "VisibilityTrackExtractor.h"
#include "SubTrackExtractor.h"
#include "FloatTrackExtractor.h"
#include "BoolTrackExtractor.h"
#include "ColorTrackExtractor.h"
#include "ConstraintTrackExtractor.h"
#include "VectorTrackExtractor.h"
#include "MaterialTrackExtractor.h"
#include "SkeletalAnimationTrackExtractor.h"
#include "SpawnTrackExtractor.h"

#include "MovieSceneTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneSubTrack.h"
#include "Tracks/MovieSceneVisibilityTrack.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneBoolTrack.h"
#include "Tracks/MovieSceneColorTrack.h"
#include "Tracks/MovieSceneVectorTrack.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "Tracks/MovieSceneSpawnTrack.h"
#include "Tracks/MovieScene3DConstraintTrack.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"

FTrackExtractorFactory& FTrackExtractorFactory::Get()
{
    static FTrackExtractorFactory Singleton;
    return Singleton;
}

FTrackExtractor* FTrackExtractorFactory::CreateExtractor(const UMovieSceneTrack* MovieSceneTrack)
{
    if (MovieSceneTrack == nullptr)
    {
        return nullptr;
    }

    if (MovieSceneTrack->IsA<UMovieScene3DTransformTrack>())
    {
        return new FTransformTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneSubTrack>())
    {
        return new FSubTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneVisibilityTrack>())
    {
        return new FVisibilityTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneFloatTrack>())
    {
        return new FFloatTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneBoolTrack>())
    {
        return new FBoolTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneColorTrack>())
    {
        return new FColorTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneVectorTrack>())
    {
        return new FVectorTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneMaterialTrack>())
    {
        return new FMaterialTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneSpawnTrack>())
    {
        return new FSpawnTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieScene3DConstraintTrack>())
    {
        return new FConstraintTrackExtractor(MovieSceneTrack);
    }
    else if (MovieSceneTrack->IsA<UMovieSceneSkeletalAnimationTrack>())
    {
        return new FSkeletalAnimationTrackExtractor(MovieSceneTrack);
    }

    return nullptr;
}