// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Engine/HLSLMaterialTranslator.h"
#include "MDLKeywords.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Containers/ArrayBuilder.h"
#include "MaterialExpressions.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MDLPathUtility.h"

using namespace MDLPathUtility;

#define FL(x) (*FString::SanitizeFloat(x))
#define DefaultRefraction (1.491f)
#define MAX_NUM_TEX_COORD_USD (4)

static const TArray<FName> ReservedParameters = TArrayBuilder<FName>()
.Add(TEXT("AOMaterialMaskTexture"))
.Add(TEXT("LightMapCoordinateIndex"))
.Add(TEXT("LightmapCoordinateScale"))
.Add(TEXT("LightmapCoordinateBias"))
.Add(TEXT("PerlinNoiseGradientTexture"))
.Add(TEXT("PerlinNoise3DTexture"))
.Add(TEXT("VertexColorCoordinateIndex"))
.Add(TEXT("NumberInstances"))
.Add(TEXT("TwoSidedSign"))
.Add(TEXT("MaxTexCoordIndex"))
.Add(TEXT("DynamicParameter1"))
.Add(TEXT("DynamicParameter2"))
.Add(TEXT("DynamicParameter3"))
.Add(TEXT("DynamicParameter4"));

class FMDLMaterialTranslator : public FHLSLMaterialTranslator
{

public:
    FMDLMaterialTranslator(FMaterial* InMaterial,
        FMaterialCompilationOutput& InMaterialCompilationOutput,
        const FStaticParameterSet& InStaticParameters,
        EShaderPlatform InPlatform,
        EMaterialQualityLevel::Type InQualityLevel,
        ERHIFeatureLevel::Type InFeatureLevel,
        UMaterial* InUMaterial,
        UMaterialInstance* InMaterialInstance,
        const FString& InPath,
        bool bLandscapeMaterial,
        bool bDDSTexture,
        bool bRayTracingTranslucency,
        bool bRayTracingRefraction,
        bool bUpZAxis
    )
        : FHLSLMaterialTranslator(InMaterial, InMaterialCompilationOutput, InStaticParameters, InPlatform, InQualityLevel, InFeatureLevel)
        , MaterialAsset(InUMaterial)
        , MaterialInstance(InMaterialInstance)
        , TexturePath(InPath)
        , CurrentTextureCache(&LocalUsedTextures)
        // Usage
        , bPerlinGradientTextureUsed(false)
        , bPerlin3DTextureUsed(false)
        , bVertexColorUsed(false)
        , bPerInstanceDataUsed(false)
        , bTwoSidedSignUsed(false)
        , bExtension17Used(false)
        , bAOMaterialMaskUsed(false)
        , bLightMapCoordinateUsed(false)
        , bMaxTexCoordinateUsed(false)
        , bSceneColorUsed(false)
        , bSceneDepthOpacity(false)
        //
        , LandscapeMaterial(bLandscapeMaterial)
        , DDSTexture(bDDSTexture)
        , InvalidRefraction(false)
        , RayTracingTranslucency(bRayTracingTranslucency)
        , RayTracingRefraction(bRayTracingRefraction)
        , UpZAxis(bUpZAxis)
    {
        ReplaceTextureCube = InUMaterial && InUMaterial->GetName() == CARLA_FAKE_INTERIOR_MAT;
    }


public:
    // porting utility
    static FString FMaterialAttributeDefinitionMap_GetDisplayName(const EMaterialProperty Property)
    {
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
        return FMaterialAttributeDefinitionMap::GetDisplayName(Property);
#else
        return FMaterialAttributeDefinitionMap::GetAttributeName(Property);
#endif
    }

    // interface Material Compiler
    virtual const TCHAR* HLSLTypeString(EMaterialValueType Type) const override
    {
        switch (Type)
        {
        case MCT_Float1:		return TEXT("float");
        case MCT_Float2:		return TEXT("float2");
        case MCT_Float3:		return TEXT("float3");
        case MCT_Float4:		return TEXT("float4");
        case MCT_Float:			return TEXT("float");
        case MCT_Texture2D:		return TEXT("texture_2d");
        case MCT_TextureCube:	return TEXT("texture_cube");
        case MCT_VolumeTexture:	return TEXT("texture_3d");
        case MCT_StaticBool:	return TEXT("bool");
        case MCT_MaterialAttributes:	return TEXT("MaterialAttributes");
        case MCT_TextureExternal:	return TEXT("texture_2d");
        case MCT_TextureVirtual:		return TEXT("texture_2d");
        case MCT_ShadingModel:			return TEXT("int");
        default:				return TEXT("unknown");
        };

    }

    const TCHAR* GetSamplerType(const UTexture* InTexture) const
    {
        switch (InTexture->CompressionSettings)
        {
            case TC_Normalmap:
                return SamplerAnnotations[ST_Normalmap];
            case TC_Grayscale:
                return SamplerAnnotations[ST_Grayscale];
            case TC_Alpha:
                return SamplerAnnotations[ST_Alpha];
            case TC_Masks:
                return SamplerAnnotations[ST_Masks];
            case TC_DistanceFieldFont:
                return SamplerAnnotations[ST_DistanceField];
            default:
                return SamplerAnnotations[ST_Color];
        }
    }

    bool IsReservedParameter(FName ParameterName)
    {
        for (auto ReservedParameter : ReservedParameters)
        {
            if (ReservedParameter == ParameterName)
            {
                return true;
            }
        }

        return false;
    }

    virtual int32 AccessCollectionParameter(UMaterialParameterCollection* ParameterCollection, int32 ParameterIndex, int32 ComponentIndex) override
    {
        if (!ParameterCollection || ParameterIndex == -1)
        {
            return INDEX_NONE;
        }

        int32 CollectionIndex = ParameterCollections.Find(ParameterCollection);

        if (CollectionIndex == INDEX_NONE)
        {
            if (ParameterCollections.Num() >= MaxNumParameterCollectionsPerMaterial)
            {
                return Error(TEXT("Material references too many MaterialParameterCollections!  A material may only reference 2 different collections."));
            }

            ParameterCollections.Add(ParameterCollection);
            CollectionIndex = ParameterCollections.Num() - 1;
        }

        const int32 VectorParameterBase = FMath::DivideAndRoundUp(ParameterCollection->ScalarParameters.Num(), 4);

        if (ParameterIndex >= VectorParameterBase) // Vector Parameters
        {
            const FCollectionVectorParameter& Parameter = ParameterCollection->VectorParameters[ParameterIndex - VectorParameterBase];
            return VectorParameter(Parameter.ParameterName, Parameter.DefaultValue);
        }
        else // Scalar Parameters
        {
            const FCollectionScalarParameter& Parameter = ParameterCollection->ScalarParameters[ParameterIndex * 4 + ComponentIndex];
            return ScalarParameter(Parameter.ParameterName, Parameter.DefaultValue);
        }
    }

    bool GetScalarParameterRange(UMaterialExpression* Expression, float& Min, float& Max)
    {
        if (Expression && Expression->IsA<UMaterialExpressionScalarParameter>())
        {
            auto ScalarParameter = Cast<UMaterialExpressionScalarParameter>(Expression);
            Min = ScalarParameter->SliderMin;
            Max = ScalarParameter->SliderMax;
            return Min < Max;
        }

        return false;
    }

    template<typename T>
    T* CheckParameterExpression(UMaterialExpression* Expression, FName ParameterName)
    {
        if (Expression->IsA<T>())
        {
            T* ParameterExpression = Cast<T>(Expression);
            if (ParameterExpression->ParameterName == ParameterName)
            {
                return ParameterExpression;
            }
        }
        else if (Expression->IsA<UMaterialExpressionMaterialFunctionCall>())
        {
            UMaterialExpressionMaterialFunctionCall* Function = Cast<UMaterialExpressionMaterialFunctionCall>(Expression);

            if (Function->MaterialFunction && Function->MaterialFunction->GetFunctionExpressions())
            {
                for (auto FunctionExpression : *Function->MaterialFunction->GetFunctionExpressions())
                {
                    T* ParameterExpression = CheckParameterExpression<T>(FunctionExpression, ParameterName);
                    if (ParameterExpression)
                    {
                        return ParameterExpression;
                    }
                }
            }
        }

        return nullptr;
    }

    template<typename T>
    T* GetParameterExpression(FName ParameterName)
    {
        if (ParameterName.IsNone())
        {
            return nullptr;
        }

        for (auto Expression : MaterialAsset->Expressions)
        {
            T* ParameterExpression = CheckParameterExpression<T>(Expression, ParameterName);
            if (ParameterExpression)
            {
                return ParameterExpression;
            }
        }

        return nullptr;
    }

    template<typename T>
    FString DefineAnnotation(FName ParameterName, bool bHidden = false, const FString& Custom = TEXT(""))
    {
        FString Annotations;
        bool bCustom = !Custom.IsEmpty();
        bool bDesc = false;
        bool bPriority = false;
        bool bGroup = false;
        float Min = 0, Max = 0;
        bool bRange = false;
        T* Expression = GetParameterExpression<T>(ParameterName);
        if (Expression)
        {
            bDesc = !Expression->Desc.IsEmpty();
            bPriority = Expression->SortPriority > 0;
            bGroup = !Expression->Group.IsNone();	
            bRange = GetScalarParameterRange(Expression, Min, Max);
        }

        // DisplayName / Description / UI priority / Group / Slide Range need getting from Expression. Make sure it's valid before defining the annotation.
        // Hidden and Custom are for special cases, don't need valid expression.
        if (!ParameterName.IsNone() || bDesc || bPriority || bGroup || bRange
            || bHidden 
            || bCustom)
        {
            Annotations += TEXT("\r\n\t[[\r\n\t\t");
            FString Annotation;
            if (!ParameterName.IsNone())
            {
                Annotation += FString::Printf(TEXT("anno::display_name(\"%s\")"), *(ParameterName.ToString().Replace(TEXT("\""), TEXT(""))));
            }
            if (bDesc)
            {
                Annotation += FString::Printf(TEXT("%sanno::description(\"%s\")"),
                    Annotation.IsEmpty() ? TEXT("") : TEXT(",\r\n\t\t"), *Expression->Desc.Replace(TEXT("\""), TEXT("\\\"")));
            }
            if (bPriority)
            {
                Annotation += FString::Printf(TEXT("%sanno::ui_order(%d)"),
                    Annotation.IsEmpty() ? TEXT("") : TEXT(",\r\n\t\t"), Expression->SortPriority);
            }
            if (bGroup)
            {
                Annotation += FString::Printf(TEXT("%sanno::in_group(\"%s\")"),
                    Annotation.IsEmpty() ? TEXT("") : TEXT(",\r\n\t\t"), *Expression->Group.ToString().Replace(TEXT("\""), TEXT("\\\"")));
            }
            if (bHidden)
            {
                Annotation += FString::Printf(TEXT("%sanno::hidden()"),
                    Annotation.IsEmpty() ? TEXT("") : TEXT(",\r\n\t\t"));
            }
            if (bRange)
            {
                Annotation += FString::Printf(TEXT("%sanno::soft_range(%s, %s)"),
                    Annotation.IsEmpty() ? TEXT("") : TEXT(",\r\n\t\t"), FL(Min), FL(Max));
            }
            if (bCustom)
            {
                Annotation += FString::Printf(TEXT("%s%s"),
                    Annotation.IsEmpty() ? TEXT("") : TEXT(",\r\n\t\t"), *Custom);
            }
            Annotations += Annotation;
            Annotations += TEXT("\r\n\t]]");
        }

        return Annotations;
    }

    virtual int32 ScalarParameter(FName ParameterName, float DefaultValue) override
    {
        FString Name = FMDLExporterUtility::GetLegalIdentifier(ParameterName.ToString());

        // instance replace the material
        float ReplacedValue = 0.0f;
        if (!GetScalarValue(MaterialInstance, ParameterName, ReplacedValue))
        {
            ReplacedValue = DefaultValue;
        }

        FString* DefinedName = DefinedScalarParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            if (IsReservedParameter(*Name))
            {
                Name = TEXT("Local") + Name;
            }

            while (DefinedParameterNames.Find(Name) != INDEX_NONE)
            {
                // Same name was found, should be changed
                Name += TEXT("_sp");
            }

            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(ParameterName);
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                IsRefractionProperty() ? UNIFORM : TEXT(""),
                HLSLTypeString(MCT_Float), *Name, FL(ReplacedValue), 
                Annotation.IsEmpty() ? TEXT("") : *Annotation));
            DefinedScalarParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);
            if (IsRefractionProperty())
            {
                IORParameters.Add(Name, Name);
            }
        }
        else
        {
            Name = *DefinedName;

            // IOR needs uniform parameter
            if (IsRefractionProperty())
            {
                // Redefine uniform parameter
                FString* DefinedIORName = IORParameters.Find(Name);
                if (DefinedIORName)
                {
                    Name = *DefinedIORName;
                }
                else
                {
                    FString IORName = Name + TEXT("_ior");
                    FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(ParameterName);
                    TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s%s"),
                        TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                        UNIFORM,
                        HLSLTypeString(MCT_Float), *IORName, FL(ReplacedValue), 
                        Annotation.IsEmpty() ? TEXT("") : *Annotation));
                    IORParameters.Add(Name, IORName);
                    DefinedParameterNames.Add(IORName);
                    Name = IORName;
                }
            }
        }

        int32 Ret = AddInlinedCodeChunk(MCT_Float,TEXT("%s"),*Name);
        if (FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel)
        {
            ConstantChunks.Add(Ret, FLinearColor(ReplacedValue, 0, 0, 0));
        }
        return Ret;
    }

    virtual int32 VectorParameter(FName ParameterName, const FLinearColor& DefaultValue) override
    {
        // SelectionColor is not needed
        if (ParameterName.IsEqual(NAME_SelectionColor))
        {
            return INDEX_NONE;
        }

        FString Name = FMDLExporterUtility::GetLegalIdentifier(ParameterName.ToString());
        
        FLinearColor ReplacedValue;
        if (!GetVectorValue(MaterialInstance, ParameterName, ReplacedValue))
        {
            ReplacedValue = DefaultValue;
        }

        FString* DefinedName = DefinedVectorParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            if (IsReservedParameter(*Name))
            {
                Name = TEXT("Local") + Name;
            }

            while (DefinedParameterNames.Find(Name) != INDEX_NONE)
            {
                // Same name was found, should be changed
                Name += TEXT("_vp");
            }

            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(ParameterName);
            // instance replace the material	
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s(%s,%s,%s,%s)%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                IsRefractionProperty() ? UNIFORM : TEXT(""),
                HLSLTypeString(MCT_Float4), *Name,
                HLSLTypeString(MCT_Float4), FL(ReplacedValue.R), FL(ReplacedValue.G), FL(ReplacedValue.B), FL(ReplacedValue.A),
                Annotation.IsEmpty() ? TEXT("") : *Annotation));

            DefinedVectorParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);

            if (IsRefractionProperty())
            {
                IORParameters.Add(Name, Name);
            }
        }
        else
        {
            Name = *DefinedName;

            // IOR needs uniform parameter
            if (IsRefractionProperty())
            {
                FString* DefinedIORName = IORParameters.Find(Name);

                if (DefinedIORName)
                {
                    Name = *DefinedIORName;
                }
                else
                {
                    FString IORName = Name + TEXT("_ior");
                    FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(ParameterName);
                    // instance replace the material	
                    TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s(%s,%s,%s,%s)%s"),
                        TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                        UNIFORM,
                        HLSLTypeString(MCT_Float4), *IORName,
                        HLSLTypeString(MCT_Float4), FL(ReplacedValue.R), FL(ReplacedValue.G), FL(ReplacedValue.B), FL(ReplacedValue.A),
                        Annotation.IsEmpty() ? TEXT("") : *Annotation));

                    IORParameters.Add(Name, IORName);
                    DefinedParameterNames.Add(IORName);
                    Name = IORName;
                }
            }
        }

        int32 Ret = AddInlinedCodeChunk(MCT_Float4,TEXT("%s"),*Name);
        if (FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel)
        {
            ConstantChunks.Add(Ret, ReplacedValue);
        }
        return Ret;
    }

#if 0
    virtual int32 StaticBoolParameter(FName ParameterName,bool bDefaultValue) override
    {
        // Look up the value we are compiling with for this static parameter.
        bool bValue = bDefaultValue;

        FMaterialParameterInfo ParameterInfo = GetParameterAssociationInfo();
        ParameterInfo.Name = ParameterName;

        for (const FStaticSwitchParameter& Parameter : StaticParameters.StaticSwitchParameters)
        {
            if (Parameter.ParameterInfo == ParameterInfo)
            {
                bValue = Parameter.Value;
                break;
            }
        }

        FString Name = RegisterBoolParameter(ParameterName, bValue);
        return AddInlinedCodeChunk(MCT_StaticBool,TEXT("%s"),*Name);
    }
