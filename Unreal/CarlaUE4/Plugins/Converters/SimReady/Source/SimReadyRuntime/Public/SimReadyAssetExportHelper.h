// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "SimReadyExportSettings.h"
#include "SceneTypes.h"

#define CARLA_FAKE_INTERIOR_MAT		TEXT("M_FakeInterior")

namespace FSimReadyExportMessageSeverity
{
    enum Type
    {
        Error,
        Warning,
    };
};

namespace FSimReadyMaterialTemplateType
{
    enum Type
    {
        UE4,
        Base,
        Surface,
    };
};

struct SIMREADYRUNTIME_API FSimReadyExportMaterialResult
{
    // Copied from MDLExporterUtility.h 
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

    TMap<EMaterialProperty, FLinearColor> ConstantProperties;
    TMap<FName, FString> ScalarParameters;
    TMap<FName, FString> VectorParameters;
    TMap<FName, FString> TextureParameters;
    uint32				UsageFlag = 0;

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

class SIMREADYRUNTIME_API FSimReadyAssetExportHelper
{
public:
    typedef TFunction<void(const class UTexture*)> FExportTexturePreCallback;

    static FString GetTextureFileName(class UTexture* InTexture, bool bDDSTexture = false);
    static FString GetLegalMDLName(const FString& InName);
    static bool ExportRawDataToPath(const void* Data, int32 DataSize, const FString& InPath, const int32 Width, const int32 Height, const ERGBFormat Format, const int32 BitDepth);
    static bool ExportTextureToPath(class UTexture* InTexture, const FString& InPath, const FSimReadyExportTextureSettings& TextureSettings = FSimReadyExportTextureSettings());
    static bool ExportMaterialToPath(class UMaterialInterface* InMaterial, const FString& InPath, FString& OutLegalName, bool bUploadToServer, bool bModular, bool bUpZAxis, bool bLandscape, const FSimReadyExportMaterialSettings& MaterialSettings, FSimReadyExportMaterialResult* OutResult = nullptr, FExportTexturePreCallback PreExportCallback = nullptr);
    static void ResetGlobalCaches();
    static bool IsTextureExported(class UTexture* InTexture);
    static void RegisterExportedTexture(class UTexture* InTexture);
    static FString GetUniqueTextureName(class UTexture* InTexture, bool bDDSTexture = false);
    static void LogMessage(const UObject* InObject, FSimReadyExportMessageSeverity::Type Type, FString Message);
    static void CopyTemplate(FSimReadyMaterialTemplateType::Type Type, const FString& DestPath, const FString& TemplateName);
    static TArray<FString> GetImportList(const FString& Name);
};