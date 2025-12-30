// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SimReadyPxr.h"

class SIMREADYUSD_API ISimReadyUSDTranslator
{
public:
    virtual ~ISimReadyUSDTranslator()
    {
    }

    virtual bool IsActorSupported(const class AActor* Actor) = 0;

    virtual bool IsComponentSupported(const class USceneComponent* Component) = 0;

    virtual bool IsObjectSupported(const class UObject* Object) = 0;

    virtual void GetExportNameAndFullName(const class USceneComponent* Component, FString& Name, FString& FullName) = 0;

    virtual pxr::UsdGeomXformable TranslateObjectToUSD(class UObject* Object, pxr::UsdStageRefPtr USDStage, const pxr::SdfPath& PrimPath) = 0;

    virtual pxr::UsdGeomXformable TranslateComponentToUSD(class USceneComponent* Component, pxr::UsdStageRefPtr USDStage, const pxr::SdfPath& PrimPath) = 0;

};

