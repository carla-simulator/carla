// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Containers/ArrayBuilder.h"


#define UNIFORM	TEXT("uniform ")
#define DEFAULT_TEXTURE_TYPE TEXT("png")
#define DEFAULT_HDR_TEXTURE_TYPE TEXT("exr")
#define DDS_TEXTURE_TYPE TEXT("dds")
#define DEFAULT_CUBE_TEXTURE_TYPE DDS_TEXTURE_TYPE

static const TArray<FName> UE4Templates = TArrayBuilder<FName>()
    .Add(TEXT("OmniUe4Function"))
    .Add(TEXT("OmniUe4Base"))
    .Add(TEXT("OmniUe4Unlit"))
    .Add(TEXT("OmniUe4Translucent"))
    .Add(TEXT("OmniUe4Subsurface"))
    .Add(TEXT("OmniUe4FunctionExtension17"))
    // deprecated
    .Add(TEXT("Ue4Function"))
    .Add(TEXT("Ue4basedMDL"))
    .Add(TEXT("Ue4Unlit"))
    .Add(TEXT("Ue4Translucent"))
    .Add(TEXT("Ue4Subsurface"));

static const TArray<FName> BaseTemplates = TArrayBuilder<FName>()
    .Add(TEXT("OmniPBR"))
    .Add(TEXT("OmniPBR_Opacity"))
    .Add(TEXT("OmniPBRBase"))
    .Add(TEXT("OmniGlass"))
    .Add(TEXT("OmniGlass_Opacity"))
    .Add(TEXT("OmniPBR_ClearCoat"))
    .Add(TEXT("OmniPBR_ClearCoat_Opacity"))
    .Add(TEXT("OmniSurface"))
    .Add(TEXT("OmniSurfaceBlend"))
    .Add(TEXT("OmniSurfaceLite"))
    .Add(TEXT("OmniSurfacePresets"))
    .Add(TEXT("OmniHair"))
    .Add(TEXT("OmniHairPresets"))
    .Add(TEXT("SimPBR"))
    .Add(TEXT("SimPBR_Model"))
    .Add(TEXT("SimPBR_Translucent"))
    .Add(TEXT("gltf/pbr"));

static const TArray<FName> ApertureTemplates = TArrayBuilder<FName>()
    .Add(TEXT("AperturePBR"))
    .Add(TEXT("AperturePBR_Opacity"))
    .Add(TEXT("AperturePBR_ThinOpaque"))
    .Add(TEXT("AperturePBR_ThinTranslucent"))
    .Add(TEXT("AperturePBR_Translucent"));

static const TArray<FName> MaterialGraphTemplates = TArrayBuilder<FName>()
    .Add(TEXT("nvidia/core_definitions")) //
    .Add(TEXT("nvidia/aux_definitions")) // ref core_definitions
    .Add(TEXT("nvidia/support_definitions")) // ref aux_definitions
    .Add(TEXT("OmniSurface/OmniSurfaceBase"))
    .Add(TEXT("OmniSurface/OmniSurfaceBlendBase"))
    .Add(TEXT("OmniSurface/OmniSurfaceLiteBase"))
    .Add(TEXT("OmniSurface/OmniHairBase"));

static const TArray<FName> ReallusionTemplates = TArrayBuilder<FName>()
    .Add(TEXT("OmniRL_PBR"))
    .Add(TEXT("OmniReallusion_PBR"))
    .Add(TEXT("OmniRL_SSS"))
    .Add(TEXT("OmniReallusion_SSS"))
    .Add(TEXT("OmniRLEye"))
    .Add(TEXT("OmniReallusionEye"))
    .Add(TEXT("OmniRL_DHHead"))
    .Add(TEXT("OmniReallusion_DHHead"));

static const TArray<FName> DistillOffTemplates = TArrayBuilder<FName>()
    .Add(TEXT("OmniGlass"))
    .Add(TEXT("OmniGlass_Opacity"))
    .Add(TEXT("OmniHair"))
    .Add(TEXT("SimPBR_Translucent"));

static const TArray<FName> MinecraftTemplates = TArrayBuilder<FName>()
    .Add(TEXT("OmniImageNN"))
    .Add(TEXT("OmniSurfaceLiteNN"));

enum MDLSamplerType
{
    ST_Color,
    ST_Normalmap,
    ST_Grayscale,
    ST_Alpha,
    ST_Masks,
    ST_DistanceField,
    ST_MAX
};

static const TCHAR* SamplerAnnotations[MDLSamplerType::ST_MAX]
{
    TEXT("sampler_color()"),
    TEXT("sampler_normal()"),
    TEXT("sampler_grayscale()"),
    TEXT("sampler_alpha()"),
    TEXT("sampler_masks()"),
    TEXT("sampler_distancefield()"),
};

enum MDLUE4Property
{
    UE4_DistillOff,
    UE4_DitherMaskedOff,
    UE4_WorldSpaceNormal,
    UE4_MAX
};

static const TCHAR* UE4Annotations[MDLUE4Property::UE4_MAX]
{
    TEXT("distill_off()"),
    TEXT("dither_masked_off()"),
    TEXT("world_space_normal()")
};

enum MDLSceneData
{
    SD_DisplayColor,
    SD_DisplayOpacity,
    SD_MAX
};

static const TCHAR* MDLSceneDataString[MDLSceneData::SD_MAX]
{
    TEXT("displayColor"),
    TEXT("displayOpacity")
};