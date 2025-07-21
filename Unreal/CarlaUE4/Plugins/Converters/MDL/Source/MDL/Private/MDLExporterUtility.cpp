// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLExporterUtility.h"
#include "MDLTemplateExporter.h"
#include "Materials/Material.h"
#include "Containers/ArrayBuilder.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "MDLKeywords.h"

TMap<UTexture*, FString>	FMDLExporterUtility::GlobalUsedTextures;
TMap<UTexture*, bool>	FMDLExporterUtility::GlobalExportedTextures;
TMap<UMaterial*, TArray<FMDLTranslatorResult>> FMDLExporterUtility::GlobalExportedMaterials; // This is for checking if the material has been exported
TMap<FString, FMDLTranslatorResult> FMDLExporterUtility::GlobalCachedMDL; // This is for checking if the same mdl file was existed
int32 FMDLExporterUtility::GlobalTextureID = 0;

bool RemoveUnderlineAtStart(FString& Value)
{
    if (Value.StartsWith(TEXT("_")))
    {
        Value = Value.RightChop(1);
        return true;
    }

    return false;
}

bool ReplaceNumberAtStart(FString& Value)
{
    if (Value.StartsWith(TEXT("0")) || Value.StartsWith(TEXT("1"))
    || Value.StartsWith(TEXT("2")) || Value.StartsWith(TEXT("3"))
    || Value.StartsWith(TEXT("4")) || Value.StartsWith(TEXT("5"))
    || Value.StartsWith(TEXT("6")) || Value.StartsWith(TEXT("7"))
    || Value.StartsWith(TEXT("8")) || Value.StartsWith(TEXT("9")))
    {
        Value.InsertAt(0, TEXT("Num"));
        return true;
    }

    return false;
}

