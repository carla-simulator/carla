// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if WITH_MDL_SDK

#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionArccosine.h"
#include "Materials/MaterialExpressionArcsine.h"
#include "Materials/MaterialExpressionArctangent.h"
#include "Materials/MaterialExpressionArctangent2.h"
#include "Materials/MaterialExpressionBlackBody.h"
#include "Materials/MaterialExpressionBlendMaterialAttributes.h"
#include "Materials/MaterialExpressionBreakMaterialAttributes.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionCameraPositionWS.h"
#include "Materials/MaterialExpressionCeil.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionClearCoatNormalCustomOutput.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionCosine.h"
#include "Materials/MaterialExpressionCrossProduct.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionDistance.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionFeatureLevelSwitch.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionFmod.h"
#include "Materials/MaterialExpressionFrac.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionLogarithm2.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMin.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionObjectOrientation.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionReflectionVectorWS.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSetMaterialAttributes.h"
#include "Materials/MaterialExpressionStep.h"
#include "Materials/MaterialExpressionSine.h"
#include "Materials/MaterialExpressionSphereMask.h"
#include "Materials/MaterialExpressionSquareRoot.h"
#include "Materials/MaterialExpressionStaticBool.h"
#include "Materials/MaterialExpressionStaticBoolParameter.h"
#include "Materials/MaterialExpressionStaticSwitch.h"
#include "Materials/MaterialExpressionStaticSwitchParameter.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionTangent.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionTextureProperty.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionTransform.h"
#include "Materials/MaterialExpressionTransformPosition.h"
#include "Materials/MaterialExpressionTwoSidedSign.h"
#include "Materials/MaterialExpressionVectorNoise.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionPixelDepth.h"
#include "Materials/MaterialFunction.h"
#include "Materials/Material.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/VolumeTexture.h"
#include "Engine/TextureCube.h"

enum EConnectionType
{
    Expression,
    Boolean,
    Float,
    Float2,
    Float3,
    Float4,
    Texture,
    TextureSelection
};

enum EMaterialAttributesIndex
{
    MAI_BaseColor = 0,
    MAI_Metallic,
    MAI_Specular,
    MAI_Roughness,
    MAI_Anisotropy, // 4.25
    MAI_EmissiveColor,
    MAI_Opacity,
    MAI_OpacityMask,
    MAI_Normal,
    MAI_Tangent,		// 4.25
    MAI_WorldPositionOffset,
    MAI_WorldDisplacement,
    MAI_TessellationMultiplier,
    MAI_SubsurfaceColor,
    MAI_CustomData0,
    MAI_CustomData1,
    MAI_AmbientOcclusion,
    MAI_Refraction,
    MAI_CustomizedUVs0,
    MAI_CustomizedUVs1,
    MAI_CustomizedUVs2,
    MAI_CustomizedUVs3,
    MAI_CustomizedUVs4,
    MAI_CustomizedUVs5,
    MAI_CustomizedUVs6,
    MAI_CustomizedUVs7,
    MAI_PixelDepthOffset,
    MAI_ShadingModel,
};

struct FExpressionData
{
    FExpressionData(UMaterialExpression* IN_Expression, int32 IN_Index, bool IN_IsDefault)
        : Expression(IN_Expression)
        , Index(IN_Index)
        , IsDefault(IN_IsDefault)
    {}

    bool operator!=(const FExpressionData& rhs)
    {
        return (Expression != rhs.Expression) || (Index != rhs.Index) || (IsDefault != rhs.IsDefault);
    }

    UMaterialExpression*	Expression;
    int32					Index;
    bool					IsDefault;
};

struct FMaterialExpressionConnection
{
    FMaterialExpressionConnection()
        : ConnectionType(EConnectionType::Expression)
        , ExpressionData(nullptr, 0, true)
    {}