#endif

    virtual int32 Constant(float X) override
    {
        int32 Ret = AddInlinedCodeChunk(MCT_Float,TEXT("%s"),FL(X));
        if (FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel)
        {
            ConstantChunks.Add(Ret, FLinearColor(X, 0, 0, 0));
        }
        ConstantExpressions.Add(Ret, FLinearColor(X, 0, 0, 0));
        return Ret;
    }

    virtual int32 Constant2(float X,float Y) override
    {
        int32 Ret = AddInlinedCodeChunk(MCT_Float2,TEXT("float2(%s,%s)"),FL(X),FL(Y));
        if (FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel)
        {
            ConstantChunks.Add(Ret, FLinearColor(X, Y, 0, 0));
        }
        ConstantExpressions.Add(Ret, FLinearColor(X, Y, 0, 0));
        return Ret;
    }

    virtual int32 Constant3(float X,float Y,float Z) override
    {
        int32 Ret = AddInlinedCodeChunk(MCT_Float3,TEXT("float3(%s,%s,%s)"),FL(X),FL(Y),FL(Z));
        if (FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel)
        {
            ConstantChunks.Add(Ret, FLinearColor(X, Y, Z, 0));
        }
        ConstantExpressions.Add(Ret, FLinearColor(X, Y, Z, 0));
        return Ret;
    }

    virtual int32 Constant4(float X,float Y,float Z,float W) override
    {
        int32 Ret = AddInlinedCodeChunk(MCT_Float4,TEXT("float4(%s,%s,%s,%s)"),FL(X),FL(Y),FL(Z),FL(W));
        if (FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel)
        {
            ConstantChunks.Add(Ret, FLinearColor(X, Y, Z, W));
        }
        ConstantExpressions.Add(Ret, FLinearColor(X, Y, Z, W));
        return Ret;
    }

    virtual int32 Sine(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Sin),MCT_Float,TEXT("math::sin(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::sin(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Cosine(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Cos),MCT_Float,TEXT("math::cos(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::cos(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Tangent(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Tan),MCT_Float,TEXT("math::tan(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::tan(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Arcsine(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Asin),MCT_Float,TEXT("math::asin(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::asin(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 ArcsineFast(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Asin),MCT_Float,TEXT("math::asin(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::asin(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Arccosine(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Acos),MCT_Float,TEXT("math::acos(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::acos(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 ArccosineFast(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Acos),MCT_Float,TEXT("math::acos(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::acos(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Arctangent(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Atan),MCT_Float,TEXT("math::atan(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::atan(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 ArctangentFast(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(X),TMO_Atan),MCT_Float,TEXT("math::atan(%s)"),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::atan(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Arctangent2(int32 Y, int32 X) override
    {
        if(Y == INDEX_NONE || X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(Y) && GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(Y),GetParameterUniformExpression(X),TMO_Atan2),MCT_Float,TEXT("math::atan2(%s, %s)"),*CoerceParameter(Y,MCT_Float),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(Y),TEXT("math::atan2(%s, %s)"),*GetParameterCode(Y),*GetParameterCode(X));
        }
    }

    virtual int32 Arctangent2Fast(int32 Y, int32 X) override
    {
        if(Y == INDEX_NONE || X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(Y) && GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTrigMath(GetParameterUniformExpression(Y),GetParameterUniformExpression(X),TMO_Atan2),MCT_Float,TEXT("math::atan2(%s, %s)"),*CoerceParameter(Y,MCT_Float),*CoerceParameter(X,MCT_Float));
        }
        else
        {
            return AddCodeChunk(GetParameterType(Y),TEXT("math::atan2(%s, %s)"),*GetParameterCode(Y),*GetParameterCode(X));
        }
    }

    virtual int32 Floor(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionFloor(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("math::floor(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::floor(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Ceil(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionCeil(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("math::ceil(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::ceil(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Round(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionRound(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("math::round(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::round(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Truncate(int32 X) override
    {
        // TODO:MDL
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionTruncate(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("trunc(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("trunc(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 Sign(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionSign(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("math::sign(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::sign(%s)"),*GetParameterCode(X));
        }
    }	

    virtual int32 Frac(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionFrac(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("math::frac(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::frac(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 PeriodicHint(int32 PeriodicCode) override
    {
        if (PeriodicCode == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        return AddCodeChunk(GetParameterType(PeriodicCode), TEXT("math::frac(%s)"), *GetParameterCode(PeriodicCode));
    }

    virtual int32 Fmod(int32 A, int32 B) override
    {
        if ((A == INDEX_NONE) || (B == INDEX_NONE))
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
        {
            return AddUniformExpression(new FMaterialUniformExpressionFmod(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),
                GetParameterType(A),TEXT("math::fmod(%s,%s)"),*GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
        }
        else
        {
            return AddCodeChunk(GetParameterType(A),
                TEXT("math::fmod(%s,%s)"),*GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
        }
    }

    /**
    * Creates the new shader code chunk needed for the Abs expression
    *
    * @param	X - Index to the FMaterialCompiler::CodeChunk entry for the input expression
    * @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
    */	
    virtual int32 Abs( int32 X ) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        // get the user input struct for the input expression
        FMaterialUniformExpression* pInputParam = GetParameterUniformExpression(X);
        if( pInputParam )
        {
            FMaterialUniformExpressionAbs* pUniformExpression = new FMaterialUniformExpressionAbs( pInputParam );
            return AddUniformExpression( pUniformExpression, GetParameterType(X), TEXT("math::abs(%s)"), *GetParameterCode(X) );
        }
        else
        {
            return AddCodeChunk( GetParameterType(X), TEXT("math::abs(%s)"), *GetParameterCode(X) );
        }
    }

    virtual int32 Dot(int32 A, int32 B) override
    {
        if (A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FMaterialUniformExpression* ExpressionA = GetParameterUniformExpression(A);
        FMaterialUniformExpression* ExpressionB = GetParameterUniformExpression(B);

        EMaterialValueType TypeA = GetParameterType(A);
        EMaterialValueType TypeB = GetParameterType(B);
        if (ExpressionA && ExpressionB)
        {
            if (TypeA == MCT_Float && TypeB == MCT_Float)
            {
                return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(ExpressionA, ExpressionB, FMO_Mul), MCT_Float, TEXT("(%s * %s)"), *GetParameterCode(A), *GetParameterCode(B));
            }
            else
            {
                if (TypeA == TypeB)
                {
                    return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(ExpressionA, ExpressionB, FMO_Dot, TypeA), MCT_Float, TEXT("math::dot(%s,%s)"), *GetParameterCode(A), *GetParameterCode(B));
                }
                else
                {
                    // Promote scalar (or truncate the bigger type)
                    if (TypeA == MCT_Float || (TypeB != MCT_Float && GetNumComponents(TypeA) > GetNumComponents(TypeB)))
                    {
                        return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(ExpressionA, ExpressionB, FMO_Dot, TypeB), MCT_Float, TEXT("math::dot(%s,%s)"), *CoerceParameter(A, TypeB), *GetParameterCode(B));
                    }
                    else
                    {
                        return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(ExpressionA, ExpressionB, FMO_Dot, TypeA), MCT_Float, TEXT("math::dot(%s,%s)"), *GetParameterCode(A), *CoerceParameter(B, TypeA));
                    }
                }
            }
        }
        else
        {
            // Promote scalar (or truncate the bigger type)
            if (TypeA == MCT_Float || (TypeB != MCT_Float && GetNumComponents(TypeA) > GetNumComponents(TypeB)))
            {
                return AddCodeChunk(MCT_Float, TEXT("math::dot(%s, %s)"), *CoerceParameter(A, TypeB), *GetParameterCode(B));
            }
            else
            {
                return AddCodeChunk(MCT_Float, TEXT("math::dot(%s, %s)"), *GetParameterCode(A), *CoerceParameter(B, TypeA));
            }
        }
    }

    virtual int32 Cross(int32 A, int32 B) override
    {
        if (A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
        {
            EMaterialValueType ResultType = GetArithmeticResultType(A, B);
            if (ResultType == MCT_Float2 || (ResultType & MCT_Float) == 0)
            {
                return Errorf(TEXT("Cross product requires 3-component vector input."));
            }
            return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A), GetParameterUniformExpression(B), FMO_Cross, ResultType), MCT_Float3, TEXT("math::cross(%s,%s)"), *GetParameterCode(A), *GetParameterCode(B));
        }
        else
        {
            return AddCodeChunk(MCT_Float3, TEXT("math::cross(%s,%s)"), *CoerceParameter(A, MCT_Float3), *CoerceParameter(B, MCT_Float3));
        }
    }

    virtual int32 Power(int32 Base, int32 Exponent) override
    {
        if (Base == INDEX_NONE || Exponent == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        // Clamp Pow input to >= 0 to help avoid common NaN cases
        EMaterialValueType ResultType = GetArithmeticResultType(Base, Exponent);
        const float POW_CLAMP = 0.000001f;
        return AddCodeChunk(ResultType, TEXT("math::pow(math::max(%s,%s(%s)),%s)"), *GetParameterCode(ForceCast(Base, ResultType)), HLSLTypeString(ResultType), FL(POW_CLAMP), *GetParameterCode(ForceCast(Exponent, ResultType)));
    }

    virtual int32 Logarithm2(int32 X) override
    {
        if (X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionLogarithm2(GetParameterUniformExpression(X)), GetParameterType(X), TEXT("math::log2(%s)"), *GetParameterCode(X));
        }

        return AddCodeChunk(GetParameterType(X), TEXT("math::log2(%s)"), *GetParameterCode(X));
    }

    virtual int32 Logarithm10(int32 X) override
    {
        if (X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionLogarithm10(GetParameterUniformExpression(X)), GetParameterType(X), TEXT("math::log10(%s)"), *GetParameterCode(X));
        }

        return AddCodeChunk(GetParameterType(X), TEXT("math::log10(%s)"), *GetParameterCode(X));
    }

    virtual int32 SquareRoot(int32 X) override
    {
        if (X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionSquareRoot(GetParameterUniformExpression(X)), GetParameterType(X), TEXT("math::sqrt(%s)"), *GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X), TEXT("math::sqrt(%s)"), *GetParameterCode(X));
        }
    }

    virtual int32 Length(int32 X) override
    {
        if (X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionLength(GetParameterUniformExpression(X), GetParameterType(X)), MCT_Float, TEXT("math::length(%s)"), *GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(MCT_Float, TEXT("math::length(%s)"), *GetParameterCode(X));
        }
    }

    virtual int32 Step(int32 Y, int32 X) override
    {
        if (X == INDEX_NONE || Y == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FMaterialUniformExpression* ExpressionX = GetParameterUniformExpression(X);
        FMaterialUniformExpression* ExpressionY = GetParameterUniformExpression(Y);

        EMaterialValueType ResultType = GetArithmeticResultType(X, Y);

        //Constant folding.
        if (ExpressionX && ExpressionY)
        {
            // when x == y return 1.0f
            if (ExpressionX == ExpressionY)
            {
                const int32 EqualResult = 1.0f;
                if (ResultType == MCT_Float || ResultType == MCT_Float1)
                {
                    return Constant(EqualResult);
                }
                if (ResultType == MCT_Float2)
                {
                    return Constant2(EqualResult, EqualResult);
                }
                if (ResultType == MCT_Float3)
                {
                    return Constant3(EqualResult, EqualResult, EqualResult);
                }
                if (ResultType == MCT_Float4)
                {
                    return Constant4(EqualResult, EqualResult, EqualResult, EqualResult);
                }
            }

            if (ExpressionX->IsConstant() && ExpressionY->IsConstant())
            {
                FLinearColor ValueX, ValueY;
                FMaterialRenderContext DummyContext(nullptr, *Material, nullptr);
                ExpressionX->GetNumberValue(DummyContext, ValueX);
                ExpressionY->GetNumberValue(DummyContext, ValueY);

                float Red = ValueX.R >= ValueY.R ? 1 : 0;
                if (ResultType == MCT_Float || ResultType == MCT_Float1)
                {
                    return Constant(Red);
                }

                float Green = ValueX.G >= ValueY.G ? 1 : 0;
                if (ResultType == MCT_Float2)
                {
                    return Constant2(Red, Green);
                }

                float Blue = ValueX.B >= ValueY.B ? 1 : 0;
                if (ResultType == MCT_Float3)
                {
                    return Constant3(Red, Green, Blue);
                }

                float Alpha = ValueX.A >= ValueY.A ? 1 : 0;
                if (ResultType == MCT_Float4)
                {
                    return Constant4(Red, Green, Blue, Alpha);
                }
            }
        }


        return AddCodeChunk(ResultType, TEXT("math::step(%s,%s)"), *CoerceParameter(Y, ResultType), *CoerceParameter(X, ResultType));
    }

    virtual int32 SmoothStep(int32 X, int32 Y, int32 A) override
    {
        if (X == INDEX_NONE || Y == INDEX_NONE || A == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FMaterialUniformExpression* ExpressionX = GetParameterUniformExpression(X);
        FMaterialUniformExpression* ExpressionY = GetParameterUniformExpression(Y);
        FMaterialUniformExpression* ExpressionA = GetParameterUniformExpression(A);
        bool bExpressionsAreEqual = false;

        // According to https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-smoothstep
        // Smoothstep's min and max and return result in the same size as the alpha.
        // Therefore the result type (and each input) should be GetParameterType(A);

        // However, for usability reasons, we will use the ArithmiticType of the three.
        // This is important to do, because it allows a user to input a vector into the min or max
        // and get a vector result, without putting inputs into the other two constants.
        // This is not exactly the behavior of raw HLSL, but it is a more intuitive experience
        // and mimics more closely the LinearInterpolate node.
        // Incompatible inputs will be caught by the CoerceParameters below.

        EMaterialValueType ResultType = GetArithmeticResultType(X, Y);
        ResultType = GetArithmeticResultType(ResultType, GetParameterType(A));


        // Skip over interpolations where inputs are equal

        float EqualResult = 0.0f;
        // smoothstep( x, y, y ) == 1.0
        if (Y == A)
        {
            bExpressionsAreEqual = true;
            EqualResult = 1.0f;
        }

        // smoothstep( x, y, x ) == 0.0
        if (X == A)
        {
            bExpressionsAreEqual = true;
            EqualResult = 0.0f;
        }

        if (bExpressionsAreEqual)
        {
            if (ResultType == MCT_Float || ResultType == MCT_Float1)
            {
                return Constant(EqualResult);
            }
            if (ResultType == MCT_Float2)
            {
                return Constant2(EqualResult, EqualResult);
            }
            if (ResultType == MCT_Float3)
            {
                return Constant3(EqualResult, EqualResult, EqualResult);
            }
            if (ResultType == MCT_Float4)
            {
                return Constant4(EqualResult, EqualResult, EqualResult, EqualResult);
            }
        }

        // smoothstep( x, x, a ) could create a div by zero depending on implementation.
        // The common implementation is to treat smoothstep as a step in these situations.
        if (X == Y)
        {
            bExpressionsAreEqual = true;
        }
        else if (ExpressionX && ExpressionY)
        {
            if (ExpressionX->IsConstant() && ExpressionY->IsConstant() && (*CurrentScopeChunks)[X].Type == (*CurrentScopeChunks)[Y].Type)
            {
                FLinearColor ValueX, ValueY;
                FMaterialRenderContext DummyContext(nullptr, *Material, nullptr);
                ExpressionX->GetNumberValue(DummyContext, ValueX);
                ExpressionY->GetNumberValue(DummyContext, ValueY);

                if (ValueX == ValueY)
                {
                    bExpressionsAreEqual = true;
                }
            }
        }

        if (bExpressionsAreEqual)
        {
            return Step(X, A);
        }

        //When all inputs are constant, we can precompile the operation.
        if (ExpressionX && ExpressionY && ExpressionA && ExpressionX->IsConstant() && ExpressionY->IsConstant() && ExpressionA->IsConstant())
        {
            FLinearColor ValueX, ValueY, ValueA;
            FMaterialRenderContext DummyContext(nullptr, *Material, nullptr);
            ExpressionX->GetNumberValue(DummyContext, ValueX);
            ExpressionY->GetNumberValue(DummyContext, ValueY);
            ExpressionA->GetNumberValue(DummyContext, ValueA);

            float Red = FMath::SmoothStep(ValueX.R, ValueY.R, ValueA.R);
            if (ResultType == MCT_Float || ResultType == MCT_Float1)
            {
                return Constant(Red);
            }

            float Green = FMath::SmoothStep(ValueX.G, ValueY.G, ValueA.G);
            if (ResultType == MCT_Float2)
            {
                return Constant2(Red, Green);
            }

            float Blue = FMath::SmoothStep(ValueX.B, ValueY.B, ValueA.B);
            if (ResultType == MCT_Float3)
            {
                return Constant3(Red, Green, Blue);
            }

            float Alpha = FMath::SmoothStep(ValueX.A, ValueY.A, ValueA.A);
            if (ResultType == MCT_Float4)
            {
                return Constant4(Red, Green, Blue, Alpha);
            }
        }

        //NOTE: Do not use math::smoothstep because mdl version is different from hlsl version
        return AddCodeChunk(ResultType, TEXT("::smoothstep(%s,%s,%s)"), *CoerceParameter(X, ResultType), *CoerceParameter(Y, ResultType), *CoerceParameter(A, ResultType));
    }

    virtual int32 Lerp(int32 X,int32 Y,int32 A) override
    {
        if(X == INDEX_NONE || Y == INDEX_NONE || A == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        bool bExpressionsAreEqual = false;

        // Skip over interpolations where inputs are equal
        if (X == Y)
        {
            bExpressionsAreEqual = true;
        }
        else if (ConstantExpressions.Find(X) != nullptr && ConstantExpressions.Find(Y) != nullptr && (*CurrentScopeChunks)[X].Type == (*CurrentScopeChunks)[Y].Type)
        {
            {
                FLinearColor ValueX(0, 0, 0), ValueY(0, 0, 0);
                ValueX = *ConstantExpressions.Find(X);
                ValueY = *ConstantExpressions.Find(Y);

                if (ValueX == ValueY)
                {
                    bExpressionsAreEqual = true;
                }
            }
        }

        if (bExpressionsAreEqual)
        {
            return X;
        }

        EMaterialValueType ResultType = GetArithmeticResultType(X,Y);
        EMaterialValueType AlphaType = ResultType == (*CurrentScopeChunks)[A].Type ? ResultType : MCT_Float1;

        if (AlphaType == MCT_Float1 && ConstantExpressions.Find(A) != nullptr)
        {
            // Skip over interpolations that explicitly select an input
            FLinearColor Value(0, 0, 0);
            Value = *ConstantExpressions.Find(A);

            if (Value.R == 0.0f)
            {
                return X;
            }
            else if (Value.R == 1.f)
            {
                return Y;
            }
        }

        if (IsRefractionProperty())
        {
            // check if math::lerp(1.0,Refraction,fresnel()), return Refraction
            const FShaderCodeChunk& CodeChunk = (*CurrentScopeChunks)[A];
            bool Fresnel = false;
            if (CodeChunk.Type == MCT_Float)
            {
                FString ACode = GetParameterCode(A);
                if (ACode.StartsWith(TEXT("::fresnel")))
                {
                    Fresnel = true;
                }
                else
                {
                    // check if not inline
                    ACode = FString::Printf(TEXT("\t%s %s = ::fresnel"), HLSLTypeString(MCT_Float), *ACode);
                    if (CodeChunk.Definition.StartsWith(ACode))
                    {
                        Fresnel = true;
                    }
                }
            }
            
            if (Fresnel)
            {
                return Y;
            }
            else
            {
                return AddCodeChunk(ResultType,TEXT("math::lerp(%s,%s,%s)"),*CoerceParameter(X,ResultType),*CoerceParameter(Y,ResultType),*CoerceParameter(A,AlphaType));
            }
        }
        else
        {
            return AddCodeChunk(ResultType,TEXT("math::lerp(%s,%s,%s)"),*CoerceParameter(X,ResultType),*CoerceParameter(Y,ResultType),*CoerceParameter(A,AlphaType));
        }
    }

    EMaterialValueType GetMinMaxType(EMaterialValueType TypeA, EMaterialValueType TypeB)
    {
        EMaterialValueType Type = MCT_Float;

        switch (TypeA)
        {
        case MCT_Float2:
            switch (TypeB)
            {
            case MCT_Float1:
            case MCT_Float:
            case MCT_Float2:
                Type = MCT_Float2;
                break;
            }
            break;
        case MCT_Float3:
            switch (TypeB)
            {
            case MCT_Float1:
            case MCT_Float:
            case MCT_Float3:
                Type = MCT_Float3;
                break;
            }
            break;
        case MCT_Float4:
            switch (TypeB)
            {
            case MCT_Float1:
            case MCT_Float:
            case MCT_Float4:
                Type = MCT_Float4;
                break;
            }
            break;
        case MCT_Float1:
        case MCT_Float:
            switch (TypeB)
            {
            case MCT_Float2:
                Type = MCT_Float2;
                break;
            case MCT_Float3:
                Type = MCT_Float3;
                break;
            case MCT_Float4:
                Type = MCT_Float4;
                break;
            }
            break;
        }

        return Type;
    }

    virtual int32 Min(int32 A,int32 B) override
    {
        if(A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
        {
            return AddUniformExpression(new FMaterialUniformExpressionMin(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetMinMaxType(GetParameterType(A),GetParameterType(B)),TEXT("math::min(%s,%s)"),*GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
        }
        else
        {
            return AddCodeChunk(GetMinMaxType(GetParameterType(A),GetParameterType(B)),TEXT("math::min(%s,%s)"),*GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
        }
    }

    virtual int32 Max(int32 A,int32 B) override
    {
        if(A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
        {
            return AddUniformExpression(new FMaterialUniformExpressionMax(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetMinMaxType(GetParameterType(A),GetParameterType(B)),TEXT("math::max(%s,%s)"),*GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
        }
        else
        {
            return AddCodeChunk(GetMinMaxType(GetParameterType(A),GetParameterType(B)),TEXT("math::max(%s,%s)"),*GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
        }
    }

    virtual int32 Clamp(int32 X,int32 A,int32 B) override
    {
        if(X == INDEX_NONE || A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X) && GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
        {
            return AddUniformExpression(new FMaterialUniformExpressionClamp(GetParameterUniformExpression(X),GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetMinMaxType(GetMinMaxType(GetParameterType(X),GetParameterType(A)),GetParameterType(B)),TEXT("math::min(math::max(%s,%s),%s)"),*GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
        }
        else
        {
            return AddCodeChunk(GetMinMaxType(GetMinMaxType(GetParameterType(X),GetParameterType(A)),GetParameterType(B)),TEXT("math::min(math::max(%s,%s),%s)"),*GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
        }
    }

    virtual int32 Saturate(int32 X) override
    {
        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(GetParameterUniformExpression(X))
        {
            return AddUniformExpression(new FMaterialUniformExpressionSaturate(GetParameterUniformExpression(X)),GetParameterType(X),TEXT("math::saturate(%s)"),*GetParameterCode(X));
        }
        else
        {
            return AddCodeChunk(GetParameterType(X),TEXT("math::saturate(%s)"),*GetParameterCode(X));
        }
    }

    virtual int32 RotateAboutAxis(int32 NormalizedRotationAxisAndAngleIndex, int32 PositionOnAxisIndex, int32 PositionIndex) override
    {
        if (NormalizedRotationAxisAndAngleIndex == INDEX_NONE
            || PositionOnAxisIndex == INDEX_NONE
            || PositionIndex == INDEX_NONE)
        {
            return INDEX_NONE;
        }
        else
        {
            return AddCodeChunk(
                MCT_Float3,
                TEXT("::rotate_about_axis(%s,%s,%s)"),
                *CoerceParameter(NormalizedRotationAxisAndAngleIndex,MCT_Float4),
                *CoerceParameter(PositionOnAxisIndex,MCT_Float3),
                *CoerceParameter(PositionIndex,MCT_Float3)
                );	
        }
    }

    virtual int32 RotateScaleOffsetTexCoords(int32 TexCoordCodeIndex, int32 RotationScale, int32 Offset) override
    {
        return AddCodeChunk(MCT_Float2,
            TEXT("::rotate_scale_offset_texcoords(%s, %s, float2(%s.x,%s.y))"),
            *GetParameterCode(TexCoordCodeIndex),
            *GetParameterCode(RotationScale),
            *GetParameterCode(Offset),
            *GetParameterCode(Offset));
    }

    virtual int32 PerInstanceFadeAmount() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Vertex)
        {
            return NonVertexOrPixelShaderExpressionError();
        }
        else
        {
            return AddInlinedCodeChunk(MCT_Float, TEXT("1.0"));
        }
    }

    virtual int32 ReflectionVector() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonPixelShaderExpressionError();
        }
        if (ShaderFrequency != SF_Vertex)
        {
            bUsesTransformVector = true;
        }
        MDLTranslatedWarning();
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3,TEXT("::reflection_vector(%s)"), *UpZ);
    }

    virtual int32 ReflectionAboutCustomWorldNormal(int32 CustomWorldNormal, int32 bNormalizeCustomWorldNormal) override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonPixelShaderExpressionError();
        }

        if (CustomWorldNormal == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (ShaderFrequency != SF_Vertex)
        {
            bUsesTransformVector = true;
        }

        const TCHAR* ShouldNormalize = (!!bNormalizeCustomWorldNormal) ? TEXT("true") : TEXT("false");
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddCodeChunk(MCT_Float3,TEXT("::reflection_custom_world_normal(%s, %s, %s)"), *GetParameterCode(CustomWorldNormal), ShouldNormalize, *UpZ);
    }

    virtual int32 TwoSidedSign() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonPixelShaderExpressionError();
        }
        RegisterFloatParameter(TEXT("TwoSidedSign"), 1.0f);
        bTwoSidedSignUsed = true;
        return AddInlinedCodeChunk(MCT_Float,TEXT("TwoSidedSign"));	
    }

    virtual int32 StaticTerrainLayerWeight(FName ParameterName,int32 Default) override
    {
        // Look up the weight-map index for this static parameter.
        int32 WeightmapIndex = INDEX_NONE;
        bool bFoundParameter = false;
        bool bAtLeastOneWeightBasedBlend = false;

        FMaterialParameterInfo ParameterInfo = GetParameterAssociationInfo();
        ParameterInfo.Name = ParameterName;

        int32 NumActiveTerrainLayerWeightParameters = 0;
        for (int32 ParameterIndex = 0; ParameterIndex < StaticParameters.TerrainLayerWeightParameters.Num(); ++ParameterIndex)
        {
            const FStaticTerrainLayerWeightParameter& Parameter = StaticParameters.TerrainLayerWeightParameters[ParameterIndex];
            if (Parameter.WeightmapIndex != INDEX_NONE)
            {
                NumActiveTerrainLayerWeightParameters++;
            }
            if (Parameter.ParameterInfo == ParameterInfo)
            {
                WeightmapIndex = Parameter.WeightmapIndex;
                bFoundParameter = true;
            }
            if (Parameter.bWeightBasedBlend)
            {
                bAtLeastOneWeightBasedBlend = true;
            }
        }

        if (!bFoundParameter)
        {
            return Default;
        }
        else
        {
            int32 WeightmapCode;
            {
                // Otherwise we sample normally
                const EMaterialSamplerType SamplerType = SAMPLERTYPE_Masks;
                FString WeightmapName = FString::Printf(TEXT("Weightmap%s"), *ParameterName.ToString());
                int32 TextureReferenceIndex = INDEX_NONE;
                int32 TextureCodeIndex = TextureParameter(FName(*WeightmapName), GEngine->WeightMapPlaceholderTexture, TextureReferenceIndex, SamplerType);
                WeightmapCode = TextureSample(TextureCodeIndex, TextureCoordinate(1, false, false), SamplerType);
            }

            //FString LayerMaskName = FString::Printf(TEXT("LayerMask_%s"), *ParameterName.ToString());
            return Dot(WeightmapCode, /*VectorParameter(FName(*LayerMaskName), FLinearColor(1.f, 0.f, 0.f, 0.f))*/Constant4(1.f, 0.f, 0.f, 0.f));
        }
    }

    virtual int32 GameTime(bool bPeriodic, float Period) override
    {
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with game time, replaced with default."));
            InvalidRefraction = true;
        }

        if (!bPeriodic)
        {
            return AddInlinedCodeChunk(MCT_Float, TEXT("state::animation_time()"));
        }
        else if (Period == 0.0f)
        {
            return Constant(0.0f);
        }

        return AddInlinedCodeChunk(MCT_Float, TEXT("state::animation_time()"));
    }

    virtual int32 RealTime(bool bPeriodic, float Period) override
    {
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with real time, replaced with default."));
            InvalidRefraction = true;
        }

        if (!bPeriodic)
        {
            return AddInlinedCodeChunk(MCT_Float, TEXT("state::animation_time()"));
        }
        else if (Period == 0.0f)
        {
            return Constant(0.0f);
        }

        return AddInlinedCodeChunk(MCT_Float, TEXT("state::animation_time()"));
    }

    virtual int32 TransformBase(EMaterialCommonBasis SourceCoordBasis, EMaterialCommonBasis DestCoordBasis, int32 A, int AWComponent) override
    {
        if (A == INDEX_NONE)
        {
            // unable to compile
            return INDEX_NONE;
        }

        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with Transform, replaced with default."));
            InvalidRefraction = true;
        }

        { // validation
            if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute && ShaderFrequency != SF_Domain && ShaderFrequency != SF_Vertex)
            {
                return NonPixelShaderExpressionError();
            }

            if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute && ShaderFrequency != SF_Vertex)
            {
                if ((SourceCoordBasis == MCB_Local || DestCoordBasis == MCB_Local))
                {
                    return Errorf(TEXT("Local space is only supported for vertex, compute or pixel shader"));
                }
            }

            if (AWComponent != 0 && (SourceCoordBasis == MCB_Tangent || DestCoordBasis == MCB_Tangent))
            {
                return Errorf(TEXT("Tangent basis not available for position transformations"));
            }

            // Construct float3(0,0,x) out of the input if it is a scalar
            // This way artists can plug in a scalar and it will be treated as height, or a vector displacement
            if (GetType(A) == MCT_Float1 && SourceCoordBasis == MCB_Tangent)
            {
                A = AppendVector(Constant2(0, 0), A);
            }
            else if (GetNumComponents(GetParameterType(A)) < 3)
            {
                return Errorf(TEXT("input must be a vector (%s: %s) or a scalar (if source is Tangent)"), *GetParameterCode(A), DescribeType(GetParameterType(A)));
            }
        }

        if (SourceCoordBasis == DestCoordBasis)
        {
            // no transformation needed
            return A;
        }

        FString CodeStr;
        EMaterialCommonBasis IntermediaryBasis = MCB_World;
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");

        switch (SourceCoordBasis)
        {
        case MCB_Tangent:
        {
            check(AWComponent == 0);
            if (DestCoordBasis == MCB_World)
            {
                CodeStr = TEXT("::transform_vector_from_tangent_to_world(float3(<A>.x,<A>.y,<A>.z), <UP_Z>)");
            }
            // else use MCB_World as intermediary basis
            break;
        }
        case MCB_Local:
        {
            if (DestCoordBasis == MCB_World)
            {
                // TODO: need <PREV>
                CodeStr = TEXT("::convert_to_left_hand(state::<TRANSFORM>(state::coordinate_object, state::coordinate_world, ::convert_to_left_hand(float3(<A>.x,<A>.y,<A>.z), <UP_Z>, <IS_POSITION>)), <UP_Z>, <IS_POSITION>)");
            }
            // else use MCB_World as intermediary basis
            break;
        }
        case MCB_TranslatedWorld:
        {
            // TODO? TranslatedWorld -> World
            if (DestCoordBasis == MCB_World)
            {
                if (AWComponent == 0)
                {
                    CodeStr = TEXT("<A>");
                }
                else
                {
                    return MDLTranslatedError();
                }
            }
            else if (DestCoordBasis == MCB_Camera)
            {
                return MDLTranslatedError();
            }
            else if (DestCoordBasis == MCB_View)
            {
                MDLTranslatedWarning();
                CodeStr = TEXT("<A>");
            }
            // else use MCB_World as intermediary basis
            break;
        }
        case MCB_World:
        {
            if (DestCoordBasis == MCB_Tangent)
            {
                CodeStr = TEXT("::transform_vector_from_world_to_tangent(float3(<A>.x,<A>.y,<A>.z), <UP_Z>)");
            }
            else if (DestCoordBasis == MCB_Local)
            {
                const EMaterialDomain Domain = (const EMaterialDomain)Material->GetMaterialDomain();

                if (Domain != MD_Surface && Domain != MD_Volume)
                {
                    // TODO: for decals we could support it
                    Errorf(TEXT("This transformation is only supported in the 'Surface' material domain."));
                    return INDEX_NONE;
                }

                CodeStr = TEXT("::convert_to_left_hand(state::<TRANSFORM>(state::coordinate_world, state::coordinate_object, ::convert_to_left_hand(float3(<A>.x,<A>.y,<A>.z), <UP_Z>, <IS_POSITION>)), <UP_Z>, <IS_POSITION>)");
            }
            else if (DestCoordBasis == MCB_TranslatedWorld)
            {
                if (AWComponent == 0)
                {
                    CodeStr = TEXT("<A>");
                }
                else
                {
                    return MDLTranslatedError();
                }
            }
            else if (DestCoordBasis == MCB_MeshParticle)
            {
                //bUsesParticleTransform = true;
                return MDLTranslatedError();
            }

            // else use MCB_TranslatedWorld as intermediary basis
            IntermediaryBasis = MCB_TranslatedWorld;
            break;
        }
        case MCB_Camera:
        {
            if (DestCoordBasis == MCB_TranslatedWorld)
            {
                return MDLTranslatedError();
            }
            // else use MCB_TranslatedWorld as intermediary basis
            IntermediaryBasis = MCB_TranslatedWorld;
            break;
        }
        case MCB_View:
        {
            if (DestCoordBasis == MCB_TranslatedWorld)
            {
                MDLTranslatedWarning();
                CodeStr = TEXT("<A>");
            }
            // else use MCB_TranslatedWorld as intermediary basis
            IntermediaryBasis = MCB_TranslatedWorld;
            break;
        }
        case MCB_MeshParticle:
        {
            if (DestCoordBasis == MCB_World)
            {
                //bUsesParticleTransform = true;
                return MDLTranslatedError();
            }
            else
            {
                return Errorf(TEXT("Can transform only to world space from particle space"));
            }
            break;
        }
        default:
            check(0);
            break;
        }

        if (CodeStr.IsEmpty())
        {
            // check intermediary basis so we don't have infinite recursion
            check(IntermediaryBasis != SourceCoordBasis);
            check(IntermediaryBasis != DestCoordBasis);

            // use intermediary basis
            const int32 IntermediaryA = TransformBase(SourceCoordBasis, IntermediaryBasis, A, AWComponent);

            return TransformBase(IntermediaryBasis, DestCoordBasis, IntermediaryA, AWComponent);
        }

        if (AWComponent != 0)
        {
            CodeStr.ReplaceInline(TEXT("<TRANSFORM>"), TEXT("transform_point"));
            CodeStr.ReplaceInline(TEXT("<IS_POSITION>"), TEXT("true"));
        }
        else
        {
            CodeStr.ReplaceInline(TEXT("<TRANSFORM>"), TEXT("transform_vector"));
            CodeStr.ReplaceInline(TEXT("<IS_POSITION>"), TEXT("false"));
        }

        CodeStr.ReplaceInline(TEXT("<UP_Z>"), *UpZ);
        CodeStr.ReplaceInline(TEXT("<A>"), *GetParameterCode(A));

        if (ShaderFrequency != SF_Vertex && (DestCoordBasis == MCB_Tangent || SourceCoordBasis == MCB_Tangent))
        {
            bUsesTransformVector = true;
        }

        return AddCodeChunk(
            MCT_Float3,
            *CodeStr
        );
    }

    virtual int32 ComponentMask(int32 Vector,bool R,bool G,bool B,bool A) override
    {
        if(Vector == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType	VectorType = GetParameterType(Vector);

        if(	(A && (VectorType & MCT_Float) < MCT_Float4) ||
            (B && (VectorType & MCT_Float) < MCT_Float3) ||
            (G && (VectorType & MCT_Float) < MCT_Float2) ||
            (R && (VectorType & MCT_Float) < MCT_Float1))
        {
            return Errorf(TEXT("Not enough components in (%s: %s) for component mask %u%u%u%u"),*GetParameterCode(Vector),DescribeType(GetParameterType(Vector)),R,G,B,A);
        }

        EMaterialValueType	ResultType;
        switch((R ? 1 : 0) + (G ? 1 : 0) + (B ? 1 : 0) + (A ? 1 : 0))
        {
        case 1: ResultType = MCT_Float; break;
        case 2: ResultType = MCT_Float2; break;
        case 3: ResultType = MCT_Float3; break;
        case 4: ResultType = MCT_Float4; break;
        default: 
            return Errorf(TEXT("Couldn't determine result type of component mask %u%u%u%u"),R,G,B,A);
        };

        FString MaskString = FString::Printf(TEXT("%s%s%s%s"),
            R ? TEXT("x") : TEXT(""),
            // If VectorType is set to MCT_Float which means it could be any of the float types, assume it is a float1
            G ? (VectorType == MCT_Float ? TEXT("x") : TEXT("y")) : TEXT(""),
            B ? (VectorType == MCT_Float ? TEXT("x") : TEXT("z")) : TEXT(""),
            A ? (VectorType == MCT_Float ? TEXT("x") : TEXT("w")) : TEXT("")
            );

        TArray<FString> Masks;
        if (R) Masks.Add(TEXT("x"));
        if (G) Masks.Add(VectorType == MCT_Float ? TEXT("x") : TEXT("y"));
        if (B) Masks.Add(VectorType == MCT_Float ? TEXT("x") : TEXT("z"));
        if (A) Masks.Add(VectorType == MCT_Float ? TEXT("x") : TEXT("w"));

        auto* Expression = GetParameterUniformExpression(Vector);

        // Same type, don't need mask
        if (ResultType == VectorType && !Expression)
        {
            return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("%s"),
                    *GetParameterCode(Vector)
                    );
        }

        switch(ResultType)
        {
        case MCT_Float:
        {
            if (Expression)
            {
                int8 Mask[4] = {-1, -1, -1, -1};
                for (int32 Index = 0; Index < MaskString.Len(); ++Index)
                {
                    Mask[Index] = SwizzleComponentToIndex(MaskString[Index]);
                }

                if (VectorType == MCT_Float)
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("%s"),
                        *GetParameterCode(Vector)
                        );
                }
                else
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("%s.%s"),
                        *GetParameterCode(Vector),
                        *Masks[0]
                        );
                }
            }

            if (VectorType == MCT_Float)
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("%s"),
                    *GetParameterCode(Vector)
                    );
            }
            else
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("%s.%s"),
                    *GetParameterCode(Vector),
                    *Masks[0]
                    );
            }
        }
        case MCT_Float2:
        {
            if (Expression)
            {
                int8 Mask[4] = {-1, -1, -1, -1};
                for (int32 Index = 0; Index < MaskString.Len(); ++Index)
                {
                    Mask[Index] = SwizzleComponentToIndex(MaskString[Index]);
                }

                if (VectorType == MCT_Float)
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("float2(%s,%s)"),
                        *GetParameterCode(Vector),
                        *GetParameterCode(Vector)
                        );
                }
                else
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("float2(%s.%s,%s.%s)"),
                        *GetParameterCode(Vector),
                        *Masks[0],
                        *GetParameterCode(Vector),
                        *Masks[1]
                        );
                }
            }

            if (VectorType == MCT_Float)
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("float2(%s,%s)"),
                    *GetParameterCode(Vector),
                    *GetParameterCode(Vector)
                    );
            }
            else
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("float2(%s.%s,%s.%s)"),
                    *GetParameterCode(Vector),
                    *Masks[0],
                    *GetParameterCode(Vector),
                    *Masks[1]
                    );
            }
        }
        case MCT_Float3:
        {
            if (Expression)
            {
                int8 Mask[4] = {-1, -1, -1, -1};
                for (int32 Index = 0; Index < MaskString.Len(); ++Index)
                {
                    Mask[Index] = SwizzleComponentToIndex(MaskString[Index]);
                }
                if (VectorType == MCT_Float)
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("float3(%s,%s,%s)"),
                        *GetParameterCode(Vector),
                        *GetParameterCode(Vector),
                        *GetParameterCode(Vector)
                        );
                }
                else
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("float3(%s.%s,%s.%s,%s.%s)"),
                        *GetParameterCode(Vector),
                        *Masks[0],
                        *GetParameterCode(Vector),
                        *Masks[1],
                        *GetParameterCode(Vector),
                        *Masks[2]
                        );
                }
            }

            if (VectorType == MCT_Float)
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("float3(%s,%s,%s)"),
                    *GetParameterCode(Vector),
                    *GetParameterCode(Vector),
                    *GetParameterCode(Vector)
                    );
            }
            else
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("float3(%s.%s,%s.%s,%s.%s)"),
                    *GetParameterCode(Vector),
                    *Masks[0],
                    *GetParameterCode(Vector),
                    *Masks[1],
                    *GetParameterCode(Vector),
                    *Masks[2]
                    );
            }
        }
        case MCT_Float4:
        {
            if (Expression)
            {
                int8 Mask[4] = {-1, -1, -1, -1};
                for (int32 Index = 0; Index < MaskString.Len(); ++Index)
                {
                    Mask[Index] = SwizzleComponentToIndex(MaskString[Index]);
                }
                if (VectorType == MCT_Float)
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("float4(%s,%s,%s,%s)"),
                        *GetParameterCode(Vector),
                        *GetParameterCode(Vector),
                        *GetParameterCode(Vector),
                        *GetParameterCode(Vector)
                        );
                }
                else
                {
                    return AddUniformExpression(
                        new FMaterialUniformExpressionComponentSwizzle(Expression, Mask[0], Mask[1], Mask[2], Mask[3]),
                        ResultType,
                        TEXT("float4(%s.%s,%s.%s,%s.%s,%s.%s)"),
                        *GetParameterCode(Vector),
                        *Masks[0],
                        *GetParameterCode(Vector),
                        *Masks[1],
                        *GetParameterCode(Vector),
                        *Masks[2],
                        *GetParameterCode(Vector),
                        *Masks[3]
                        );
                }
            }

            if (VectorType == MCT_Float)
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("float4(%s,%s,%s,%s)"),
                    *GetParameterCode(Vector),
                    *GetParameterCode(Vector),
                    *GetParameterCode(Vector),
                    *GetParameterCode(Vector)
                    );
            }
            else
            {
                return AddInlinedCodeChunk(
                    ResultType,
                    TEXT("float4(%s.%s,%s.%s,%s.%s,%s.%s)"),
                    *GetParameterCode(Vector),
                    *Masks[0],
                    *GetParameterCode(Vector),
                    *Masks[1],
                    *GetParameterCode(Vector),
                    *Masks[2],
                    *GetParameterCode(Vector),
                    *Masks[3]
                    );
            }
        }
        default:
            return Errorf(TEXT("Couldn't determine result type of component mask %u%u%u%u"),R,G,B,A);
        }
    }

    virtual int32 AppendVector(int32 A,int32 B) override
    {
        if(A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType ParameterTypeA = GetParameterType(A);
        EMaterialValueType ParameterTypeB = GetParameterType(B);

        int32 NumResultComponents = GetNumComponents(ParameterTypeA) + GetNumComponents(ParameterTypeB);
        EMaterialValueType	ResultType = GetVectorType(NumResultComponents);

        if (ResultType == MCT_Unknown)
        {
            return INDEX_NONE;
        }

        FString CodeParameterA;
        switch (ParameterTypeA)
        {
        case MCT_Float: 
        case MCT_Float1: CodeParameterA = FString::Printf(TEXT("%s"), *GetParameterCode(A)); break;
        case MCT_Float2: CodeParameterA = FString::Printf(TEXT("%s.x,%s.y"), *GetParameterCode(A), *GetParameterCode(A)); break;
        case MCT_Float3: CodeParameterA = FString::Printf(TEXT("%s.x,%s.y,%s.z"), *GetParameterCode(A), *GetParameterCode(A), *GetParameterCode(A)); break;
        case MCT_Float4: CodeParameterA = FString::Printf(TEXT("%s.x,%s.y,%s.z,%s.w"), *GetParameterCode(A), *GetParameterCode(A), *GetParameterCode(A), *GetParameterCode(A)); break;
        default: return INDEX_NONE;
        }

        FString CodeParameterB;
        switch (ParameterTypeB)
        {
        case MCT_Float:
        case MCT_Float1: CodeParameterB = FString::Printf(TEXT("%s"), *GetParameterCode(B)); break;
        case MCT_Float2: CodeParameterB = FString::Printf(TEXT("%s.x,%s.y"), *GetParameterCode(B), *GetParameterCode(B)); break;
        case MCT_Float3: CodeParameterB = FString::Printf(TEXT("%s.x,%s.y,%s.z"), *GetParameterCode(B), *GetParameterCode(B), *GetParameterCode(B)); break;
        case MCT_Float4: CodeParameterB = FString::Printf(TEXT("%s.x,%s.y,%s.z,%s.w"), *GetParameterCode(B), *GetParameterCode(B), *GetParameterCode(B), *GetParameterCode(B)); break;
        default: return INDEX_NONE;
        }

        FString CodeChunk = FString::Printf(TEXT("float%u(%s,%s)"), NumResultComponents, *CodeParameterA, *CodeParameterB);

        if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
        {
            return AddUniformExpression(new FMaterialUniformExpressionAppendVector(GetParameterUniformExpression(A),GetParameterUniformExpression(B),GetNumComponents(ParameterTypeA)),ResultType,*CodeChunk);
        }
        else
        {
            return AddInlinedCodeChunk(ResultType,*CodeChunk);
        }
    }

    bool SimplifyCoordinate(int32 Index, FString& OutUV)
    {
        FString UV = GetParameterCode(Index);

        for (int32 CoordinateIndex = 0; CoordinateIndex < MAX_NUM_TEX_COORD_USD; ++CoordinateIndex)
        {
            if (FString::Printf(TEXT("float2(state::texture_coordinate(math::min(%u,MaxTexCoordIndex)).x,1.0-state::texture_coordinate(math::min(%u,MaxTexCoordIndex)).y)"), CoordinateIndex, CoordinateIndex) == UV)
            {
                OutUV = FString::Printf(TEXT("float2(state::texture_coordinate(math::min(%u,MaxTexCoordIndex)).x, state::texture_coordinate(math::min(%u,MaxTexCoordIndex)).y)"), CoordinateIndex, CoordinateIndex);
                return true;
            }
        }

        return false;
    }

    FString CoerceUVParameter(int32 Index, EMaterialValueType DestType)
    {
        check(Index >= 0 && Index < CurrentScopeChunks->Num());
        const FShaderCodeChunk&	CodeChunk = (*CurrentScopeChunks)[Index];
        if (CodeChunk.Type == DestType)
        {
            switch (DestType)
            {
            case MCT_Float2:
            {
                FString UVParameter;
                if (SimplifyCoordinate(Index, UVParameter))
                {
                    return UVParameter;
                }
                return FString::Printf(TEXT("float2(%s.x,1.0-%s.y)"), *GetParameterCode(Index), *GetParameterCode(Index));
            }
            case MCT_Float3:
            case MCT_Float4:
                return GetParameterCode(Index);
            default:
                return TEXT("");
            }
        }
        else
        {
            if ((CodeChunk.Type & DestType) && (CodeChunk.Type & MCT_Float))
            {
                switch (DestType)
                {
                case MCT_Float2:
                    return FString::Printf(TEXT("float2(%s,1.0-%s)"), *GetParameterCode(Index), *GetParameterCode(Index));
                case MCT_Float3:
                    return FString::Printf(TEXT("float3(%s)"), *GetParameterCode(Index));
                case MCT_Float4:
                    return FString::Printf(TEXT("float4(%s)"), *GetParameterCode(Index));
                default:
                    return TEXT("");
                }
            }
            else
            {
                Errorf(TEXT("Coercion failed: %s: %s -> %s"), *CodeChunk.Definition, DescribeType(CodeChunk.Type), DescribeType(DestType));
                return TEXT("");
            }
        }
    }

    virtual FString CoerceParameter(int32 Index, EMaterialValueType DestType) override
    {
        check(Index >= 0 && Index < CurrentScopeChunks->Num());
        const FShaderCodeChunk&	CodeChunk = (*CurrentScopeChunks)[Index];
        if (CodeChunk.Type == DestType)
        {
            return GetParameterCode(Index);
        }
        else
        {
            if ((CodeChunk.Type & DestType) && (CodeChunk.Type & MCT_Float))
            {
                switch (DestType)
                {
                case MCT_Float1:
                    return FString::Printf(TEXT("%s"), *GetParameterCode(Index));
                case MCT_Float2:
                    return FString::Printf(TEXT("float2(%s,%s)"), *GetParameterCode(Index), *GetParameterCode(Index));
                case MCT_Float3:
                    return FString::Printf(TEXT("float3(%s,%s,%s)"), *GetParameterCode(Index), *GetParameterCode(Index), *GetParameterCode(Index));
                case MCT_Float4:
                    return FString::Printf(TEXT("float4(%s,%s,%s,%s)"), *GetParameterCode(Index), *GetParameterCode(Index), *GetParameterCode(Index), *GetParameterCode(Index));
                default:
                    return FString::Printf(TEXT("%s"), *GetParameterCode(Index));
                }
            }
            else
            {
                Errorf(TEXT("Coercion failed: %s: %s -> %s"), *CodeChunk.Definition, DescribeType(CodeChunk.Type), DescribeType(DestType));
                return TEXT("");
            }
        }
    }

    virtual int32 TextureCoordinate(uint32 CoordinateIndex, bool UnMirrorU, bool UnMirrorV) override
    {
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with TextureCoordinate, replaced with default."));
            InvalidRefraction = true;
        }

        // For WebGL 1 which is essentially GLES2.0, we can safely assume a higher number of supported vertex attributes
        // even when we are compiling ES 2 feature level shaders.
        // For UI materials can safely use more texture coordinates due to how they are packed in the slate material shader
        // Landscape materials also calculate their texture coordinates in the vertex factory and do not need to be sent using an interpolator
        const uint32 MaxNumCoordinates = ((Platform == SP_OPENGL_ES2_WEBGL_REMOVED) || (FeatureLevel != ERHIFeatureLevel::ES2_REMOVED) || Material->IsUIMaterial() || Material->IsUsedWithLandscape()) ? 8 : 3;

        if (CoordinateIndex >= MaxNumCoordinates)
        {
            return Errorf(TEXT("Only %u texture coordinate sets can be used by this feature level, currently using %u"), MaxNumCoordinates, CoordinateIndex + 1);
        }

        if (ShaderFrequency == SF_Vertex)
        {
            AllocateSlot(AllocatedUserVertexTexCoords, CoordinateIndex);
        }
        else
        {
            AllocateSlot(AllocatedUserTexCoords, CoordinateIndex);
        }

        FString	SampleCode;
#if 0 // TODO:MDL Mirror UV
        if ( UnMirrorU && UnMirrorV )
        {
            SampleCode = TEXT("UnMirrorUV(Parameters.TexCoords[%u].xy, Parameters)");
        }
        else if ( UnMirrorU )
        {
            SampleCode = TEXT("UnMirrorU(Parameters.TexCoords[%u].xy, Parameters)");
        }
        else if ( UnMirrorV )
        {
            SampleCode = TEXT("UnMirrorV(Parameters.TexCoords[%u].xy, Parameters)");
        }
        else
#endif
        {
            if ((uint32)GetNumUserTexCoords() > CoordinateIndex && ShaderFrequency != SF_Vertex)
            {
                const EMaterialProperty Property = (EMaterialProperty)(MP_CustomizedUVs0 + CoordinateIndex);
                FString PropertyName = FMaterialAttributeDefinitionMap_GetDisplayName(Property);
                SampleCode = PropertyName + TEXT("_mdl");
            }
            else
            {
                bMaxTexCoordinateUsed = true;
                RegisterIntParameter(TEXT("MaxTexCoordIndex"), MAX_NUM_TEX_COORD_USD - 1); // Max num of coordinates in Create is 4
                SampleCode = TEXT("float2(state::texture_coordinate(math::min(%u,MaxTexCoordIndex)).x,1.0-state::texture_coordinate(math::min(%u,MaxTexCoordIndex)).y)");
            }
        }

        // Note: inlining is important so that on ES2 devices, where half precision is used in the pixel shader, 
        // The UV does not get assigned to a half temporary in cases where the texture sample is done directly from interpolated UVs
        return AddInlinedCodeChunk(
                MCT_Float2,
                *SampleCode,
                CoordinateIndex,
                CoordinateIndex
                );
    }

    virtual int32 TextureSample(
        int32 TextureIndex,
        int32 CoordinateIndex,
        EMaterialSamplerType SamplerType,
        int32 MipValue0Index=INDEX_NONE,
        int32 MipValue1Index=INDEX_NONE,
        ETextureMipValueMode MipValueMode=TMVM_None,
        ESamplerSourceMode SamplerSource=SSM_FromTextureAsset,
        int32 TextureReferenceIndex=INDEX_NONE,
        bool AutomaticViewMipBias=false,
        bool AdaptiveVirtualTexture = false
        ) override
    {
        if(TextureIndex == INDEX_NONE || CoordinateIndex == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (FeatureLevel == ERHIFeatureLevel::ES2_REMOVED && ShaderFrequency == SF_Vertex)
        {
            if (MipValueMode != TMVM_MipLevel)
            {
                Errorf(TEXT("Sampling from vertex textures requires an absolute mip level on feature level ES2!"));
                return INDEX_NONE;
            }
        }
        else if (ShaderFrequency != SF_Pixel
            && ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType TextureType = GetParameterType(TextureIndex);

        if(!(TextureType & MCT_Texture))
        {
            Errorf(TEXT("Sampling unknown texture type: %s"),DescribeType(TextureType));
            return INDEX_NONE;
        }

        if(ShaderFrequency != SF_Pixel && MipValueMode == TMVM_MipBias)
        {
            Errorf(TEXT("MipBias is only supported in the pixel shader"));
            return INDEX_NONE;
        }

        const bool bVirtualTexture = TextureType == MCT_TextureVirtual;
        if (bVirtualTexture)
        {
            if (Material->GetMaterialDomain() == MD_DeferredDecal)
            {
                if (Material->GetDecalBlendMode() == DBM_Volumetric_DistanceFunction)
                {
                    return Errorf(TEXT("Sampling a virtual texture is currently only supported inside a volumetric decal."));
                }
            }
            else if (Material->GetMaterialDomain() != MD_Surface)
            {
                return Errorf(TEXT("Sampling a virtual texture is currently only supported inside surface and decal shaders."));
            }
        }

        if (MipValueMode == TMVM_Derivative)
        {
            if (MipValue0Index == INDEX_NONE)
            {
                return Errorf(TEXT("Missing DDX(UVs) parameter"));
            }
            else if (MipValue1Index == INDEX_NONE)
            {
                return Errorf(TEXT("Missing DDY(UVs) parameter"));
            }
            else if (!(GetParameterType(MipValue0Index) & MCT_Float))
            {
                return Errorf(TEXT("Invalid DDX(UVs) parameter"));
            }
            else if (!(GetParameterType(MipValue1Index) & MCT_Float))
            {
                return Errorf(TEXT("Invalid DDY(UVs) parameter"));
            }
        }
        else if (MipValueMode != TMVM_None && MipValue0Index != INDEX_NONE && !(GetParameterType(MipValue0Index) & MCT_Float))
        {
            return Errorf(TEXT("Invalid mip map parameter"));
        }

        // IOR can't set from sampling texture for MDL, replaced with constant for now.
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with texture lookup, replaced with default."));
            InvalidRefraction = true;
        }

        // if we are not in the PS we need a mip level
        if(ShaderFrequency != SF_Pixel)
        {
            MipValueMode = TMVM_MipLevel;
            AutomaticViewMipBias = false;

            if (MipValue0Index == INDEX_NONE)
            {
                MipValue0Index = Constant(0.f);
            }
        }

        // Automatic view mip bias is only for surface and decal domains.
        if (Material->GetMaterialDomain() != MD_Surface && Material->GetMaterialDomain() != MD_DeferredDecal)
        {
            AutomaticViewMipBias = false;
        }

        // If mobile, then disabling AutomaticViewMipBias.
        if (FeatureLevel < ERHIFeatureLevel::SM5)
        {
            AutomaticViewMipBias = false;
        }

        // If not 2D texture, disable AutomaticViewMipBias.
        if (!(TextureType & (MCT_Texture2D|MCT_TextureVirtual)))
        {
            AutomaticViewMipBias = false;
        }

        FString SamplerStateCode;
        bool RequiresManualViewMipBias = AutomaticViewMipBias;
        const UObject* TextureObj = nullptr;
        if (SamplerSource == SSM_FromTextureAsset)
        {
            // default is repeat
            if (TextureType == MCT_VolumeTexture)
            {
                SamplerStateCode = TEXT("tex::wrap_repeat,tex::wrap_repeat,tex::wrap_repeat");
            }
            else
            {
                SamplerStateCode = TEXT("tex::wrap_repeat,tex::wrap_repeat");
            }

            FMaterialUniformExpression* UniformExpression = GetParameterUniformExpression(TextureIndex);
            if (UniformExpression && UniformExpression->GetTextureUniformExpression())
            {
                auto TextureUniformExpression = UniformExpression->GetTextureUniformExpression();
                if (TextureUniformExpression->GetTextureIndex() < Material->GetReferencedTextures().Num())
                {
                    TextureObj = Material->GetReferencedTextures()[TextureUniformExpression->GetTextureIndex()];

                    if (TextureObj && TextureObj->IsA<UTexture2D>())
                    {
                        auto Texture2D = Cast<UTexture2D>(TextureObj);

                        auto GetAddressCode = [](TextureAddress Address)
                        {
                            switch (Address)
                            {
                            case TA_Clamp:
                                return TEXT("tex::wrap_clamp");
                            case TA_Mirror:
                                return TEXT("tex::wrap_mirrored_repeat");
                            case TA_Wrap:
                            default:
                                return TEXT("tex::wrap_repeat");
                            }
                        };

                        if (bVirtualTexture && Texture2D->Source.GetNumBlocks() > 1)
                        {
                            // UDIM (multi-block) texture are forced to use wrap address mode
                            // This is important for supporting VT stacks made from UDIMs with differing number of blocks, as this requires wrapping vAddress for certain layers
                            SamplerStateCode = GetAddressCode(TA_Wrap);
                            SamplerStateCode += TEXT(",");
                            SamplerStateCode += GetAddressCode(TA_Wrap);
                        }
                        else
                        {
                            SamplerStateCode = GetAddressCode(Texture2D->AddressX);
                            SamplerStateCode += TEXT(",");
                            SamplerStateCode += GetAddressCode(Texture2D->AddressY);
                        }
                    }				
                }
            }
        }
        else if (SamplerSource == SSM_Wrap_WorldGroupSettings)
        {
            // Use the shared sampler to save sampler slots
            if (TextureType == MCT_VolumeTexture)
            {
                SamplerStateCode = TEXT("tex::wrap_repeat,tex::wrap_repeat,tex::wrap_repeat");
            }
            else
            {
                SamplerStateCode = TEXT("tex::wrap_repeat,tex::wrap_repeat");
            }
            RequiresManualViewMipBias = false;
        }
        else if (SamplerSource == SSM_Clamp_WorldGroupSettings)
        {
            // Use the shared sampler to save sampler slots
            if (TextureType == MCT_VolumeTexture)
            {
                SamplerStateCode = TEXT("tex::wrap_clamp,tex::wrap_clamp,tex::wrap_clamp");
            }
            else
            {
                SamplerStateCode = TEXT("tex::wrap_clamp,tex::wrap_clamp");
            }
            RequiresManualViewMipBias = false;
        }

        FString SampleCode;
        SampleCode += TEXT("tex::lookup_float4");
        
        EMaterialValueType UVsType = (TextureType == MCT_TextureCube || TextureType == MCT_VolumeTexture) ? MCT_Float3 : MCT_Float2;
    
        //if (RequiresManualViewMipBias)
        //{
        //	if (MipValueMode == TMVM_Derivative)
        //	{
        //		// When doing derivative based sampling, multiply.
        //		int32 Multiplier = AddInlinedCodeChunk(MCT_Float, TEXT("View.MaterialTextureDerivativeMultiply"));
        //		MipValue0Index = Mul(MipValue0Index, Multiplier);
        //		MipValue1Index = Mul(MipValue1Index, Multiplier);
        //	}
        //	else if (MipValue0Index != INDEX_NONE && MipValueMode != TMVM_None)
        //	{
        //		// Adds bias to existing input level bias.
        //		MipValue0Index = Add(MipValue0Index, AddInlinedCodeChunk(MCT_Float, TEXT("View.MaterialTextureMipBias")));
        //	}
        //	else
        //	{
        //		// Sets bias.
        //		MipValue0Index = AddInlinedCodeChunk(MCT_Float1, TEXT("View.MaterialTextureMipBias"));
        //	}

        //	// If no Mip mode, then use MipBias.
        //	MipValueMode = MipValueMode == TMVM_None ? TMVM_MipBias : MipValueMode;
        //}

        FString MipValue0Code = TEXT("0.0f");
        FString MipValue1Code = TEXT("0.0f");

        if (MipValue0Index != INDEX_NONE && (MipValueMode == TMVM_MipBias || MipValueMode == TMVM_MipLevel))
        {
            MipValue0Code = CoerceParameter(MipValue0Index, MCT_Float1);
        }

        if (TextureType == MCT_TextureCube)
        {
            SampleCode += TEXT("(%s,%s)");
        }
        else
        {
            SampleCode += TEXT("(%s,%s,%s)");
        }

        if(MipValueMode == TMVM_None)
        {

        }
        else if(MipValueMode == TMVM_MipLevel)
        {
            // Mobile: Sampling of a particular level depends on an extension; iOS does have it by default but
            // there's a driver as of 7.0.2 that will cause a GPU hang if used with an Aniso > 1 sampler, so show an error for now
            if ((Platform != SP_OPENGL_ES2_WEBGL_REMOVED) && // WebGL 2/GLES3.0 (or browsers with the texture lod extension) it is possible to sample from specific mip levels
                ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
            {
                Errorf(TEXT("Sampling for a specific mip-level is not supported for ES2"));
                return INDEX_NONE;
            }
        }
        else if(MipValueMode == TMVM_MipBias)
        {
        }
        else if(MipValueMode == TMVM_Derivative)
        {
            MipValue0Code = CoerceParameter(MipValue0Index, UVsType);
            MipValue1Code = CoerceParameter(MipValue1Index, UVsType);
        }
        else
        {
            check(0);
        }

        switch( SamplerType )
        {
            case SAMPLERTYPE_External:
                // fall through since should be treated same as SAMPLERTYPE_Color
            case SAMPLERTYPE_Color:
            case SAMPLERTYPE_VirtualColor:
                SampleCode = FString::Printf( TEXT("%s"), *SampleCode );
                break;

            case SAMPLERTYPE_LinearColor:
            case SAMPLERTYPE_VirtualLinearColor:
                SampleCode = FString::Printf(TEXT("%s"), *SampleCode);
            break;

            case SAMPLERTYPE_Alpha:
            case SAMPLERTYPE_VirtualAlpha:
            case SAMPLERTYPE_DistanceFieldFont:
                // Sampling a single channel texture in D3D9 gives: (G,G,G)
                // Sampling a single channel texture in D3D11 gives: (G,0,0)
                // This replication reproduces the D3D9 behavior in all cases.
                SampleCode = FString::Printf( TEXT("::greyscale_texture_lookup(%s)"), *SampleCode );
                break;
            
            case SAMPLERTYPE_Grayscale:
            case SAMPLERTYPE_VirtualGrayscale:
                // Sampling a greyscale texture in D3D9 gives: (G,G,G)
                // Sampling a greyscale texture in D3D11 gives: (G,0,0)
                // This replication reproduces the D3D9 behavior in all cases.
                SampleCode = FString::Printf( TEXT("::greyscale_texture_lookup(%s)"), *SampleCode );
                break;

            case SAMPLERTYPE_LinearGrayscale:
            case SAMPLERTYPE_VirtualLinearGrayscale:
                // Sampling a greyscale texture in D3D9 gives: (G,G,G)
                // Sampling a greyscale texture in D3D11 gives: (G,0,0)
                // This replication reproduces the D3D9 behavior in all cases.
                SampleCode = FString::Printf(TEXT("::greyscale_texture_lookup(%s)"), *SampleCode);
                break;

            case SAMPLERTYPE_Normal:
            case SAMPLERTYPE_VirtualNormal:
                // Normal maps need to be unpacked in the pixel shader.
                SampleCode = FString::Printf( TEXT("::unpack_normal_map(%s)"), *SampleCode );
                break;
            case SAMPLERTYPE_Masks:
            case SAMPLERTYPE_VirtualMasks:
                break;
        }

        FString TextureName;
        if (TextureType == MCT_TextureCube)
        {
            TextureName = CoerceParameter(TextureIndex, MCT_TextureCube);
        }
        else if (TextureType == MCT_VolumeTexture)
        {
            TextureName = CoerceParameter(TextureIndex, MCT_VolumeTexture);
        }
        else if (TextureType == MCT_TextureExternal)
        {
            TextureName = CoerceParameter(TextureIndex, MCT_TextureExternal);
        }
        else if (TextureType == MCT_TextureVirtual)
        {
            TextureName = CoerceParameter(TextureIndex, MCT_TextureVirtual);
        }
        else // MCT_Texture2D
        {
            TextureName = CoerceParameter(TextureIndex, MCT_Texture2D);
        }

        FString UVs;
        if (ReplaceTextureCube && TextureObj && TextureObj->IsA<UTextureCube>())
        {
            const EMaterialProperty Property = (EMaterialProperty)(MP_CustomizedUVs0);
            FString PropertyName = FMaterialAttributeDefinitionMap_GetDisplayName(Property) + TEXT("_mdl");
            UVs = FString::Printf(TEXT("float2(%s.y, %s.x)"), *PropertyName, *PropertyName);
        }
        else
        {
            UVs = CoerceUVParameter(CoordinateIndex, UVsType);
        }

        if (NormalSampleToPack.Find(TextureIndex) != INDEX_NONE)
        {
            SampleCode = FString::Printf( TEXT("::pack_normal_map(::unpack_normal_map(%s))"), *SampleCode );
        }

        int32 SamplingCodeIndex = AddCodeChunk(
            MCT_Float4,
            *SampleCode,
            *TextureName,
            *UVs,
            *SamplerStateCode
            );
    
        AddEstimatedTextureSample();

        return SamplingCodeIndex;
    }

    virtual int32 Texture(UTexture* InTexture,int32& TextureReferenceIndex, EMaterialSamplerType SamplerType, ESamplerSourceMode SamplerSource=SSM_FromTextureAsset, ETextureMipValueMode MipValueMode = TMVM_None) override
    {
        if (FeatureLevel == ERHIFeatureLevel::ES2_REMOVED && ShaderFrequency == SF_Vertex)
        {
            if (MipValueMode != TMVM_MipLevel)
            {
                Errorf(TEXT("Sampling from vertex textures requires an absolute mip level on feature level ES2"));
                return INDEX_NONE;
            }
        }
        else if (ShaderFrequency != SF_Pixel
            && ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType ShaderType = InTexture->GetMaterialType();
        bool bNeedReplaceTextureCube = ReplaceTextureCube && ShaderType == MCT_TextureCube;
        if (bNeedReplaceTextureCube)
        {
            ShaderType = MCT_Texture2D;
        }

        TextureReferenceIndex = Material->GetReferencedTextures().Find(InTexture);
        checkf(TextureReferenceIndex != INDEX_NONE, TEXT("Material expression called Compiler->Texture() without implementing UMaterialExpression::GetReferencedTexture properly"));

        if(ShaderType & MCT_Texture)
        {
            const bool bVirtualTexturesEnabeled = UseVirtualTexturing(FeatureLevel, TargetPlatform);
            bool bVirtual = ShaderType == MCT_TextureVirtual;
            if (bVirtualTexturesEnabeled == false && ShaderType == MCT_TextureVirtual)
            {
                bVirtual = false;
            }

            TCHAR FormattedCode[MAX_SPRINTF] = TEXT("");

            CheckUniqueTextureName(InTexture);

            FString TextureFile = (*CurrentTextureCache)[InTexture];
            if (bNeedReplaceTextureCube)
            {
                TextureFile += TEXT("_f");
            }

            FCString::Sprintf(FormattedCode, TEXT("\"./%s/%s%s.%s\",%s"),
                *TexturePath,
                *TextureFile,
                bVirtual ? TEXT(".<UDIM>") : TEXT(""),
                bNeedReplaceTextureCube ? TEXT("dds") : *FMDLExporterUtility::GetFileExtension(InTexture, DDSTexture),
                InTexture->Source.GetFormat() == TSF_RGBA16F ?
                    TEXT("::tex::gamma_linear") :
                    (InTexture->SRGB ? TEXT("::tex::gamma_srgb") : TEXT("::tex::gamma_linear")));

            return AddUniformExpression(new FMaterialUniformExpressionTexture(TextureReferenceIndex, SamplerType, SamplerSource, false),ShaderType,FormattedCode);

        }
        return INDEX_NONE;
    }

    bool GetScalarValue(UMaterialInstance* Instance, FName ParameterName, float& Return)
    {
        if (Instance)
        {
            for (auto ScalarParameter : Instance->ScalarParameterValues)
            {
                if (ScalarParameter.ParameterInfo.Name.Compare(ParameterName) == 0)
                {
                    Return = ScalarParameter.ParameterValue;
                    return true;
                }
            }

            if (Instance->Parent->IsA<UMaterialInstance>())
            {
                return GetScalarValue(Cast<UMaterialInstance>(Instance->Parent), ParameterName, Return);
            }
        }

        return false;
    }

    bool GetVectorValue(UMaterialInstance* Instance, FName ParameterName, FLinearColor& Return)
    {
        if (Instance)
        {
            for (auto VectorParameter : Instance->VectorParameterValues)
            {
                if (VectorParameter.ParameterInfo.Name.Compare(ParameterName) == 0)
                {
                    Return = VectorParameter.ParameterValue;
                    return true;
                }
            }

            if (Instance->Parent->IsA<UMaterialInstance>())
            {
                return GetVectorValue(Cast<UMaterialInstance>(Instance->Parent), ParameterName, Return);
            }
        }

        return false;
    }

    bool GetTextureValue(UMaterialInstance* Instance, FName ParameterName, UTexture*& Return)
    {
        if (Instance)
        {
            for (auto TextureParameter : Instance->TextureParameterValues)
            {
                if (TextureParameter.ParameterInfo.Name.Compare(ParameterName) == 0)
                {
                    if (TextureParameter.ParameterValue != nullptr)
                    {
                        Return = TextureParameter.ParameterValue;
                        return true;
                    }
                }
            }

            if (Instance->Parent->IsA<UMaterialInstance>())
            {
                return GetTextureValue(Cast<UMaterialInstance>(Instance->Parent), ParameterName, Return);
            }
        }

        return false;
    }

    virtual int32 TextureParameter(FName ParameterName,UTexture* DefaultValue,int32& TextureReferenceIndex, EMaterialSamplerType SamplerType, ESamplerSourceMode SamplerSource=SSM_FromTextureAsset) override
    {
        if (ShaderFrequency != SF_Pixel
            && ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType ShaderType = DefaultValue->GetMaterialType();
        bool bNeedReplaceTextureCube = ReplaceTextureCube && ShaderType == MCT_TextureCube;
        if (bNeedReplaceTextureCube)
        {
            ShaderType = MCT_Texture2D;
        }

        TextureReferenceIndex = Material->GetReferencedTextures().Find(DefaultValue);
        checkf(TextureReferenceIndex != INDEX_NONE, TEXT("Material expression called Compiler->TextureParameter() without implementing UMaterialExpression::GetReferencedTexture properly"));

        UTexture* ReplacedTexture = nullptr;
        if (!GetTextureValue(MaterialInstance, ParameterName, ReplacedTexture))
        {
            ReplacedTexture = DefaultValue;

            if (GEngine->WeightMapPlaceholderTexture == DefaultValue)
            {
                UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/WeightMapNullTexture"), nullptr, LOAD_None, nullptr);
                ReplacedTexture = Texture;
            }
        }
    
        CheckUniqueTextureName(ReplacedTexture);

        FString Name = FMDLExporterUtility::GetLegalIdentifier(ParameterName.ToString());

        FString* DefinedName = DefinedTextureParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            if (IsReservedParameter(*Name))
            {
                Name = TEXT("Local") + Name;
            }

            while (DefinedParameterNames.Find(Name) != INDEX_NONE)
            {
                // Same name was found, should be changed
                Name += TEXT("_tp");
            }

            if (GEngine->WeightMapPlaceholderTexture == DefaultValue)
            {
                FString Annotation = DefineAnnotation<UMaterialExpressionTextureSampleParameter>(NAME_None, true, SamplerAnnotations[ST_Masks]);
                TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s(\"./%s/%s.%s\",%s)%s"),
                    TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                    ShaderType & MCT_Texture ? UNIFORM : TEXT(""),
                    HLSLTypeString(ShaderType), *Name,
                    HLSLTypeString(ShaderType),
                    *TexturePath,
                    *(*CurrentTextureCache)[ReplacedTexture],
                    DDSTexture ? DDS_TEXTURE_TYPE : DEFAULT_TEXTURE_TYPE,
                    TEXT("::tex::gamma_linear"),
                    Annotation.IsEmpty() ? TEXT("") : *Annotation
                    ));
            }
            else
            {
                const bool bVirtualTexturesEnabled = UseVirtualTexturing(FeatureLevel, TargetPlatform);
                bool bVirtual = ShaderType == MCT_TextureVirtual;
                if (bVirtualTexturesEnabled == false && ShaderType == MCT_TextureVirtual)
                {
                    bVirtual = false;
                }

                FString TextureFile = (*CurrentTextureCache)[ReplacedTexture];
                if (bNeedReplaceTextureCube)
                {
                    TextureFile += TEXT("_f");
                }
                FString Annotation = DefineAnnotation<UMaterialExpressionTextureSampleParameter>(ParameterName, false, GetSamplerType(ReplacedTexture));
                TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s(\"./%s/%s%s.%s\",%s)%s"),
                    TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                    ShaderType & MCT_Texture ? UNIFORM : TEXT(""),
                    HLSLTypeString(ShaderType), *Name,
                    HLSLTypeString(ShaderType),
                    *TexturePath,
                    *TextureFile,
                    bVirtual ? TEXT(".<UDIM>") : TEXT(""),
                    bNeedReplaceTextureCube ? TEXT("dds") : *FMDLExporterUtility::GetFileExtension(ReplacedTexture, DDSTexture),
                    ReplacedTexture->Source.GetFormat() == TSF_RGBA16F ?
                        TEXT("::tex::gamma_linear") :
                        (ReplacedTexture->SRGB ? TEXT("::tex::gamma_srgb") : TEXT("::tex::gamma_linear")),
                    Annotation.IsEmpty() ? TEXT("") : *Annotation
                ));
            }
            DefinedTextureParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);
        }
        else
        {
            Name = *DefinedName;
        }

        FMaterialParameterInfo ParameterInfo = GetParameterAssociationInfo();
        ParameterInfo.Name = *Name;
        int32 Code = AddUniformExpression(new FMaterialUniformExpressionTextureParameter(ParameterInfo, TextureReferenceIndex, SamplerType, SamplerSource, false),ShaderType,TEXT(""));

        // check sample type
        EMaterialSamplerType ReplacedSamplerType = UMaterialExpressionTextureBase::GetSamplerTypeForTexture( ReplacedTexture );
        EMaterialSamplerType DefaultSamplerType = UMaterialExpressionTextureBase::GetSamplerTypeForTexture( DefaultValue );
        if (DefaultSamplerType != ReplacedSamplerType && ReplacedSamplerType == SAMPLERTYPE_Normal)
        {
            NormalSampleToPack.AddUnique(Code);
        }

        return Code;
    }

    virtual int32 TextureProperty(int32 TextureIndex, EMaterialExposedTextureProperty Property) override
    {
        EMaterialValueType TextureType = GetParameterType(TextureIndex);

        if(TextureType != MCT_Texture2D)
        {
            return Errorf(TEXT("Texture size only available for Texture2D, not %s"),DescribeType(TextureType));
        }

        FString TextureWidth = FString::Printf(TEXT("tex::width(%s)"), *GetParameterCode(TextureIndex));
        FString TextureHeight = FString::Printf(TEXT("tex::height(%s)"), *GetParameterCode(TextureIndex));

        if (Property == TMTM_TextureSize)
        {
            return AddInlinedCodeChunk(MCT_Float2, TEXT("float2(%s,%s)"), *TextureWidth, *TextureHeight);
        }
        else // texel size
        {
            return AddInlinedCodeChunk(MCT_Float2, TEXT("float2(1.0/%s,1.0/%s)"), *TextureWidth, *TextureHeight);
        }
    }

    virtual int32 TextureDecalMipmapLevel(int32 TextureSizeInput) override
    {
        return MDLTranslatedError();
    }

    virtual int32 TextureDecalDerivative(bool bDDY) override
    {
        return MDLTranslatedError();
    }

    virtual int32 WorldPosition(EWorldPositionIncludedOffsets WorldPositionIncludedOffsets) override
    {
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with WorldPosition, replaced with default."));
            InvalidRefraction = true;
        }

        // force convert to cm
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3, TEXT("(::convert_to_left_hand(state::transform_point(state::coordinate_internal,state::coordinate_world,state::position()), %s)*state::meters_per_scene_unit()*100.0)"), *UpZ);
    }

    virtual int32 AccessUniformExpression(int32 Index) override
    {
        check(Index >= 0 && Index < CurrentScopeChunks->Num());
        const FShaderCodeChunk&	CodeChunk = (*CurrentScopeChunks)[Index];
        check(CodeChunk.UniformExpression && !CodeChunk.UniformExpression->IsConstant());

        FMaterialUniformExpressionTexture* TextureUniformExpression = CodeChunk.UniformExpression->GetTextureUniformExpression();
        FMaterialUniformExpressionExternalTexture* ExternalTextureUniformExpression = CodeChunk.UniformExpression->GetExternalTextureUniformExpression();

        // Any code chunk can have a texture uniform expression (eg FMaterialUniformExpressionFlipBookTextureParameter),
        // But a texture code chunk must have a texture uniform expression
        check(!(CodeChunk.Type & MCT_Texture) || TextureUniformExpression || ExternalTextureUniformExpression);
        // External texture samples must have a corresponding uniform expression
        check(!(CodeChunk.Type & MCT_TextureExternal) || ExternalTextureUniformExpression);

        TCHAR FormattedCode[MAX_SPRINTF] = TEXT("");
        if (CodeChunk.Type == MCT_Float)
        {
            FLinearColor Value(0, 0, 0);
            FMaterialRenderContext DummyContext(nullptr, *Material, nullptr);
            CodeChunk.UniformExpression->GetNumberValue(DummyContext, Value);

            // Update the above FMemory::Malloc if this FCString::Sprintf grows in size, e.g. %s, ...
            FCString::Sprintf(FormattedCode, TEXT("%s"), FL(Value.R));
        }
        else if (CodeChunk.Type & MCT_Float)
        {
            FLinearColor Value(0, 0, 0);
            FMaterialRenderContext DummyContext(nullptr, *Material, nullptr);
            CodeChunk.UniformExpression->GetNumberValue(DummyContext, Value);

            switch (CodeChunk.Type)
            {
            case MCT_Float:
            case MCT_Float1: FCString::Sprintf(FormattedCode, TEXT("%s"), FL(Value.R)); break;
            case MCT_Float2: FCString::Sprintf(FormattedCode, TEXT("float2(%s, %s)"), FL(Value.R), FL(Value.G)); break;
            case MCT_Float3: FCString::Sprintf(FormattedCode, TEXT("float3(%s, %s, %s)"), FL(Value.R), FL(Value.G), FL(Value.B)); break;
            default: FCString::Sprintf(FormattedCode, TEXT("float4(%s, %s, %s, %s)"), FL(Value.R), FL(Value.G), FL(Value.B), FL(Value.A)); break;
            };
        }
        else if (CodeChunk.Type & MCT_Texture)
        {
            FMaterialUniformExpressionTextureParameter* TextureParameterUniformExpression = TextureUniformExpression ? TextureUniformExpression->GetTextureParameterUniformExpression() : nullptr;

            if (TextureParameterUniformExpression)
            {
                FCString::Sprintf(FormattedCode, TEXT("%s"), *TextureParameterUniformExpression->GetParameterName().ToString());
            }
            else
            {
                const TCHAR* BaseName = TEXT("");
                switch (CodeChunk.Type)
                {
                case MCT_Texture2D:
                    BaseName = TEXT("texture_2d");
                    break;
                case MCT_TextureCube:
                    BaseName = TEXT("texture_cube");
                    break;
                case MCT_VolumeTexture:
                    BaseName = TEXT("texture_3d");
                    break;
                case MCT_TextureExternal:
                    BaseName = TEXT("texture_2d");
                    break;
                case MCT_TextureVirtual:
                    BaseName = TEXT("texture_2d");
                    break;
                    //default: UE_LOG(LogMaterial, Fatal,TEXT("Unrecognized texture material value type: %u"),(int32)CodeChunk.Type);
                };
                FCString::Sprintf(FormattedCode, TEXT("%s(%s)"), BaseName, *CodeChunk.Definition);
            }
        }
        else
        {
            //UE_LOG(LogMaterial, Fatal,TEXT("User input of unknown type: %s"),DescribeType(CodeChunk.Type));
        }

        return AddInlinedCodeChunk((*CurrentScopeChunks)[Index].Type, FormattedCode);
    }

    /**
     * Casts the passed in code to DestType, or generates a compile error if the cast is not valid.
     * This will truncate a type (float4 -> float3) but not add components (float2 -> float3), however a float1 can be cast to any float type by replication.
     */
    virtual int32 ValidCast(int32 Code, EMaterialValueType DestType) override
    {
        if (Code == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType SourceType = GetParameterType(Code);
        int32 CompiledResult = INDEX_NONE;

        if (SourceType & DestType)
        {
            CompiledResult = Code;
        }
        else if (GetParameterUniformExpression(Code) && !GetParameterUniformExpression(Code)->IsConstant())
        {
            if ((SourceType & MCT_TextureVirtual) && (DestType & MCT_Texture2D))
            {
                return Code;
            }
            else
            {			
                return ValidCast(AccessUniformExpression(Code), DestType);
            }
        }
        else if ((SourceType & MCT_Float) && (DestType & MCT_Float))
        {
            const uint32 NumSourceComponents = GetNumComponents(SourceType);
            const uint32 NumDestComponents = GetNumComponents(DestType);

            if (NumSourceComponents > NumDestComponents) // Use a mask to select the first NumDestComponents components from the source.
            {
                switch (NumDestComponents)
                {
                case 1: return AddInlinedCodeChunk(DestType, TEXT("%s.x"), *GetParameterCode(Code));
                case 2: return AddInlinedCodeChunk(DestType, TEXT("float2(%s.x,%s.y)"), *GetParameterCode(Code), *GetParameterCode(Code));
                case 3: return AddInlinedCodeChunk(DestType, TEXT("float3(%s.x,%s.y,%s.z)"), *GetParameterCode(Code), *GetParameterCode(Code), *GetParameterCode(Code));
                default: /*UE_LOG(LogMaterial, Fatal,TEXT("Should never get here!"));*/ return INDEX_NONE;
                };
            }
            else if (NumSourceComponents < NumDestComponents) // Pad the source vector up to NumDestComponents.
            {
                // Only allow replication when the Source is a Float1
                if (NumSourceComponents == 1)
                {
                    const uint32 NumPadComponents = NumDestComponents - NumSourceComponents;
                    FString CommaParameterCodeString = FString::Printf(TEXT(",%s"), *GetParameterCode(Code));

                    CompiledResult = AddInlinedCodeChunk(
                        DestType,
                        TEXT("%s(%s%s%s%s)"),
                        HLSLTypeString(DestType),
                        *GetParameterCode(Code),
                        NumPadComponents >= 1 ? *CommaParameterCodeString : TEXT(""),
                        NumPadComponents >= 2 ? *CommaParameterCodeString : TEXT(""),
                        NumPadComponents >= 3 ? *CommaParameterCodeString : TEXT("")
                    );
                }
                else
                {
                    CompiledResult = Errorf(TEXT("Cannot cast from %s to %s."), DescribeType(SourceType), DescribeType(DestType));
                }
            }
            else
            {
                CompiledResult = Code;
            }
        }
        else
        {
            //We can feed any type into a material attributes socket as we're really just passing them through.
            if (DestType == MCT_MaterialAttributes)
            {
                CompiledResult = Code;
            }
            else
            {
                CompiledResult = Errorf(TEXT("Cannot cast from %s to %s."), DescribeType(SourceType), DescribeType(DestType));
            }
        }

        return CompiledResult;
    }

    virtual int32 ForceCast(int32 Code, EMaterialValueType DestType, uint32 ForceCastFlags = 0) override
    {
        if (Code == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterUniformExpression(Code) && !GetParameterUniformExpression(Code)->IsConstant())
        {
            return ForceCast(AccessUniformExpression(Code), DestType, ForceCastFlags);
        }

        EMaterialValueType	SourceType = GetParameterType(Code);

        bool bConstant = ConstantChunks.Find(Code) != nullptr && FMaterialAttributeDefinitionMap::GetShaderFrequency(MaterialProperty) == SF_Pixel;

        bool bExactMatch = true;
        bool bReplicateValue = true;

        if (bExactMatch ? (SourceType == DestType) : (SourceType & DestType))
        {
            return Code;
        }
        else if ((SourceType & MCT_Float) && (DestType & MCT_Float))
        {
            const uint32 NumSourceComponents = GetNumComponents(SourceType);
            const uint32 NumDestComponents = GetNumComponents(DestType);

            if (NumSourceComponents > NumDestComponents) // Use a mask to select the first NumDestComponents components from the source.
            {
                int32 Ret = INDEX_NONE;
                switch (NumDestComponents)
                {
                case 1: 
                    Ret = AddInlinedCodeChunk(DestType, TEXT("%s.x"), *GetParameterCode(Code));
                    break;
                case 2: 
                    Ret = AddInlinedCodeChunk(DestType, TEXT("float2(%s.x,%s.y)"), *GetParameterCode(Code), *GetParameterCode(Code));
                    break;
                case 3: 
                    Ret = AddInlinedCodeChunk(DestType, TEXT("float3(%s.x,%s.y,%s.z)"), *GetParameterCode(Code), *GetParameterCode(Code), *GetParameterCode(Code));
                    break;
                default: /*UE_LOG(LogMaterial, Fatal,TEXT("Should never get here!"));*/ 
                    return INDEX_NONE;
                };

                if (bConstant)
                {
                    ConstantChunks.Add(Ret, *ConstantChunks.Find(Code));
                }
                return Ret;

            }
            else if (NumSourceComponents < NumDestComponents) // Pad the source vector up to NumDestComponents.
            {
                // Only allow replication when the Source is a Float1
                if (NumSourceComponents != 1)
                {
                    bReplicateValue = false;
                }

                const uint32 NumPadComponents = NumDestComponents - NumSourceComponents;

                if (NumSourceComponents == 1)
                {
                    FString CommaParameterCodeString = FString::Printf(TEXT(",%s"), *GetParameterCode(Code));
                    int32 Ret = AddInlinedCodeChunk(
                        DestType,
                        TEXT("%s(%s%s%s%s)"),
                        HLSLTypeString(DestType),
                        *GetParameterCode(Code),
                        NumPadComponents >= 1 ? (bReplicateValue ? *CommaParameterCodeString : TEXT(",0.0")) : TEXT(""),
                        NumPadComponents >= 2 ? (bReplicateValue ? *CommaParameterCodeString : TEXT(",0.0")) : TEXT(""),
                        NumPadComponents >= 3 ? (bReplicateValue ? *CommaParameterCodeString : TEXT(",0.0")) : TEXT("")
                    );

                    if (bConstant)
                    {
                        float R = ConstantChunks.Find(Code)->R;
                        ConstantChunks.Add(Ret, FLinearColor(R, bReplicateValue ? R : 0.0f, bReplicateValue ? R : 0.0f, bReplicateValue ? R : 0.0f));
                    }
                    return Ret;
                }
                else
                {
                    FString SourceParameterCodeString;
                    FLinearColor DestResult(ForceInit);
                    switch(SourceType)
                    {
                    case MCT_Float2:
                        SourceParameterCodeString = FString::Printf(TEXT("%s.x,%s.y"), *GetParameterCode(Code), *GetParameterCode(Code));
                        if (bConstant)
                        {
                            DestResult.R = ConstantChunks.Find(Code)->R;
                            DestResult.G = ConstantChunks.Find(Code)->G;
                        }
                        break;
                    case MCT_Float3:
                        SourceParameterCodeString = FString::Printf(TEXT("%s.x,%s.y,%s.z"), *GetParameterCode(Code), *GetParameterCode(Code), *GetParameterCode(Code));
                        if (bConstant)
                        {
                            DestResult.R = ConstantChunks.Find(Code)->R;
                            DestResult.G = ConstantChunks.Find(Code)->G;
                            DestResult.B = ConstantChunks.Find(Code)->B;
                        }
                        break;
                    }

                    int32 Ret = AddInlinedCodeChunk(
                        DestType,
                        TEXT("%s(%s%s%s%s)"),
                        HLSLTypeString(DestType),
                        *SourceParameterCodeString,
                        NumPadComponents >= 1 ? TEXT(",0.0") : TEXT(""),
                        NumPadComponents >= 2 ? TEXT(",0.0") : TEXT(""),
                        NumPadComponents >= 3 ? TEXT(",0.0") : TEXT("")
                    );
                    if (bConstant)
                    {
                        ConstantChunks.Add(Ret, DestResult);
                    }
                    return Ret;
                }
            }
            else
            {
                return Code;
            }
        }
        else if ((SourceType & MCT_TextureVirtual) && (DestType & MCT_Texture2D))
        {
            return Code;
        }
        else
        {
            return Errorf(TEXT("Cannot force a cast between non-numeric types."));
        }
    }

    virtual int32 If(int32 A,int32 B,int32 AGreaterThanB,int32 AEqualsB,int32 ALessThanB,int32 ThresholdArg) override
    {
        if(A == INDEX_NONE || B == INDEX_NONE || AGreaterThanB == INDEX_NONE || ALessThanB == INDEX_NONE || ThresholdArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (AEqualsB != INDEX_NONE)
        {
            EMaterialValueType ResultType = GetArithmeticResultType(GetParameterType(AGreaterThanB),GetArithmeticResultType(AEqualsB,ALessThanB));

            int32 CoercedAGreaterThanB = ForceCast(AGreaterThanB,ResultType);
            int32 CoercedAEqualsB = ForceCast(AEqualsB,ResultType);
            int32 CoercedALessThanB = ForceCast(ALessThanB,ResultType);

            if(CoercedAGreaterThanB == INDEX_NONE || CoercedAEqualsB == INDEX_NONE || CoercedALessThanB == INDEX_NONE)
            {
                return INDEX_NONE;
            }

            if (CoercedAGreaterThanB == CoercedALessThanB
            || GetParameterCode(CoercedAGreaterThanB) == GetParameterCode(CoercedALessThanB))
            {
                return AddCodeChunk(
                    ResultType,
                    TEXT("((math::abs(%s - %s) > %s) ? (%s) : %s)"),
                    *GetParameterCode(A),
                    *GetParameterCode(B),
                    *GetParameterCode(ThresholdArg),
                    *GetParameterCode(CoercedAGreaterThanB),
                    *GetParameterCode(CoercedAEqualsB)
                    );
            }
            else
            {
                return AddCodeChunk(
                    ResultType,
                    TEXT("((math::abs(%s - %s) > %s) ? (%s >= %s ? %s : %s) : %s)"),
                    *GetParameterCode(A),
                    *GetParameterCode(B),
                    *GetParameterCode(ThresholdArg),
                    *GetParameterCode(A),
                    *GetParameterCode(B),
                    *GetParameterCode(CoercedAGreaterThanB),
                    *GetParameterCode(CoercedALessThanB),
                    *GetParameterCode(CoercedAEqualsB)
                    );
            }
        }
        else
        {
            EMaterialValueType ResultType = GetArithmeticResultType(AGreaterThanB,ALessThanB);

            int32 CoercedAGreaterThanB = ForceCast(AGreaterThanB,ResultType);
            int32 CoercedALessThanB = ForceCast(ALessThanB,ResultType);

            if(CoercedAGreaterThanB == INDEX_NONE || CoercedALessThanB == INDEX_NONE)
            {
                return INDEX_NONE;
            }

            if (CoercedAGreaterThanB == CoercedALessThanB
            || GetParameterCode(CoercedAGreaterThanB) == GetParameterCode(CoercedALessThanB))
            {
                return AddCodeChunk(
                    ResultType,
                    TEXT("(%s)"),
                    *GetParameterCode(CoercedALessThanB)
                    );
            }
            else
            {
                return AddCodeChunk(
                    ResultType,
                    TEXT("((%s >= %s) ? %s : %s)"),
                    *GetParameterCode(A),
                    *GetParameterCode(B),
                    *GetParameterCode(CoercedAGreaterThanB),
                    *GetParameterCode(CoercedALessThanB)
                    );
            }
        }
    }

    virtual int32 VertexNormal() override
    {
        if (ShaderFrequency != SF_Vertex)
        {
            bUsesTransformVector = true;
        }

        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with VertexNormalWS, replaced with default."));
            InvalidRefraction = true;
        }

        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        if (LandscapeMaterial)
        {
            return AddInlinedCodeChunk(MCT_Float3, TEXT("::landscape_normal_world_space(%s)"), *UpZ);
        }
        else
        {
            return AddInlinedCodeChunk(MCT_Float3, TEXT("::vertex_normal_world_space(%s)"), *UpZ);
        }
    }

    virtual int32 VertexTangent() override
    {
        return AddInlinedCodeChunk(MCT_Float3, TEXT("state::texture_tangent_u(0)"));
    }

    virtual int32 PixelNormalWS() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonPixelShaderExpressionError();
        }
        if(MaterialProperty == MP_Normal)
        {
            return Errorf(TEXT("Invalid node PixelNormalWS used for Normal input."));
        }
        if (ShaderFrequency != SF_Vertex)
        {
            bUsesTransformVector = true;
        }

        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with PixelNormalWS, replaced with default."));
            InvalidRefraction = true;
        }

        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::pixel_normal_world_space(%s)"), *UpZ);
    }

    virtual int32 TemporalSobol(int32 Index, int32 Seed) override
    {
        return MDLTranslatedError();
    }

    FString CastFunctionOutput(const FString& Code, EMaterialValueType ResultType)
    {
        static const TArray<FString> ReturnFunctions = TArrayBuilder<FString>()
            .Add(FString(TEXT("return GetTanHalfFieldOfView();")));

        static const TArray<EMaterialValueType> FunctionTypes = TArrayBuilder<EMaterialValueType>()
            .Add(MCT_Float2);

        FString OutCode = Code;
        for (int32 Index = 0; Index < ReturnFunctions.Num(); Index++)
        {
            if (ResultType == FunctionTypes[Index])
            {
                continue;
            }

            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(ReturnFunctions[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    StartPos += ReturnFunctions[Index].Len();
                    switch(ResultType)
                    {
                    case MCT_Float:
                        OutCode.InsertAt(StartPos - 1, TEXT(".x"));
                        break;
                    case MCT_Float2:
                        break;
                    case MCT_Float3:
                        break;
                    case MCT_Float4:
                        break;
                    }
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    virtual int32 CustomExpression(class UMaterialExpressionCustom* Custom, int32 OutputIndex, TArray<int32>& CompiledInputs) override
    {
        if (Custom->Description == TEXT("DepthOcclusion"))
        {
            return AddInlinedCodeChunk(MCT_Float3,TEXT("float3(0)"));
        }

        int32 ResultIdx = INDEX_NONE;

        FString OutputTypeString;
        EMaterialValueType ResultType;
        ECustomMaterialOutputType OutputType = Custom->OutputType;

        bool bAdditionalOutput = OutputIndex >= 1 && OutputIndex - 1 < Custom->AdditionalOutputs.Num();
        if (bAdditionalOutput)
        {
            OutputType = Custom->AdditionalOutputs[OutputIndex - 1].OutputType;
        }

        switch (OutputType)
        {
        case CMOT_Float2:
            ResultType = MCT_Float2;
            OutputTypeString = TEXT("float2");
            break;
        case CMOT_Float3:
            ResultType = MCT_Float3;
            OutputTypeString = TEXT("float3");
            break;
        case CMOT_Float4:
            ResultType = MCT_Float4;
            OutputTypeString = TEXT("float4");
            break;
        default:
            ResultType = MCT_Float;
            OutputTypeString = TEXT("float");
            break;
        }

        // Declare implementation function
        FString InputParamDecl;
        check(Custom->Inputs.Num() == CompiledInputs.Num());
        TMap<FString, FString> ChangedInputs;
        for (int32 i = 0; i < Custom->Inputs.Num(); i++)
        {
            // skip over unnamed inputs
            if (Custom->Inputs[i].InputName.IsNone())
            {
                continue;
            }

            const FString InputName = Custom->Inputs[i].InputName.ToString();
            FString LegalName = FMDLExporterUtility::GetLegalIdentifier(InputName);

            if (LegalName.Compare(InputName) != 0)
            {
                ChangedInputs.Add(InputName, LegalName);
            }

            if (!InputParamDecl.IsEmpty())
            {
                InputParamDecl += TEXT(",");
            }

            switch (GetParameterType(CompiledInputs[i]))
            {
            case MCT_Float:
            case MCT_Float1:
                InputParamDecl += TEXT("float ");
                InputParamDecl += LegalName;
                break;
            case MCT_Float2:
                InputParamDecl += TEXT("float2 ");
                InputParamDecl += LegalName;
                break;
            case MCT_Float3:
                InputParamDecl += TEXT("float3 ");
                InputParamDecl += LegalName;
                break;
            case MCT_Float4:
                InputParamDecl += TEXT("float4 ");
                InputParamDecl += LegalName;
                break;
            case MCT_Texture2D:
                InputParamDecl += TEXT("uniform texture_2d ");
                InputParamDecl += LegalName;
                break;
            case MCT_TextureCube:
                InputParamDecl += TEXT("uniform texture_cube ");
                InputParamDecl += LegalName;
                break;
            case MCT_TextureExternal:
                InputParamDecl += TEXT("uniform texture_2d ");
                InputParamDecl += LegalName;
                break;
            case MCT_VolumeTexture:
                InputParamDecl += TEXT("uniform texture_3d ");
                InputParamDecl += LegalName;
                break;
            default:
                return Errorf(TEXT("Bad type %s for %s input %s"), DescribeType(GetParameterType(CompiledInputs[i])), *Custom->Description, *LegalName);
                break;
            }
        }

        FSHA1 HashState;
        FSHAHash OutHash;
        HashState.Update((uint8*)&OutputIndex, sizeof(int32));
        HashState.UpdateWithString(*OutputTypeString, OutputTypeString.Len());
        HashState.UpdateWithString(*InputParamDecl, InputParamDecl.Len());
        HashState.UpdateWithString(*Custom->Code, Custom->Code.Len());
        HashState.Final();
        HashState.GetHash(&OutHash.Hash[0]);

        FString CustomExpressionHash = OutHash.ToString();
        int32 CustomExpressionIndex = INDEX_NONE;
        int32* CacheIndex = CustomExpressionCache.Find(CustomExpressionHash);
        if (CacheIndex == nullptr)
        {
            TArray<FString> PreDefinedCodes;
            for (int32 AdditionalOutputIndex = 0; AdditionalOutputIndex < Custom->AdditionalOutputs.Num(); ++AdditionalOutputIndex)
            {
                FString AdditionalOutputTypeString = TEXT("float");
                switch (Custom->AdditionalOutputs[AdditionalOutputIndex].OutputType)
                {
                case CMOT_Float2:
                    AdditionalOutputTypeString = TEXT("float2");
                    break;
                case CMOT_Float3:
                    AdditionalOutputTypeString = TEXT("float3");
                    break;
                case CMOT_Float4:
                    AdditionalOutputTypeString = TEXT("float4");
                    break;
                }

                PreDefinedCodes.AddUnique(FString::Printf(TEXT("%s %s;\n"), *AdditionalOutputTypeString, *Custom->AdditionalOutputs[AdditionalOutputIndex].OutputName.ToString()));
            }

            FString Code = RemoveComments(Custom->Code);
            Code = RemoveMacros(Code);
            Code = ReplaceHlslTypeWithMdlType(Code, ChangedInputs);
            Code = ReplaceHlslConstructor(Code);
            Code = ReplaceHlslConversion(Code);
            Code = ReplaceWithMdlMath(Code);
            Code = ConvertToMdlParameter(Code);
            Code = ReplaceUnsupportedHlslWithConstant(Code);
            
            Code = ReplaceVariableWithConstant(Code, PreDefinedCodes);
            Code = ReplaceImplicitCast(Code);
            Code = ReplaceInvalidMdlConstructor(Code);
            Code = ReplaceTextureSample(Code);
            Code = ReplaceSampleGrad(Code);
            Code = SwapHlslMultiply(Code);
            Code = CastFunctionOutput(Code, ResultType);

            if (bAdditionalOutput)
            {
                Code = ReplaceAdditionalOutput(Code, Custom->AdditionalOutputs[OutputIndex - 1].OutputName);
            }
            else if (!Code.Contains(TEXT("return ")))
            {
                Code = FString(TEXT("return ")) + Code + TEXT(";");
            }

            for (auto PreDefine : PreDefinedCodes)
            {
                Code.InsertAt(0, PreDefine);
            }

            Code.ReplaceInline(TEXT("\n"), TEXT("\r\n"), ESearchCase::CaseSensitive);

            CustomExpressionIndex = CustomExpressions.Num();
            FMaterialCustomExpressionEntry CustomExpression;
            CustomExpression.ScopeID = 0;
            CustomExpression.Expression = nullptr;
            CustomExpression.Implementation = FString::Printf(TEXT("%s CustomExpression%d(%s)\r\n{\r\n%s\r\n}\r\n"), *OutputTypeString, CustomExpressionIndex, *InputParamDecl, *Code);
            CustomExpressions.Add(CustomExpression);
            CustomExpressionCache.Add(CustomExpressionHash, CustomExpressionIndex);
        }
        else
        {
            CustomExpressionIndex = *CacheIndex;
        }

        // Add call to implementation function
        FString CodeChunk = FString::Printf(TEXT("CustomExpression%d("), CustomExpressionIndex);
        FString ParamChunk;
        for (int32 i = 0; i < CompiledInputs.Num(); i++)
        {
            // skip over unnamed inputs
            if (Custom->Inputs[i].InputName.IsNone())
            {
                continue;
            }

            FString ParamCode = GetParameterCode(CompiledInputs[i]);
            EMaterialValueType ParamType = GetParameterType(CompiledInputs[i]);

            if (!ParamChunk.IsEmpty())
            {
                ParamChunk += TEXT(",");
            }
            ParamChunk += *ParamCode;
        }
        CodeChunk += ParamChunk;
        CodeChunk += TEXT(")");

        ResultIdx = AddCodeChunk(
            ResultType,
            *CodeChunk
        );
        return ResultIdx;
    }

    virtual int32 CustomOutput(class UMaterialExpressionCustomOutput* Custom, int32 OutputIndex, int32 OutputCode) override
    {
        if (Custom->IsA<UMaterialExpressionVertexInterpolator>())
        {
            return INDEX_NONE;
        }

        if (MaterialProperty != MP_MAX)
        {
            return Errorf(TEXT("A Custom Output node should not be attached to the %s material property"), *FMaterialAttributeDefinitionMap_GetDisplayName(MaterialProperty));
        }

        if (OutputCode == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        EMaterialValueType OutputType = GetParameterType(OutputCode);
        FString OutputTypeString;
        switch (OutputType)
        {
            case MCT_Float:
            case MCT_Float1:
                OutputTypeString = TEXT("float");
                break;
            case MCT_Float2:
                OutputTypeString += TEXT("float2");
                break;
            case MCT_Float3:
                OutputTypeString += TEXT("float3");
                break;
            case MCT_Float4:
                OutputTypeString += TEXT("float4");
                break;
            default:
                return Errorf(TEXT("Bad type %s for %s"), DescribeType(GetParameterType(OutputCode)), *Custom->GetDescription());
                break;
        }

        FString Definitions;
        FString Body;

        if ((*CurrentScopeChunks)[OutputCode].UniformExpression && !(*CurrentScopeChunks)[OutputCode].UniformExpression->IsConstant())
        {
            Body = GetParameterCode(OutputCode);
        }
        else
        {
            GetFixedParameterCode(OutputCode, *CurrentScopeChunks, Definitions, Body);
        }

        FString Parameter = FString::Printf(TEXT("%s%d_mdl"), *Custom->GetFunctionName(), OutputIndex);
        FString ImplementationCode = FString::Printf(TEXT("%s\r\n\t%s %s = %s;\r\n"), *Definitions, *OutputTypeString, *Parameter, *Body);

        if (ShaderFrequency != SF_Vertex)
        {
            CustomPSOutputImplementations.Add(ImplementationCode);
        }
        else
        {
            CustomVSOutputImplementations.Add(ImplementationCode);
        }
        // return value is not used
        return INDEX_NONE;
    }

    void RegisterIntParameter(FName ParameterName, int DefaultValue = 0)
    {
        FString* DefinedName = DefinedScalarParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            FString Name = ParameterName.ToString();
            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(NAME_None, true);
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %d%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                TEXT(""),
                TEXT("int"), *Name, DefaultValue,
                Annotation.IsEmpty() ? TEXT("") : *Annotation
            ));
            DefinedScalarParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);
        }
    }

    void RegisterFloatParameter(FName ParameterName, float DefaultValue)
    {
        FString* DefinedName = DefinedScalarParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            FString Name = ParameterName.ToString();
            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(NAME_None, true);
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                TEXT(""),
                TEXT("float"), *Name, FL(DefaultValue),
                Annotation.IsEmpty() ? TEXT("") : *Annotation
            ));
            DefinedScalarParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);
        }
    }

    void RegisterFloat2Parameter(FName ParameterName)
    {
        FString* DefinedName = DefinedVectorParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            FString Name = ParameterName.ToString();
            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(NAME_None, true);
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                TEXT(""),
                TEXT("float2"), *Name, TEXT("float2(0.0)"),
                Annotation.IsEmpty() ? TEXT("") : *Annotation));
            DefinedVectorParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);
        }
    }

    FString GetLightMapCoordinateChunk()
    {
        // Register (1) LightMapCoordinateIndex (2) LightmapCoordinateScale (3) LightmapCoordinateBias
        bLightMapCoordinateUsed = true;
        RegisterIntParameter(TEXT("LightMapCoordinateIndex"));
        RegisterFloat2Parameter(TEXT("LightmapCoordinateScale"));
        RegisterFloat2Parameter(TEXT("LightmapCoordinateBias"));

        FString LightMapCoordinate = TEXT("(float2(state::texture_coordinate(LightMapCoordinateIndex).x,1.0-state::texture_coordinate(LightMapCoordinateIndex).y)*LightmapCoordinateScale+LightmapCoordinateBias)");
        return  LightMapCoordinate;
    }

    virtual int32 DynamicParameter(FLinearColor& DefaultValue, uint32 ParameterIndex) override
    {
        if (ShaderFrequency != SF_Vertex && ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonVertexOrPixelShaderExpressionError();
        }

        FString DynamicParameterName = FString::Printf(TEXT("DynamicParameter%u"), ParameterIndex);
        FString* DefinedName = DefinedVectorParameters.Find(*DynamicParameterName);
        if (DefinedName == nullptr)
        {
            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(NAME_None, true);
            // instance replace the material	
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s(%s,%s,%s,%s)%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                IsRefractionProperty() ? UNIFORM : TEXT(""),
                HLSLTypeString(MCT_Float4), *DynamicParameterName,
                HLSLTypeString(MCT_Float4), FL(DefaultValue.R), FL(DefaultValue.G), FL(DefaultValue.B), FL(DefaultValue.A),
                Annotation.IsEmpty() ? TEXT("") : *Annotation));

            DefinedVectorParameters.Add(*DynamicParameterName, DynamicParameterName);
            DefinedParameterNames.Add(DynamicParameterName);

            if (IsRefractionProperty())
            {
                IORParameters.Add(DynamicParameterName, DynamicParameterName);
            }
        }

        return AddInlinedCodeChunk(MCT_Float4,TEXT("DynamicParameter%u"),ParameterIndex);
    }

    virtual int32 LightmapUVs() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonPixelShaderExpressionError();
        }

        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        bUsesLightmapUVs = true;

        int32 ResultIdx = INDEX_NONE;

        FString CodeChunk = GetLightMapCoordinateChunk();
        ResultIdx = AddCodeChunk(
            MCT_Float2,
            *CodeChunk
        );
        return ResultIdx;
    }

    virtual int32 PrecomputedAOMask() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Compute)
        {
            return NonPixelShaderExpressionError();
        }

        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        bAOMaterialMaskUsed = true;
        // Register AOMaterialMaskTexture
        FName AOMaterialMaskTexture = TEXT("AOMaterialMaskTexture");
        UTexture* DefaultAOMaterialMask = LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/DefaultLinearColor"), nullptr, LOAD_None, nullptr);
        CheckUniqueTextureName(DefaultAOMaterialMask);

        FString* DefinedName = DefinedTextureParameters.Find(AOMaterialMaskTexture);
        if (DefinedName == nullptr)
        {
            FString Name = AOMaterialMaskTexture.ToString();
            FString Annotation = DefineAnnotation<UMaterialExpressionTextureSampleParameter>(NAME_None, true);
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\t%s%s %s = %s(\"./%s/%s.%s\",%s)%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                UNIFORM,
                HLSLTypeString(MCT_Texture2D), *Name,
                HLSLTypeString(MCT_Texture2D),
                *TexturePath,
                *(*CurrentTextureCache)[DefaultAOMaterialMask],
                DDSTexture ? DDS_TEXTURE_TYPE : DEFAULT_TEXTURE_TYPE,
                TEXT("::tex::gamma_linear"),
                Annotation.IsEmpty() ? TEXT("") : *Annotation));
            
            DefinedTextureParameters.Add(AOMaterialMaskTexture, Name);
            DefinedParameterNames.Add(Name);
        }

        FString LightmapUVs = GetLightMapCoordinateChunk();
        FString AOMaterialMask = FString::Printf(TEXT("tex::lookup_float4(AOMaterialMaskTexture, float2(%s.x, 1.0-%s.y), tex::wrap_repeat, tex::wrap_repeat).x"), *LightmapUVs, *LightmapUVs);
        int32 ResultIdx = AddCodeChunk(MCT_Float, TEXT("%s * %s"), *AOMaterialMask, *AOMaterialMask);
        return ResultIdx;
    }

    virtual int32 CameraVector() override
    {
        MDLTranslatedWarning();
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::camera_vector(%s)"), *UpZ);
    }

    virtual int32 PixelDepth() override
    {
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float, TEXT("::pixel_depth()"));
    }

    virtual int32 VertexColor() override
    {
        bUsesVertexColor |= (ShaderFrequency != SF_Vertex);
        // Because vertex color using state::texture_coordinate
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with VertexColor, replaced with default."));
            InvalidRefraction = true;
        }
        return AddInlinedCodeChunk(MCT_Float4, TEXT("float4(scene::data_lookup_float3(\"displayColor\").x, scene::data_lookup_float3(\"displayColor\").y, scene::data_lookup_float3(\"displayColor\").z, scene::data_lookup_float(\"displayOpacity\"))"));
    }

    virtual int32 PreSkinnedPosition() override
    {
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with PreSkinnedPosition, replaced with default."));
            InvalidRefraction = true;
        }

        if (ShaderFrequency != SF_Vertex)
        {
            return Errorf(TEXT("Pre-skinned position is only available in the vertex shader, pass through custom interpolators if needed."));
        }
        
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3, TEXT("(::convert_to_left_hand(state::transform_point(state::coordinate_internal,state::coordinate_world,state::position()), %s)*state::meters_per_scene_unit()*100.0)"), *UpZ);
    }

    virtual int32 PreSkinnedNormal() override
    {
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::vertex_normal_world_space(%s)"), *UpZ);
    }

    virtual int32 VertexInterpolator(uint32 InterpolatorIndex) override
    {
        if (ShaderFrequency != SF_Pixel)
        {
            return Errorf(TEXT("Custom interpolator outputs only available in pixel shaders."));
        }

        UMaterialExpressionVertexInterpolator** InterpolatorPtr = CustomVertexInterpolators.FindByPredicate([InterpolatorIndex](const UMaterialExpressionVertexInterpolator* Item) { return Item && Item->InterpolatorIndex == InterpolatorIndex; });
        if (InterpolatorPtr == nullptr)
        {
            return Errorf(TEXT("Invalid custom interpolator index."));
        }

        UMaterialExpressionVertexInterpolator* Interpolator = *InterpolatorPtr;
        check(Interpolator->InterpolatorIndex == InterpolatorIndex);
        check(Interpolator->InterpolatedType & MCT_Float);

        // Assign interpolator offset and accumulate size
        int32 InterpolatorSize = 0;
        switch (Interpolator->InterpolatedType)
        {
        case MCT_Float4:	InterpolatorSize = 4; break;
        case MCT_Float3:	InterpolatorSize = 3; break;
        case MCT_Float2:	InterpolatorSize = 2; break;
        default:			InterpolatorSize = 1;
        };

        if (Interpolator->InterpolatorOffset == INDEX_NONE)
        {
            Interpolator->InterpolatorOffset = CurrentCustomVertexInterpolatorOffset;
            CurrentCustomVertexInterpolatorOffset += InterpolatorSize;
        }
        check(CurrentCustomVertexInterpolatorOffset != INDEX_NONE && Interpolator->InterpolatorOffset < CurrentCustomVertexInterpolatorOffset);

        // Copy interpolated data from pixel parameters to local
        const TCHAR* TypeName = HLSLTypeString(Interpolator->InterpolatedType);
        const TCHAR* Swizzle[2] = { TEXT("x"), TEXT("y") };
        const int32 Offset = Interpolator->InterpolatorOffset;
    
        // Note: We reference the UV define directly to avoid having to pre-accumulate UV counts before property translation
        FString GetValueCode = FString::Printf(TEXT("%s(VertexInterpolator%i_mdl.%s"), TypeName, InterpolatorIndex, Swizzle[Offset%2]);
        if (InterpolatorSize == 1)
        {
            GetValueCode = FString::Printf(TEXT("(VertexInterpolator%i_mdl"), InterpolatorIndex);
        }
        else if (InterpolatorSize >= 2)
        {
            GetValueCode += FString::Printf(TEXT(", VertexInterpolator%i_mdl.%s"), InterpolatorIndex, Swizzle[(Offset+1)%2]);

            if (InterpolatorSize >= 3)
            {
                GetValueCode += FString::Printf(TEXT(", VertexInterpolator%i_mdl.%s"), InterpolatorIndex, Swizzle[(Offset+2)%2]);

                if (InterpolatorSize >= 4)
                {
                    check(InterpolatorSize == 4);
                    GetValueCode += FString::Printf(TEXT(", VertexInterpolator%i_mdl.%s"), InterpolatorIndex, Swizzle[(Offset+3)%2]);
                }
            }
        }

        GetValueCode.Append(TEXT(")"));

        int32 RetCode = AddCodeChunk(Interpolator->InterpolatedType, *GetValueCode);
        return RetCode;
    }

    int32 SwitchByBoolean(int32 BoolIndex, int32 A, int32 B)
    {
        if (A == INDEX_NONE || B == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (A == B)
        {
            return AddInlinedCodeChunk(GetParameterType(A), TEXT("%s"), *GetParameterCode(A));
        }

        EMaterialValueType ResultType = GetArithmeticResultType(A, B);

        int32 CoercedA = ForceCast(A, ResultType);
        int32 CoercedB = ForceCast(B, ResultType);

        if (CoercedA == INDEX_NONE || CoercedB == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (GetParameterCode(CoercedA).Compare(GetParameterCode(CoercedB)) == 0)
        {
            return AddInlinedCodeChunk(ResultType, TEXT("%s"), *GetParameterCode(CoercedA));
        }
        else
        {
            return AddCodeChunk(ResultType, TEXT("%s ? %s : %s"), *GetParameterCode(BoolIndex), *GetParameterCode(CoercedA), *GetParameterCode(CoercedB));
        }
    }

    int32 StaticSwitch(class UMaterialExpressionStaticSwitch* StaticSwitchExpression, FMaterialCompiler* Compiler)
    {
        bool bValue = StaticSwitchExpression->DefaultValue;

        if (StaticSwitchExpression->Value.GetTracedInput().Expression)
        {
            int32 BoolIndex = StaticSwitchExpression->Value.Compile(Compiler);	
            bool bSucceeded;
            bValue = Compiler->GetStaticBoolValue(BoolIndex, bSucceeded);

            if (!bSucceeded)
            {
                return INDEX_NONE;
            }

            if (!GetParameterCode(BoolIndex).Contains(TEXT("true")) && !GetParameterCode(BoolIndex).Contains(TEXT("false")))
            {
                return SwitchByBoolean(BoolIndex, StaticSwitchExpression->A.Compile(Compiler), StaticSwitchExpression->B.Compile(Compiler));
            }
        }
    
        // We only call Compile on the branch that is taken to avoid compile errors in the disabled branch.
        if (bValue)
        {
            return StaticSwitchExpression->A.Compile(Compiler);
        }
        else
        {
            return StaticSwitchExpression->B.Compile(Compiler);
        }
    }

    int32 StaticSwitchParameter(class UMaterialExpressionStaticSwitchParameter* StaticSwitchParameterExpression, FMaterialCompiler* Compiler)
    {
        bool bSucceeded;
        int32 BoolIndex = Compiler->StaticBoolParameter(StaticSwitchParameterExpression->ParameterName,StaticSwitchParameterExpression->DefaultValue);
        const bool bValue = Compiler->GetStaticBoolValue(BoolIndex, bSucceeded);
    
        //Both A and B must be connected in a parameter. 
        if( !StaticSwitchParameterExpression->A.GetTracedInput().IsConnected() )
        {
            Compiler->Errorf(TEXT("Missing A input"));
            bSucceeded = false;
        }
        if( !StaticSwitchParameterExpression->B.GetTracedInput().IsConnected() )
        {
            Compiler->Errorf(TEXT("Missing B input"));
            bSucceeded = false;
        }

        if (!bSucceeded)
        {
            return INDEX_NONE;
        }

        if (!GetParameterCode(BoolIndex).Contains(TEXT("true")) && !GetParameterCode(BoolIndex).Contains(TEXT("false")))
        {
            return SwitchByBoolean(BoolIndex, StaticSwitchParameterExpression->A.Compile(Compiler), StaticSwitchParameterExpression->B.Compile(Compiler));
        }

        if (bValue)
        {
            return StaticSwitchParameterExpression->A.Compile(Compiler);
        }
        else
        {
            return StaticSwitchParameterExpression->B.Compile(Compiler);
        }
    }

    int32 Fresnel(class UMaterialExpressionFresnel* FresnelExpression, FMaterialCompiler* Compiler)
    {
        int32 NormalArg = FresnelExpression->Normal.GetTracedInput().Expression ? FresnelExpression->Normal.Compile(Compiler) : (IsRefractionProperty() ? Compiler->Constant3(0, 0, 1.0f) : Compiler->PixelNormalWS());
        int32 ExponentArg = FresnelExpression->ExponentIn.GetTracedInput().Expression ? FresnelExpression->ExponentIn.Compile(Compiler) : Compiler->Constant(FresnelExpression->Exponent);
        int32 BaseReflectFractionArg = FresnelExpression->BaseReflectFractionIn.GetTracedInput().Expression ? FresnelExpression->BaseReflectFractionIn.Compile(Compiler) : Compiler->Constant(FresnelExpression->BaseReflectFraction);
        
        if (NormalArg == INDEX_NONE || ExponentArg == INDEX_NONE || BaseReflectFractionArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        ExponentArg = ForceCast(ExponentArg, MCT_Float);
        BaseReflectFractionArg = ForceCast(BaseReflectFractionArg, MCT_Float);

        return AddCodeChunk(MCT_Float,TEXT("::fresnel(%s, %s, %s)"), *GetParameterCode(ExponentArg), *GetParameterCode(BaseReflectFractionArg), *CoerceParameter(NormalArg,MCT_Float3));
    }

    int32 VertexInterpolator(class UMaterialExpressionVertexInterpolator* VertexInterpolatorExpression, FMaterialCompiler* Compiler)
    {
        int32 Ret = INDEX_NONE;
        if (VertexInterpolatorExpression->Input.GetTracedInput().Expression)
        {
            Ret = VertexInterpolatorExpression->Input.Compile(Compiler);			
        }

        return Ret;
    }

    int32 FresnelFunction(class UMaterialExpressionMaterialFunctionCall* FresnelFunc, FMaterialCompiler* Compiler, int32 OutputIndex)
    {
        check(FresnelFunc->FunctionInputs.Num() == 8);
        int32 NormalArg = FresnelFunc->FunctionInputs[0].Input.Expression ? 
                            FresnelFunc->FunctionInputs[0].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[0].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 CameraArg = FresnelFunc->FunctionInputs[1].Input.Expression ? 
                            FresnelFunc->FunctionInputs[1].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[1].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 InvertFresnelArg = FresnelFunc->FunctionInputs[2].Input.Expression ? 
                            FresnelFunc->FunctionInputs[2].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[2].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 PowerArg = FresnelFunc->FunctionInputs[3].Input.Expression ? 
                            FresnelFunc->FunctionInputs[3].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[3].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 UseCheapContrastArg = FresnelFunc->FunctionInputs[4].Input.Expression ? 
                            FresnelFunc->FunctionInputs[4].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[4].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 CheapContrastDarkArg = FresnelFunc->FunctionInputs[5].Input.Expression ? 
                            FresnelFunc->FunctionInputs[5].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[5].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 CheapContrastBrightArg = FresnelFunc->FunctionInputs[6].Input.Expression ? 
                            FresnelFunc->FunctionInputs[6].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[6].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 ClampFresnelDotProductArg = FresnelFunc->FunctionInputs[7].Input.Expression ? 
                            FresnelFunc->FunctionInputs[7].Input.Compile(Compiler) :
                            FresnelFunc->FunctionInputs[7].ExpressionInput->Compile(Compiler, OutputIndex);

        if (NormalArg == INDEX_NONE || CameraArg == INDEX_NONE || InvertFresnelArg == INDEX_NONE 
            || PowerArg == INDEX_NONE || UseCheapContrastArg == INDEX_NONE || CheapContrastDarkArg == INDEX_NONE 
            || CheapContrastBrightArg == INDEX_NONE || ClampFresnelDotProductArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        PowerArg = ForceCast(PowerArg, MCT_Float);
        CheapContrastDarkArg = ForceCast(CheapContrastDarkArg, MCT_Float);
        CheapContrastBrightArg = ForceCast(CheapContrastBrightArg, MCT_Float);

        return AddCodeChunk(MCT_Float,TEXT("%s ? 1.0f : 0.0f"),  			
            *GetParameterCode(InvertFresnelArg)
            );
    }

    int32 DitherTemporalAA(class UMaterialExpressionMaterialFunctionCall* TemporalAAFunc, FMaterialCompiler* Compiler, int32 OutputIndex)
    {
        check(TemporalAAFunc->FunctionInputs.Num() == 2);
        int32 AlphaThresholdArg = TemporalAAFunc->FunctionInputs[0].Input.Expression ? 
                            TemporalAAFunc->FunctionInputs[0].Input.Compile(Compiler) :
                            TemporalAAFunc->FunctionInputs[0].ExpressionInput->Compile(Compiler, OutputIndex);
        int32 RandomArg = TemporalAAFunc->FunctionInputs[1].Input.Expression ? 
                            TemporalAAFunc->FunctionInputs[1].Input.Compile(Compiler) :
                            TemporalAAFunc->FunctionInputs[1].ExpressionInput->Compile(Compiler, OutputIndex);

        if (AlphaThresholdArg == INDEX_NONE || RandomArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        return AddCodeChunk(MCT_Float,TEXT("::dither_temporalAA(%s, %s)"), *GetParameterCode(ForceCast(AlphaThresholdArg,MCT_Float)), *GetParameterCode(ForceCast(RandomArg,MCT_Float)));
    }

    int32 ParallaxOcclusionMapping(class UMaterialExpressionMaterialFunctionCall* POMFunc, FMaterialCompiler* Compiler, int32 OutputIndex)
    {
        check(POMFunc->FunctionOutputs.Num() == 7);

        if (OutputIndex == 0) // Parallax UVs
        {
            POMFunc->LinkFunctionIntoCaller(Compiler);

            // Some functions (e.g. layers) don't benefit from re-using state so we locally create one as we did before sharing was added
            FMaterialFunctionCompileState LocalState(POMFunc);

            // Tell the compiler that we are entering a function
            Compiler->PushFunction(&LocalState);


            int32 ReturnValue = INDEX_NONE;
            auto Expression = POMFunc->FunctionOutputs[OutputIndex].ExpressionOutput->A.Expression;
            if (Expression)
            {
                auto FeatureLevelSwitch = Cast<UMaterialExpressionFeatureLevelSwitch>(Expression);
                if (FeatureLevelSwitch)
                {
                    ReturnValue = FeatureLevelSwitch->Inputs[0].Compile(Compiler);
                }
            }

            // Tell the compiler that we are leaving a function
            Compiler->PopFunction();

            POMFunc->UnlinkFunctionFromCaller(Compiler);
            return ReturnValue;
        }
        else if (OutputIndex == 1 || OutputIndex == 3) // Offset and PixelDepth Offset
        {
            return Compiler->Constant(0.0f);
        }
        else if (OutputIndex == 2) // Shadow
        {
            return Compiler->Constant(1.0f);
        }
        else
        {
            return POMFunc->Compile(Compiler, OutputIndex);
        }
    }

    int32 CameraDepthFade(class UMaterialExpressionMaterialFunctionCall* Func, FMaterialCompiler* Compiler, int32 OutputIndex)
    {
        // Always return 0 for MDL, means closing to view
        return Compiler->Constant(0.0f);
    }

    int32 DeriveTangentBasis(class UMaterialExpressionMaterialFunctionCall* DTBFunc, FMaterialCompiler* Compiler, int32 OutputIndex)
    {
        int32 NormalArg = DTBFunc->FunctionInputs[1].Input.Expression ?
            DTBFunc->FunctionInputs[1].Input.Compile(Compiler) :
            DTBFunc->FunctionInputs[1].ExpressionInput->Compile(Compiler, OutputIndex);

        if (NormalArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");

        if (OutputIndex == 0)
        {
            return AddCodeChunk(MCT_Float3, TEXT("state::texture_tangent_u(1)"));
        }
        else if (OutputIndex == 1)
        {
            return AddCodeChunk(MCT_Float3, TEXT("state::texture_tangent_v(1)"));
        }
        else
        {
            return AddCodeChunk(MCT_Float3, TEXT("::transform_vector_from_tangent_to_world(%s, %s, state::texture_tangent_u(1), state::texture_tangent_v(1))"), *GetParameterCode(ForceCast(NormalArg, MCT_Float3)), *UpZ);
        }
    }

    int32 UnpackIntegerAsFloat(class UMaterialExpressionMaterialFunctionCall* UnpackFunc, FMaterialCompiler* Compiler, int32 OutputIndex)
    {
        int32 InputFloatArg = UnpackFunc->FunctionInputs[0].Input.Expression ?
            UnpackFunc->FunctionInputs[0].Input.Compile(Compiler) :
            UnpackFunc->FunctionInputs[0].ExpressionInput->Compile(Compiler, OutputIndex);

        if (InputFloatArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        bExtension17Used = true;
        return AddCodeChunk(MCT_Float, TEXT("::unpack_integer_as_float(%s)"), *GetParameterCode(ForceCast(InputFloatArg, MCT_Float)));
    }

    virtual int32 CallExpression(FMaterialExpressionKey ExpressionKey, FMaterialCompiler* Compiler) override
    {
        // For any translated result not relying on material attributes, we can discard the attribute ID from the key
        // to allow result sharing. In cases where we detect an expression loop we must err on the side of caution
        if (ExpressionKey.Expression && !ExpressionKey.Expression->ContainsInputLoop() && !ExpressionKey.Expression->IsResultMaterialAttributes(ExpressionKey.OutputIndex))
        {
            ExpressionKey.MaterialAttributeID = FGuid(0, 0, 0, 0);
        }

        // Some expressions can discard output indices and share compiles with a swizzle/mask
        if (ExpressionKey.Expression && ExpressionKey.Expression->CanIgnoreOutputIndex())
        {
            ExpressionKey.OutputIndex = INDEX_NONE;
        }

        // Check if this expression has already been translated.
        check(ShaderFrequency < SF_NumFrequencies);
        auto& CurrentFunctionStack = FunctionStacks[ShaderFrequency];
        FMaterialFunctionCompileState* CurrentFunctionState = CurrentFunctionStack.Last();

        int32* ExistingCodeIndex = CurrentFunctionState->ExpressionCodeMap.Find(ExpressionKey);
        if (ExistingCodeIndex && MaterialProperty != MP_Refraction)
        {
            return *ExistingCodeIndex;
        }
        else
        {
            // Disallow reentrance.
            if (CurrentFunctionState->ExpressionStack.Find(ExpressionKey) != INDEX_NONE)
            {
                return Error(TEXT("Reentrant expression"));
            }

            // The first time this expression is called, translate it.
            CurrentFunctionState->ExpressionStack.Add(ExpressionKey);
            const int32 FunctionDepth = CurrentFunctionStack.Num();
            
            // Attempt to share function states between function calls
            UMaterialExpressionMaterialFunctionCall* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression);
            if (FunctionCall)
            {
                FMaterialExpressionKey ReuseCompileStateExpressionKey = ExpressionKey;
                ReuseCompileStateExpressionKey.OutputIndex = INDEX_NONE; // Discard the output so we can share the stack internals

                FMaterialFunctionCompileState* SharedFunctionState = CurrentFunctionState->FindOrAddSharedFunctionState(ReuseCompileStateExpressionKey, FunctionCall);
                FunctionCall->SetSharedCompileState(SharedFunctionState);
            }

            int32 Result = INDEX_NONE;
            if (ExpressionKey.Expression->IsA<UMaterialExpressionFresnel>())
            {
                UMaterialExpressionFresnel* FresnelExpression = Cast<UMaterialExpressionFresnel>(ExpressionKey.Expression);
                Result = ((FMDLMaterialTranslator*)Compiler)->Fresnel(FresnelExpression, Compiler);
            }
            else if (ExpressionKey.Expression->IsA<UMaterialExpressionVertexInterpolator>())
            {
                UMaterialExpressionVertexInterpolator* VertexInterpolatorExpression = Cast<UMaterialExpressionVertexInterpolator>(ExpressionKey.Expression);
                Result = ((FMDLMaterialTranslator*)Compiler)->VertexInterpolator(VertexInterpolatorExpression, Compiler);
            }
#if 0
            else if (ExpressionKey.Expression->IsA<UMaterialExpressionStaticSwitch>())
            {
                UMaterialExpressionStaticSwitch* StaticSwitch = Cast<UMaterialExpressionStaticSwitch>(ExpressionKey.Expression);
                Result = ((FMDLMaterialTranslator*)Compiler)->StaticSwitch(StaticSwitch, Compiler);
            }
            else if (ExpressionKey.Expression->IsA<UMaterialExpressionStaticSwitchParameter>())
            {
                UMaterialExpressionStaticSwitchParameter* StaticSwitchParameter = Cast<UMaterialExpressionStaticSwitchParameter>(ExpressionKey.Expression);
                Result = ((FMDLMaterialTranslator*)Compiler)->StaticSwitchParameter(StaticSwitchParameter, Compiler);
            }
#endif
            else
            {
                bool bReplacedFunction = false;

                if (ExpressionKey.Expression->IsA<UMaterialExpressionMaterialFunctionCall>())
                {
                    UMaterialExpressionMaterialFunctionCall* Function = Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression);

                    if (Function->MaterialFunction)
                    {
                        UMaterialFunction* Fresnel_Function = LoadObject<UMaterialFunction>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions02/Fresnel_Function.Fresnel_Function"));
                        UMaterialFunction* DitherTemporalAA = LoadObject<UMaterialFunction>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility/DitherTemporalAA.DitherTemporalAA"));
                        UMaterialFunction* ParallaxOcclusionMapping = LoadObject<UMaterialFunction>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions01/Texturing/ParallaxOcclusionMapping.ParallaxOcclusionMapping"));
                        UMaterialFunction* CameraDepthFade = LoadObject<UMaterialFunction>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions01/Opacity/CameraDepthFade.CameraDepthFade"));
                        UMaterialFunction* DeriveTangentBasis = LoadObject<UMaterialFunction>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions01/Texturing/DeriveTangentBasis.DeriveTangentBasis"));
                        UMaterialFunction* MS_PivotPainter2_UnpackIntegerAsFloat = LoadObject<UMaterialFunction>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions02/ms_PivotPainter2_UnpackIntegerAsFloat.ms_PivotPainter2_UnpackIntegerAsFloat"));

                        if (Fresnel_Function != nullptr && Function->MaterialFunction == Fresnel_Function)
                        {
                            Result = ((FMDLMaterialTranslator*)Compiler)->FresnelFunction(Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression), Compiler, ExpressionKey.OutputIndex);
                            bReplacedFunction = true;
                        }
                        else if (DitherTemporalAA != nullptr && Function->MaterialFunction == DitherTemporalAA)
                        {
                            Result = ((FMDLMaterialTranslator*)Compiler)->DitherTemporalAA(Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression), Compiler, ExpressionKey.OutputIndex);
                            bReplacedFunction = true;
                        }
                        else if (ParallaxOcclusionMapping != nullptr && Function->MaterialFunction == ParallaxOcclusionMapping)
                        {
                            Result = ((FMDLMaterialTranslator*)Compiler)->ParallaxOcclusionMapping(Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression), Compiler, ExpressionKey.OutputIndex);
                            bReplacedFunction = true;
                        }
                        else if (CameraDepthFade != nullptr && Function->MaterialFunction == CameraDepthFade)
                        {
                            Result = ((FMDLMaterialTranslator*)Compiler)->CameraDepthFade(Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression), Compiler, ExpressionKey.OutputIndex);
                            bReplacedFunction = true;
                        }
                        else if (DeriveTangentBasis != nullptr && Function->MaterialFunction == DeriveTangentBasis)
                        {
                            Result = ((FMDLMaterialTranslator*)Compiler)->DeriveTangentBasis(Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression), Compiler, ExpressionKey.OutputIndex);
                            bReplacedFunction = true;
                        }
                        else if (MS_PivotPainter2_UnpackIntegerAsFloat != nullptr && Function->MaterialFunction == MS_PivotPainter2_UnpackIntegerAsFloat)
                        {
                            Result = ((FMDLMaterialTranslator*)Compiler)->UnpackIntegerAsFloat(Cast<UMaterialExpressionMaterialFunctionCall>(ExpressionKey.Expression), Compiler, ExpressionKey.OutputIndex);
                            bReplacedFunction = true;
                        }
                    }
                }

                if (!bReplacedFunction)
                {
                    Result = ExpressionKey.Expression->Compile(Compiler, ExpressionKey.OutputIndex);
                }

                // NOTE: UMaterialExpressionDepthFade and UMaterialExpressionSceneDepth are unavailable in the plugin (without MinimalAPI)
                // Using a tricky way to restore SceneDepthOpacity
                if (!bSceneDepthOpacity && Result != INDEX_NONE && MaterialProperty == MP_Opacity)
                {
                    if ((*CurrentScopeChunks)[Result].Definition.StartsWith(TEXT("::scene_depth()")))
                    {
                        bSceneDepthOpacity = true;
                    }
                }
            }

            // Restore state
            if (FunctionCall)
            {
                FunctionCall->SetSharedCompileState(nullptr);
            }

            FMaterialExpressionKey PoppedExpressionKey = CurrentFunctionState->ExpressionStack.Pop();

            // Verify state integrity
            check(PoppedExpressionKey == ExpressionKey);
            check(FunctionDepth == CurrentFunctionStack.Num());

            // Cache the translation.
            CurrentFunctionStack.Last()->ExpressionCodeMap.Add(ExpressionKey,Result);

            return Result;
        }
    }

    bool IsLeftParenthese(const TCHAR Char)
    {
        return (Char == 0x28);
    }

    bool IsRightParenthese(const TCHAR Char)
    {
        return (Char == 0x29);
    }

    bool IsBlank(const TCHAR Char)
    {
        return (Char == 0x20);
    }

    bool IsComma(const TCHAR Char)
    {
        return (Char == 0x2C);
    }

    bool IsPeriod(const TCHAR Char)
    {
        return (Char == 0x2E);
    }

    bool IsColon(const TCHAR Char)
    {
        return (Char == 0x3A);
    }

    bool IsSemicolon(const TCHAR Char)
    {
        return (Char == 0x3B);
    }

    bool IsLeftBracket(const TCHAR Char)
    {
        return (Char == 0x7B);
    }

    bool IsRightBracket(const TCHAR Char)
    {
        return (Char == 0x7D);
    }

    bool IsLeftSquareBracket(const TCHAR Char)
    {
        return (Char == 0x5B);
    }

    bool IsRightSquareBracket(const TCHAR Char)
    {
        return (Char == 0x5D);
    }

    bool IsEqualSign(const TCHAR Char)
    {
        return (Char == 0x3D);
    }

    bool IsNameAscii(const TCHAR Char)
    {
        if ((Char >= 0x30 && Char <= 0x39) // 0-9
        || (Char >= 0x41 && Char <= 0x5A) // A-Z
        || (Char >= 0x61 && Char <= 0x7A) // a-z
        || Char == 0x5F // _
            )
        {
            return true;
        }

        return false;
    }

    bool IsOperand(const TCHAR Char)
    {
        return IsNameAscii(Char) || IsColon(Char) || IsPeriod(Char) || IsLeftSquareBracket(Char) || IsRightSquareBracket(Char);
    }

    // removing the unsupported hlsl function
    FString ReplaceUnsupportedHlslWithConstant(const FString& Code)
    {
        static const TArray<FString> MathFunctions = TArrayBuilder<FString>()
            .Add(FString(TEXT("fwidth")))
            .Add(FString(TEXT("ddx")))
            .Add(FString(TEXT("ddy")))
            .Add(FString(TEXT("GetTanHalfFieldOfView")))
            .Add(FString(TEXT("MakePrecise")))
            .Add(FString(TEXT("Square")))
            .Add(FString(TEXT("Pow2")))
            .Add(FString(TEXT("Pow3")))
            .Add(FString(TEXT("Pow4")))
            .Add(FString(TEXT("Pow5")))
            .Add(FString(TEXT("Pow6")));

        static const TArray<FString> ReplacedConstant = TArrayBuilder<FString>()
            .Add(FString(TEXT("float fwidth(float x) {return 0.0001;}\nfloat2 fwidth(float2 x) {return float2(0.0001);}\nfloat3 fwidth(float3 x) {return float3(0.0001);}\nfloat4 fwidth(float4 x) {return float4(0.0001);}\n")))
            .Add(FString(TEXT("float ddx(float x) {return 0.0001;}\nfloat2 ddx(float2 x) {return float2(0.0001);}\nfloat3 ddx(float3 x) {return float3(0.0001);}\nfloat4 ddx(float4 x) {return float4(0.0001);}\n")))
            .Add(FString(TEXT("float ddy(float x) {return 0.0001;}\nfloat2 ddy(float2 x) {return float2(0.0001);}\nfloat3 ddy(float3 x) {return float3(0.0001);}\nfloat4 ddy(float4 x) {return float4(0.0001);}\n")))
            .Add(FString(TEXT("float2 GetTanHalfFieldOfView() {return math::tan(float2(0.5, 1.0) * 0.5);}")))
            .Add(FString(TEXT("float MakePrecise(float v) { return v; }\nfloat2 MakePrecise(float2 v) { return v; }\nfloat3 MakePrecise(float3 v) { return v; }\nfloat4 MakePrecise(float4 v) { return v; }\n")))
            .Add(FString(TEXT("float Square(float x) { return x*x; }\nfloat2 Square(float2 x) { return x*x; }\nfloat3 Square(float3 x) { return x*x; }\nfloat4 Square(float4 x) { return x*x; }\n")))
            .Add(FString(TEXT("float Pow2(float x) { return x*x; }\nfloat2 Pow2(float2 x) { return x*x; }\nfloat3 Pow2(float3 x) { return x*x; }\nfloat4 Pow2(float4 x) { return x*x; }\n")))
            .Add(FString(TEXT("float Pow3(float x) { return x*x*x; }\nfloat2 Pow3(float2 x) { return x*x*x; }\nfloat3 Pow3(float3 x) { return x*x*x; }\nfloat4 Pow3(float4 x) { return x*x*x; }\n")))
            .Add(FString(TEXT("float Pow4(float x) { float xx = x*x; return xx * xx; }\nfloat2 Pow4(float2 x) { float2 xx = x*x; return xx * xx; }\nfloat3 Pow4(float3 x) { float3 xx = x*x; return xx * xx; }\nfloat4 Pow4(float4 x) { float4 xx = x*x; return xx * xx; }\n")))
            .Add(FString(TEXT("float Pow5(float x) { float xx = x*x; return xx * xx * x; }\nfloat2 Pow5(float2 x) { float2 xx = x*x; return xx * xx * x; }\nfloat3 Pow5(float3 x) { float3 xx = x*x; return xx * xx * x; }\nfloat4 Pow5(float4 x) { float4 xx = x*x; return xx * xx * x; }\n")))
            .Add(FString(TEXT("float Pow6(float x) { float xx = x*x; return xx * xx * xx; }\nfloat2 Pow6(float2 x) { float2 xx = x*x; return xx * xx * xx; }\nfloat3 Pow6(float3 x) { float3 xx = x*x; return xx * xx * xx; }\nfloat4 Pow6(float4 x) { float4 xx = x*x; return xx * xx * xx; }\n")));

        FString OutCode = Code;

        for (int32 Index = 0; Index < MathFunctions.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(MathFunctions[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    bool PrevOK = false;
                    // check previous
                    if (StartPos != 0)
                    {
                        if (!IsNameAscii(OutCode[StartPos - 1]))
                        {
                            PrevOK = true;
                        }
                    }
                    else
                    {
                        PrevOK = true;
                    }

                    int32 EndPos = StartPos + MathFunctions[Index].Len();
                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }
                    else if (PrevOK)
                    {
                        // check latter
                        if (IsLeftParenthese(OutCode[EndPos]))
                        {
                            FMaterialCustomExpressionEntry CustomExpression;
                            CustomExpression.ScopeID = 0;
                            CustomExpression.Expression = nullptr;
                            CustomExpression.Implementation = ReplacedConstant[Index];
                            CustomExpressions.AddUnique(CustomExpression);
                        }
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    // MDL didn't have uint, replacing with int
    FString ReplaceHlslTypeWithMdlType(const FString& Code, const TMap<FString, FString>& Inputs)
    {
        static const TArray<FString> Types = TArrayBuilder<FString>()
            .Add(FString(TEXT("uint")))
            .Add(FString(TEXT("uint2")))
            .Add(FString(TEXT("uint3")))
            .Add(FString(TEXT("uint4")))
            .Add(FString(TEXT("const")));

        static const TArray<FString> MdlTypes = TArrayBuilder<FString>()
            .Add(FString(TEXT("int")))
            .Add(FString(TEXT("int2")))
            .Add(FString(TEXT("int3")))
            .Add(FString(TEXT("int4")))
            .Add(FString(TEXT("")));

        FString OutCode = Code;

        auto ReplaceName = [&](const FString& Source, const FString& Dest)
        {
            int32 StartPos = INDEX_NONE;
            do
            {
                StartPos = OutCode.Find(Source, ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    bool PrevOK = false;
                    // check previous
                    if (StartPos != 0)
                    {
                        if (!IsNameAscii(OutCode[StartPos - 1]))
                        {
                            PrevOK = true;
                        }
                    }
                    else
                    {
                        PrevOK = true;
                    }

                    int32 EndPos = StartPos + Source.Len();
                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }
                    else if (PrevOK)
                    {
                        if (!IsNameAscii(OutCode[EndPos]))
                        {
                            OutCode.RemoveAt(StartPos, Source.Len());
                            OutCode.InsertAt(StartPos, Dest);

                            EndPos -= Source.Len();
                            EndPos += Dest.Len();
                        }	
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        };

        for (int32 Index = 0; Index < Types.Num(); Index++)
        {
            ReplaceName(Types[Index], MdlTypes[Index]);
        }

        for (auto Input : Inputs)
        {
            ReplaceName(Input.Key, Input.Value);
        }

        return OutCode;
    }

    // Convert from (int) to int(), (float) to float()
    FString ReplaceHlslConversion(const FString& Code)
    {
        static const TArray<FString> Types = TArrayBuilder<FString>()
            .Add(FString(TEXT("(float)")))
            .Add(FString(TEXT("(int)")));
        static const TArray<FString> Conversions = TArrayBuilder<FString>()
            .Add(FString(TEXT("float")))
            .Add(FString(TEXT("int")));

        FString OutCode = Code;

        for (int32 Index = 0; Index < Types.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(Types[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    if ((StartPos + Types[Index].Len()) < OutCode.Len())
                    {
                        if (IsLeftParenthese(OutCode[StartPos + Types[Index].Len()]))
                        {
                            OutCode.RemoveAt(StartPos, Types[Index].Len());
                            OutCode.InsertAt(StartPos, Conversions[Index]);
                        }
                        else
                        {
                            int32 EndPos = StartPos + Types[Index].Len();
                            while(EndPos < OutCode.Len() && IsOperand(OutCode[EndPos]))
                            {
                                EndPos++;
                            }
                            OutCode.InsertAt(EndPos, TEXT(")"));
                            OutCode.RemoveAt(StartPos, Types[Index].Len());
                            OutCode.InsertAt(StartPos, Conversions[Index] + TEXT("("));
                        }
                    }
                }
            }while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    // convert from XXX = {} to XXX = XXX()
    FString ReplaceHlslConstructor(const FString& Code)
    {
        static const TArray<FString> Types = TArrayBuilder<FString>()
            .Add(FString(TEXT("float4x4")))
            .Add(FString(TEXT("float4x3")))
            .Add(FString(TEXT("float3x4")))
            .Add(FString(TEXT("float3x3")))
            .Add(FString(TEXT("float3x2")))
            .Add(FString(TEXT("float2x3")))
            .Add(FString(TEXT("float2x2")))
            .Add(FString(TEXT("float4")))
            .Add(FString(TEXT("float3")))
            .Add(FString(TEXT("float2")));

        FString OutCode = Code;

        for (int32 Index = 0; Index < Types.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(Types[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    bool PrevOK = false;
                    // check previous
                    if (StartPos != 0)
                    {
                        if (!IsNameAscii(OutCode[StartPos - 1]))
                        {
                            PrevOK = true;
                        }
                    }
                    else
                    {
                        PrevOK = true;
                    }

                    int32 EndPos = StartPos + Types[Index].Len();
                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }
                    else if (PrevOK)
                    {
                        if (!IsNameAscii(OutCode[EndPos]))
                        {
                            TOptional<int32> LeftAssign;
                            while (EndPos < OutCode.Len())
                            {
                                if (IsEqualSign(OutCode[EndPos]))
                                {
                                    LeftAssign = EndPos;
                                    if (EndPos > 0 && IsEqualSign(OutCode[EndPos - 1]))
                                    {
                                        LeftAssign = INDEX_NONE;
                                    }
                                }

                                if (IsSemicolon(OutCode[EndPos]))
                                {
                                    break;
                                }
                                ++EndPos;
                            }

                            if (LeftAssign.IsSet() && LeftAssign != INDEX_NONE)
                            {
                                int32 SearchStart = LeftAssign.GetValue();
                                while (SearchStart < EndPos)
                                {
                                    if (IsLeftBracket(OutCode[SearchStart]))
                                    {
                                        OutCode[SearchStart] = '(';
                                        OutCode.InsertAt(SearchStart, Types[Index]);
                                        EndPos += Types[Index].Len();
                                    }
                                    else if (IsRightBracket(OutCode[SearchStart]))
                                    {
                                        OutCode[SearchStart] = ')';
                                    }
                                    ++SearchStart;
                                }
                            }
                        }
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    // hlsl vector constructor to mdl vector constructor
    // support float4(float2, float, float), float4(float3, float) and float3(float2, float)
    FString ReplaceInvalidMdlConstructor(const FString& Code)
    {
        static const TArray<FString> Types = TArrayBuilder<FString>()
            .Add(FString(TEXT("float4")))
            .Add(FString(TEXT("float3")));

        static const TArray<int32> NumParamsOfType = TArrayBuilder<int32>()
            .Add(4)
            .Add(3);

        FString OutCode = Code;
        for (int32 Index = 0; Index < Types.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(Types[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    bool PrevOK = false;
                    // check previous
                    if (StartPos != 0)
                    {
                        if (!IsNameAscii(OutCode[StartPos - 1]))
                        {
                            PrevOK = true;
                        }
                    }
                    else
                    {
                        PrevOK = true;
                    }

                    int32 EndPos = StartPos + Types[Index].Len();
                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }
                    else if (PrevOK)
                    {
                        if (!IsNameAscii(OutCode[EndPos]))
                        {
                            while (EndPos < OutCode.Len() && IsBlank(OutCode[EndPos]))
                            {
                                ++EndPos;
                            }

                            if (IsLeftParenthese(OutCode[EndPos]))
                            {
                                // check the number of comma to match the parameters number
                                TArray<FString> Parameters;
                                TArray<int32> ParameterEnds; 
                                GetHlslFunctionParameters(OutCode, EndPos + 1, Parameters, ParameterEnds);
                                if (Parameters.Num() > 1 && Parameters.Num() < NumParamsOfType[Index])
                                {
                                    FString ConstructorString;
                                    //TODO: this could be issue here, only assuming the first parameter is the problem
                                    if (Parameters.Num() == NumParamsOfType[Index] - 1) // float3(float2, float) or float4(float2, float, float)
                                    {
                                        if (Types[Index] == TEXT("float3"))
                                        {
                                            ConstructorString = FString::Printf(TEXT("(%s).x, (%s).y, %s"), *Parameters[0], *Parameters[0], *Parameters[1]);
                                        }
                                        else
                                        {
                                            ConstructorString = FString::Printf(TEXT("(%s).x, (%s).y, %s, %s"), *Parameters[0], *Parameters[0], *Parameters[1], *Parameters[2]);
                                        }
                                    }
                                    else // float4(float3, float)
                                    {
                                        ConstructorString = FString::Printf(TEXT("(%s).x, (%s).y, (%s).z, %s"), *Parameters[0], *Parameters[0], *Parameters[0], *Parameters[1]);
                                    }

                                    //
                                    OutCode.RemoveAt(EndPos + 1, ParameterEnds[ParameterEnds.Num() - 1] - (EndPos + 1));
                                    OutCode.InsertAt(EndPos + 1, ConstructorString);
                                }
                            }
                        }
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    // Hlsl math function -> mdl math function
    FString ReplaceWithMdlMath(const FString& Code)
    {
        static const TArray<FString> MathFunctions = TArrayBuilder<FString>()
            .Add(FString(TEXT("PI")))
            .Add(FString(TEXT("sin")))
            .Add(FString(TEXT("cos")))
            .Add(FString(TEXT("sincos")))
            .Add(FString(TEXT("tan")))
            .Add(FString(TEXT("asin")))
            .Add(FString(TEXT("acos")))
            .Add(FString(TEXT("atan")))
            .Add(FString(TEXT("atan2")))
            .Add(FString(TEXT("floor")))
            .Add(FString(TEXT("ceil")))
            .Add(FString(TEXT("round")))
            .Add(FString(TEXT("sign")))
            .Add(FString(TEXT("frac")))
            .Add(FString(TEXT("fmod")))
            .Add(FString(TEXT("abs")))
            .Add(FString(TEXT("dot")))
            .Add(FString(TEXT("cross")))
            .Add(FString(TEXT("pow")))
            .Add(FString(TEXT("log2")))
            .Add(FString(TEXT("log10")))
            .Add(FString(TEXT("sqrt")))
            .Add(FString(TEXT("length")))
            .Add(FString(TEXT("lerp")))
            .Add(FString(TEXT("min")))
            .Add(FString(TEXT("max")))
            .Add(FString(TEXT("saturate")))
            .Add(FString(TEXT("smoothstep")))
            .Add(FString(TEXT("normalize")))
            .Add(FString(TEXT("distance")))
            .Add(FString(TEXT("degrees")))
            .Add(FString(TEXT("radians")))
            .Add(FString(TEXT("mul")))
            .Add(FString(TEXT("clamp")))
            .Add(FString(TEXT("exp")))
            .Add(FString(TEXT("exp2")));

        FString OutCode = Code;

        for (int32 Index = 0; Index < MathFunctions.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(MathFunctions[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    bool PrevOK = false;
                    // check previous
                    if (StartPos != 0)
                    {
                        if (!IsNameAscii(OutCode[StartPos - 1]))
                        {
                            PrevOK = true;
                        }
                    }
                    else
                    {
                        PrevOK = true;
                    }

                    int32 EndPos = StartPos + MathFunctions[Index].Len();
                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }
                    else if (PrevOK)
                    {
                        bool bMathPi = (MathFunctions[Index] == TEXT("PI") && !IsNameAscii(OutCode[EndPos]));
                        
                        while (EndPos < OutCode.Len() && IsBlank(OutCode[EndPos]))
                        {
                            ++EndPos;
                        }

                        // check latter
                        if (IsLeftParenthese(OutCode[EndPos]) || bMathPi)
                        {
                            if (MathFunctions[Index] == TEXT("mul"))
                            {
                                // no specific mdl function, replace with *
                                int32 RemoveSize = MathFunctions[Index].Len();
                                OutCode.RemoveAt(StartPos, RemoveSize);
                                EndPos -= RemoveSize;

                                TArray<FString> Parameters;
                                TArray<int32> ParameterEnds;
                                GetHlslFunctionParameters(OutCode, EndPos + 1, Parameters, ParameterEnds);
                                if (ParameterEnds.Num() > 0)
                                {
                                    OutCode[ParameterEnds[0]] = '*';
                                }
                            }
                            else
                            {
                                FString ModuleName = MathFunctions[Index] == TEXT("smoothstep") ? TEXT("::") : TEXT("math::");
                                OutCode.InsertAt(StartPos, ModuleName);
                                EndPos += ModuleName.Len();
                            }
                        }
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    bool IsLineCommentStart(const TCHAR* Line)
    {
        return (*Line == '/' && *(Line + 1) == '/');
    }

    bool IsBlockCommentStart(const TCHAR* Line)
    {
        return (*Line == '/' && *(Line + 1) == '*');
    }

    bool IsBlockCommentEnd(const TCHAR* Line)
    {
        return (*Line == '*' && *(Line + 1) == '/');
    }

    bool IsCommentStart(const TCHAR* Line)
    {
        return IsLineCommentStart(Line) || IsBlockCommentStart(Line);
    }

    FString RemoveComments(const FString& Code)
    {
        FString OutCode = Code;

        TCHAR* Start = &OutCode[0];
        TCHAR* End = Start + OutCode.Len();
        // check /* and //

        while (*Start != '\0')
        {
            while (Start != (End - 1) && !IsCommentStart(Start))
            {
                Start++;
            }

            if (Start >= End - 1)
            {
                break;
            }
            else
            {
                if (IsLineCommentStart(Start))
                {
                    TCHAR* LineEnd = Start;

                    while (*LineEnd != '\n' && LineEnd != End)
                    {
                        LineEnd++;
                    }

                    int Count = End - LineEnd;
                    FMemory::Memmove(Start, LineEnd, sizeof(TCHAR) * Count);

                    OutCode.RemoveAt((Start - &OutCode[0]) + Count, End - (Start + Count), false);
                    End = Start + Count;
                }
                else
                {
                    TCHAR* BlockEnd = Start;

                    while (BlockEnd != (End - 1) && !IsBlockCommentEnd(BlockEnd))
                    {
                        BlockEnd++;
                    }

                    if (BlockEnd != (End - 1))
                    {
                        BlockEnd += 2; // Skip */
                    }
                    else
                    {
                        BlockEnd = End;
                    }

                    int Count = End - BlockEnd;
                    FMemory::Memmove(Start, BlockEnd, sizeof(TCHAR) * Count);

                    OutCode.RemoveAt((Start - &OutCode[0]) + Count, End - (Start + Count), false);
                    End = Start + Count;
                }
            }
        }

        return OutCode;
    }

    bool IsMacroStart(const TCHAR& Line)
    {
        return (Line == '#');
    }

    FString RemoveMacros(const FString& Code)
    {
        FString OutCode = Code;
        
        auto GetNextMacro = [&](int32 StartIndex, FString& MacroName, int32& MacroStart, int32& LineEnd)
        {
            int32 Len = OutCode.Len();
            while (StartIndex < OutCode.Len())
            {
                if (IsMacroStart(OutCode[StartIndex]))
                {
                    MacroStart = StartIndex;
                    ++StartIndex;
                    // Skip blank
                    while (StartIndex < OutCode.Len() && IsBlank(OutCode[StartIndex]))
                    {
                        ++StartIndex;
                    }

                    // Get MacroName
                    int32 NameStart = StartIndex;
                    while (StartIndex < OutCode.Len() && (!IsBlank(OutCode[StartIndex]) && OutCode[StartIndex] != '\r' && OutCode[StartIndex] != '\n'))
                    {
                        ++StartIndex;
                    }
                    MacroName = OutCode.Mid(NameStart, StartIndex - NameStart);

                    while (StartIndex < OutCode.Len() && OutCode[StartIndex] != '\n')
                    {
                        ++StartIndex;
                    }
                    LineEnd = StartIndex;

                    return true;
                }

                ++StartIndex;
            }

            return false;
        };

        auto RemoveLine = [&](int32 RemoveStart, int32 RemoveEnd)
        {
            int32 RemoveCount = RemoveEnd - RemoveStart + 1;
            OutCode.RemoveAt(RemoveStart, RemoveCount);
        };

        int32 Start = 0;
        bool bHasMacro = false;
        TArray<int32> RemoveMacros;
        do
        {
            FString MacroName;
            int32 MacroStart;
            int32 LineEnd;
            bHasMacro = GetNextMacro(Start, MacroName, MacroStart, LineEnd);
            if (bHasMacro)
            {
                if (MacroName == TEXT("if") || MacroName == TEXT("ifdef")
                || MacroName == TEXT("define") || MacroName == TEXT("undef") || MacroName == TEXT("include"))
                {
                    // Remove line
                    RemoveLine(MacroStart, LineEnd);
                    Start = MacroStart;
                }
                else if (MacroName == TEXT("ifndef")
                || MacroName == TEXT("else") || MacroName == TEXT("elif"))
                {
                    RemoveMacros.Push(MacroStart);
                    Start = LineEnd;
                }
                else if (MacroName == TEXT("endif"))
                {
                    // Remove the line
                    if (RemoveMacros.Num() > 0)
                    {
                        int32 RemoveStart = RemoveMacros.Pop();
                        RemoveLine(RemoveStart, LineEnd);
                        Start = RemoveStart;
                    }
                    else
                    {
                        // Remove the line
                        RemoveLine(MacroStart, LineEnd);
                        Start = MacroStart;
                    }
                }
            }
        }
        while(bHasMacro);

        return OutCode;
    }

    // Position starts after '('
    void GetHlslFunctionParameters(const FString& Code, int32 Position, TArray<FString>& Parameters, TArray<int32>& ParameterEnds)
    {
        int32 ParameterStart = Position;
        TOptional<int32> LastCommaStart;
        int32 StackDepth = 0;
        while (ParameterStart < Code.Len())
        {
            if (IsLeftParenthese(Code[ParameterStart]))
            {
                ++StackDepth;
            }

            if (IsRightParenthese(Code[ParameterStart]))
            {
                if (StackDepth == 0)
                {
                    if (LastCommaStart.IsSet())
                    {
                        Parameters.Add(Code.Mid(LastCommaStart.GetValue() + 1, ParameterStart - LastCommaStart.GetValue() - 1));
                    }
                    else
                    {
                        Parameters.Add(Code.Mid(Position, ParameterStart - Position));
                    }
                    ParameterEnds.Add(ParameterStart);
                    break;
                }

                --StackDepth;
            }

            if (StackDepth == 0 && IsComma(Code[ParameterStart]))
            {
                if (LastCommaStart.IsSet())
                {
                    Parameters.Add(Code.Mid(LastCommaStart.GetValue() + 1, ParameterStart - LastCommaStart.GetValue() - 1));
                }
                else
                {
                    Parameters.Add(Code.Mid(Position, ParameterStart - Position));
                }
                ParameterEnds.Add(ParameterStart);
                LastCommaStart = ParameterStart;
            }
            ++ParameterStart;
        }
    }

    // sincos function has different parameters in hlsl and mdl
    FString ConvertToMdlParameter(const FString& Code)
    {
        FString OutCode = Code;

        static const TArray<FString> HlslFunctions = TArrayBuilder<FString>()
            .Add(FString(TEXT("math::sincos")));

        static const TArray<FString> FunctionReturns = TArrayBuilder<FString>()
            .Add(FString(TEXT("float[2] sc")));

        static const TArray<FString> NewParams = TArrayBuilder<FString>()
            .Add(FString(TEXT("sc[0]")))
            .Add(FString(TEXT("sc[1]")));

        for (int32 Index = 0; Index < HlslFunctions.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(HlslFunctions[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    FString Return = FunctionReturns[Index].IsEmpty() ? TEXT("") : (FunctionReturns[Index] + TEXT(" = "));
                    OutCode.InsertAt(StartPos, Return);
                    
                    int32 EndPos = StartPos + HlslFunctions[Index].Len() + Return.Len();
                    while (EndPos < OutCode.Len() && IsBlank(OutCode[EndPos]))
                    {
                        ++EndPos;
                    }

                    TArray<FString> Parameters;
                    if (IsLeftParenthese(OutCode[EndPos]))
                    {
                        int32 ParameterStart = EndPos + 1;					
                        TOptional<int32> RemovedStart;
                        TOptional<int32> LastCommaStart;
                        int32 StackDepth = 0;
                        while (ParameterStart < OutCode.Len())
                        {
                            if (IsLeftParenthese(Code[ParameterStart]))
                            {
                                ++StackDepth;
                            }

                            if (IsRightParenthese(OutCode[ParameterStart]))
                            {
                                if (StackDepth == 0)
                                {
                                    if (LastCommaStart.IsSet())
                                    {
                                        Parameters.Add(OutCode.Mid(LastCommaStart.GetValue() + 1, ParameterStart - LastCommaStart.GetValue() - 1));
                                    }
                                    else
                                    {
                                        Parameters.Add(OutCode.Mid(EndPos + 1, ParameterStart - EndPos - 1));
                                    }

                                    if (RemovedStart.IsSet())
                                    {
                                        OutCode.RemoveAt(RemovedStart.GetValue(), ParameterStart - RemovedStart.GetValue());
                                    }

                                    ParameterStart -= ParameterStart - RemovedStart.GetValue();

                                    break;
                                }
                                --StackDepth;
                            }

                            if (StackDepth == 0 && IsComma(OutCode[ParameterStart]))
                            {
                                if (!RemovedStart.IsSet())
                                {
                                    RemovedStart = ParameterStart;
                                }

                                if (LastCommaStart.IsSet())
                                {
                                    Parameters.Add(OutCode.Mid(LastCommaStart.GetValue() + 1, ParameterStart - LastCommaStart.GetValue() - 1));
                                }
                                else
                                {
                                    Parameters.Add(OutCode.Mid(EndPos + 1, ParameterStart - EndPos - 1));
                                }

                                LastCommaStart = ParameterStart;
                            }
                            ++ParameterStart;
                        }

                        EndPos = ParameterStart;
                    }

                    while (EndPos < OutCode.Len())
                    {
                        if (IsSemicolon(OutCode[EndPos]))
                        {
                            FString NewAssign;
                            for (int ParamIndex = 0; ParamIndex < NewParams.Num(); ++ParamIndex)
                            {
                                NewAssign += FString::Printf(TEXT("\n%s = %s;"), *Parameters[ParamIndex + 1], *NewParams[ParamIndex]);
                            }

                            OutCode.InsertAt(EndPos + 1, NewAssign);
                            EndPos += NewAssign.Len();
                            break;
                        }

                        ++EndPos;
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    FString ReplaceVariableWithConstant(const FString& Code, TArray<FString>& PreDefinedCodes)
    {
        static const TArray<FString> Variables = TArrayBuilder<FString>()
            .Add(FString(TEXT("GlobalRayCone_TexArea")))
            .Add(FString(TEXT("GetPrimitiveData(Parameters.PrimitiveId).ObjectBounds")))
            .Add(FString(TEXT("GetPrimitiveData(Parameters.PrimitiveId).LocalObjectBoundsMin")))
            .Add(FString(TEXT("GetPrimitiveData(Parameters.PrimitiveId).LocalObjectBoundsMax")))
            .Add(FString(TEXT("Rand3DPCG16")))
            .Add(FString(TEXT("View.TemporalAAParams")))
            .Add(FString(TEXT("View.ClipToView[0][0]")))
            .Add(FString(TEXT("View.ViewForward")))
            .Add(FString(TEXT("GetCotanHalfFieldOfView().x")))
            .Add(FString(TEXT("GetCotanHalfFieldOfView().y")))
            .Add(FString(TEXT("GetCotanHalfFieldOfView()")));

        static const TArray<FString> ReplacedConstant = TArrayBuilder<FString>()
            .Add(FString(TEXT("(1.0 / 16777216.0)")))
            .Add(FString(TEXT("::object_bounds()")))
            .Add(FString(TEXT("::local_object_bounds_min()")))
            .Add(FString(TEXT("::local_object_bounds_max()")))
            .Add(FString(TEXT("Rand3DPCG16")))
            .Add(FString(TEXT("float2(1.0)")))
            .Add(FString(TEXT("1.0")))
            .Add(FString(TEXT("float3(0.0, 0.0, 1.0)")))
            .Add(FString(TEXT("1.0")))
            .Add(FString(TEXT("1.0")))
            .Add(FString(TEXT("float2(1.0)")));


        static const TArray<bool> SearchFunction = TArrayBuilder<bool>()
            .Add(false)
            .Add(false)
            .Add(false)
            .Add(false)
            .Add(true)
            .Add(false)
            .Add(false)
            .Add(false)
            .Add(false)
            .Add(false)
            .Add(false);

        static const TArray<FString> PreDefine = TArrayBuilder<FString>()
            .Add(FString())
            .Add(FString())
            .Add(FString())
            .Add(FString())
            .Add(FString(TEXT("int3 Rand3DPCG16 = int3(0);\n")))
            .Add(FString())
            .Add(FString())
            .Add(FString())
            .Add(FString())
            .Add(FString())
            .Add(FString());

        FString OutCode = Code;

        for (int32 Index = 0; Index < Variables.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(Variables[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    int32 EndPos = StartPos + Variables[Index].Len();

                    if (SearchFunction[Index])
                    {
                        while (EndPos < OutCode.Len() && IsBlank(OutCode[EndPos]))
                        {
                            ++EndPos;
                        }

                        if (IsLeftParenthese(OutCode[EndPos]))
                        {
                            ++EndPos;
                            int32 FunctionStack = 1;
                            while (EndPos < OutCode.Len())
                            {
                                if (IsLeftParenthese(OutCode[EndPos]))
                                {
                                    ++FunctionStack;
                                }
                                else if(IsRightParenthese(OutCode[EndPos]))
                                {
                                    --FunctionStack;
                                    if (FunctionStack == 0)
                                    {
                                        break;
                                    }
                                }
                                ++EndPos;
                            }
                            
                            OutCode.RemoveAt(StartPos, EndPos + 1 - StartPos);
                            OutCode.InsertAt(StartPos, ReplacedConstant[Index]);
                            EndPos = StartPos + ReplacedConstant[Index].Len();

                            if (!PreDefine[Index].IsEmpty())
                            {
                                PreDefinedCodes.AddUnique(PreDefine[Index]);
                            }
                        }	
                    }
                    else
                    {
                        if (EndPos == OutCode.Len() || (!IsLeftParenthese(OutCode[EndPos]) && !IsNameAscii(OutCode[EndPos])))
                        {
                            OutCode.RemoveAt(StartPos, Variables[Index].Len());
                            OutCode.InsertAt(StartPos, ReplacedConstant[Index]);

                            if (!PreDefine[Index].IsEmpty())
                            {
                                PreDefinedCodes.AddUnique(PreDefine[Index]);
                            }
                        }
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    TArray<FString> GetOutputType(const FString& Code)
    {
        TArray<FString> OutputType;

        if (Code == TEXT(".xxx"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".yyy"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".zzz"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".xyz"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".xzy"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".yxz"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".yzx"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".zyx"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".zxy"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".xy"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".yx"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".xz"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".zx"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".yz"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".zy"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".rrr"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".ggg"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".bbb"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".rgb"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".rbg"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".grb"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".gbr"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".bgr"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".brg"))
        {
            OutputType.Add(TEXT("float3"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".rg"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".y"));
        }
        else if (Code == TEXT(".gr"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".rb"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".x"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".br"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".x"));
        }
        else if (Code == TEXT(".gb"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".y"));
            OutputType.Add(TEXT(".z"));
        }
        else if (Code == TEXT(".bg"))
        {
            OutputType.Add(TEXT("float2"));
            OutputType.Add(TEXT(".z"));
            OutputType.Add(TEXT(".y"));
        }

        return OutputType;
    }

    // Hlsl can work with per-component operation, but MDL cannot.
    FString ReplaceImplicitCast(const FString& Code)
    {
        static const TArray<FString> TypeCast = TArrayBuilder<FString>()
            .Add(FString(TEXT(".xxx")))
            .Add(FString(TEXT(".yyy")))
            .Add(FString(TEXT(".zzz")))
            .Add(FString(TEXT(".xyz")))
            .Add(FString(TEXT(".xzy")))
            .Add(FString(TEXT(".yxz")))
            .Add(FString(TEXT(".yzx")))
            .Add(FString(TEXT(".zyx")))
            .Add(FString(TEXT(".zxy")))
            .Add(FString(TEXT(".xy")))
            .Add(FString(TEXT(".yx")))
            .Add(FString(TEXT(".xz")))
            .Add(FString(TEXT(".zx")))
            .Add(FString(TEXT(".yz")))
            .Add(FString(TEXT(".zy")))
            .Add(FString(TEXT(".rrr")))
            .Add(FString(TEXT(".ggg")))
            .Add(FString(TEXT(".bbb")))
            .Add(FString(TEXT(".rgb")))
            .Add(FString(TEXT(".rbg")))
            .Add(FString(TEXT(".grb")))
            .Add(FString(TEXT(".gbr")))
            .Add(FString(TEXT(".bgr")))
            .Add(FString(TEXT(".brg")))
            .Add(FString(TEXT(".rg")))
            .Add(FString(TEXT(".gr")))
            .Add(FString(TEXT(".rb")))
            .Add(FString(TEXT(".br")))
            .Add(FString(TEXT(".gb")))
            .Add(FString(TEXT(".bg")))
            .Add(FString(TEXT(".r")))
            .Add(FString(TEXT(".g")))
            .Add(FString(TEXT(".b")))
            .Add(FString(TEXT(".a")));

        FString OutCode = Code;

        struct FAbbrType
        {
            int32 StartPos;
            TArray<FString> Types;
            FString Variable;
            int32 EqualPos;
            int32 EndPos;
        };

        TArray<FAbbrType> AbbrCast;

        for (int32 Index = 0; Index < TypeCast.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;
            do
            {
                StartPos = OutCode.Find(TypeCast[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos > 0)
                {
                    // check if it's per-component operation
                    if (StartPos + TypeCast[Index].Len() < OutCode.Len())
                    {
                        if (IsNameAscii(OutCode[StartPos + TypeCast[Index].Len()]))
                        {
                            StartPos += TypeCast[Index].Len();
                            continue;
                        }
                    }

                    if (TypeCast[Index] == TEXT(".r"))
                    {
                        OutCode[StartPos + 1] = 'x';
                        StartPos += TypeCast[Index].Len();
                        continue;
                    }
                    else if (TypeCast[Index] == TEXT(".g"))
                    {
                        OutCode[StartPos + 1] = 'y';
                        StartPos += TypeCast[Index].Len();
                        continue;
                    }
                    else if (TypeCast[Index] == TEXT(".b"))
                    {
                        OutCode[StartPos + 1] = 'z';
                        StartPos += TypeCast[Index].Len();
                        continue;
                    }
                    else if (TypeCast[Index] == TEXT(".a"))
                    {
                        OutCode[StartPos + 1] = 'w';
                        StartPos += TypeCast[Index].Len();
                        continue;
                    }

                    // get variable name
                    int32 VariableStart = StartPos;

                    if (VariableStart > 0)
                    {
                        if (IsOperand(OutCode[VariableStart - 1])) // Variable
                        {
                            while (VariableStart > 0 && IsOperand(OutCode[VariableStart - 1]))
                            {
                                --VariableStart;
                            }
                        }
                        else if (IsRightParenthese(OutCode[VariableStart - 1])) // Function
                        {
                            int ParentheseStart = VariableStart - 1;
                            int32 FunctionStack = 1;
                            while (ParentheseStart > 0)
                            {
                                if (IsLeftParenthese(OutCode[ParentheseStart - 1]))
                                {
                                    --FunctionStack;
                                    if (FunctionStack == 0)
                                    {
                                        --ParentheseStart;
                                        break;
                                    }
                                }
                                else if (IsRightParenthese(OutCode[ParentheseStart - 1]))
                                {
                                    ++FunctionStack;

                                }
                                --ParentheseStart;
                            }

                            VariableStart = ParentheseStart;
                            while (VariableStart > 0 && IsOperand(OutCode[VariableStart - 1]))
                            {
                                --VariableStart;
                            }
                        }
                    }

                    FString Variable = OutCode.Mid(VariableStart, StartPos - VariableStart);
                    TArray<FString> Types = GetOutputType(TypeCast[Index]);

                    FString TypeStr = Types[0] + TEXT("(");
                    OutCode.RemoveAt(StartPos, TypeCast[Index].Len());
                    OutCode.InsertAt(VariableStart, TypeStr);

                    AbbrCast.Add({ VariableStart, Types, Variable, INDEX_NONE, INDEX_NONE });

                    int32 EndPos = VariableStart + TypeStr.Len() + Variable.Len();

                    FString ConvertedStr = Types[1] + TEXT(",") + Variable + Types[2] + (Types.Num() > 3 ? (TEXT(",") + Variable + Types[3] + TEXT(")")) : TEXT(")"));
                    OutCode.InsertAt(EndPos, ConvertedStr);

                    EndPos += ConvertedStr.Len();

                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }

                    StartPos = EndPos;
                }
            } while (StartPos > 0);
        }

        TArray<int32> LeftAssignIndices;
        for (int32 Index = 0; Index < AbbrCast.Num(); ++Index)
        {
            int32 StartPos = AbbrCast[Index].StartPos;
            TOptional<int32> LeftAssign;
            while (StartPos < OutCode.Len())
            {
                if (IsEqualSign(OutCode[StartPos]))
                {
                    LeftAssign = StartPos;
                    if (StartPos > 0 && IsEqualSign(OutCode[StartPos - 1]))
                    {
                        LeftAssign = INDEX_NONE;
                    }
                }

                if (IsSemicolon(OutCode[StartPos]))
                {
                    break;
                }
                ++StartPos;
            }

            if (LeftAssign.IsSet() && LeftAssign != INDEX_NONE)
            {
                AbbrCast[Index].EqualPos = LeftAssign.GetValue();
                AbbrCast[Index].EndPos = StartPos;
                LeftAssignIndices.Add(Index);
            }
        }

        FString NewOut;
        int32 CopyStart = 0;
        for (auto Index : LeftAssignIndices)
        {
            if (AbbrCast[Index].StartPos > 0)
            {
                NewOut.Append(OutCode.Mid(CopyStart, AbbrCast[Index].StartPos - CopyStart));
            }

            CopyStart = AbbrCast[Index].EndPos + 1;

            FString Assign;
            for (int32 TypeIndex = 1; TypeIndex < AbbrCast[Index].Types.Num(); ++TypeIndex)
            {
                Assign += AbbrCast[Index].Variable + AbbrCast[Index].Types[TypeIndex];
                Assign += TEXT("=(");
                Assign += OutCode.Mid(AbbrCast[Index].EqualPos + 1, AbbrCast[Index].EndPos - AbbrCast[Index].EqualPos - 1);
                Assign += TEXT(")");
                Assign += AbbrCast[Index].Types[TypeIndex];
                Assign += TEXT(";\n");
            }
            
            NewOut.Append(Assign);
        }

        NewOut.Append(OutCode.Mid(CopyStart, OutCode.Len() - CopyStart));
        return NewOut;
    }

    // Hlsl texture sampler -> mdl tex operation
    FString ReplaceTextureSample(const FString& Code)
    {
        // TODO: more sample function in hlsl
        static const TArray<FString> UETextureSampleFunctions = TArrayBuilder<FString>()
            .Add(FString(TEXT("Texture2DSample")))
            .Add(FString(TEXT("UnpackNormalMap")));

        static const TArray<FString> MdlTextureSampleFunctions = TArrayBuilder<FString>()
            .Add(FString(TEXT("tex::lookup_float4")))
            .Add(FString(TEXT("::unpack_normal_map")));

        FString OutCode = Code;

        for (int32 Index = 0; Index < UETextureSampleFunctions.Num(); Index++)
        {
            int32 StartPos = INDEX_NONE;

            do
            {
                StartPos = OutCode.Find(UETextureSampleFunctions[Index], ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);
                if (StartPos != INDEX_NONE)
                {
                    bool PrevOK = false;
                    // check previous
                    if (StartPos != 0)
                    {
                        if (!IsNameAscii(OutCode[StartPos - 1]))
                        {
                            PrevOK = true;
                        }
                    }
                    else
                    {
                        PrevOK = true;
                    }

                    int32 EndPos = StartPos + UETextureSampleFunctions[Index].Len();
                    if (EndPos >= OutCode.Len())
                    {
                        EndPos = OutCode.Len() - 1;
                    }
                    else if (PrevOK)
                    {
                        while (EndPos < OutCode.Len() && IsBlank(OutCode[EndPos]))
                        {
                            ++EndPos;
                        }

                        // check latter
                        if (IsLeftParenthese(OutCode[EndPos]))
                        {
                            // replace hlsl sample with mdl sample
                            OutCode.RemoveAt(StartPos, UETextureSampleFunctions[Index].Len());
                            EndPos -= UETextureSampleFunctions[Index].Len();
                            OutCode.InsertAt(StartPos, MdlTextureSampleFunctions[Index]);
                            EndPos += MdlTextureSampleFunctions[Index].Len();

                            if (UETextureSampleFunctions[Index] == TEXT("Texture2DSample"))
                            {
                                // remove 2nd sampler parameter
                                int32 CommaIndex = EndPos + 1;
                                int32 RemoveStart = INDEX_NONE;
                                int32 FunctionStack = 0;
                                while (CommaIndex < OutCode.Len())
                                {
                                    if (FunctionStack == 0 && IsComma(OutCode[CommaIndex]))
                                    {
                                        if (RemoveStart == INDEX_NONE)
                                        {
                                            RemoveStart = CommaIndex;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else if (IsLeftParenthese(OutCode[CommaIndex]))
                                    {
                                        ++FunctionStack;
                                    }
                                    else if (IsRightParenthese(OutCode[CommaIndex]))
                                    {
                                        --FunctionStack;
                                    }

                                    ++CommaIndex;
                                }

                                OutCode.RemoveAt(RemoveStart, CommaIndex - RemoveStart);
                            }
                        }
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    // Parallax occlusion mapping workaround
    FString ReplaceSampleGrad(const FString& Code)
    {
        FString OutCode = Code;

        // SampleGrad is not supported, replace it with float4(1.0)
        if (Code.Contains("SampleGrad"))
        {
            Warning(TEXT("SampleGrad inside the custom expression, using fixed value instead."));

            TArray<int32> LoopStarts;
            TArray<int32> LoopEnds;

            {
                int32 StartPos = INDEX_NONE;
                do
                {
                    StartPos = OutCode.Find(TEXT("while"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);

                    if (StartPos != INDEX_NONE)
                    {
                        LoopStarts.Add(StartPos);

                        int32 LoopStart = StartPos;
                        TOptional<int32> NumBracket;
                        while (LoopStart < OutCode.Len())
                        {
                            if (!NumBracket.IsSet() && IsSemicolon(OutCode[LoopStart]))
                            {
                                break;
                            }

                            if (IsLeftBracket(OutCode[LoopStart]))
                            {
                                if (NumBracket.IsSet())
                                {
                                    NumBracket = NumBracket.GetValue() + 1;
                                }
                                else
                                {
                                    NumBracket = 1;
                                }
                            }

                            if (IsRightBracket(OutCode[LoopStart]))
                            {
                                if (NumBracket.IsSet())
                                {
                                    NumBracket = NumBracket.GetValue() - 1;
                                }
                            }

                            if (NumBracket.IsSet() && NumBracket == 0)
                            {
                                LoopEnds.Add(LoopStart);
                                break;
                            }

                            ++LoopStart;
                        }

                        StartPos = LoopStart;
                    }
                    
                }
                while (StartPos != INDEX_NONE);
            }

            auto IsInLoop = [&LoopStarts, &LoopEnds](int32 Pos) -> int32
            {
                for (int32 Index = 0; Index < LoopStarts.Num(); ++Index)
                {
                    if (Pos > LoopStarts[Index] && Index < LoopEnds.Num() && Pos < LoopEnds[Index])
                    {
                        return Index;
                    }
                }

                return INDEX_NONE;
            };

            {
                int32 StartPos = INDEX_NONE;
                do
                {
                    StartPos = OutCode.Find(TEXT(".SampleGrad"), ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);

                    if (StartPos != INDEX_NONE)
                    {
                        int32 LoopIndex = IsInLoop(StartPos);
                        if (LoopIndex != INDEX_NONE)
                        {
                            // remove looping
                            int32 RemoveSize = LoopEnds[LoopIndex] - LoopStarts[LoopIndex] + 1;
                            OutCode.RemoveAt(LoopStarts[LoopIndex], RemoveSize);
                            LoopStarts.RemoveAt(LoopIndex);
                            LoopEnds.RemoveAt(LoopIndex);
                            for (int32 Index = LoopIndex; Index < LoopStarts.Num(); ++Index)
                            {
                                LoopStarts[Index] -= RemoveSize;
                                if (Index < LoopEnds.Num())
                                {
                                    LoopEnds[Index] -= RemoveSize;
                                }
                            }
                        }
                        else
                        {
                            while (StartPos > 0 && IsNameAscii(OutCode[StartPos - 1]))
                            {
                                --StartPos;
                            }

                            int32 SampleStart = StartPos;
                            TOptional<int32> NumParenthese;
                            while (SampleStart < OutCode.Len())
                            {
                                if (IsLeftParenthese(OutCode[SampleStart]))
                                {
                                    if (NumParenthese.IsSet())
                                    {
                                        NumParenthese = NumParenthese.GetValue() + 1;
                                    }
                                    else
                                    {
                                        NumParenthese = 1;
                                    }
                                }

                                if (IsRightParenthese(OutCode[SampleStart]))
                                {
                                    if (NumParenthese.IsSet())
                                    {
                                        NumParenthese = NumParenthese.GetValue() - 1;
                                    }
                                }

                                if (NumParenthese.IsSet() && NumParenthese == 0)
                                {
                                    break;
                                }

                                ++SampleStart;
                            }

                            if (NumParenthese.IsSet() && NumParenthese == 0)
                            {
                                OutCode.RemoveAt(StartPos, SampleStart - StartPos + 1);
                                OutCode.InsertAt(StartPos, TEXT("float4(1.0)"));

                                int32 RemoveSize = SampleStart - StartPos + 1 - 11;
                                //update loop checking
                                for (int32 Index = 0; Index < LoopStarts.Num(); ++Index)
                                {
                                    LoopStarts[Index] -= RemoveSize;
                                    if (Index < LoopEnds.Num())
                                    {
                                        LoopEnds[Index] -= RemoveSize;
                                    }
                                }
                            }
                            else
                            {
                                StartPos = SampleStart;
                            }
                        }
                    }
                } while (StartPos != INDEX_NONE);
            }
        }

        return OutCode;
    }

    FString SwapHlslMultiply(const FString& Code)
    {
        FString OutCode = Code;

        int32 StartPos = INDEX_NONE;
        do
        {
            StartPos = OutCode.Find(TEXT("*"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, StartPos);
            if (StartPos != INDEX_NONE)
            {
                // get right variable name
                int32 RightStart = StartPos + 1;
                int32 LeftEnd = StartPos - 1;

                while (RightStart < OutCode.Len() && IsBlank(OutCode[RightStart]))
                {
                    ++RightStart;
                }

                while (LeftEnd >= 0 && IsBlank(OutCode[LeftEnd]))
                {
                    --LeftEnd;
                }

                FString LeftOperand;
                FString RightOperand;
                int32 RightEnd = RightStart;
                if (RightStart < OutCode.Len())
                {				
                    if (IsOperand(OutCode[RightEnd])) // Variable
                    {
                        while (RightEnd < OutCode.Len() && IsOperand(OutCode[RightEnd]))
                        {
                            ++RightEnd;
                        }
                    }
                    
                    if (RightEnd < OutCode.Len() && IsLeftParenthese(OutCode[RightEnd])) // Function
                    {
                        int ParentheseEnd = RightEnd + 1;
                        int32 FunctionStack = 1;
                        while (ParentheseEnd < OutCode.Len())
                        {
                            if (IsRightParenthese(OutCode[ParentheseEnd]))
                            {
                                --FunctionStack;
                                if (FunctionStack == 0)
                                {
                                    ++ParentheseEnd;
                                    break;
                                }
                            }
                            else if (IsLeftParenthese(OutCode[ParentheseEnd]))
                            {
                                ++FunctionStack;

                            }
                            ++ParentheseEnd;
                        }
                        RightEnd = ParentheseEnd;
                    }

                    RightOperand = OutCode.Mid(RightStart, RightEnd - RightStart);
                }

                int32 LeftStart = LeftEnd;
                if (LeftEnd >= 0)
                {					
                    if (IsOperand(OutCode[LeftStart])) // Variable
                    {
                        while (LeftStart >= 0 && IsOperand(OutCode[LeftStart]))
                        {
                            --LeftStart;
                        }
                    }
                    else if (IsRightParenthese(OutCode[LeftStart])) // Function
                    {
                        int ParentheseStart = LeftStart - 1;
                        int32 FunctionStack = 1;
                        while (ParentheseStart >= 0)
                        {
                            if (IsLeftParenthese(OutCode[ParentheseStart]))
                            {
                                --FunctionStack;
                                if (FunctionStack == 0)
                                {
                                    --ParentheseStart;
                                    break;
                                }
                            }
                            else if (IsRightParenthese(OutCode[ParentheseStart]))
                            {
                                ++FunctionStack;

                            }
                            --ParentheseStart;
                        }

                        LeftStart = ParentheseStart;
                        while (LeftStart >= 0 && IsOperand(OutCode[LeftStart]))
                        {
                            --LeftStart;
                        }
                    }

                    LeftOperand = OutCode.Mid(LeftStart + 1, LeftEnd - LeftStart);
                }

                if (!RightOperand.IsEmpty() && !LeftOperand.IsEmpty())
                {
                    OutCode.RemoveAt(RightStart, RightOperand.Len());
                    OutCode.InsertAt(RightStart, LeftOperand);

                    OutCode.RemoveAt(LeftStart + 1, LeftOperand.Len());
                    OutCode.InsertAt(LeftStart + 1, RightOperand);

                    StartPos -= LeftOperand.Len();
                    StartPos += RightOperand.Len();
                }		
            }
        } while (StartPos != INDEX_NONE);

        return OutCode;
    }

    // MDL didn't support out parameter, replacing the return value with out parameter
    FString ReplaceAdditionalOutput(const FString& Code, const FName& OutputName)
    {
        FString OutCode = Code;
        const FString ReturnString = TEXT("return ");
        if (!Code.Contains(ReturnString))
        {
            OutCode.Append(FString(TEXT("\r\nreturn ")) + OutputName.ToString() + TEXT(";\r\n"));
        }
        else
        {
            // replace original return with the additional output
            int32 StartPos = INDEX_NONE;
            do
            {
                StartPos = OutCode.Find(ReturnString, ESearchCase::CaseSensitive, ESearchDir::FromStart, StartPos);

                if (StartPos != INDEX_NONE)
                {
                    StartPos += ReturnString.Len();
                    int32 EndPos = StartPos;
                    while (EndPos < OutCode.Len())
                    {
                        if (IsSemicolon(OutCode[EndPos]))
                        {
                            OutCode.RemoveAt(StartPos, EndPos - StartPos);
                            EndPos -= EndPos - StartPos;
                            OutCode.InsertAt(StartPos, OutputName.ToString());
                            EndPos += OutputName.ToString().Len();
                            break;
                        }
                        ++EndPos;
                    }

                    StartPos = EndPos;
                }
            } while (StartPos != INDEX_NONE);
        }

        return OutCode;
    }

    void ExtractConstantProperties(const TArray<EMaterialProperty>& IgnoredProperties, TMap<EMaterialProperty, FLinearColor>& OutConstantProperties)
    {
        for (int32 PropertyIndex = 0; PropertyIndex < MP_MAX; ++PropertyIndex)
        {
            // Skip non-shared properties
            if (!SharedPixelProperties[PropertyIndex])
            {
                continue;
            }

            const EMaterialProperty Property = (EMaterialProperty)PropertyIndex;

            if (IgnoredProperties.Find(Property) != INDEX_NONE)
            {
                continue;
            }

            const EShaderFrequency PropertyShaderFrequency = FMaterialAttributeDefinitionMap::GetShaderFrequency(Property);
            const TArray<FShaderCodeChunk>& ResultChunk = SharedPropertyCodeChunks[PropertyShaderFrequency];

            bool ConstantInput = true;
            for (auto Chunk : ResultChunk)
            {
                if (Chunk.bInline)
                {
                    continue;
                }

                if (TranslatedCodeChunks[PropertyIndex].Contains(Chunk.SymbolName))
                {
                    ConstantInput = false;
                    break;
                }
            }

            if (ConstantInput)
            {	
                auto ValueRet = ConstantChunks.Find(PropertyFinalChunk[PropertyIndex]);
                if (!ValueRet)
                {
                    continue;
                }
                OutConstantProperties.Add(Property, *ValueRet);
            }
        }
    }

    /** Adds an already formatted inline or referenced code chunk */
    virtual int32 AddCodeChunkInner(uint64 Hash, const TCHAR* FormattedCode, EMaterialValueType Type, bool bInlined) override
    {
        const int32 NewIndex = CurrentScopeChunks->Num();
        int32 CodeIndex = FHLSLMaterialTranslator::AddCodeChunkInner(Hash, FormattedCode, Type, bInlined);	
        if (CodeIndex != INDEX_NONE && !bInlined && CodeIndex == NewIndex)
        {
            LocalVariables.Add((*CurrentScopeChunks)[CodeIndex].SymbolName);
        }
        return CodeIndex;
    }

    void GetSharedInputsMaterialCode(const TArray<EMaterialProperty>& IgnoredProperties, TArray<FString>& ParametersCode, FString& NormalCode, FString& NormalAssignment, FString& VertexMembersInitializationEpilog, FString& PixelMembersInitializationEpilog)
    {
        FMaterialShadingModelField MaterialShadingModels = Material->GetShadingModels(); 
        bool bSceneColorTranslucent = MaterialShadingModels.GetFirstShadingModel() == MSM_Unlit && IsTranslucentBlendMode(Material->GetBlendMode()) && bSceneColorUsed && TranslatedCodeChunks[MP_Opacity] == TEXT("1.0");
        bool bSceneDepthTranslucent = MaterialShadingModels.GetFirstShadingModel() == MSM_Unlit && IsTranslucentBlendMode(Material->GetBlendMode()) && bSceneDepthOpacity;
        {
            int32 LastProperty = -1;

            FString PixelInputInitializerValues;
            FString VertexInputInitializerValues;
            FString NormalInitializerValue;

            for (int32 PropertyIndex = 0; PropertyIndex < MP_MAX; ++PropertyIndex)
            {
                // Skip non-shared properties
                if (!SharedPixelProperties[PropertyIndex])
                {
                    continue;
                }

                const EMaterialProperty Property = (EMaterialProperty)PropertyIndex;

                if (IgnoredProperties.Find(Property) != INDEX_NONE)
                {
                    continue;
                }

                check(FMaterialAttributeDefinitionMap::GetShaderFrequency(Property) == SF_Pixel);
                // Special case MP_SubsurfaceColor as the actual property is a combination of the color and the profile but we don't want to expose the profile
                FString PropertyName = FMaterialAttributeDefinitionMap_GetDisplayName(Property);
                check(PropertyName.Len() > 0);
                PropertyName += TEXT("_mdl");
                const EMaterialValueType Type = FMaterialAttributeDefinitionMap::GetValueType(Property);

                // Normal requires its own separate initializer
                if (Property == MP_Normal)
                {
                    NormalInitializerValue = FString::Printf(TEXT("\t%s %s = %s;\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[Property]);
                }
                else
                {
                    if (TranslatedCodeChunkDefinitions[Property].Len() > 0)
                    {
                        if (LastProperty >= 0)
                        {
                            // Verify that all code chunks have the same contents
                            check(TranslatedCodeChunkDefinitions[Property].Len() == TranslatedCodeChunkDefinitions[LastProperty].Len());
                        }

                        LastProperty = Property;
                    }

                    if (Property == MP_OpacityMask && Material->GetBlendMode() == BLEND_Masked && !Material->WritesEveryPixel())
                    {
                        PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = (%s - %s) < 0.0f ? 0.0f : 1.0f;\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[Property], FL(Material->GetOpacityMaskClipValue()));
                    }
                    else if (Property == MP_Refraction)
                    {
                        // UE4 bug: material attribute will cause no refraction.
                        if (Material->HasMaterialAttributesConnected() || InvalidRefraction)
                        {
                            PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = %s;\r\n"), HLSLTypeString(Type), *PropertyName, FL(DefaultRefraction));
                        }
                        else
                        {
                            PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = math::max(%s, 1.0f);\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[Property]);
                        }
                    }
                    else if (Property == MP_Specular && RayTracingTranslucency)
                    {
                        if (RayTracingRefraction)
                        {
                            if (InvalidRefraction)
                            {
                                // Using default glass IOR instead of the invalid UE4 expressions
                                PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = %s;\r\n"), HLSLTypeString(Type), *PropertyName, FL(DefaultRefraction));
                            }
                            else
                            {
                                // Dielectric specular to IOR
                                PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = 2.0f / (1.0f - math::sqrt(0.08f * math::saturate(%s))) - 1.0f;\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[Property]);
                            }	
                        }
                        else
                        {
                            // Disable refraction
                            PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = 1.0f;\r\n"), HLSLTypeString(Type), *PropertyName);
                        }
                    }
                    else if (Property == MP_EmissiveColor && bSceneColorTranslucent)
                    {
                        PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = float3(0.0f);\r\n"), HLSLTypeString(Type), *PropertyName);
                    }
                    else if (Property == MP_Opacity && bSceneColorTranslucent)
                    {
                        PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = 1.0f - %s.x;\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[MP_EmissiveColor]);
                    }
                    else if (Property == MP_Opacity && bSceneDepthTranslucent)
                    {
                        PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = 0.05f;\r\n"), HLSLTypeString(Type), *PropertyName);
                    }
                    else
                    {
                        PixelInputInitializerValues += FString::Printf(TEXT("\t%s %s = %s;\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[Property]);
                    }

                    if (Property == MP_Opacity)
                    {
                        if (IsTranslucentBlendMode(Material->GetBlendMode()))
                        {
                            if (Material->GetCastDynamicShadowAsMasked())
                            {
                                if (bSceneColorTranslucent)
                                {
                                    PixelInputInitializerValues += FString::Printf(TEXT("\tfloat OpacityMask_mdl = (math::saturate(1.0f - %s.x) - %s) < 0.0f ? 0.0f : 1.0f;\r\n"), *TranslatedCodeChunks[MP_EmissiveColor], FL(Material->GetOpacityMaskClipValue()));
                                }
                                else
                                {
                                    PixelInputInitializerValues += FString::Printf(TEXT("\tfloat OpacityMask_mdl = (math::saturate(%s) - %s) < 0.0f ? 0.0f : 1.0f;\r\n"), *TranslatedCodeChunks[Property], FL(Material->GetOpacityMaskClipValue()));
                                }
                            }
                            else if (Material->GetBlendMode() == BLEND_AlphaComposite
                                || Material->GetBlendMode() == BLEND_AlphaHoldout
                                || Material->GetBlendMode() == BLEND_Translucent)
                            {
                                if (bSceneColorTranslucent)
                                {
                                    PixelInputInitializerValues += FString::Printf(TEXT("\tfloat OpacityMask_mdl = (math::saturate(1.0f - %s.x) - 1.0f / 255.0f) < 0.0f ? 0.0f : 1.0f;\r\n"), *TranslatedCodeChunks[MP_EmissiveColor]);
                                }
                                else
                                {
                                    PixelInputInitializerValues += FString::Printf(TEXT("\tfloat OpacityMask_mdl = (math::saturate(%s) - 1.0f / 255.0f) < 0.0f ? 0.0f : 1.0f;\r\n"), *TranslatedCodeChunks[Property]);
                                }
                            }
                        }
                    }
                }
            }

            NormalAssignment = NormalInitializerValue;
            if (LastProperty != -1)
            {
                PixelMembersInitializationEpilog += TranslatedCodeChunkDefinitions[LastProperty] + LINE_TERMINATOR;
            }

            PixelMembersInitializationEpilog += PixelInputInitializerValues;

            // output vertex properties
            LastProperty = -1;
            const uint32 NumUserTexCoords = GetNumUserTexCoords();
            for (int32 PropertyIndex = 0; PropertyIndex < MP_MAX; ++PropertyIndex)
            {
                // Skip non-vertex properties
                const EMaterialProperty Property = (EMaterialProperty)PropertyIndex;

                if (IgnoredProperties.Find(Property) != INDEX_NONE)
                {
                    continue;
                }

                if (FMaterialAttributeDefinitionMap::GetShaderFrequency(Property) != SF_Vertex)
                {
                    continue;
                }

                if (PropertyIndex >= MP_CustomizedUVs0 && PropertyIndex <= MP_CustomizedUVs7)
                {
                    if ((PropertyIndex - MP_CustomizedUVs0) >= (int32)NumUserTexCoords)
                    {
                        continue;
                    }
                }

                FString PropertyName = FMaterialAttributeDefinitionMap_GetDisplayName(Property);
                check(PropertyName.Len() > 0);
                PropertyName += TEXT("_mdl");
                const EMaterialValueType Type = FMaterialAttributeDefinitionMap::GetValueType(Property);

                if (TranslatedCodeChunkDefinitions[Property].Len() > 0)
                {
                    if (LastProperty >= 0)
                    {
                        // Verify that all code chunks have the same contents
                        check(TranslatedCodeChunkDefinitions[Property].Len() == TranslatedCodeChunkDefinitions[LastProperty].Len());
                    }

                    LastProperty = Property;
                }

                VertexInputInitializerValues += FString::Printf(TEXT("\t%s %s = %s;\r\n"), HLSLTypeString(Type), *PropertyName, *TranslatedCodeChunks[Property]);		
            }

            if (LastProperty != -1)
            {
                VertexMembersInitializationEpilog += TranslatedCodeChunkDefinitions[LastProperty] + LINE_TERMINATOR;
            }

            VertexMembersInitializationEpilog += VertexInputInitializerValues;

            NormalCode = TranslatedCodeChunkDefinitions[MP_Normal];

            ParametersCode = TranslatedCodeParameters;
        }
    }

    void CheckUniqueTextureName(UTexture* InTexture)
    {
        if (CurrentTextureCache->Find(InTexture) == nullptr)
        {
            FString ID = TEXT("");

            auto TextureName = MDLPathUtility::UnmangleAssetPath(InTexture->GetName(), false);
            if (CurrentTextureCache->FindKey(TextureName))
            {
                // name is not unique
                ID = TEXT("_") + FString::FromInt(FMDLExporterUtility::GlobalTextureID++);
            }

            CurrentTextureCache->Add(InTexture, TextureName + ID);
        }
    }

    FString GetErrorMessage(const TCHAR* Text)
    {
        FString ErrorString;
        check(ShaderFrequency < SF_NumFrequencies);
        auto& CurrentFunctionStack = FunctionStacks[ShaderFrequency];
        if (CurrentFunctionStack.Num() > 1)
        {
            // If we are inside a function, add that to the error message.  
            // Only add the function call node to ErrorExpressions, since we can't add a reference to the expressions inside the function as they are private objects.
            // Add the first function node on the stack because that's the one visible in the material being compiled, the rest are all nested functions.
            UMaterialExpressionMaterialFunctionCall* ErrorFunction = CurrentFunctionStack[1]->FunctionCall;
            ErrorString = FString(TEXT("Function ")) + ErrorFunction->MaterialFunction->GetName() + TEXT(": ");
        }

        if (CurrentFunctionStack.Last()->ExpressionStack.Num() > 0)
        {
            UMaterialExpression* ErrorExpression = CurrentFunctionStack.Last()->ExpressionStack.Last().Expression;
            check(ErrorExpression);

            if (ErrorExpression->GetClass() != UMaterialExpressionMaterialFunctionCall::StaticClass()
                && ErrorExpression->GetClass() != UMaterialExpressionFunctionInput::StaticClass()
                && ErrorExpression->GetClass() != UMaterialExpressionFunctionOutput::StaticClass())
            {
                TArray<FString> OutCaptions;
                ErrorExpression->GetCaption(OutCaptions);

                // Add the node type to the error message
                if (OutCaptions.Num() > 0)
                {
                    ErrorString += TEXT("[");
                    for (int32 CapIndex = 0; CapIndex < OutCaptions.Num(); ++CapIndex)
                    {
                        ErrorString += OutCaptions[CapIndex] + (CapIndex == (OutCaptions.Num() - 1) ? TEXT("] ") : TEXT(" "));
                    }
                }
            }
        }
            
        ErrorString += Text;

        return ErrorString;
    }

    virtual int32 Error(const TCHAR* Text) override
    {
        switch (MaterialProperty)
        {
        case MP_Anisotropy: //425
        case MP_Tangent:	//425
        case MP_WorldDisplacement:
        case MP_AmbientOcclusion:
        case MP_PixelDepthOffset:
            return INDEX_NONE;
        }

        FString ErrorString = GetErrorMessage(Text);

        //add the error string to the material's CompileErrors array
        TranslatedErrors.AddUnique(ErrorString);

        bSuccess = false;
        
        return INDEX_NONE;
    }

    void Warning(const TCHAR* Text)
    {
        FString WarningString = GetErrorMessage(Text);

        //add the error string to the material's CompileErrors array
        TranslatedWarnings.AddUnique(WarningString);
    }

    //-------------------------------------------------------------------------------------------------------------
    // Unsupported Node
    //-------------------------------------------------------------------------------------------------------------
    int32 MDLTranslatedError()
    {
        return Error(TEXT("cannot be translated to MDL."));
    }

    void MDLTranslatedWarning()
    {
        Warning(TEXT("cannot be translated to MDL, using a fixed value instead."));
    }

    virtual int32 PerInstanceRandom() override
    {
        if (ShaderFrequency != SF_Pixel && ShaderFrequency != SF_Vertex)
        {
            return NonVertexOrPixelShaderExpressionError();
        }
        else
        {
            bPerInstanceDataUsed = true;
            bPerlinGradientTextureUsed = true;
            RegisterIntParameter(TEXT("NumberInstances"));
            FString PerlinNoiseTexture = FString::Printf(TEXT("texture_2d(\"./%s/PerlinNoiseGradientTexture.png\",tex::gamma_linear)"), *TexturePath);
            return AddInlinedCodeChunk(MCT_Float, TEXT("::per_instance_random(%s, NumberInstances)"), *PerlinNoiseTexture);
        }
    }

    virtual int32 LightVector() override
    {
        return MDLTranslatedError();
    }

    virtual int32 ViewProperty(EMaterialExposedViewProperty Property, bool InvProperty) override
    {
        if (Property == EMaterialExposedViewProperty::MEVP_WorldSpaceCameraPosition)
        {
            MDLTranslatedWarning();
            return AddCodeChunk(MCT_Float3, TEXT("::camera_position()"));
        }
        else if (Property == EMaterialExposedViewProperty::MEVP_ViewSize)
        {
            MDLTranslatedWarning();
            return AddCodeChunk(MCT_Float3, TEXT("float3(1000.0f)"));
        }

        return MDLTranslatedError();
    }

    virtual int32 ParticleMacroUV() override 
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleSubUV(int32 TextureIndex, EMaterialSamplerType SamplerType, bool bBlend) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleColor() override
    {
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float4, TEXT("float4(1)"));
    }

    virtual int32 ParticlePosition() override
    {
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0)"));
    }

    virtual int32 ParticleRadius() override
    {
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float, TEXT("0"));
    }

    virtual int32 SphericalParticleOpacity(int32 Density) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleRelativeTime() override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleMotionBlurFade() override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleRandom() override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleDirection() override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleSpeed() override
    {
        return MDLTranslatedError();
    }

    virtual int32 ParticleSize() override
    {
        return MDLTranslatedError();
    }

    virtual int32 SceneDepth(int32 Offset, int32 ViewportUV, bool bUseOffset) override
    {
        if (ShaderFrequency == SF_Vertex && FeatureLevel <= ERHIFeatureLevel::ES3_1)
        {
            // mobile currently does not support this, we need to read a separate copy of the depth, we must disable framebuffer fetch and force scene texture reads.
            return Errorf(TEXT("Cannot read scene depth from the vertex shader with feature level ES3.1 or below."));
        }

        if (Offset == INDEX_NONE && bUseOffset)
        {
            return INDEX_NONE;
        }

        MDLTranslatedWarning();

        return AddInlinedCodeChunk(MCT_Float, TEXT("::scene_depth()"));
    }

    virtual int32 SceneColor(int32 Offset, int32 ViewportUV, bool bUseOffset) override
    {
        if (Offset == INDEX_NONE && bUseOffset)
        {
            return INDEX_NONE;
        }

        if (ShaderFrequency != SF_Pixel)
        {
            return NonPixelShaderExpressionError();
        }

        if(Material->GetMaterialDomain() != MD_Surface)
        {
            Errorf(TEXT("SceneColor lookups are only available when MaterialDomain = Surface."));
        }

        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        bSceneColorUsed = true;
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::scene_color()"));
    }

    virtual int32 SceneTextureLookup(int32 ViewportUV, uint32 InSceneTextureId, bool bFiltered) override
    {
        ESceneTextureId SceneTextureId = (ESceneTextureId)InSceneTextureId;

        if (SceneTextureId == PPI_SceneDepth)
        {
            return SceneDepth(INDEX_NONE, ViewportUV, false);
        }
        else if (SceneTextureId == PPI_SceneColor)
        {
            return SceneColor(INDEX_NONE, ViewportUV, false);
        }
        return MDLTranslatedError();
    }

    virtual int32 GetSceneTextureViewSize(int32 SceneTextureId, bool InvProperty) override
    {
        return MDLTranslatedError();
    }

    virtual int32 GetViewportUV() override
    {
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float2, TEXT("float2(0)"));
    }

    virtual int32 GetPixelPosition() override
    {
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float2, TEXT("float2(1.0f)"));
    }

    // Media texture
    virtual int32 ExternalTexture(const FGuid& ExternalTextureGuid) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ExternalTexture(UTexture* InTexture, int32& TextureReferenceIndex) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ExternalTextureParameter(FName ParameterName, UTexture* DefaultValue, int32& TextureReferenceIndex) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ExternalTextureCoordinateScaleRotation(int32 TextureReferenceIndex, TOptional<FName> ParameterName) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ExternalTextureCoordinateScaleRotation(const FGuid& ExternalTextureGuid) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ExternalTextureCoordinateOffset(int32 TextureReferenceIndex, TOptional<FName> ParameterName) override
    {
        return MDLTranslatedError();
    }

    virtual int32 ExternalTextureCoordinateOffset(const FGuid& ExternalTextureGuid) override
    {
        return MDLTranslatedError();
    }

    virtual int32 DDX( int32 X ) override
    {
        if ((Platform != SP_OPENGL_ES2_WEBGL_REMOVED) && // WebGL 2/GLES3.0 - DDX() function is available
            ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (ShaderFrequency == SF_Compute)
        {
            // running a material in a compute shader pass (e.g. when using SVOGI)
            return AddInlinedCodeChunk(MCT_Float, TEXT("0"));	
        }

        if (ShaderFrequency != SF_Pixel)
        {
            return NonPixelShaderExpressionError();
        }

        MDLTranslatedWarning();
        if (GetParameterType(X) == MCT_Float2)
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("float2(0.0001)"));
        }
        else if (GetParameterType(X) == MCT_Float3)
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("float3(0.0001)"));
        }
        else if (GetParameterType(X) == MCT_Float4)
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("float4(0.0001)"));
        }
        else
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("0.0001"));
        }
    }

    virtual int32 DDY( int32 X ) override
    {
        if ((Platform != SP_OPENGL_ES2_WEBGL_REMOVED) && // WebGL 2/GLES3.0 - DDY() function is available
            ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if(X == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (ShaderFrequency == SF_Compute)
        {
            // running a material in a compute shader pass
            return AddInlinedCodeChunk(MCT_Float, TEXT("0"));	
        }
        if (ShaderFrequency != SF_Pixel)
        {
            return NonPixelShaderExpressionError();
        }

        MDLTranslatedWarning();

        if (GetParameterType(X) == MCT_Float2)
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("float2(0.0001)"));
        }
        else if (GetParameterType(X) == MCT_Float3)
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("float3(0.0001)"));
        }
        else if (GetParameterType(X) == MCT_Float4)
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("float4(0.0001)"));
        }
        else
        {
            return AddInlinedCodeChunk(GetParameterType(X), TEXT("0.0001"));
        }
    }

    virtual int32 Noise(int32 Position, float Scale, int32 Quality, uint8 NoiseFunction, bool bTurbulence, int32 Levels, float OutputMin, float OutputMax, float LevelScale, int32 FilterWidth, bool bTiling, uint32 RepeatSize) override
    {
        // GradientTex3D uses 3D texturing, which is not available on ES2
        if (NoiseFunction == NOISEFUNCTION_GradientTex3D)
        {
            if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
            {
                Errorf(TEXT("3D textures are not supported for ES2"));
                return INDEX_NONE;
            }
            // Replace with GradientTex
            NoiseFunction = NOISEFUNCTION_GradientTex;
        }
        // all others are fine for ES2 feature level
        else if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES2_REMOVED) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (Position == INDEX_NONE || FilterWidth == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (NoiseFunction == NOISEFUNCTION_SimplexTex ||
            NoiseFunction == NOISEFUNCTION_GradientTex ||
            NoiseFunction == NOISEFUNCTION_GradientTex3D)
        {
            AddEstimatedTextureSample();
        }

        // to limit performance problems due to values outside reasonable range
        Levels = FMath::Clamp(Levels, 1, 10);

        int32 ScaleConst = Constant(Scale);
        int32 QualityConst = Constant(Quality);
        int32 NoiseFunctionConst = Constant(NoiseFunction);
        int32 TurbulenceConst = Constant(bTurbulence);
        int32 LevelsConst = Constant(Levels);
        int32 OutputMinConst = Constant(OutputMin);
        int32 OutputMaxConst = Constant(OutputMax);
        int32 LevelScaleConst = Constant(LevelScale);
        int32 TilingConst = Constant(bTiling);
        int32 RepeatSizeConst = Constant(RepeatSize);

        if (NoiseFunction == NOISEFUNCTION_SimplexTex ||
            NoiseFunction == NOISEFUNCTION_GradientTex)
        {
            FString PerlinNoiseTexture = FString::Printf(TEXT("texture_2d(\"./%s/PerlinNoiseGradientTexture.png\",tex::gamma_linear)"), *TexturePath);
            bPerlinGradientTextureUsed = true;
            return AddCodeChunk(MCT_Float,
                TEXT("::noise(%s, texture_3d(), %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)"),
                *PerlinNoiseTexture,
                *GetParameterCode(Position),
                *GetParameterCode(ScaleConst),
                *GetParameterCode(QualityConst),
                *GetParameterCode(NoiseFunctionConst),
                *GetParameterCode(TurbulenceConst),
                *GetParameterCode(LevelsConst),
                *GetParameterCode(OutputMinConst),
                *GetParameterCode(OutputMaxConst),
                *GetParameterCode(LevelScaleConst),
                *GetParameterCode(FilterWidth),
                *GetParameterCode(TilingConst),
                *GetParameterCode(RepeatSizeConst));
        }
        else if (NoiseFunction == NOISEFUNCTION_GradientTex3D)
        {
            // 
            // TODO: 3D texture format for Kit?
            //
            bPerlin3DTextureUsed = true;
            return AddCodeChunk(MCT_Float,
                TEXT("::noise(texture_2d(), texture_3d(), %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)"),
                *GetParameterCode(Position),
                *GetParameterCode(ScaleConst),
                *GetParameterCode(QualityConst),
                *GetParameterCode(NoiseFunctionConst),
                *GetParameterCode(TurbulenceConst),
                *GetParameterCode(LevelsConst),
                *GetParameterCode(OutputMinConst),
                *GetParameterCode(OutputMaxConst),
                *GetParameterCode(LevelScaleConst),
                *GetParameterCode(FilterWidth),
                *GetParameterCode(TilingConst),
                *GetParameterCode(RepeatSizeConst));
        }
        else
        {
            return AddCodeChunk(MCT_Float,
                TEXT("::noise(texture_2d(), texture_3d(), %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)"),
                *GetParameterCode(Position),
                *GetParameterCode(ScaleConst),
                *GetParameterCode(QualityConst),
                *GetParameterCode(NoiseFunctionConst),
                *GetParameterCode(TurbulenceConst),
                *GetParameterCode(LevelsConst),
                *GetParameterCode(OutputMinConst),
                *GetParameterCode(OutputMaxConst),
                *GetParameterCode(LevelScaleConst),
                *GetParameterCode(FilterWidth),
                *GetParameterCode(TilingConst),
                *GetParameterCode(RepeatSizeConst));
        }
    }

    virtual int32 VectorNoise(int32 Position, int32 Quality, uint8 NoiseFunction, bool bTiling, uint32 TileSize) override
    {
        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES2_REMOVED) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (Position == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        int32 QualityConst = Constant(Quality);
        int32 NoiseFunctionConst = Constant(NoiseFunction);
        int32 TilingConst = Constant(bTiling);
        int32 TileSizeConst = Constant(TileSize);

        if (NoiseFunction == VNF_GradientALU || NoiseFunction == VNF_VoronoiALU)
        {
            return AddCodeChunk(MCT_Float4,
                TEXT("::vector4_noise(%s,%s,%s,%s,%s)"),
                *GetParameterCode(Position),
                *GetParameterCode(QualityConst),
                *GetParameterCode(NoiseFunctionConst),
                *GetParameterCode(TilingConst),
                *GetParameterCode(TileSizeConst));
        }
        else
        {
            return AddCodeChunk(MCT_Float3,
                TEXT("::vector3_noise(%s,%s,%s,%s,%s)"),
                *GetParameterCode(Position),
                *GetParameterCode(QualityConst),
                *GetParameterCode(NoiseFunctionConst),
                *GetParameterCode(TilingConst),
                *GetParameterCode(TileSizeConst));
        }
    }

    virtual int32 BlackBody(int32 Temp) override
    {
        if (Temp == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        return AddCodeChunk(MCT_Float3, TEXT("::black_body(%s)"), *GetParameterCode(Temp));
    }

    virtual int32 GetHairColorFromMelanin(int32 Melanin, int32 Redness, int32 DyeColor) override
    {
        if (Melanin == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (Redness == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (DyeColor == INDEX_NONE)
        {
            return INDEX_NONE;
        }
        return AddCodeChunk(MCT_Float3, TEXT("::get_hair_color_from_melanin(%s, %s, %s)"), *GetParameterCode(Melanin), *GetParameterCode(Redness), *GetParameterCode(DyeColor));
    }

    virtual int32 ObjectWorldPosition() override
    {
        MDLTranslatedWarning();
        // TOFIX:
        // Using pixel world position instead.
        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::object_world_position(%s)"), *UpZ);
    }

    virtual int32 ObjectRadius() override
    {
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float, TEXT("::object_radius()"));	
    }

    virtual int32 ObjectBounds() override
    {
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::object_bounds()"));	
    }

    virtual int32 ObjectOrientation() override
    {
        MDLTranslatedWarning();
        return AddInlinedCodeChunk(MCT_Float3, TEXT("::object_orientation()"));
    }

    virtual int32 SpeedTree(int32 GeometryArg, int32 WindArg, int32 LODArg, float BillboardThreshold, bool bAccurateWindVelocities, bool bExtraBend, int32 ExtraBendArg) override
    { 
        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::ES3_1) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (Material && Material->IsUsedWithSkeletalMesh())
        {
            return Error(TEXT("SpeedTree node not currently supported for Skeletal Meshes, please disable usage flag."));
        }

        if (ShaderFrequency != SF_Vertex)
        {
            return NonVertexShaderExpressionError();
        }
        else
        {
            bUsesSpeedTree = true;

            AllocateSlot(AllocatedUserVertexTexCoords, 2, 6);

            // Only generate previous frame's computations if required and opted-in
            const bool bEnablePreviousFrameInformation = bCompilingPreviousFrame && bAccurateWindVelocities;
            MDLTranslatedWarning();
            return AddCodeChunk(MCT_Float3, TEXT("float3(0)"));
        }
    }

    virtual int32 ActorWorldPosition() override
    {
        // NOTE: use world position instead
        if (IsRefractionProperty())
        {
            Warning(TEXT("IOR in mdl can't work with ActorWorldPosition, replaced with default."));
            InvalidRefraction = true;
        }

        FString UpZ = UpZAxis ? TEXT("true") : TEXT("false");
        if (bCompilingPreviousFrame && ShaderFrequency == SF_Vertex)
        {
            // Decal VS doesn't have material code so FMaterialVertexParameters
            // and primitve uniform buffer are guaranteed to exist if ActorPosition
            // material node is used in VS
            return AddInlinedCodeChunk(
                MCT_Float3,
                TEXT("(::convert_to_left_hand(state::transform_point(state::coordinate_internal,state::coordinate_world,state::position()), %s)*state::meters_per_scene_unit()*100.0)"), *UpZ);
        }
        else
        {
            return AddInlinedCodeChunk(MCT_Float3, TEXT("(::convert_to_left_hand(state::transform_point(state::coordinate_internal,state::coordinate_world,state::position()), %s)*state::meters_per_scene_unit()*100.0)"), *UpZ);
        }
    }

    virtual int32 AtmosphericLightVector() override
    {
        bUsesAtmosphericFog = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0,0,-1)"));

    }

    virtual int32 AtmosphericLightColor() override
    {
        bUsesAtmosphericFog = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(1)"));

    }

    virtual int32 DistanceToNearestSurface(int32 PositionArg) override
    {
        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (PositionArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        MDLTranslatedWarning();
        // should be 0 or negative
        return AddCodeChunk(MCT_Float, TEXT("0.0"));
    }

    virtual int32 DistanceFieldGradient(int32 PositionArg) override
    {
        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        if (PositionArg == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("0.0"));
    }

    FString RegisterBoolParameter(FName ParameterName, bool bInitialValue)
    {
        FString Name = FMDLExporterUtility::GetLegalIdentifier(ParameterName.ToString());

        FString* DefinedName = DefinedBoolParameters.Find(ParameterName);
        if (DefinedName == nullptr)
        {
            if (IsReservedParameter(*Name))
            {
                Name = TEXT("Local") + Name;
            }

            while (DefinedParameterNames.Find(Name) != INDEX_NONE)
            {
                // Same name was found, should be changed
                Name += TEXT("_bp");
            }
            FString Annotation = DefineAnnotation<UMaterialExpressionParameter>(NAME_None, true);
            TranslatedCodeParameters.Add(FString::Printf(TEXT("%s\tuniform %s %s = %s%s"),
                TranslatedCodeParameters.Num() == 0 ? TEXT("") : TEXT(",\r\n"),
                HLSLTypeString(MCT_StaticBool), *Name, bInitialValue ? TEXT("true") : TEXT("false"),
                Annotation.IsEmpty() ? TEXT("") : *Annotation));
            DefinedBoolParameters.Add(ParameterName, Name);
            DefinedParameterNames.Add(Name);
        }
        else
        {
            Name = *DefinedName;
        }
        
        return Name;
    }

    virtual int32 GIReplace(int32 Direct, int32 StaticIndirect, int32 DynamicIndirect) override 
    { 
        if(Direct == INDEX_NONE || DynamicIndirect == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FString Name = RegisterBoolParameter(TEXT("GIReplaceState"), true);
        EMaterialValueType ResultType = GetArithmeticResultType(Direct, DynamicIndirect);
        return AddCodeChunk(ResultType,TEXT("(%s ? (%s) : (%s))"), *Name, *GetParameterCode(DynamicIndirect), *GetParameterCode(Direct));
    }

    virtual int32 ShadowReplace(int32 Default, int32 Shadow) override
    {
        if (Default == INDEX_NONE || Shadow == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FString Name = RegisterBoolParameter(TEXT("ShadowReplaceState"), false);
        EMaterialValueType ResultType = GetArithmeticResultType(Default, Shadow);
        return AddCodeChunk(ResultType, TEXT("(%s ? (%s) : (%s))"), *Name, *GetParameterCode(Shadow), *GetParameterCode(Default));
    }

    virtual int32 RayTracingQualitySwitchReplace(int32 Normal, int32 RayTraced) override
    {
        if (Normal == INDEX_NONE || RayTraced == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FString Name = RegisterBoolParameter(TEXT("RayTracingQualitySwitch"), false);
        EMaterialValueType ResultType = GetArithmeticResultType(Normal, RayTraced);
        return AddCodeChunk(ResultType, TEXT("(%s ? (%s) : (%s))"), *Name, *GetParameterCode(RayTraced), *GetParameterCode(Normal));
    }

    virtual int32 AtmosphericFogColor(int32 WorldPosition) override
    {
        if (ErrorUnlessFeatureLevelSupported(ERHIFeatureLevel::SM5) == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        bUsesAtmosphericFog = true;

        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float4, TEXT("float4(0)"));
    }

    virtual int32 SkyAtmosphereLightIlluminance(int32 WorldPosition, int32 LightIndex) override
    {
        bUsesSkyAtmosphere = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0)"));
    }

    virtual int32 SkyAtmosphereLightDirection(int32 LightIndex) override
    {
        bUsesSkyAtmosphere = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0,0,-1)"));
    }

    virtual int32 SkyAtmosphereLightDiskLuminance(int32 LightIndex) override
    {
        bUsesSkyAtmosphere = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0)"));
    }

    virtual int32 SkyAtmosphereViewLuminance() override
    {
        bUsesSkyAtmosphere = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0)"));
    }

    virtual int32 SkyAtmosphereAerialPerspective(int32 WorldPosition) override
    {
        bUsesSkyAtmosphere = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float4, TEXT("float4(0)"));
    }

    virtual int32 SkyAtmosphereDistantLightScatteredLuminance() override
    {
        bUsesSkyAtmosphere = true;
        MDLTranslatedWarning();
        return AddCodeChunk(MCT_Float3, TEXT("float3(0)"));
    }

    virtual int32 VirtualTextureOutputReplace(int32 Default, int32 VirtualTexture) override
    {
        if (Default == INDEX_NONE || VirtualTexture == INDEX_NONE)
        {
            return INDEX_NONE;
        }

        FString Name = RegisterBoolParameter(TEXT("RuntimeVirtualTextureOutputSwitch"), false);
        EMaterialValueType ResultType = GetArithmeticResultType(Default, VirtualTexture);
        return AddCodeChunk(ResultType, TEXT("(%s ? (%s) : (%s))"), *Name, *GetParameterCode(VirtualTexture), *GetParameterCode(Default));
    }

    bool IsRefractionProperty() const
    {
        return (!RayTracingTranslucency && (MaterialProperty == MP_Refraction)) || (RayTracingTranslucency && (MaterialProperty == MP_Specular));
    }

public:
    TArray<FString>				TranslatedCodeParameters;
    UMaterial*					MaterialAsset;
    UMaterialInstance*			MaterialInstance;
    TMap<FName, FString>		DefinedScalarParameters;
    TMap<FName, FString>		DefinedVectorParameters;
    TMap<FName, FString>		DefinedTextureParameters;
    TMap<FName, FString>		DefinedBoolParameters;
    TArray<FString>				DefinedParameterNames;
    FString						TexturePath;
    TArray<FString>				TranslatedErrors;
    TArray<FString>				TranslatedWarnings;
    TArray<int32>				NormalSampleToPack;
    TMap<UTexture*, FString>*	CurrentTextureCache;
    TMap<FString, FString>		IORParameters;
    TArray<FString>				LocalVariables;

    bool						bPerlinGradientTextureUsed;
    bool						bPerlin3DTextureUsed;
    bool						bVertexColorUsed;
    bool						bPerInstanceDataUsed;
    bool						bTwoSidedSignUsed;
    bool						bExtension17Used;
    bool						bAOMaterialMaskUsed;
    bool						bLightMapCoordinateUsed;
    bool						bMaxTexCoordinateUsed;
    bool						bSceneColorUsed;
    bool						bSceneDepthOpacity;

    const TArray<FMaterialCustomExpressionEntry>& GetCustomExpressions() { return CustomExpressions; }
    const TArray<FString>& GetCustomVSOutputImplementations() { return CustomVSOutputImplementations; }
    const TArray<FString>& GetCustomPSOutputImplementations() { return CustomPSOutputImplementations; }

private:
    TMap<UTexture*, FString>	LocalUsedTextures;
    TMap<int32, FLinearColor>	ConstantChunks;
    TMap<int32, FLinearColor>	ConstantExpressions;
    TMap<FString, int32>		CustomExpressionCache; // Hash
    TArray<FString>				CustomVSOutputImplementations;
    TArray<FString>				CustomPSOutputImplementations;
    bool						LandscapeMaterial;
    bool						DDSTexture;
    bool						InvalidRefraction;
    bool						RayTracingTranslucency;
    bool						RayTracingRefraction;
    bool						UpZAxis;
    bool						ReplaceTextureCube;
};

#undef FL