static const TArray<FString> MDLReservedWords = TArrayBuilder<FString>()
    .Add(FString(TEXT("annotation")))
    .Add(FString(TEXT("bool")))
    .Add(FString(TEXT("bool2")))
    .Add(FString(TEXT("bool3")))
    .Add(FString(TEXT("bool4")))
    .Add(FString(TEXT("break")))
    .Add(FString(TEXT("bsdf")))
    .Add(FString(TEXT("bsdf_measurement")))
    .Add(FString(TEXT("case")))
    .Add(FString(TEXT("color")))
    .Add(FString(TEXT("const")))
    .Add(FString(TEXT("continue")))
    .Add(FString(TEXT("default")))
    .Add(FString(TEXT("do")))
    .Add(FString(TEXT("double")))
    .Add(FString(TEXT("double2")))
    .Add(FString(TEXT("double2x2")))
    .Add(FString(TEXT("double2x3")))
    .Add(FString(TEXT("double3")))
    .Add(FString(TEXT("double3x2")))
    .Add(FString(TEXT("double3x3")))
    .Add(FString(TEXT("double3x4")))
    .Add(FString(TEXT("double4")))
    .Add(FString(TEXT("double4x3")))
    .Add(FString(TEXT("double4x4")))
    .Add(FString(TEXT("double4x2")))
    .Add(FString(TEXT("double2x4")))
    .Add(FString(TEXT("edf")))
    .Add(FString(TEXT("else")))
    .Add(FString(TEXT("enum")))
    .Add(FString(TEXT("export")))
    .Add(FString(TEXT("false")))
    .Add(FString(TEXT("float")))
    .Add(FString(TEXT("float2")))
    .Add(FString(TEXT("float2x2")))
    .Add(FString(TEXT("float2x3")))
    .Add(FString(TEXT("float3")))
    .Add(FString(TEXT("float3x2")))
    .Add(FString(TEXT("float3x3")))
    .Add(FString(TEXT("float3x4")))
    .Add(FString(TEXT("float4")))
    .Add(FString(TEXT("float4x3")))
    .Add(FString(TEXT("float4x4")))
    .Add(FString(TEXT("float4x2")))
    .Add(FString(TEXT("float2x4")))
    .Add(FString(TEXT("for")))
    .Add(FString(TEXT("if")))
    .Add(FString(TEXT("import")))
    .Add(FString(TEXT("in")))
    .Add(FString(TEXT("int")))
    .Add(FString(TEXT("int2")))
    .Add(FString(TEXT("int3")))
    .Add(FString(TEXT("int4")))
    .Add(FString(TEXT("intensity_mode")))
    .Add(FString(TEXT("intensity_power")))
    .Add(FString(TEXT("intensity_radiant_exitance")))
    .Add(FString(TEXT("let")))
    .Add(FString(TEXT("light_profile")))
    .Add(FString(TEXT("material")))
    .Add(FString(TEXT("material_emission")))
    .Add(FString(TEXT("material_geometry")))
    .Add(FString(TEXT("material_surface")))
    .Add(FString(TEXT("material_volume")))
    .Add(FString(TEXT("mdl")))
    .Add(FString(TEXT("module")))
    .Add(FString(TEXT("package")))
    .Add(FString(TEXT("return")))
    .Add(FString(TEXT("string")))
    .Add(FString(TEXT("struct")))
    .Add(FString(TEXT("switch")))
    .Add(FString(TEXT("texture_2d")))
    .Add(FString(TEXT("texture_3d")))
    .Add(FString(TEXT("texture_cube")))
    .Add(FString(TEXT("texture_ptex")))
    .Add(FString(TEXT("true")))
    .Add(FString(TEXT("typedef")))
    .Add(FString(TEXT("uniform")))
    .Add(FString(TEXT("using")))
    .Add(FString(TEXT("varying")))
    .Add(FString(TEXT("vdf")))
    .Add(FString(TEXT("while")))
    .Add(FString(TEXT("auto")))
    .Add(FString(TEXT("catch")))
    .Add(FString(TEXT("char")))
    .Add(FString(TEXT("class")))
    .Add(FString(TEXT("const_cast")))
    .Add(FString(TEXT("delete")))
    .Add(FString(TEXT("dynamic_cast")))
    .Add(FString(TEXT("explicit")))
    .Add(FString(TEXT("extern")))
    .Add(FString(TEXT("external")))
    .Add(FString(TEXT("foreach")))
    .Add(FString(TEXT("friend")))
    .Add(FString(TEXT("goto")))
    .Add(FString(TEXT("graph")))
    .Add(FString(TEXT("half")))
    .Add(FString(TEXT("half2")))
    .Add(FString(TEXT("half2x2")))
    .Add(FString(TEXT("half2x3")))
    .Add(FString(TEXT("half3")))
    .Add(FString(TEXT("half3x2")))
    .Add(FString(TEXT("half3x3")))
    .Add(FString(TEXT("half3x4")))
    .Add(FString(TEXT("half4")))
    .Add(FString(TEXT("half4x3")))
    .Add(FString(TEXT("half4x4")))
    .Add(FString(TEXT("half4x2")))
    .Add(FString(TEXT("half2x4")))
    .Add(FString(TEXT("inline")))
    .Add(FString(TEXT("inout")))
    .Add(FString(TEXT("lambda")))
    .Add(FString(TEXT("long")))
    .Add(FString(TEXT("mutable")))
    .Add(FString(TEXT("namespace")))
    .Add(FString(TEXT("native")))
    .Add(FString(TEXT("new")))
    .Add(FString(TEXT("operator")))
    .Add(FString(TEXT("out")))
    .Add(FString(TEXT("phenomenon")))
    .Add(FString(TEXT("private")))
    .Add(FString(TEXT("protected")))
    .Add(FString(TEXT("public")))
    .Add(FString(TEXT("reinterpret_cast")))
    .Add(FString(TEXT("sampler")))
    .Add(FString(TEXT("shader")))
    .Add(FString(TEXT("short")))
    .Add(FString(TEXT("signed")))
    .Add(FString(TEXT("sizeof")))
    .Add(FString(TEXT("static")))
    .Add(FString(TEXT("static_cast")))
    .Add(FString(TEXT("technique")))
    .Add(FString(TEXT("template")))
    .Add(FString(TEXT("this")))
    .Add(FString(TEXT("throw")))
    .Add(FString(TEXT("try")))
    .Add(FString(TEXT("typeid")))
    .Add(FString(TEXT("typename")))
    .Add(FString(TEXT("union")))
    .Add(FString(TEXT("unsigned")))
    .Add(FString(TEXT("virtual")))
    .Add(FString(TEXT("void")))
    .Add(FString(TEXT("volatile")))
    .Add(FString(TEXT("wchar_t")));