    FMaterialExpressionConnection(UMaterialExpression* Expression, int32 OutputIndex = 0, bool IsDefault = false)
        : ConnectionType(EConnectionType::Expression)
        , ExpressionData(Expression, OutputIndex, IsDefault)
    {}

    FMaterialExpressionConnection(bool IN_Boolean)
        //: ConnectionType(EConnectionType::Boolean)
        //, Boolean(IN_Boolean)
        : ConnectionType(EConnectionType::Float)
        , Values{ IN_Boolean ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f }
    {}

    FMaterialExpressionConnection(float Value)
        : ConnectionType(EConnectionType::Float)
        , Values{ Value, 0.0f, 0.0f, 0.0f }
    {}

    FMaterialExpressionConnection(float Value0, float Value1)
        : ConnectionType(EConnectionType::Float2)
        , Values{ Value0, Value1, 0.0f, 0.0f }
    {}

    FMaterialExpressionConnection(float Value0, float Value1, float Value2)
        : ConnectionType(EConnectionType::Float3)
        , Values{ Value0, Value1, Value2, 0.0f }
    {}

    FMaterialExpressionConnection(float Value0, float Value1, float Value2, float Value3)
        : ConnectionType(EConnectionType::Float4)
        , Values{ Value0, Value1, Value2, Value3 }
    {}

    FMaterialExpressionConnection(UTexture* IN_Texture)
        : ConnectionType(EConnectionType::Texture)
        , Texture(IN_Texture)
    {}

    FMaterialExpressionConnection(int Value)
        : ConnectionType(EConnectionType::Float)
        , Values{ static_cast<float>(Value), 0.0f, 0.0f, 0.0f }
    {}

    FMaterialExpressionConnection(double Value)
        : ConnectionType(EConnectionType::Float)
        , Values{ static_cast<float>(Value), 0.0f, 0.0f, 0.0f }
    {}

    FMaterialExpressionConnection(FExpressionData const& Value, FExpressionData const& True, FExpressionData const& False)
        : ConnectionType(EConnectionType::TextureSelection)
    {
        TextureSelectionData[0] = Value;
        TextureSelectionData[1] = True;
        TextureSelectionData[2] = False;
    }

    bool operator!=(const FMaterialExpressionConnection& rhs)
    {
        return (ConnectionType != rhs.ConnectionType)
            || ((ConnectionType == EConnectionType::Expression) && (ExpressionData != rhs.ExpressionData))
            || ((ConnectionType == EConnectionType::Boolean) && (Boolean != rhs.Boolean))
            || ((ConnectionType == EConnectionType::Float) && (Values[0] != rhs.Values[0]))
            || ((ConnectionType == EConnectionType::Float2) && ((Values[0] != rhs.Values[0]) || (Values[1] != rhs.Values[1])))
            || ((ConnectionType == EConnectionType::Float3) && ((Values[0] != rhs.Values[0]) || (Values[1] != rhs.Values[1]) || (Values[2] != rhs.Values[2])))
            || ((ConnectionType == EConnectionType::Float4) && ((Values[0] != rhs.Values[0]) || (Values[1] != rhs.Values[1]) || (Values[2] != rhs.Values[2]) || (Values[3] != rhs.Values[3])))
            || ((ConnectionType == EConnectionType::Texture) && (Texture != rhs.Texture))
            || ((ConnectionType == EConnectionType::TextureSelection) && ((TextureSelectionData[0] != rhs.TextureSelectionData[0]) || (TextureSelectionData[1] != rhs.TextureSelectionData[1]) || (TextureSelectionData[2] != rhs.TextureSelectionData[2])));
    }

    EConnectionType		ConnectionType;
    union
    {
        FExpressionData	ExpressionData;
        bool			Boolean;
        float			Values[4];
        UTexture*		Texture;
        FExpressionData	TextureSelectionData[3];
    };
};

