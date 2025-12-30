// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "TimeSamplesData.h"

FTranslateRotateScaleTimeSamples::FTranslateRotateScaleTimeSamples()
{
}

FTranslateRotateScaleTimeSamples::FTranslateRotateScaleTimeSamples(const FTranslateRotateScaleTimeSamples& A)
{
    Translate = A.Translate;
    Rotate = A.Rotate;
    Scale = A.Scale;
}

FTranslateRotateScaleTimeSamples::FTranslateRotateScaleTimeSamples(const FTransformTimeSamples& A)
{
    for (auto Pair : A.Transform)
    {
        Translate.Add(Pair.Key, Pair.Value.GetTranslation());
        FRotator Rotator = Pair.Value.GetRotation().Rotator();
        Rotate.Add(Pair.Key, FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw));
        Scale.Add(Pair.Key, Pair.Value.GetScale3D());
    }
}

int32 FTranslateRotateScaleTimeSamples::GetNumTranslateTimeSamples() const
{
    return Translate.Num();
}

int32 FTranslateRotateScaleTimeSamples::GetNumRotateTimeSamples() const
{
    return Rotate.Num();
}

int32 FTranslateRotateScaleTimeSamples::GetNumScaleTimeSamples() const
{
    return Scale.Num();
}

void FTranslateRotateScaleTimeSamples::TraverseTranslate(FTraverseCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Translate)
        {
            InCallback(Sample.Key, Sample.Value);
        }	
    }
}
void FTranslateRotateScaleTimeSamples::TraverseRotate(FTraverseCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Rotate)
        {
            InCallback(Sample.Key, Sample.Value);
        }
    }
}
void FTranslateRotateScaleTimeSamples::TraverseScale(FTraverseCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Scale)
        {
            InCallback(Sample.Key, Sample.Value);
        }
    }
}

void FTranslateRotateScaleTimeSamples::Traverse(FTraverseTransformCallback InCallback) const
{
    if (InCallback)
    {
        TArray<int32> KeyArray;
        Scale.GenerateKeyArray(KeyArray);

        if (KeyArray.Num() > 0)
        {
            KeyArray.Sort();
            FTransform Transform; //Must put out of the loop
            for (int32 Key = KeyArray[0]; Key <= KeyArray[KeyArray.Num()-1]; ++Key)
            {
                const FVector* T = Translate.Find(Key);
                if (T)
                {
                    Transform.SetTranslation(*T);
                }

                const FVector* R = Rotate.Find(Key);
                if (R)
                {
                    FRotator Rotator = FRotator(R->Y, R->Z, R->X);
                    Transform.SetRotation(Rotator.Quaternion());
                }

                const FVector* S = Scale.Find(Key);
                if (S)
                {
                    Transform.SetScale3D(*S);
                }
                InCallback(Key, Transform);
            }
        }
    }
}

bool FTranslateRotateScaleTimeSamples::HasAnyTimeSamples() const
{
    return (GetNumTranslateTimeSamples() > 0 || GetNumRotateTimeSamples() > 0 || GetNumScaleTimeSamples() > 0);
}

int32 FTransformTimeSamples::GetNumTranslateTimeSamples() const
{
    return Transform.Num();
}

int32 FTransformTimeSamples::GetNumRotateTimeSamples() const
{
    return Transform.Num();
}

int32 FTransformTimeSamples::GetNumScaleTimeSamples() const
{
    return Transform.Num();
}

void FTransformTimeSamples::TraverseTranslate(FTraverseCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Transform)
        {
            InCallback(Sample.Key, Sample.Value.GetTranslation());
        }
    }
}
void FTransformTimeSamples::TraverseRotate(FTraverseCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Transform)
        {
            FRotator Rotator = Sample.Value.GetRotation().Rotator();
            InCallback(Sample.Key, FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw));
        }
    }
}
void FTransformTimeSamples::TraverseScale(FTraverseCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Transform)
        {
            InCallback(Sample.Key, Sample.Value.GetScale3D());
        }
    }
}

void FTransformTimeSamples::Traverse(FTraverseTransformCallback InCallback) const
{
    if (InCallback)
    {
        for (auto Sample : Transform)
        {
            InCallback(Sample.Key, Sample.Value);
        }
    }
}

bool FTransformTimeSamples::HasAnyTimeSamples() const
{
    return (Transform.Num() > 0);
}

bool FNamedParameterTimeSamples::HasAnyParameter() const
{
    return (BooleanTimeSamples.Num() + ScalarTimeSamples.Num() + VectorTimeSamples.Num() + ColorTimeSamples.Num()) > 0;
}

bool FNamedParameterTimeSamples::HasNamedParameter(const FName& Name, ENamedParameterType Type) const
{
    switch(Type)
    {
    case Boolean:
        return BooleanTimeSamples.Find(Name) != nullptr;
    case Scalar:
        return ScalarTimeSamples.Find(Name) != nullptr;
    case Vector:
        return VectorTimeSamples.Find(Name) != nullptr;
    case Color:
        return ColorTimeSamples.Find(Name) != nullptr;
    }

    return false;
}

void FNamedParameterTimeSamples::TraverseBooleanTimeSamples(const FName& Name, FTraverseBooleanCallback InCallback) const
{
    auto TimeSamples = BooleanTimeSamples.Find(Name);
    if (TimeSamples && InCallback)
    {
        for (auto Sample : TimeSamples->TimeSamples)
        {
            InCallback(Sample.Key, Sample.Value);
        }
    }
}

void FNamedParameterTimeSamples::TraverseScalarTimeSamples(const FName& Name, FTraverseScalarCallback InCallback) const
{
    auto TimeSamples = ScalarTimeSamples.Find(Name);
    if (TimeSamples && InCallback)
    {
        for (auto Sample : TimeSamples->TimeSamples)
        {
            InCallback(Sample.Key, Sample.Value);
        }
    }
}

void FNamedParameterTimeSamples::TraverseVectorTimeSamples(const FName& Name, FTraverseVectorCallback InCallback) const
{
    auto TimeSamples = VectorTimeSamples.Find(Name);
    if (TimeSamples && InCallback)
    {
        for (auto Sample : TimeSamples->TimeSamples)
        {
            InCallback(Sample.Key, TimeSamples->ChannelsUsed, Sample.Value);
        }
    }
}

void FNamedParameterTimeSamples::TraverseColorTimeSamples(const FName& Name, FTraverseColorCallback InCallback) const
{
    auto TimeSamples = ColorTimeSamples.Find(Name);
    if (TimeSamples && InCallback)
    {
        for (auto Sample : TimeSamples->TimeSamples)
        {
            InCallback(Sample.Key, Sample.Value);
        }
    }
}

void FBooleanTimeSamples::TraverseTimeSamples(FTraverseBooleanCallback InCallback) const
{
    for (auto Sample : TimeSamples)
    {
        InCallback(Sample.Key, Sample.Value);
    }
}

void FFloatTimeSamples::TraverseTimeSamples(FTraverseScalarCallback InCallback) const
{
    for (auto Sample : TimeSamples)
    {
        InCallback(Sample.Key, Sample.Value);
    }
}

void FVectorTimeSamples::TraverseTimeSamples(FTraverseVectorCallback InCallback) const
{
    for (auto Sample : TimeSamples)
    {
        InCallback(Sample.Key, ChannelsUsed, Sample.Value);
    }
}

void FColorTimeSamples::TraverseTimeSamples(FTraverseColorCallback InCallback) const
{
    for (auto Sample : TimeSamples)
    {
        InCallback(Sample.Key, Sample.Value);
    }
}