FString FMDLExporterUtility::GetLegalIdentifier(const FString& InName, bool bRemoveSpaces)
{
    FString OutName = TCHAR_TO_UTF8(*InName);

    if (bRemoveSpaces)
    {
        OutName.RemoveSpacesInline();
    }
    else
    {
        OutName = OutName.Replace(TEXT(" "), TEXT("__space__"));
    }

    // NOTE::UE4 Material Parameter allowed a lot of characters which can't be as the input of mdl
    int32 NameLen = OutName.Len();
    for (int32 Index = 0; Index < NameLen; ++Index)
    {
        if ((OutName[Index] < '0' || OutName[Index] > '9')
            && (OutName[Index] < 'A' || OutName[Index] > 'Z')
            && (OutName[Index] < 'a' || OutName[Index] > 'z')
            && OutName[Index] != '_')
        {
            OutName[Index] = '_';
        }
    }

    for(auto ReservedWord : MDLReservedWords)
    {
        if (ReservedWord.Equals(OutName))
        {
            OutName[0] = FChar::ToUpper(OutName[0]);
            break;
        }
    }

    while (ReplaceNumberAtStart(OutName) || RemoveUnderlineAtStart(OutName));

    return OutName;
}

bool FMDLExporterUtility::IsLegalIdentifier(const FString& InName)
{
    if (InName.StartsWith(TEXT("0")) || InName.StartsWith(TEXT("1"))
    || InName.StartsWith(TEXT("2")) || InName.StartsWith(TEXT("3"))
    || InName.StartsWith(TEXT("4")) || InName.StartsWith(TEXT("5"))
    || InName.StartsWith(TEXT("6")) || InName.StartsWith(TEXT("7"))
    || InName.StartsWith(TEXT("8")) || InName.StartsWith(TEXT("9"))
    || InName.StartsWith(TEXT("_"))	)
    {
        return false;
    }

    if (InName.Contains(TEXT("(")) || InName.Contains(TEXT(")"))
    || InName.Contains(TEXT("+")) || InName.Contains(TEXT("-"))
    || InName.Contains(TEXT("*")) || InName.Contains(TEXT("/"))
    || InName.Contains(TEXT(".")))
    {
        return false;
    }

    for(auto ReservedWord : MDLReservedWords)
    {
        if (ReservedWord.Equals(InName))
        {
            return false;
        }
    }

    return true;
}


bool FMDLExporterUtility::ExportMDL(UMaterialInterface* InMaterial, const FMDLExporterSetting& InSetting, FString& MDL, FMDLTranslatorResult* OutResult, FExportTextureCallback InCallback)
{
    if (InSetting.bResetCache)
    {
        ResetCaches();
    }

    TSharedPtr<FMDLTemplateExporter> TemplateExporter = MakeShareable<FMDLTemplateExporter>(new FMDLTemplateExporter(InMaterial, InSetting, InCallback));
    return TemplateExporter->ExportModule(MDL, OutResult);

}

FString FMDLExporterUtility::GetFileExtension(class UTexture* Texture, bool bDDSTexture)
{
    //TODO: Virtual Texture to DDS
    return (bDDSTexture && !Texture->IsCurrentlyVirtualTextured()) ? DDS_TEXTURE_TYPE : (Texture->IsA<UTextureCube>() ? DEFAULT_CUBE_TEXTURE_TYPE : Texture->Source.GetFormat() == TSF_RGBA16F ? DEFAULT_HDR_TEXTURE_TYPE : DEFAULT_TEXTURE_TYPE);
}

FName FMDLExporterUtility::GetTemplateName(EMDL_Templates Template)
{
    if (Template < UE4Templates.Num())
    {
        return UE4Templates[Template];
    }
    return FName();
}

FString FMDLExporterUtility::GetFullPathOfUE4Templates()
{
    return FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin("MDL")->GetBaseDir() / TEXT("Library/mdl/Ue4"));
}

FString FMDLExporterUtility::GetFullPathOfBaseTemplates()
{
    return FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin("MDL")->GetBaseDir() / TEXT("Library/mdl/Base"));
}

FString FMDLExporterUtility::GetFullPathOfSurfaceTemplates()
{
    return FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin("MDL")->GetBaseDir() / TEXT("Library/mdl/mdl"));
}

void FMDLExporterUtility::ResetCaches()
{
    GlobalUsedTextures.Reset();
    GlobalExportedTextures.Reset();
    GlobalExportedMaterials.Reset();
    GlobalCachedMDL.Reset();
}