template <typename T>
inline T* NewMaterialExpression(UObject* Parent)
{
    check(Parent != nullptr);

    auto Expression = NewObject<T>(Parent);
    Expression->MaterialExpressionGuid = FGuid::NewGuid();
    Expression->bCollapsed = true;

    if (Parent->IsA<UMaterial>())
    {
        Cast<UMaterial>(Parent)->Expressions.Push(Expression);
    }
    if (Parent->IsA<UMaterialFunction>())
    {
        Cast<UMaterialFunction>(Parent)->FunctionExpressions.Push(Expression);
    }

    return Expression;
}

MDL_API uint32 ComponentCount(FMaterialExpressionConnection const& Input);
MDL_API bool EvaluateBool(FMaterialExpressionConnection const& Connection);
MDL_API float EvaluateFloat(FMaterialExpressionConnection const& Connection);
MDL_API bool IsBool(FMaterialExpressionConnection const& Input);
MDL_API bool IsMaterialAttribute(FMaterialExpressionConnection const& Input);
MDL_API bool IsScalar(FMaterialExpressionConnection const& Input);
MDL_API bool IsStatic(FMaterialExpressionConnection const& Input);
MDL_API bool IsTexture(FMaterialExpressionConnection const& Input);
MDL_API bool IsType(EFunctionInputType InputType, FMaterialExpressionConnection const& Input);
MDL_API bool IsVector2(FMaterialExpressionConnection const& Input);
MDL_API bool IsVector3(FMaterialExpressionConnection const& Input);
MDL_API bool IsVector4(FMaterialExpressionConnection const& Input);

