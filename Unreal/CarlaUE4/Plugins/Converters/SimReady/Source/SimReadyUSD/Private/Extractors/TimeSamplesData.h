// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#define DEFAULT_TIMECODESPERSECOND (24.0)
#define DEFAULT_STARTTIMECODE (0.0)
#define DEFAULT_ENDTIMECODE (150.0)

typedef TFunction<void(int32, const FVector&)> FTraverseCallback;
typedef TFunction<void(int32, bool)> FTraverseBooleanCallback;
typedef TFunction<void(int32, float)> FTraverseScalarCallback;
typedef TFunction<void(int32, int32, const FVector4&)> FTraverseVectorCallback;
typedef TFunction<void(int32, const FLinearColor&)> FTraverseColorCallback;
typedef TFunction<void(int32, const FTransform&)> FTraverseTransformCallback;

struct FTransformTimeSamples
{
    TMap<int32, FTransform> Transform;

    int32 GetNumTranslateTimeSamples() const;
    int32 GetNumRotateTimeSamples() const;
    int32 GetNumScaleTimeSamples() const;

    bool HasAnyTimeSamples() const;

    void TraverseTranslate(FTraverseCallback InCallback) const;
    void TraverseRotate(FTraverseCallback InCallback) const;
    void TraverseScale(FTraverseCallback InCallback) const;
    void Traverse(FTraverseTransformCallback InCallback) const;
};

struct FTranslateRotateScaleTimeSamples
{
    FTranslateRotateScaleTimeSamples();
    FTranslateRotateScaleTimeSamples(const FTranslateRotateScaleTimeSamples&);
    FTranslateRotateScaleTimeSamples(const FTransformTimeSamples&);

    TMap<int32, FVector> Translate;
    TMap<int32, FVector> Rotate; // Not use FRotator here because of blending
    TMap<int32, FVector> Scale;

    int32 GetNumTranslateTimeSamples() const;
    int32 GetNumRotateTimeSamples() const;
    int32 GetNumScaleTimeSamples() const;

    bool HasAnyTimeSamples() const;

    void TraverseTranslate(FTraverseCallback InCallback) const;
    void TraverseRotate(FTraverseCallback InCallback) const;
    void TraverseScale(FTraverseCallback InCallback) const;
    void Traverse(FTraverseTransformCallback InCallback) const;
};

struct FBooleanTimeSamples
{
    TMap<int32, bool> TimeSamples;

    void TraverseTimeSamples(FTraverseBooleanCallback InCallback) const;
};

struct FFloatTimeSamples
{
    TMap<int32, float> TimeSamples;

    void TraverseTimeSamples(FTraverseScalarCallback InCallback) const;
};

struct FVectorTimeSamples
{
    int32 ChannelsUsed = 4;
    TMap<int32, FVector4> TimeSamples;

    void TraverseTimeSamples(FTraverseVectorCallback InCallback) const;
};

struct FColorTimeSamples
{
    TMap<int32, FLinearColor> TimeSamples;

    void TraverseTimeSamples(FTraverseColorCallback InCallback) const;
};

enum ENamedParameterType{
    Boolean = 0,
    Scalar = 1,
    Vector = 2,
    Color = 3,
};

struct FNamedParameterTimeSamples
{
    TMap<FName, FBooleanTimeSamples>	BooleanTimeSamples;
    TMap<FName, FFloatTimeSamples>		ScalarTimeSamples;
    TMap<FName, FVectorTimeSamples>		VectorTimeSamples;
    TMap<FName, FColorTimeSamples>		ColorTimeSamples;

    bool HasAnyParameter() const;
    bool HasNamedParameter(const FName& Name, ENamedParameterType Type) const;
    void TraverseBooleanTimeSamples(const FName& Name, FTraverseBooleanCallback InCallback) const;
    void TraverseScalarTimeSamples(const FName& Name, FTraverseScalarCallback InCallback) const;
    void TraverseVectorTimeSamples(const FName& Name, FTraverseVectorCallback InCallback) const;
    void TraverseColorTimeSamples(const FName& Name, FTraverseColorCallback InCallback) const;
};

