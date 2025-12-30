// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInterface.h"

#define CARLA_FAKE_INTERIOR_MAT		TEXT("M_FakeInterior")

// UTexture was the texture to export
// FString was the folder name to save textures
typedef TFunction<void(UTexture*, const FString&, const FString&)> FExportTextureCallback;

enum EMDLMaterialUsage
{
    MaterialUsage_TwoSidedSign,
    MaterialUsage_Extension17,
    MaterialUsage_PerlinGradientTexture,
    MaterialUsage_Perlin3DTexture,
    MaterialUsage_VertexColor,
    MaterialUsage_PerInstanceData,
    MaterialUsage_AOMaterialMaskTexture,
    MaterialUsage_LightMapCoordinate,
    MaterialUsage_MaxTexCoordinate
};

struct MDL_API FMDLTranslatorResult
{
    TMap<FName, FString>						ScalarParameters;
    TMap<FName, FString>						VectorParameters;
    TMap<FName, FString>						TextureParameters;
    TMap<FName, FString>						BoolParameters;
    TArray<TSharedRef<class FTokenizedMessage>> Messages;
    TMap<EMaterialProperty, FLinearColor>		ConstantProperties;

    // Material Compiled
    FStaticParameterSet							StaticParamSet;
    EBlendMode									BlendMode;
    EMaterialShadingModel						ShadingModel;
    float										OpacityMaskClipValue;
    bool										TwoSided;
    bool										LandscapeMaterial;
    FString										MDLName;

    // Material Usage
    uint32										UsageFlag;

    void SetUsageFlag(EMDLMaterialUsage Usage)
    {
        UsageFlag |= 1 << Usage;
    }

    void ClearUsageFlag(EMDLMaterialUsage Usage)
    {
        UsageFlag &= ~(1 << Usage);
    }

    bool IsTwoSidedSignUsed() const
    {
        return (UsageFlag >> MaterialUsage_TwoSidedSign) & 0x1;
    }

    bool IsExtension17Used() const
    {
        return (UsageFlag >> MaterialUsage_Extension17) & 0x1;
    }

    bool IsPerlinGradientTextureUsed() const
    {
        return (UsageFlag >> MaterialUsage_PerlinGradientTexture) & 0x1;
    }

    bool IsPerlin3DTextureUsed() const
    {
        return (UsageFlag >> MaterialUsage_Perlin3DTexture) & 0x1;
    }

    bool IsVertexColorUsed() const
    {
        return (UsageFlag >> MaterialUsage_VertexColor) & 0x1;
    }

    bool IsPerInstanceDataUsed() const
    {
        return (UsageFlag >> MaterialUsage_PerInstanceData) & 0x1;
    }

    bool IsAOMaterialMaskTextureUsed() const
    {
        return (UsageFlag >> MaterialUsage_AOMaterialMaskTexture) & 0x1;
    }

    bool IsLightMapCoordinateUsed() const
    {
        return (UsageFlag >> MaterialUsage_LightMapCoordinate) & 0x1;
    }

    bool IsMaxTexCoordinateUsed() const
    {
        return (UsageFlag >> MaterialUsage_MaxTexCoordinate) & 0x1;
    }
};

struct MDL_API FMDLExporterSetting
{
    FString Name;
    bool bModular;
    bool bResetCache;
    bool bUpZAxis;
    bool bLandscape;
    bool bIgnoreCache;
    bool bDDSTexture;
    bool bRayTracingTranslucency;
    bool bRayTracingRefraction;

    FMDLExporterSetting(const FString& InName, bool bInModular = false, bool bInResetCache = false, bool bInUpZAxis = true, bool bInLandscape = false, bool bInIgnoreCache = false, bool bInDDSTexture = false, bool bInRayTracingTranslucency = false, bool bInRayTracingRefraction = true)
        : Name(InName)
        , bModular(bInModular)
        , bResetCache(bInResetCache)
        , bUpZAxis(bInUpZAxis)
        , bLandscape(bInLandscape)
        , bIgnoreCache(bInIgnoreCache)
        , bDDSTexture(bInDDSTexture)
        , bRayTracingTranslucency(bInRayTracingTranslucency)
        , bRayTracingRefraction(bInRayTracingRefraction)
    { }
};

class MDL_API FMDLExporterUtility
{
public:
    enum EMDL_Templates
    {
        MDL_Function,
        MDL_Base,
        MDL_Unlit,
        MDL_Translucent,
        MDL_Subsurface,
        MDL_FunctionExtension17,
        MDL_MAX_Templates
    };

    static FName GetTemplateName(EMDL_Templates Template);
    static bool IsLegalIdentifier(const FString& InName);
    static FString GetLegalIdentifier(const FString& InName, bool bRemoveSpaces = false);
    static bool ExportMDL(UMaterialInterface* InMaterial, const FMDLExporterSetting& InSetting, FString& MDL, FMDLTranslatorResult* OutResult = nullptr, FExportTextureCallback InCallback = nullptr);
    static FString GetFileExtension(class UTexture* Texture, bool bDDSTexture = false);
    static FString GetFullPathOfUE4Templates();
    static FString GetFullPathOfBaseTemplates();
    static FString GetFullPathOfSurfaceTemplates();
    static void ResetCaches();

public:
    static TMap<UTexture*, FString>	GlobalUsedTextures;
    static TMap<UTexture*, bool>	GlobalExportedTextures;
    static TMap<UMaterial*, TArray<FMDLTranslatorResult>> GlobalExportedMaterials;
    static TMap<FString, FMDLTranslatorResult> GlobalCachedMDL;
    static int32					GlobalTextureID;
};