// Abs
MDL_API UMaterialExpressionAbs* NewMaterialExpressionAbs(UObject* Parent, const FMaterialExpressionConnection& Input);
// Add
MDL_API UMaterialExpressionAdd* NewMaterialExpressionAdd(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
MDL_API UMaterialExpressionAdd* NewMaterialExpressionAdd(UObject* Parent, const TArray<FMaterialExpressionConnection>& Arguments);
// AppendVector
MDL_API UMaterialExpressionAppendVector* NewMaterialExpressionAppendVector(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
MDL_API UMaterialExpressionAppendVector* NewMaterialExpressionAppendVector(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& C);
// Arccosine
MDL_API UMaterialExpressionArccosine* NewMaterialExpressionArccosine(UObject* Parent, const FMaterialExpressionConnection& Input);
// Arcsine
MDL_API UMaterialExpressionArcsine* NewMaterialExpressionArcsine(UObject* Parent, const FMaterialExpressionConnection& Input);
// Arctangent
MDL_API UMaterialExpressionArctangent* NewMaterialExpressionArctangent(UObject* Parent, const FMaterialExpressionConnection& Input);
// Arctangent2
MDL_API UMaterialExpressionArctangent2* NewMaterialExpressionArctangent2(UObject* Parent, const FMaterialExpressionConnection& Y, const FMaterialExpressionConnection& X);
// BlackBody
MDL_API UMaterialExpressionBlackBody* NewMaterialExpressionBlackBody(UObject* Parent, const FMaterialExpressionConnection& Temperature);
// BreakMaterialAttributes
MDL_API UMaterialExpressionBreakMaterialAttributes* NewMaterialExpressionBreakMaterialAttributes(UObject* Parent, const FMaterialExpressionConnection& MaterialAttributes);
// CameraVectorWS
MDL_API UMaterialExpressionCameraVectorWS* NewMaterialExpressionCameraVectorWS(UObject* Parent);
// Ceil
MDL_API UMaterialExpressionCeil* NewMaterialExpressionCeil(UObject* Parent, const FMaterialExpressionConnection& Input);
// Clamp
MDL_API UMaterialExpressionClamp* NewMaterialExpressionClamp(UObject* Parent, const FMaterialExpressionConnection& Input, const FMaterialExpressionConnection& Min, const FMaterialExpressionConnection& Max);
// ComponentMask
MDL_API UMaterialExpressionComponentMask* NewMaterialExpressionComponentMask(UObject* Parent, const FMaterialExpressionConnection& Input, uint32 Mask);
// ClearCoatNormalCustomOutput
MDL_API UMaterialExpressionClearCoatNormalCustomOutput* NewMaterialExpressionClearCoatNormalCustomOutput(UObject* Parent, const FMaterialExpressionConnection& Input);
// Constant
MDL_API UMaterialExpressionConstant* NewMaterialExpressionConstant(UObject* Parent, float X);
// ConstantVector2
MDL_API UMaterialExpressionConstant2Vector* NewMaterialExpressionConstant(UObject* Parent, float X, float Y);
// ConstantVector3
MDL_API UMaterialExpressionConstant3Vector* NewMaterialExpressionConstant(UObject* Parent, float X, float Y, float Z);
// ConstantVector4
MDL_API UMaterialExpressionConstant4Vector* NewMaterialExpressionConstant(UObject* Parent, float X, float Y, float Z, float W);
// Cosine
MDL_API UMaterialExpressionCosine* NewMaterialExpressionCosine(UObject* Parent, const FMaterialExpressionConnection& Input);
// CrossProduct
MDL_API UMaterialExpressionCrossProduct* NewMaterialExpressionCrossProduct(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
// Distance
MDL_API UMaterialExpressionDistance* NewMaterialExpressionDistance(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
// Divide
MDL_API UMaterialExpressionDivide* NewMaterialExpressionDivide(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
// DotProduct
MDL_API UMaterialExpressionDotProduct* NewMaterialExpressionDotProduct(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
// Floor
MDL_API UMaterialExpressionFloor* NewMaterialExpressionFloor(UObject* Parent, const FMaterialExpressionConnection& Input);
// Fmod
MDL_API UMaterialExpressionFmod* NewMaterialExpressionFmod(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
// Frac
MDL_API UMaterialExpressionFrac* NewMaterialExpressionFrac(UObject* Parent, const FMaterialExpressionConnection& Input);
// Fresnel
MDL_API UMaterialExpressionFresnel* NewMaterialExpressionFresnel(UObject* Parent, const FMaterialExpressionConnection& Exponent = {}, const FMaterialExpressionConnection& BaseReflectFraction = {}, const FMaterialExpressionConnection& Normal = {});
// FunctionInput
MDL_API UMaterialExpressionFunctionInput* NewMaterialExpressionFunctionInput(UObject* Parent, const FName& Name, EFunctionInputType Type);
MDL_API UMaterialExpressionFunctionInput* NewMaterialExpressionFunctionInput(UObject* Parent, const FName& Name, EFunctionInputType Type, const FMaterialExpressionConnection& DefaultExpression);
// FunctionOutput
MDL_API UMaterialExpressionFunctionOutput* NewMaterialExpressionFunctionOutput(UObject* Parent, const FName& Name, const FMaterialExpressionConnection& Output);
// If
MDL_API UMaterialExpressionIf* NewMaterialExpressionIf(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& Less, const FMaterialExpressionConnection& Equal, const FMaterialExpressionConnection& Greater, bool bCheckSkip = false);
MDL_API UMaterialExpressionIf* NewMaterialExpressionIfEqual(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& Bx, const FMaterialExpressionConnection& Yes, const FMaterialExpressionConnection& No, bool bCheckSkip = false);
MDL_API UMaterialExpressionIf* NewMaterialExpressionIfGreater(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& Yes, const FMaterialExpressionConnection& No, bool bCheckSkip = false);
MDL_API UMaterialExpressionIf* NewMaterialExpressionIfLess(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& Yes, const FMaterialExpressionConnection& No, bool bCheckSkip = false);
MDL_API UMaterialExpressionIf* NewMaterialExpressionSwitch(UObject* Parent, const FMaterialExpressionConnection& Switch, const TArray<FMaterialExpressionConnection>& Cases, bool bCheckSkip = false);
// LinearInterpolate
MDL_API UMaterialExpressionLinearInterpolate* NewMaterialExpressionLinearInterpolate(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& Alpha);
MDL_API UMaterialExpression* NewMaterialExpressionLinearInterpolate(UObject* Function, const TArray<FMaterialExpressionConnection>& Values, const FMaterialExpressionConnection& Alpha);
// Logarithm2
MDL_API UMaterialExpressionLogarithm2* NewMaterialExpressionLogarithm2(UObject* Parent, const FMaterialExpressionConnection& X);
// MaterialFunctionCall
MDL_API UMaterialExpressionMaterialFunctionCall* NewMaterialExpressionFunctionCall(UObject* Parent, UMaterialFunction* Function, const TArray<FMaterialExpressionConnection>& Inputs);
// MakeMaterialAttributes
// Note: force ClearCoat to 0.0f per default !!
MDL_API UMaterialExpressionMakeMaterialAttributes* NewMaterialExpressionMakeMaterialAttributes(UObject* Parent,
    const FMaterialExpressionConnection& BaseColor = {}, const FMaterialExpressionConnection& Metallic = {}, const FMaterialExpressionConnection& Specular = {},
    const FMaterialExpressionConnection& Roughness = {}, const FMaterialExpressionConnection& EmissiveColor = {}, const FMaterialExpressionConnection& Opacity = {},
    const FMaterialExpressionConnection& OpacityMask = {}, const FMaterialExpressionConnection& Normal = {}, const FMaterialExpressionConnection& WorldPositionOffset = {},
    const FMaterialExpressionConnection& WorldDisplacement = {}, const FMaterialExpressionConnection& TessellationMultiplier = {}, const FMaterialExpressionConnection& SubsurfaceColor = {},
    const FMaterialExpressionConnection& ClearCoat = 0.0f, const FMaterialExpressionConnection& ClearCoatRoughness = {}, const FMaterialExpressionConnection& AmbientOcclusion = {},
    const FMaterialExpressionConnection& Refraction = {}, const FMaterialExpressionConnection& CustomizedUVs0 = {}, const FMaterialExpressionConnection& CustomizedUVs1 = {},
    const FMaterialExpressionConnection& CustomizedUVs2 = {}, const FMaterialExpressionConnection& CustomizedUVs3 = {}, const FMaterialExpressionConnection& CustomizedUVs4 = {},
    const FMaterialExpressionConnection& CustomizedUVs5 = {}, const FMaterialExpressionConnection& CustomizedUVs6 = {}, const FMaterialExpressionConnection& CustomizedUVs7 = {},
    const FMaterialExpressionConnection& PixelDepthOffset = {});
// Max
MDL_API UMaterialExpressionMax* NewMaterialExpressionMax(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
MDL_API UMaterialExpressionMax* NewMaterialExpressionMax(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& C);
MDL_API UMaterialExpressionMax* NewMaterialExpressionMax(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& C, const FMaterialExpressionConnection& D);
// Min
MDL_API UMaterialExpressionMin* NewMaterialExpressionMin(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
MDL_API UMaterialExpressionMin* NewMaterialExpressionMin(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& C);
MDL_API UMaterialExpressionMin* NewMaterialExpressionMin(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& C, const FMaterialExpressionConnection& D);
// Multiply
MDL_API UMaterialExpressionMultiply* NewMaterialExpressionMultiply(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
MDL_API UMaterialExpressionMultiply* NewMaterialExpressionMultiply(UObject* Parent, const TArray<FMaterialExpressionConnection>& Arguments);
MDL_API UMaterialExpressionMultiply* NewMaterialExpressionSquare(UObject* Parent, const FMaterialExpressionConnection& A);
// Negate
MDL_API UMaterialExpressionSubtract* NewMaterialExpressionNegate(UObject* Parent, const FMaterialExpressionConnection& Input);
// Noise
MDL_API UMaterialExpressionNoise* NewMaterialExpressionNoise(UObject* Parent, const FMaterialExpressionConnection& Position, const FMaterialExpressionConnection& FilterWidth, int32 Quality);
// Normalize
MDL_API UMaterialExpressionNormalize* NewMaterialExpressionNormalize(UObject* Parent, const FMaterialExpressionConnection& Input);
// OneMinus
MDL_API UMaterialExpressionOneMinus* NewMaterialExpressionOneMinus(UObject* Parent, const FMaterialExpressionConnection& Input);
// Power
MDL_API UMaterialExpressionPower* NewMaterialExpressionPower(UObject* Parent, const FMaterialExpressionConnection& Base, const FMaterialExpressionConnection& Exponent);
// ReflectionVectorWS
MDL_API UMaterialExpressionReflectionVectorWS* NewMaterialExpressionReflectionVectorWS(UObject* Parent);
MDL_API UMaterialExpressionReflectionVectorWS* NewMaterialExpressionReflectionVectorWS(UObject* Parent, const FMaterialExpressionConnection& CustomWorldNormal);
// Saturate
MDL_API UMaterialExpressionSaturate* NewMaterialExpressionSaturate(UObject* Parent, const FMaterialExpressionConnection& Input);
// ScalarParameter
MDL_API UMaterialExpressionScalarParameter* NewMaterialExpressionScalarParameter(UObject* Parent, const FString& Name, float DefaultValue);
// Sine
MDL_API UMaterialExpressionSine* NewMaterialExpressionSine(UObject* Parent, const FMaterialExpressionConnection& Input);
// Sign
// NOTE: UE4 Material Expression Sign can't be used directly caused by MinimalAPI
MDL_API UMaterialExpressionCustom* NewMaterialExpressionSign(UObject* Parent, const FMaterialExpressionConnection& Input);
// SquareRoot
MDL_API UMaterialExpressionSquareRoot* NewMaterialExpressionSquareRoot(UObject* Parent, const FMaterialExpressionConnection& Input);
#if 0
// StaticBool
MDL_API UMaterialExpressionStaticBool* NewMaterialExpressionStaticBool(UObject* Parent, bool Value);
// StaticBoolParameter
MDL_API UMaterialExpressionStaticBoolParameter* NewMaterialExpressionStaticBoolParameter(UObject* Parent, const FString& Name, bool DefaultValue, const FString& Group = "");
// StaticSwitch
MDL_API UMaterialExpressionStaticSwitch* NewMaterialExpressionStaticSwitch(UObject* Parent, const FMaterialExpressionConnection& Value, const FMaterialExpressionConnection& True, const FMaterialExpressionConnection& False);
#else
// StaticBool
MDL_API UMaterialExpressionConstant* NewMaterialExpressionStaticBool(UObject* Parent, bool Value);
// StaticBoolParameter
MDL_API UMaterialExpressionScalarParameter* NewMaterialExpressionStaticBoolParameter(UObject* Parent, const FString& Name, bool DefaultValue, const FString& Group = "");
// StaticSwitch
MDL_API UMaterialExpressionIf* NewMaterialExpressionStaticSwitch(UObject* Parent, const FMaterialExpressionConnection& Value, const FMaterialExpressionConnection& True, const FMaterialExpressionConnection& False);
#endif
// Subtract
MDL_API UMaterialExpressionSubtract* NewMaterialExpressionSubtract(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B);
MDL_API UMaterialExpressionSubtract* NewMaterialExpressionSubtract(UObject* Parent, const FMaterialExpressionConnection& A, const FMaterialExpressionConnection& B, const FMaterialExpressionConnection& C);
// Tangent
MDL_API UMaterialExpressionTangent* NewMaterialExpressionTangent(UObject* Parent, const FMaterialExpressionConnection& Input);
// TextureCoordinate
MDL_API UMaterialExpressionTextureCoordinate* NewMaterialExpressionTextureCoordinate(UObject* Parent, int32 CoordinateIndex);
// TextureObject
MDL_API UMaterialExpressionTextureObject* NewMaterialExpressionTextureObject(UObject* Parent, UTexture* Texture);
// TextureObjectParameter
MDL_API UMaterialExpressionTextureObjectParameter* NewMaterialExpressionTextureObjectParameter(UObject* Parent, const FString& Name, UTexture* Texture);
// TextureProperty
MDL_API UMaterialExpressionTextureProperty* NewMaterialExpressionTextureProperty(UObject* Parent, const FMaterialExpressionConnection& TextureObject, EMaterialExposedTextureProperty Property);
// TextureSample
MDL_API UMaterialExpressionTextureSample* NewMaterialExpressionTextureSample(UObject* Parent, const FMaterialExpressionConnection& TextureObject, const FMaterialExpressionConnection& Coordinates, ESamplerSourceMode SamplerSource = SSM_FromTextureAsset);
// Transform
MDL_API UMaterialExpressionTransform* NewMaterialExpressionTransform(UObject* Parent, const FMaterialExpressionConnection& Input, EMaterialVectorCoordTransformSource SourceType, EMaterialVectorCoordTransform DestinationType);
// TransformPosition
MDL_API UMaterialExpressionTransformPosition* NewMaterialExpressionTransformPosition(UObject* Parent, const FMaterialExpressionConnection& Input, EMaterialPositionTransformSource SourceType, EMaterialPositionTransformSource DestinationType);
// TwoSidedSign
MDL_API UMaterialExpressionTwoSidedSign* NewMaterialExpressionTwoSidedSign(UObject* Parent);
// VectorNoise
MDL_API UMaterialExpressionVectorNoise* NewMaterialExpressionVectorNoise(UObject* Parent, const FMaterialExpressionConnection& Position, EVectorNoiseFunction NoiseFunction, int32 Quality);
// VectorParameter
MDL_API UMaterialExpressionVectorParameter* NewMaterialExpressionVectorParameter(UObject* Parent, const FString& Name, const FLinearColor& DefaultValue);
// Smoothstep
MDL_API UMaterialExpressionCustom* NewMaterialExpressionSmoothStep(UObject* Parent, const FMaterialExpressionConnection& Min, const FMaterialExpressionConnection& Max, const FMaterialExpressionConnection& Input);
// NormalMapping
MDL_API UMaterialExpressionCustom* NewMaterialExpressionNormalMapping(UObject* Parent, const FMaterialExpressionConnection& Normal, const FMaterialExpressionConnection& TangentBitangentMapping);

MDL_API UMaterialExpressionObjectOrientation* NewMaterialExpressionObjectOrientation(UObject* Parent);

// NOTE: Object Bounds/PositionWS/Radius header file doesn't have minimalAPI or ENGINE_API, have to use custom instead
MDL_API UMaterialExpressionCustom* NewMaterialExpressionObjectBounds(UObject* Parent);
MDL_API UMaterialExpressionCustom* NewMaterialExpressionObjectPositionWS(UObject* Parent);
MDL_API UMaterialExpressionCustom* NewMaterialExpressionObjectRadius(UObject* Parent);
MDL_API UMaterialExpressionCustom* NewMaterialExpressionLocalObjectBoundsMin(UObject* Parent);
MDL_API UMaterialExpressionCustom* NewMaterialExpressionLocalObjectBoundsMax(UObject* Parent);

MDL_API UMaterialExpressionCustom* NewMaterialExpressionTexelLookup(UObject* Parent, const FMaterialExpressionConnection& Texture, const FMaterialExpressionConnection& Location);
MDL_API UMaterialExpressionCustom* NewMaterialExpressionNormalTexelLookup(UObject* Parent, const FMaterialExpressionConnection& Texture, const FMaterialExpressionConnection& Location);

#endif