typedef TMap<FName, FTransformTimeSamples> FSocketTimeSamples;
typedef TMap<int32, FNamedParameterTimeSamples> FMaterialTimeSamples;

struct FTimeSampleRange
{
    FTimeSampleRange(int32 InStartTimeCode, int32 InEndTimeCode)
        : StartTimeCode(InStartTimeCode)
        , EndTimeCode(InEndTimeCode)
    {
    }

    FTimeSampleRange()
        : StartTimeCode(0)
        , EndTimeCode(0)
    {
    }

    int32 StartTimeCode;
    int32 EndTimeCode;

    bool IsInRange(int32 TimeCode);
    bool IsValid() { return StartTimeCode <= EndTimeCode; }
    bool IsInteractWithRange(const FTimeSampleRange&);
    void ClampToRange(const FTimeSampleRange&);
    TArray<FTimeSampleRange> ClipRange(const FTimeSampleRange&);
};

struct FObjectBindingTimeSamples
{
    FTimeSampleRange Range;
    FGuid BindingObjectGuid;
    AActor* BindingObject = nullptr;
};

struct FSkeletalAnimationTimeSamples
{
    FTimeSampleRange Range;
    class UAnimSequenceBase* Animation = nullptr;
    int32 FirstLoopStartFrameOffset;
    int32 StartFrameOffset;
    int32 EndFrameOffset;
    float PlayRate;
    uint32 bReverse : 1;
    FFloatTimeSamples Weights;
};

struct FTimeSamplesData
{
    float										Weight = 1.0f;
    FTranslateRotateScaleTimeSamples			TransformTimeSamples;
    FBooleanTimeSamples							VisibilityTimeSamples;
    FBooleanTimeSamples							SpawnTimeSamples;
    FNamedParameterTimeSamples					ParametersTimeSamples;
    FMaterialTimeSamples						MaterialsTimeSamples;
    TArray<FObjectBindingTimeSamples>			ObjectBindingTimeSamples;
    FSocketTimeSamples							SocketTimeSamples;
    TArray<FSkeletalAnimationTimeSamples>		SkeletalAnimationTimeSamples;
    TArray<FTimeSampleRange>					CameraShotsTimeSamples;
    TArray<FTimeSampleRange>					ViewportShotsTimeSamples;
};

struct FExtractorInput
{
    double TimeCodesPerSecond;
    double StartTimeCode; // sequence start time code
    double EndTimeCode;   // sequence end time code
    double StartTimeCodeOffset;
    double InternalStartTimeCode; // section start time code
    float TimeScale;
};

FORCEINLINE bool FTimeSampleRange::IsInRange(int32 TimeCode)
{
    return TimeCode >= StartTimeCode && TimeCode <= EndTimeCode;
}

FORCEINLINE bool FTimeSampleRange::IsInteractWithRange(const FTimeSampleRange& B)
{
    return EndTimeCode >= B.StartTimeCode && StartTimeCode <= B.EndTimeCode;
}

FORCEINLINE void FTimeSampleRange::ClampToRange(const FTimeSampleRange& B)
{
    StartTimeCode = FMath::Max(StartTimeCode, B.StartTimeCode);
    EndTimeCode = FMath::Min(EndTimeCode, B.EndTimeCode);
}

FORCEINLINE TArray<FTimeSampleRange> FTimeSampleRange::ClipRange(const FTimeSampleRange& B)
{
    TArray<FTimeSampleRange> ClippedRanges;
    if (IsInteractWithRange(B))
    {
        if (B.StartTimeCode > StartTimeCode)
        {
            ClippedRanges.Add({ StartTimeCode, B.StartTimeCode - 1 });
        }
        
        if (B.EndTimeCode < EndTimeCode)
        {
            ClippedRanges.Add({ B.EndTimeCode + 1, EndTimeCode });
        }
    }
    
    return ClippedRanges;
}