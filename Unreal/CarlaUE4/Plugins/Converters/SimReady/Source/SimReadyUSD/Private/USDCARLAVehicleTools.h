// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadyPxr.h"


class FUSDCARLAVehicleTools
{
public:
    static void SplitVehicleSkelByJoints(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdSkelRoot& SkelRoot);
    static void ConvertSkelToMesh(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdSkelRoot& SkelRoot);
    static void SplitVehicleLightByTexcoords(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& MeshPrim);
    static bool BelongToCarlaVehicle(const class USceneComponent* Component);
    static bool IsCarlaVehicleSubMesh(const FString& Name, int SectionIndex);
    static bool IsCarlaVehicleLight(const class UStaticMeshComponent* StaticMeshComponent);
    static bool IsCarlaVehicleLightMaterial(const class UMaterialInterface* MaterialInterface);
    static bool IsCarlaVehiclePlateMaterial(const class UMaterialInterface* MaterialInterface);
    static bool IsCarlaVehicleSirenLightMaterial(const class UMaterialInterface* MaterialInterface);
    static void BindMaterialsToSplitVehicleLight(const pxr::UsdStageRefPtr& USDStage, const TArray<pxr::UsdPrim>& MeshPrims, const class UMaterialInterface* Material);
    
    static void ExportSimReadyVehicleLight(const pxr::UsdPrim& Prim, const class ULightComponentBase& LightComponent);
    static void ExportSimReadyVehicleComponentType(const pxr::UsdPrim& Prim, const class UStaticMeshComponent* StaticMeshComponent);
    static void ExportSimReadyVehicleComponentType(const pxr::UsdPrim& Prim, const class UMaterialInterface* MaterialInterface);
    static void ExportSimReadyVehicle(const pxr::UsdPrim& Prim, const class UPrimitiveComponent& Component);

    static void ExportSimReadyVehicleNonVisualGlass(const pxr::UsdPrim& Prim);
    static void ExportSimReadyVehicleNonVisualMaterial(const pxr::UsdPrim& Prim, const class UMaterialInterface& MaterialInterface);
    static void ExportSimReadyVehicleNonVisualRubber(const pxr::UsdPrim& Prim);

    static void InitPartOffsets(const pxr::UsdStageRefPtr& USDStage);
    static FVector GetPartOffset(const pxr::UsdGeomMesh& USDMesh);
};