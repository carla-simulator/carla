// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadyPxr.h"

class FUSDHashGenerator
{
public:
    static FSHAHash ComputeSHAHash(const pxr::UsdGeomMesh& USDMesh);
    static FSHAHash ComputeSHAHash(const pxr::UsdSkelCache& SkelCache, const pxr::UsdSkelBinding& SkelBinding);
    static FSHAHash ComputeSHAHash(const pxr::UsdSkelAnimQuery& AnimQuery, double StartTimeCode, double EndTimeCode, double TimeCodesPerSecond);
    static FSHAHash ComputeSHAHash(const pxr::UsdSkelSkeleton& SkelSkeleton, bool bWithRestTransforms);
    static FSHAHash ComputeSHAHash(const pxr::UsdShadeMaterial& USDMaterial);
};