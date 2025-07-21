// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

class ISectionExtractor
{
public:
    virtual ~ISectionExtractor() {}

public:
    virtual void Extract(const struct FExtractorInput& Input) = 0;

    virtual bool GetBooleanTimeSamples(struct FBooleanTimeSamples& BoolTimeSamples) { return false; }
    virtual bool GetFloatTimeSamples(struct FFloatTimeSamples& FloatTimeSamples) { return false; }
    virtual bool GetVectorTimeSamples(struct FVectorTimeSamples& VectorTimeSamples) { return false; }
    virtual bool GetColorTimeSamples(struct FColorTimeSamples& ColorTimeSamples) { return false; }
    virtual bool GetTransformTimeSamples(struct FTranslateRotateScaleTimeSamples& TransformTimeSamples) { return false; }
    virtual bool GetParameterTimeSamples(struct FNamedParameterTimeSamples& ParameterTimeSamples) { return false; }
    virtual bool GetObjectBindingTimeSamples(struct FObjectBindingTimeSamples& ObjectBindingTimeSamples) { return false; }
    virtual bool GetSkeletalAnimationTimeSamples(struct FSkeletalAnimationTimeSamples& OutSkeletalAnimationTimeSamples) { return false; }
};