// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "TimeSamplesData.h"

namespace ETrackBlendMode
{
    enum Type
    {
        Keep,
        Overwrite,
        Accumulate,
    };
}

typedef TFunction<void(class UObject*, const TArray<FTimeSampleRange>&)> FViewportShotCallback;

class FSequenceExtractor
{
public:
    /**
     * Default Constructor, used when you are using the C++ "new" syntax. UObject::UObject will set the object pointer
     **/
    FSequenceExtractor()
    {

    }

    ~FSequenceExtractor()
    {

    }

public:
    static FSequenceExtractor& Get();

    bool SetData(UWorld* InWorldContext, const TArray<class ALevelSequenceActor*>& LevelSequenceActors);
    bool SetData(UWorld* InWorldContext, const TArray<class UMovieSceneSequence*>& Sequences);
    void ClearData();

    void Extract(class UMovieSceneSequence* Sequence, const double StartTimeCode, const double EndTimeCode, const double StartOffset, const float TimeScale, ETrackBlendMode::Type BlendMode);
    void RestoreState(const double StartTimeCode, const double EndTimeCode);

    double GetMasterTimeCodesPerSecond() { return MasterTimeCodesPerSecond != 0.0 ? MasterTimeCodesPerSecond : DEFAULT_TIMECODESPERSECOND; }
    double GetMasterStartTimeCode() { return MasterStartTimeCode.IsSet() ? MasterStartTimeCode.GetValue() : 0.0; }
    double GetMasterEndTimeCode() { return MasterEndTimeCode.IsSet() ? MasterEndTimeCode.GetValue() : 0.0; }

    const struct FTranslateRotateScaleTimeSamples* GetTransformTimeSamples(const class UObject*);
    const struct FBooleanTimeSamples* GetVisibilityTimeSamples(const class UObject*);
    const struct FNamedParameterTimeSamples* GetParameterTimeSamples(const class UObject*);
    const FMaterialTimeSamples* GetMaterialTimeSamples(const class UObject*);
    const TArray<struct FObjectBindingTimeSamples>* GetObjectBindingTimeSamples(const class UObject*);
    const struct FTransformTimeSamples* GetSocketTimeSamples(const class UObject*, const FName&);
    const TArray<struct FSkeletalAnimationTimeSamples>* GetSkeletalAnimationTimeSamples(const class UObject*);
    const TArray<struct FTimeSampleRange>* GetCameraShotsTimeSamples(const class UObject*);
    void SetTransformTimeSamples(class UObject*, const FTranslateRotateScaleTimeSamples&);
    void SetSocketTimeSamples(class UObject*, const FName&, const FTransformTimeSamples&);
    void SetViewportShotsTimeSamples(FViewportShotCallback InCallback);
    void GetSequenceObjects(TArray<class UObject*>& OutObjects);
    void GetSpawnableObjects(TArray<class UObject*>& OutObjects);
    FTimeSampleRange GetRangeByPriority(const FTimeSampleRange& InRange, int32 RowIndex, int32 OverlapPriority);
    void ClipCameraShots(const FTimeSampleRange& InRange);

private:
    void SetDataInternal(const TArray<class UMovieSceneSequence*>& Sequences);
    void EmplaceTimeSamples(class UObject* SequenceObject, const struct FTimeSamplesData& TimeSamplesData, ETrackBlendMode::Type BlendMode);
    void PostExtract();
    UObject* SpawnObject(struct FMovieSceneSpawnable& Spawnable);

private:
    TMap<class UObject*, struct FTimeSamplesData> ObjectsTimeSamplesData;
    TMap<FGuid, UObject*> SpawnedObjectsLUT;
    TArray<UObject*> SpawnedActors;
    TArray<UObject*> CurrentSequenceSpawnedActors;
    double MasterTimeCodesPerSecond = 0.0;
    TOptional<double> MasterStartTimeCode;
    TOptional<double> MasterEndTimeCode;
    UWorld* WorldContext;

    struct FSectionPriority
    {
        int32 RowIndex = 0;
        int32 OverlapPriority = 0;
    };

    TMap<int32, FSectionPriority> PriorityTimeline;
private:

    template<typename T>
    void EmplaceTimeSamples(T& Target, const T& Source)
    {
        for (auto Pair : Source)
        {
            auto Result = Target.Find(Pair.Key);
            if (Result)
            {
                Result->TimeSamples.Append(Pair.Value.TimeSamples);
            }
            else
            {
                Target.Add(Pair);
            }
        }
    }
};