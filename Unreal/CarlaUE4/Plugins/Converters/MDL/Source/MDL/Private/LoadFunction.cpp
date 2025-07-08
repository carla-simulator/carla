// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#if WITH_MDL_SDK

#include "MDLDependencies.h"
#include "AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Factories/MaterialFunctionFactoryNew.h"
#include "MaterialExpressions.h"
#include "MaterialGraph/MaterialGraph.h"
#include "MaterialGraph/MaterialGraphSchema.h"
#include "MDLImporterUtility.h"
#include "NodeArrangement.h"
#include "Engine/Texture2D.h"
#include "MDLSettings.h"
#include "MaterialEditingLibrary.h"

enum color_layer_mode
{
    color_layer_blend,
    color_layer_add,
    color_layer_multiply,
    color_layer_screen,
    color_layer_overlay,
    color_layer_brightness,
    color_layer_color,
    color_layer_exclusion,
    color_layer_average,
    color_layer_lighten,
    color_layer_darken,
    color_layer_sub,
    color_layer_negation,
    color_layer_difference,
    color_layer_softlight,
    color_layer_colordodge,
    color_layer_reflect,
    color_layer_colorburn,
    color_layer_phoenix,
    color_layer_hardlight,
    color_layer_pinlight,
    color_layer_hardmix,
    color_layer_lineardodge,
    color_layer_linearburn,
    color_layer_spotlight,
    color_layer_spotlightblend,
    color_layer_hue,
    color_layer_saturation
};

enum coordinate_space
{
    coordinate_internal,
    coordinate_object,
    coordinate_world,
    coordinate_tangent, // UE4 only, internal using
};

enum mono_mode
{
    mono_alpha,
    mono_average,
    mono_luminance,
    mono_maximum
};

enum scatter_mode
{
    scatter_reflect,
    scatter_transmit,
    scatter_reflect_transmit
};

enum texture_coordinate_system
{
    texture_coordinate_uvw,
    texture_coordinate_world,
    texture_coordinate_object
};

enum wrap_mode
{
    wrap_clamp,
    wrap_repeat,
    wrap_mirrored_repeat,
    wrap_clip
};

FName& operator +=(FName& Name, const FString& Str)
{
    Name = *(Name.ToString() + Str);
    return Name;
}

UMaterialFunction* LoadFunction(const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes = TArray<int32>());

static void Generator_BaseAbbeNumberIOR(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Calculate spectral index of refraction.");

    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Scalar, 1.5f);
    UMaterialExpressionFunctionInput* AbbeNumber = NewMaterialExpressionFunctionInput(Function, TEXT("abbe_number"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Result = NewMaterialExpressionFunctionCall(Function, MakeFloat3, { IOR, IOR, IOR });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseAnisotropyConversion(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Convert old anisotropy controls into new ones.");

    UMaterialExpressionFunctionInput* Roughness = NewMaterialExpressionFunctionInput(Function, TEXT("roughness"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* Anisotropy = NewMaterialExpressionFunctionInput(Function, TEXT("anisotropy"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnisotropyRotation = NewMaterialExpressionFunctionInput(Function, TEXT("anisotropy_rotation"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* MiaAnisotropySemantic = NewMaterialExpressionFunctionInput(Function, TEXT("mia_anisotropy_semantic"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));

    const float MinimalIso = 0.0001f;
    UMaterialExpressionMax* IsoGreater = NewMaterialExpressionMax(Function, NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionDivide(Function, 1.0f, Anisotropy)), MinimalIso);
    UMaterialExpressionMax* IsoLess = NewMaterialExpressionMax(Function, NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionSquareRoot(Function, Anisotropy)), MinimalIso);

    UMaterialExpressionIf* AnisotropicRoughnessU =
        NewMaterialExpressionIfGreater(Function, Anisotropy, 1.0f,
            NewMaterialExpressionDivide(Function, Roughness, IsoGreater),
            NewMaterialExpressionMultiply(Function, Roughness, IsoLess));
    UMaterialExpressionIf* RoughnessU = NewMaterialExpressionStaticSwitch(Function, MiaAnisotropySemantic, AnisotropicRoughnessU, Roughness);

    UMaterialExpressionIf* AnisotropicRoughnessV =
        NewMaterialExpressionIfGreater(Function, Anisotropy, 1.0f,
            NewMaterialExpressionMultiply(Function, Roughness, IsoGreater),
            NewMaterialExpressionDivide(Function, Roughness, IsoLess));
    UMaterialExpressionIf* RoughnessV =
        NewMaterialExpressionStaticSwitch(Function, MiaAnisotropySemantic,
            AnisotropicRoughnessV,
            NewMaterialExpressionDivide(Function,
                Roughness,
                NewMaterialExpressionIfEqual(Function, Anisotropy, 1.0f, 0.0001f, NewMaterialExpressionOneMinus(Function, Anisotropy))));

    UMaterialExpressionMultiply* RotationAngle = NewMaterialExpressionMultiply(Function, 2.0f * PI, AnisotropyRotation);
    // As UE4 is left-handed, swap StateNormal and TangentU in this cross product to get the TangentV !
    UMaterialExpressionCrossProduct* TangentV = NewMaterialExpressionCrossProduct(Function, TangentU, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionSubtract* RotatedTangentU =
        NewMaterialExpressionSubtract(Function,
            NewMaterialExpressionMultiply(Function, TangentU, NewMaterialExpressionCosine(Function, RotationAngle)),
            NewMaterialExpressionMultiply(Function, TangentV, NewMaterialExpressionSine(Function, RotationAngle)));
    UMaterialExpressionIf* TangentUOut = NewMaterialExpressionIfEqual(Function, AnisotropyRotation, 0.0f, TangentU, RotatedTangentU);

    NewMaterialExpressionFunctionOutput(Function, TEXT("roughness_u"), RoughnessU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("roughness_v"), RoughnessV);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TangentUOut);
}

static void Generator_BaseArchitecturalGlossToRough(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Glossiness according to mia material semantic.");

    UMaterialExpressionFunctionInput* Glossiness = NewMaterialExpressionFunctionInput(Function, TEXT("glossiness"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionIf* Result = NewMaterialExpressionIfLess(Function, Glossiness, 1.0f,
        NewMaterialExpressionSquareRoot(Function,
            NewMaterialExpressionMultiply(Function,
                2.0f,
                NewMaterialExpressionPower(Function, 2.0f, NewMaterialExpressionSubtract(Function, -4.0f, NewMaterialExpressionMultiply(Function, 14.0f, Glossiness))))),
        0.0f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseBlendColorLayers(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterBlendColors = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_blend_colors"));
    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));

    Function->Description = TEXT("Texture layering functionality similar to the functionality known from painting programs.");

    TArray<UMaterialExpressionFunctionInput*> LayerColors, Weights, Modes;
    int32 ArraySize = ArrayInputSizes[0];
    for (int32 i = 0; i < ArraySize; i++)
    {
        LayerColors.Add(NewMaterialExpressionFunctionInput(Function, TEXT("layer_color"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f }));
        Weights.Add(NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar, 1.0f));
        Modes.Add(NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, color_layer_blend));
    }
    if (1 < ArraySize)
    {
        for (int32 i = 0; i < ArraySize; i++)
        {
            FString Appendix = TEXT("_") + FString::FromInt(i);
            LayerColors[i]->InputName += Appendix;
            Weights[i]->InputName += Appendix;
            Modes[i]->InputName += Appendix;
        }
    }

    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* MonoSource = NewMaterialExpressionFunctionInput(Function, TEXT("mono_source"), EFunctionInputType::FunctionInput_Scalar, mono_average);

    TArray<UMaterialExpressionComponentMask*> Tints;
    Tints.Add(NewMaterialExpressionComponentMask(Function, NewMaterialExpressionFunctionCall(Function, ImporterBlendColors, { LayerColors[0], Base, Weights[0], Modes[0] }), 7));
    for (int32 i = 1; i < ArraySize; i++)
    {
        Tints.Add(NewMaterialExpressionComponentMask(Function, NewMaterialExpressionFunctionCall(Function, ImporterBlendColors, { LayerColors[i], Tints[i-1], Weights[i], Modes[i] }), 7));
    }

    UMaterialExpressionMaterialFunctionCall* Average = NewMaterialExpressionFunctionCall(Function, MathAverage, { Tints.Last() });
    UMaterialExpressionMaterialFunctionCall* Luminance = NewMaterialExpressionFunctionCall(Function, MathLuminance, { Tints.Last() });
    UMaterialExpressionMaterialFunctionCall* Maximum = NewMaterialExpressionFunctionCall(Function, MathMaxValue, { Tints.Last() });
    UMaterialExpressionIf* Mono = NewMaterialExpressionSwitch(Function, MonoSource, { Maximum, Average, Luminance, Maximum });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tints.Last());
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_BaseCheckerBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterEvalChecker = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_eval_checker"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* CheckerPosition = NewMaterialExpressionFunctionInput(Function, TEXT("checker_position"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    float Delta = 0.025f;		// magic, looks good with this value

    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, ImporterEvalChecker, { UVWPosition, CheckerPosition, Blur });
    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, ImporterEvalChecker,
    {
        NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionConstant(Function, Delta, 0.0f, 0.0f)),
        CheckerPosition,
        Blur
    });
    UMaterialExpressionMaterialFunctionCall* Result2 = NewMaterialExpressionFunctionCall(Function, ImporterEvalChecker,
    {
        NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionConstant(Function, 0.0f, Delta, 0.0f)),
        CheckerPosition,
        Blur
    });
    UMaterialExpressionMaterialFunctionCall* Result3 = NewMaterialExpressionFunctionCall(Function, ImporterEvalChecker,
    {
        NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionConstant(Function, 0.0f, 0.0f, Delta)),
        CheckerPosition,
        Blur
    });

    UMaterialExpressionNormalize* CalculatedNormal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionMultiply(Function,
                    Normal,
                    NewMaterialExpressionAdd(Function,
                        NewMaterialExpressionMultiply(Function,
                            NewMaterialExpressionAbs(Function, NewMaterialExpressionSubtract(Function, Result3, Result0)),
                            Factor),
                        1.0f)),
                NewMaterialExpressionMultiply(Function, { UVWTangentU, NewMaterialExpressionSubtract(Function, Result1, Result0), Factor}),
                NewMaterialExpressionMultiply(Function, { UVWTangentV, NewMaterialExpressionSubtract(Function, Result2, Result0) })
            }));
    UMaterialExpressionIf* Result = NewMaterialExpressionIfEqual(Function, Factor, 0.0f, Normal, CalculatedNormal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseCheckerTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterEvalChecker = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_eval_checker"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("3D color checker pattern.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* CheckerPosition = NewMaterialExpressionFunctionInput(Function, TEXT("checker_position"), EFunctionInputType::FunctionInput_Scalar, 0.5f);

    UMaterialExpressionMaterialFunctionCall* Alpha = NewMaterialExpressionFunctionCall(Function, ImporterEvalChecker, { UVWPosition, CheckerPosition, Blur });
    UMaterialExpressionLinearInterpolate* Tint = NewMaterialExpressionLinearInterpolate(Function, Color2, Color1, Alpha);
    UMaterialExpressionMaterialFunctionCall* Mono = NewMaterialExpressionFunctionCall(Function, MathLuminance, { Tint });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_BaseCoordinateProjection(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterComputeCubicTransform = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_compute_cubic_transform"));
    UMaterialFunction* ImporterComputeCylindricTransform = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_compute_cylindric_transform"));
    UMaterialFunction* ImporterComputeSphericTransform = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_compute_spheric_transform"));
    UMaterialFunction* ImporterComputeTangentsTransformed = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_compute_tangents_transformed"));
    UMaterialFunction* MathMultiplyFloat4x4Float4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StatePosition = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_position"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTransformPoint = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_transform_point"));
    UMaterialFunction* StateTransformVector = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_transform_vector"));

    enum projection_mode
    {
        projection_invalid,
        projection_cubic,
        projection_spherical,
        projection_cylindrical,
        projection_infinite_cylindrical,
        projection_planar,
        projection_spherical_normalized,
        projection_cylindrical_normalized,
        projection_infinite_cylindrical_normalized,
        projection_tri_planar
    };

    UMaterialExpressionFunctionInput* CoordinateSystem = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate_system"), EFunctionInputType::FunctionInput_Scalar, texture_coordinate_object);
    UMaterialExpressionFunctionInput* TextureSpace = NewMaterialExpressionFunctionInput(Function, TEXT("texture_space"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ProjectionType = NewMaterialExpressionFunctionInput(Function, TEXT("projection_type"), EFunctionInputType::FunctionInput_Scalar, projection_planar);
    UMaterialExpressionFunctionInput* ProjectionTransform0 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_0"), EFunctionInputType::FunctionInput_Vector4, { 1.0f, 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* ProjectionTransform1 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_1"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 1.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* ProjectionTransform2 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_2"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 1.0f, 0.0f });
    UMaterialExpressionFunctionInput* ProjectionTransform3 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_3"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 1.0f });

    UMaterialExpressionMaterialFunctionCall* StatePositionCall = NewMaterialExpressionFunctionCall(Function, StatePosition, {});
    UMaterialExpressionMaterialFunctionCall* StateNormalCall = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialExpressionIf* IntPos = NewMaterialExpressionSwitch(Function, CoordinateSystem,
    {
        NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate,{ TextureSpace }),
        NewMaterialExpressionFunctionCall(Function, StateTransformPoint,{ coordinate_internal, coordinate_world, StatePositionCall }),
        NewMaterialExpressionFunctionCall(Function, StateTransformPoint,{ coordinate_internal, coordinate_object, StatePositionCall })
    });

    UMaterialExpressionIf* CoordinateSpace = NewMaterialExpressionIfEqual(Function, CoordinateSystem, texture_coordinate_world, coordinate_world, coordinate_object);
    UMaterialExpressionMaterialFunctionCall* Normal = NewMaterialExpressionFunctionCall(Function, StateTransformVector, { coordinate_tangent, CoordinateSpace, StateNormalCall });		// would need to transform a normal here! but I'm not aware of such a method in UE4!

    UMaterialExpressionMaterialFunctionCall* Cubic = NewMaterialExpressionFunctionCall(Function, ImporterComputeCubicTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, Normal, IntPos });
    UMaterialExpressionMaterialFunctionCall* Spherical = NewMaterialExpressionFunctionCall(Function, ImporterComputeSphericTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, IntPos, false });
    UMaterialExpressionMaterialFunctionCall* Cylindrical = NewMaterialExpressionFunctionCall(Function, ImporterComputeCylindricTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, Normal, IntPos, false, false });
    UMaterialExpressionMaterialFunctionCall* InfiniteCylindrical = NewMaterialExpressionFunctionCall(Function, ImporterComputeCylindricTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, Normal, IntPos, true, false });
    UMaterialExpressionMaterialFunctionCall* SphericalNormalized = NewMaterialExpressionFunctionCall(Function, ImporterComputeSphericTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, IntPos, true });
    UMaterialExpressionMaterialFunctionCall* CylindricalNormalized = NewMaterialExpressionFunctionCall(Function, ImporterComputeCylindricTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, Normal, IntPos, false, true });
    UMaterialExpressionMaterialFunctionCall* InfiniteCylindricalNormalized = NewMaterialExpressionFunctionCall(Function, ImporterComputeCylindricTransform, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, Normal, IntPos, true, true });
    UMaterialExpression* TriPlanar = Cubic;

    UMaterialExpressionIf* FinalTransform0 = NewMaterialExpressionSwitch(Function, ProjectionType, { {},{ Cubic, 0 },{ Spherical, 0 },{ Cylindrical, 0 },{ InfiniteCylindrical, 0 },ProjectionTransform0,{ SphericalNormalized, 0 },{ CylindricalNormalized, 0 },{ InfiniteCylindricalNormalized, 0 },{ TriPlanar, 0 } });
    UMaterialExpressionIf* FinalTransform1 = NewMaterialExpressionSwitch(Function, ProjectionType, { {},{ Cubic, 1 },{ Spherical, 1 },{ Cylindrical, 1 },{ InfiniteCylindrical, 1 },ProjectionTransform1,{ SphericalNormalized, 1 },{ CylindricalNormalized, 1 },{ InfiniteCylindricalNormalized, 1 },{ TriPlanar, 1 } });
    UMaterialExpressionIf* FinalTransform2 = NewMaterialExpressionSwitch(Function, ProjectionType, { {},{ Cubic, 2 },{ Spherical, 2 },{ Cylindrical, 2 },{ InfiniteCylindrical, 2 },ProjectionTransform2,{ SphericalNormalized, 2 },{ CylindricalNormalized, 2 },{ InfiniteCylindricalNormalized, 2 },{ TriPlanar, 2 } });
    UMaterialExpressionIf* FinalTransform3 = NewMaterialExpressionSwitch(Function, ProjectionType, { {},{ Cubic, 3 },{ Spherical, 3 },{ Cylindrical, 3 },{ InfiniteCylindrical, 3 },ProjectionTransform3,{ SphericalNormalized, 3 },{ CylindricalNormalized, 3 },{ InfiniteCylindricalNormalized, 3 },{ TriPlanar, 3 } });

    UMaterialExpressionMaterialFunctionCall* RPos =
        NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4,
        { FinalTransform0, FinalTransform1, FinalTransform2, FinalTransform3, NewMaterialExpressionAppendVector(Function, IntPos, 1.0f) });
    UMaterialExpressionAppendVector* Position = NewMaterialExpressionAppendVector(Function, NewMaterialExpressionComponentMask(Function, RPos, 3), 0.0f);

    UMaterialExpressionMaterialFunctionCall* Tangents = NewMaterialExpressionFunctionCall(Function, ImporterComputeTangentsTransformed,
    {
        CoordinateSystem,
        NewMaterialExpressionComponentMask(Function, FinalTransform0, 7),
        NewMaterialExpressionComponentMask(Function, FinalTransform1, 7),
        NewMaterialExpressionComponentMask(Function, FinalTransform2, 7)
    });

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    //UMaterialExpressionDivide* TextureCoordinateWorld = NewMaterialExpressionDivide(Function, StatePositionCall, 100.0f);
#endif

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), { Tangents, 1 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), { Tangents, 2 });
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xy"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 3));
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xz"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 5));
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_yz"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 6));
#endif
}

static void Generator_BaseCoordinateSource(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterComputeTangents = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_compute_tangents"));
    UMaterialFunction* StatePosition = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_position"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* StateTransformPoint = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_transform_point"));

    Function->Description = TEXT("Access to world coordinates, object coordinates or specifically defined texture spaces.");

    UMaterialExpressionFunctionInput* CoordinateSystem = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate_system"), EFunctionInputType::FunctionInput_Scalar, texture_coordinate_uvw);
    UMaterialExpressionFunctionInput* TextureSpace = NewMaterialExpressionFunctionInput(Function, TEXT("texture_space"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* StatePositionCall = NewMaterialExpressionFunctionCall(Function, StatePosition, {});
    //UMaterialExpressionDivide* TextureCoordinateWorld = NewMaterialExpressionDivide(Function, StatePositionCall, 100.0f);
    UMaterialExpressionIf* Position = NewMaterialExpressionIf(Function, CoordinateSystem, texture_coordinate_world,
        NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { TextureSpace }),
        StatePositionCall,//TextureCoordinateWorld,
        NewMaterialExpressionFunctionCall(Function, StateTransformPoint, { coordinate_internal, coordinate_object, StatePositionCall }));

    UMaterialExpressionMaterialFunctionCall* TangentsWorld = NewMaterialExpressionFunctionCall(Function, ImporterComputeTangents, { coordinate_world });
    UMaterialExpressionMaterialFunctionCall* TangentsObject = NewMaterialExpressionFunctionCall(Function, ImporterComputeTangents, { coordinate_object });

    UMaterialExpressionIf* TangentU = NewMaterialExpressionSwitch(Function, CoordinateSystem,
    {
        NewMaterialExpressionFunctionCall(Function, StateTextureTangentU,{ TextureSpace }),
        { TangentsWorld, 1 },
        { TangentsObject, 1 }
    });
    UMaterialExpressionIf* TangentV = NewMaterialExpressionSwitch(Function, CoordinateSystem,
    {
        NewMaterialExpressionFunctionCall(Function, StateTextureTangentV,{ TextureSpace }),
        { TangentsWorld, 2 },
        { TangentsObject, 2 }
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TangentU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), TangentV);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xy"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 3));
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xz"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 5));
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_yz"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 6));
#endif
}

static void Generator_BaseFileBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterMonoMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_mono_mode"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialFunction* WorldAlignedTextureFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_world_aligned_texture_float4"));
#endif

    Function->Description = TEXT("Computes a normal based on a heightfield-style bump texture.");

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* BumpSource = NewMaterialExpressionFunctionInput(Function, TEXT("bump_source"), EFunctionInputType::FunctionInput_Scalar, mono_average);
    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionFunctionInput* Clip = NewMaterialExpressionFunctionInput(Function, TEXT("clip"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* Spline = NewMaterialExpressionFunctionInput(Function, TEXT("b_spline"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* AnimationStartTime = NewMaterialExpressionFunctionInput(Function, TEXT("animation_start_time"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnimationCrop = NewMaterialExpressionFunctionInput(Function, TEXT("animation_crop"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* AnimationWrap = NewMaterialExpressionFunctionInput(Function, TEXT("animation_wrap"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* AnimationFPS = NewMaterialExpressionFunctionInput(Function, TEXT("animation_fps"), EFunctionInputType::FunctionInput_Scalar, 30.0f);

#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionComponentMask* UVWPositionX = NewMaterialExpressionComponentMask(Function, UVWPosition, 1);
    UMaterialExpressionComponentMask* UVWPositionY = NewMaterialExpressionComponentMask(Function, UVWPosition, 2);

#if 0
    // I think, this is exactly what base.mdl says... but it doesn't work -> created my own version below, using just central differences, ignoring cropping
    UMaterialFunction* ImporterInterpolateTexspace = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_interpolate_texspace"));
    UMaterialFunction* ImporterTexremapu2 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texremapu2"));

    UMaterialExpressionComponentMask* CropUX = NewMaterialExpressionComponentMask(Function, CropU, 1);
    UMaterialExpressionComponentMask* CropUY = NewMaterialExpressionComponentMask(Function, CropU, 2);
    UMaterialExpressionComponentMask* CropVX = NewMaterialExpressionComponentMask(Function, CropV, 1);
    UMaterialExpressionComponentMask* CropVY = NewMaterialExpressionComponentMask(Function, CropV, 2);

    UMaterialExpressionTextureProperty* TextureSize = NewMaterialExpressionTextureProperty(Function, Texture, TMTM_TextureSize);
    UMaterialExpressionMultiply* CropOffset = NewMaterialExpressionMultiply(Function, TextureSize, NewMaterialExpressionFunctionCall(Function, MakeFloat2, { CropUX, CropVX }));
    UMaterialExpressionMultiply* CropTexres =
        NewMaterialExpressionMultiply(Function,
            TextureSize,
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionSubtract(Function, CropUY, CropUX),
                NewMaterialExpressionSubtract(Function, CropVY, CropVX)
            }));
    UMaterialExpressionMultiply* Tex2 = NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), CropTexres);
    UMaterialExpressionMaterialFunctionCall* Texi0 = NewMaterialExpressionFunctionCall(Function, ImporterTexremapu2, { CropTexres, CropOffset, NewMaterialExpressionSubtract(Function, Tex2,{ 1.0f, 1.0f }), WrapU, WrapV });
    UMaterialExpressionMaterialFunctionCall* Texi1 = NewMaterialExpressionFunctionCall(Function, ImporterTexremapu2, { CropTexres, CropOffset, Tex2, WrapU, WrapV });
    UMaterialExpressionMaterialFunctionCall* Texi2 = NewMaterialExpressionFunctionCall(Function, ImporterTexremapu2, { CropTexres, CropOffset, NewMaterialExpressionAdd(Function, Tex2,{ 1.0f, 1.0f }), WrapU, WrapV });
    UMaterialExpressionMaterialFunctionCall* Texi3 = NewMaterialExpressionFunctionCall(Function, ImporterTexremapu2, { CropTexres, CropOffset, NewMaterialExpressionAdd(Function, Tex2,{ 2.0f, 2.0f }), WrapU, WrapV });
    UMaterialExpressionComponentMask* Texi0X = NewMaterialExpressionComponentMask(Function, Texi0, 1);
    UMaterialExpressionComponentMask* Texi0Y = NewMaterialExpressionComponentMask(Function, Texi0, 2);
    UMaterialExpressionComponentMask* Texi1X = NewMaterialExpressionComponentMask(Function, Texi1, 1);
    UMaterialExpressionComponentMask* Texi1Y = NewMaterialExpressionComponentMask(Function, Texi1, 2);
    UMaterialExpressionComponentMask* Texi2X = NewMaterialExpressionComponentMask(Function, Texi2, 1);
    UMaterialExpressionComponentMask* Texi2Y = NewMaterialExpressionComponentMask(Function, Texi2, 2);
    UMaterialExpressionComponentMask* Texi3X = NewMaterialExpressionComponentMask(Function, Texi3, 1);
    UMaterialExpressionComponentMask* Texi3Y = NewMaterialExpressionComponentMask(Function, Texi3, 2);
    UMaterialExpressionSubtract* L = NewMaterialExpressionSubtract(Function, Tex2, NewMaterialExpressionFloor(Function, Tex2));
    UMaterialExpressionMultiply* Lerp =
        NewMaterialExpressionMultiply(Function,
        {
            L,
            L,
            L,
            NewMaterialExpressionAdd(Function,
                NewMaterialExpressionMultiply(Function,
                    L,
                    NewMaterialExpressionSubtract(Function,
                        NewMaterialExpressionMultiply(Function, L, 6.0f),
                        15.f)),
                10.0f)
        });
    UMaterialExpressionComponentMask* LerpX = NewMaterialExpressionComponentMask(Function, Lerp, 1);
    UMaterialExpressionComponentMask* LerpY = NewMaterialExpressionComponentMask(Function, Lerp, 2);
    UMaterialExpressionMaterialFunctionCall* ST =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionMultiply(Function, LerpX, LerpY),
            NewMaterialExpressionMultiply(Function, LerpX, NewMaterialExpressionOneMinus(Function, LerpY)),
            NewMaterialExpressionMultiply(Function, NewMaterialExpressionOneMinus(Function, LerpX), NewMaterialExpressionOneMinus(Function, LerpY)),
            NewMaterialExpressionMultiply(Function, NewMaterialExpressionOneMinus(Function, LerpX), LerpY)
        });
    UMaterialExpressionMaterialFunctionCall* BumpTexspace =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionSubtract(Function,
                    NewMaterialExpressionFunctionCall(Function,
                        ImporterInterpolateTexspace,{ Texture, ST, NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ Texi0X, Texi1Y, Texi1X, Texi2Y }), BumpSource }),
                    NewMaterialExpressionFunctionCall(Function,
                        ImporterInterpolateTexspace,{ Texture, ST, NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ Texi2X, Texi1Y, Texi3X, Texi2Y }), BumpSource })),
                Factor),
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionSubtract(Function,
                    NewMaterialExpressionFunctionCall(Function,
                        ImporterInterpolateTexspace,{ Texture, ST, NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ Texi1X, Texi0Y, Texi2X, Texi1Y }), BumpSource }),
                    NewMaterialExpressionFunctionCall(Function,
                        ImporterInterpolateTexspace,{ Texture, ST, NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ Texi1X, Texi2Y, Texi2X, Texi3Y }), BumpSource })),
                Factor)
        });

    UMaterialExpressionNormalize* UnclippedResult =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                Normal,
                NewMaterialExpressionMultiply(Function, UVWTangentU, NewMaterialExpressionComponentMask(Function, BumpTexspace, 1)),
                NewMaterialExpressionMultiply(Function, UVWTangentV, NewMaterialExpressionComponentMask(Function, BumpTexspace, 2))
            }));

#else

    UMaterialExpressionTextureProperty* TextureScale = NewMaterialExpressionTextureProperty(Function, Texture, TMTM_TexelSize);
    UMaterialExpressionComponentMask* OneOverWidth = NewMaterialExpressionComponentMask(Function, TextureScale, 1);
    UMaterialExpressionComponentMask* OneOverHeight = NewMaterialExpressionComponentMask(Function, TextureScale, 2);

    UMaterialExpressionComponentMask* Center = NewMaterialExpressionComponentMask(Function, UVWPosition, 3);

    UMaterialExpressionMaterialFunctionCall* OffsetU = NewMaterialExpressionFunctionCall(Function, MakeFloat2, { OneOverWidth, 0.0f });
    UMaterialExpressionMaterialFunctionCall* TexCenterMinusU = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionSubtract(Function, Center, OffsetU), WrapU, WrapV, CropU, CropV, 0 });
    UMaterialExpressionMaterialFunctionCall* TexCenterPlusU = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionAdd(Function, Center, OffsetU), WrapU, WrapV, CropU, CropV, 0 });

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionComponentMask* OffsetTangentU = NewMaterialExpressionComponentMask(Function, NewMaterialExpressionMultiply(Function, OneOverWidth, UVWTangentU), 3);
    UMaterialExpressionMaterialFunctionCall* TexCenterMinusTangentU =
        NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat4,
        {
            Texture,
            NewMaterialExpressionSubtract(Function, WorldAlignedPositionXY, OffsetTangentU),
            NewMaterialExpressionSubtract(Function, WorldAlignedPositionXZ, OffsetTangentU),
            NewMaterialExpressionSubtract(Function, WorldAlignedPositionYZ, OffsetTangentU)
        });
    UMaterialExpressionMaterialFunctionCall* TexCenterPlusTangentU =
        NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat4,
        {
            Texture,
            NewMaterialExpressionAdd(Function, WorldAlignedPositionXY, OffsetTangentU),
            NewMaterialExpressionAdd(Function, WorldAlignedPositionXZ, OffsetTangentU),
            NewMaterialExpressionAdd(Function, WorldAlignedPositionYZ, OffsetTangentU)
        });

# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionIf* CentralDifferenceU =
        NewMaterialExpressionIfEqual(Function, UseWorldAlignedTexture, 1.0f,
# else
    UMaterialExpressionIf* CentralDifferenceU =
        NewMaterialExpressionStaticSwitch(Function, UseWorldAlignedTexture,
# endif
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterMinusTangentU, BumpSource }),
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterPlusTangentU, BumpSource })),
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterMinusU, BumpSource }),
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterPlusU, BumpSource })));
#else
    UMaterialExpressionSubtract* CentralDifferenceU =
        NewMaterialExpressionSubtract(Function,
            NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterMinusU, BumpSource }),
            NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterPlusU, BumpSource }));
#endif
    UMaterialExpressionMultiply* FactorU = NewMaterialExpressionMultiply(Function, Factor, CentralDifferenceU);

    UMaterialExpressionMaterialFunctionCall* OffsetV = NewMaterialExpressionFunctionCall(Function, MakeFloat2, { 0.0f, OneOverHeight });
    UMaterialExpressionMaterialFunctionCall* TexCenterMinusV = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionSubtract(Function, Center, OffsetV), WrapU, WrapV, CropU, CropV, 0 });
    UMaterialExpressionMaterialFunctionCall* TexCenterPlusV = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionAdd(Function, Center, OffsetV), WrapU, WrapV, CropU, CropV, 0 });

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionComponentMask* OffsetTangentV = NewMaterialExpressionComponentMask(Function, NewMaterialExpressionMultiply(Function, OneOverHeight, UVWTangentV), 3);
    UMaterialExpressionMaterialFunctionCall* TexCenterMinusTangentV =
        NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat4,
        {
            Texture,
            NewMaterialExpressionSubtract(Function, WorldAlignedPositionXY, OffsetTangentV),
            NewMaterialExpressionSubtract(Function, WorldAlignedPositionXZ, OffsetTangentV),
            NewMaterialExpressionSubtract(Function, WorldAlignedPositionYZ, OffsetTangentV)
        });
    UMaterialExpressionMaterialFunctionCall* TexCenterPlusTangentV =
        NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat4,
        {
            Texture,
            NewMaterialExpressionAdd(Function, WorldAlignedPositionXY, OffsetTangentV),
            NewMaterialExpressionAdd(Function, WorldAlignedPositionXZ, OffsetTangentV),
            NewMaterialExpressionAdd(Function, WorldAlignedPositionYZ, OffsetTangentV)
        });

# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionIf* CentralDifferenceV =
        NewMaterialExpressionIfEqual(Function, UseWorldAlignedTexture, 1.0f,
# else
    UMaterialExpressionIf* CentralDifferenceV =
        NewMaterialExpressionStaticSwitch(Function, UseWorldAlignedTexture,
# endif
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterMinusTangentV, BumpSource }),
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterPlusTangentV, BumpSource })),
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterMinusV, BumpSource }),
                NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterPlusV, BumpSource })));
#else
    UMaterialExpressionSubtract* CentralDifferenceV =
        NewMaterialExpressionSubtract(Function,
            NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterMinusV, BumpSource }),
            NewMaterialExpressionFunctionCall(Function, ImporterMonoMode, { TexCenterPlusV, BumpSource }));
#endif
    UMaterialExpressionMultiply* FactorV = NewMaterialExpressionMultiply(Function, Factor, CentralDifferenceV);

    UMaterialExpressionNormalize* UnclippedResult =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                Normal,
                NewMaterialExpressionMultiply(Function, FactorU, UVWTangentU),
                NewMaterialExpressionMultiply(Function, FactorV, UVWTangentV)
            }));
#endif

    UMaterialExpressionIf* ClipVCheck =
        NewMaterialExpressionIfEqual(Function, WrapV, wrap_clip,
            NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f,
                Normal,
                NewMaterialExpressionIfGreater(Function, UVWPositionY, 1.0f, Normal, UnclippedResult)),
            UnclippedResult);
    UMaterialExpressionIf* ClipUCheck =
        NewMaterialExpressionIfEqual(Function, WrapU, wrap_clip,
            NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f,
                Normal,
                NewMaterialExpressionIfGreater(Function, UVWPositionX, 1.0f, Normal, ClipVCheck)),
            ClipVCheck);
    UMaterialExpressionIf* ClampVCheck =
        NewMaterialExpressionIfEqual(Function, WrapV, wrap_clamp,
            NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f,
                Normal,
                NewMaterialExpressionIfGreater(Function, UVWPositionY, 1.0f, Normal, ClipUCheck)),
            ClipUCheck);
    UMaterialExpressionIf* Result =
        NewMaterialExpressionStaticSwitch(Function, Clip,
            NewMaterialExpressionIfEqual(Function, WrapU, wrap_clamp,
                NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f,
                    Normal,
                    NewMaterialExpressionIfGreater(Function, UVWPositionX, 1.0f, Normal, ClampVCheck)),
                ClampVCheck),
            ClipUCheck);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseFileTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* TexLookupFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float3_2d"));
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialFunction* WorldAlignedTextureFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_world_aligned_texture_float3"));
    UMaterialFunction* WorldAlignedTextureFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_world_aligned_texture_float4"));
#endif

    Function->Description = TEXT("General texturing function for 2D bitmap texture stored in a file.");

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* ColorOffset = NewMaterialExpressionFunctionInput(Function, TEXT("color_offset"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* ColorScale = NewMaterialExpressionFunctionInput(Function, TEXT("color_scale"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MonoSource = NewMaterialExpressionFunctionInput(Function, TEXT("mono_source"), EFunctionInputType::FunctionInput_Scalar, mono_alpha);
    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* Clip = NewMaterialExpressionFunctionInput(Function, TEXT("clip"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* AnimationStartTime = NewMaterialExpressionFunctionInput(Function, TEXT("animation_start_time"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnimationCrop = NewMaterialExpressionFunctionInput(Function, TEXT("animation_crop"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* AnimationWrap = NewMaterialExpressionFunctionInput(Function, TEXT("animation_wrap"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* AnimationFPS = NewMaterialExpressionFunctionInput(Function, TEXT("animation_fps"), EFunctionInputType::FunctionInput_Scalar, 30.0f);

#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionAdd* Float4Result =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionMultiply(Function,
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
                NewMaterialExpressionIfEqual(Function, UseWorldAlignedTexture, 1.0f,
# else
                NewMaterialExpressionStaticSwitch(Function, UseWorldAlignedTexture,
# endif
                    NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat4, { Texture, WorldAlignedPositionXY, WorldAlignedPositionXZ, WorldAlignedPositionYZ }),
                    NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 })),
#else
                NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 }),
#endif
                NewMaterialExpressionAppendVector(Function, ColorScale, 1.0f)),
            NewMaterialExpressionAppendVector(Function, ColorOffset, 0.0f));
    UMaterialExpressionAdd* ColorResult =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionMultiply(Function,
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
                NewMaterialExpressionIfEqual(Function, UseWorldAlignedTexture, 1.0f,
# else
                NewMaterialExpressionStaticSwitch(Function, UseWorldAlignedTexture,
# endif
                    NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat3, { Texture, WorldAlignedPositionXY, WorldAlignedPositionXZ, WorldAlignedPositionYZ }),
                    NewMaterialExpressionFunctionCall(Function, TexLookupFloat3, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 })),
#else
                NewMaterialExpressionFunctionCall(Function, TexLookupFloat3, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 }),
#endif
                ColorScale),
            ColorOffset);
    UMaterialExpressionIf* UnclippedTint = NewMaterialExpressionIfEqual(Function, MonoSource, mono_alpha, NewMaterialExpressionComponentMask(Function, Float4Result, 7), ColorResult);
    UMaterialExpressionIf* UnclippedMono = NewMaterialExpressionSwitch(Function, MonoSource,
    {
        NewMaterialExpressionComponentMask(Function, Float4Result, 8),
        NewMaterialExpressionFunctionCall(Function, MathAverage,{ ColorResult }),
        NewMaterialExpressionFunctionCall(Function, MathLuminance,{ ColorResult }),
        NewMaterialExpressionFunctionCall(Function, MathMaxValue,{ ColorResult })
    });

    UMaterialExpressionComponentMask* UVWPositionX = NewMaterialExpressionComponentMask(Function, UVWPosition, 1);
    UMaterialExpressionComponentMask* UVWPositionY = NewMaterialExpressionComponentMask(Function, UVWPosition, 2);

    UMaterialExpressionConstant3Vector* TintBlack = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f);
    UMaterialExpressionIf* TintPositionYGreaterCheck = NewMaterialExpressionIfGreater(Function, UVWPositionY, 1.0f, TintBlack, UnclippedTint);
    UMaterialExpressionIf* TintPositionYLessCheck = NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f, TintBlack, TintPositionYGreaterCheck);
    UMaterialExpressionIf* TintWrapVCheck = NewMaterialExpressionIfEqual(Function, WrapV, wrap_clamp, TintPositionYLessCheck, UnclippedTint);
    UMaterialExpressionIf* TintPositionXGreaterCheck = NewMaterialExpressionIfGreater(Function, UVWPositionX, 1.0f, TintBlack, TintWrapVCheck);
    UMaterialExpressionIf* TintPositionXLessCheck = NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f, TintBlack, TintPositionXGreaterCheck);
    UMaterialExpressionIf* TintWrapUCheck = NewMaterialExpressionIfEqual(Function, WrapU, wrap_clamp, TintPositionXLessCheck, TintWrapVCheck);
    UMaterialExpressionIf* Tint = NewMaterialExpressionStaticSwitch(Function, Clip, TintWrapUCheck, UnclippedTint);

    UMaterialExpressionConstant* MonoBlack = NewMaterialExpressionConstant(Function, 0.0f);
    UMaterialExpressionIf* MonoPositionYGreaterCheck = NewMaterialExpressionIfGreater(Function, UVWPositionY, 1.0f, MonoBlack, UnclippedMono);
    UMaterialExpressionIf* MonoPositionYLessCheck = NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f, MonoBlack, MonoPositionYGreaterCheck);
    UMaterialExpressionIf* MonoWrapVCheck = NewMaterialExpressionIfEqual(Function, WrapV, wrap_clamp, MonoPositionYLessCheck, UnclippedMono);
    UMaterialExpressionIf* MonoPositionXGreaterCheck = NewMaterialExpressionIfGreater(Function, UVWPositionX, 1.0f, MonoBlack, MonoWrapVCheck);
    UMaterialExpressionIf* MonoPositionXLessCheck = NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f, MonoBlack, MonoPositionXGreaterCheck);
    UMaterialExpressionIf* MonoWrapUCheck = NewMaterialExpressionIfEqual(Function, WrapU, wrap_clamp, MonoPositionXLessCheck, MonoWrapVCheck);
    UMaterialExpressionIf* Mono = NewMaterialExpressionStaticSwitch(Function, Clip, MonoWrapUCheck, UnclippedMono);

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_BaseFileNormalTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BaseFileTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_file_texture"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* ColorOffset = NewMaterialExpressionFunctionInput(Function, TEXT("color_offset"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* ColorScale = NewMaterialExpressionFunctionInput(Function, TEXT("color_scale"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MonoSource = NewMaterialExpressionFunctionInput(Function, TEXT("mono_source"), EFunctionInputType::FunctionInput_Scalar, mono_alpha);
    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* Clip = NewMaterialExpressionFunctionInput(Function, TEXT("clip"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* AnimationStartTime = NewMaterialExpressionFunctionInput(Function, TEXT("animation_start_time"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnimationCrop = NewMaterialExpressionFunctionInput(Function, TEXT("animation_crop"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* AnimationWrap = NewMaterialExpressionFunctionInput(Function, TEXT("animation_wrap"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* AnimationFPS = NewMaterialExpressionFunctionInput(Function, TEXT("animation_fps"), EFunctionInputType::FunctionInput_Scalar, 30.0f);

#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, BaseFileTexture, 
        {
            Texture,
            ColorOffset,
            ColorScale,
            MonoSource,
            UVWPosition,
            UVWTangentU,
            UVWTangentV,
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            WorldAlignedPositionXY,
            WorldAlignedPositionXZ,
            WorldAlignedPositionYZ,
#endif
            CropU,
            CropV,
            WrapU,
            WrapV,
            Clip,
            AnimationStartTime,
            AnimationCrop,
            AnimationWrap,
            AnimationFPS
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            , UseWorldAlignedTexture
#endif
        });
    UMaterialExpressionComponentMask* MaskRG = NewMaterialExpressionComponentMask(Function, { Sample, 0 }, 3);
    UMaterialExpressionMultiply* PackNormal = NewMaterialExpressionMultiply(Function, NewMaterialExpressionAdd(Function, MaskRG, 1.0f), 0.5f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), NewMaterialExpressionAppendVector(Function, PackNormal, 1.0f));
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), { Sample, 1 });
}

static void Generator_BaseFlakeNoiseBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterMiNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_mi_noise"));
    UMaterialFunction* ImporterWorleyNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_worley_noise"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Bump-mapping flake noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Strength = NewMaterialExpressionFunctionInput(Function, TEXT("strength"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseType = NewMaterialExpressionFunctionInput(Function, TEXT("noise_type"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MaximumSize = NewMaterialExpressionFunctionInput(Function, TEXT("maximum_size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Metric = NewMaterialExpressionFunctionInput(Function, TEXT("metric"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionDivide* Pos = NewMaterialExpressionDivide(Function, UVWPosition, Scale);
    UMaterialExpressionMaterialFunctionCall* WorleyNoise = NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoise, { Pos, 1.0f, Metric });
    UMaterialExpressionIf* CellDistance = NewMaterialExpressionIfEqual(Function, NoiseType, 1.0f, NewMaterialExpressionComponentMask(Function, { WorleyNoise, 2 }, 1), 0.0f);
    UMaterialExpressionIf* Pos2 = NewMaterialExpressionIfEqual(Function, NoiseType, 1.0f, { WorleyNoise, 0 }, Pos);
    UMaterialExpressionMaterialFunctionCall* MiNoise = NewMaterialExpressionFunctionCall(Function, ImporterMiNoise, { Pos2 });
    UMaterialExpressionAdd* Pos3 = NewMaterialExpressionAdd(Function, Pos2, NewMaterialExpressionMultiply(Function, { MiNoise, 0 }, 2.0f));
    UMaterialExpressionMaterialFunctionCall* MiNoise2 = NewMaterialExpressionFunctionCall(Function, ImporterMiNoise, { NewMaterialExpressionFloor(Function, Pos3) });
    UMaterialExpressionIf* Grad = NewMaterialExpressionIfEqual(Function, NoiseType, 0.0f, { MiNoise2, 0 }, { MiNoise, 0 });

    UMaterialExpressionNormalize* CalculatedNormal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionMultiply(Function,
                    Normal,
                    NewMaterialExpressionAdd(Function,
                        NewMaterialExpressionAbs(Function, NewMaterialExpressionComponentMask(Function, Grad, 4)),
                        NewMaterialExpressionDivide(Function, 1.0f, Strength))),
                NewMaterialExpressionMultiply(Function,
                    UVWTangentU,
                    NewMaterialExpressionDivide(Function, NewMaterialExpressionComponentMask(Function, Grad, 1), Strength)),
                NewMaterialExpressionMultiply(Function,
                    UVWTangentV,
                    NewMaterialExpressionDivide(Function, NewMaterialExpressionComponentMask(Function, Grad, 2), Strength))
            }));
    UMaterialExpressionIf* MaximumSizeCheck = NewMaterialExpressionIfGreater(Function, CellDistance, MaximumSize, Normal, CalculatedNormal);
    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfEqual(Function, Strength, 0.0f,
            Normal,
            NewMaterialExpressionIfEqual(Function, Scale, 0.0f, Normal, NewMaterialExpressionIfEqual(Function, NoiseType, 1.0f, MaximumSizeCheck, CalculatedNormal)));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseFlakeNoiseTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterMiNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_mi_noise"));
    UMaterialFunction* ImporterWorleyNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_worley_noise"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Flake noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Intensity = NewMaterialExpressionFunctionInput(Function, TEXT("intensity"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Density = NewMaterialExpressionFunctionInput(Function, TEXT("density"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseType = NewMaterialExpressionFunctionInput(Function, TEXT("noise_type"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MaximumSize = NewMaterialExpressionFunctionInput(Function, TEXT("maximum_size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Metric = NewMaterialExpressionFunctionInput(Function, TEXT("metric"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionDivide* Pos = NewMaterialExpressionDivide(Function, UVWPosition, Scale);
    UMaterialExpressionMaterialFunctionCall* WorleyNoise = NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoise, { Pos, 1.0f, Metric });
    UMaterialExpressionComponentMask* CellDistance = NewMaterialExpressionComponentMask(Function, { WorleyNoise, 2 }, 1);
    UMaterialExpressionIf* Pos2 = NewMaterialExpressionIfEqual(Function, NoiseType, 1.0f, { WorleyNoise, 0 }, Pos);
    UMaterialExpressionMaterialFunctionCall* MiNoise = NewMaterialExpressionFunctionCall(Function, ImporterMiNoise, { Pos2 });
    UMaterialExpressionAdd* Pos3 = NewMaterialExpressionAdd(Function, Pos2, NewMaterialExpressionMultiply(Function, { MiNoise, 0 }, 2.0f));
    UMaterialExpressionMaterialFunctionCall* MiNoise2 = NewMaterialExpressionFunctionCall(Function, ImporterMiNoise, { NewMaterialExpressionFloor(Function, Pos3) });
    UMaterialExpressionIf* NoiseScale = NewMaterialExpressionIfEqual(Function, NoiseType, 0.0f, { MiNoise2, 1 }, { MiNoise, 1 });
    UMaterialExpressionMultiply* ReflectivityCalculation =
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionPower(Function, NoiseScale, NewMaterialExpressionDivide(Function, 1.0f, Density)), Intensity);
    UMaterialExpressionIf* CellDistanceCheck = NewMaterialExpressionIfGreater(Function, CellDistance, MaximumSize, 0.0f, ReflectivityCalculation);
    UMaterialExpressionIf* Reflectivity =
        NewMaterialExpressionIfEqual(Function, Scale, 0.0f, 0.0f, NewMaterialExpressionIfEqual(Function, NoiseType, 1.0f, CellDistanceCheck, ReflectivityCalculation));
    UMaterialExpressionIf* Tint =
        NewMaterialExpressionIfEqual(Function, Scale, 0.0f, { 0.0f, 0.0f, 0.0f }, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Reflectivity, Reflectivity, Reflectivity }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Reflectivity);
}

static void Generator_BaseFlowNoiseBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterSummedFlowNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_summed_flow_noise"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Bump-mapping low noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Phase = NewMaterialExpressionFunctionInput(Function, TEXT("phase"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Levels = NewMaterialExpressionFunctionInput(Function, TEXT("levels"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* AbsoluteNoise = NewMaterialExpressionFunctionInput(Function, TEXT("absolute_noise"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* LevelGain = NewMaterialExpressionFunctionInput(Function, TEXT("level_gain"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* LevelScale = NewMaterialExpressionFunctionInput(Function, TEXT("level_scale"), EFunctionInputType::FunctionInput_Scalar, 2.0f);
    UMaterialExpressionFunctionInput* LevelProgressiveUScale = NewMaterialExpressionFunctionInput(Function, TEXT("level_progressive_u_scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* LevelProgressiveUMotion = NewMaterialExpressionFunctionInput(Function, TEXT("level_progressive_u_motion"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionComponentMask* Pos = NewMaterialExpressionComponentMask(Function, UVWPosition, 3);
    UMaterialExpressionMultiply* Delta = NewMaterialExpressionMultiply(Function, 0.1f, Size);

    UMaterialExpressionAdd* Result0 =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterSummedFlowNoise,
                {
                    NewMaterialExpressionDivide(Function, Pos, Size), Phase, Levels, AbsoluteNoise, LevelGain, LevelScale, LevelProgressiveUScale, LevelProgressiveUMotion
                }),
                0.5f),
            0.5f);
    UMaterialExpressionAdd* Result1 =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterSummedFlowNoise,
                {
                    NewMaterialExpressionDivide(Function,
                    NewMaterialExpressionAdd(Function, Pos, NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ Delta, 0.0f })),
                    Size),
                    Phase, Levels, AbsoluteNoise, LevelGain, LevelScale, LevelProgressiveUScale, LevelProgressiveUMotion
                }),
                0.5f),
            0.5f);
    UMaterialExpressionAdd* Result2 =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionFunctionCall(Function, ImporterSummedFlowNoise,
                {
                    NewMaterialExpressionDivide(Function,
                    NewMaterialExpressionAdd(Function, Pos, NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ 0.0f, Delta })),
                    Size),
                    Phase, Levels, AbsoluteNoise, LevelGain, LevelScale, LevelProgressiveUScale, LevelProgressiveUMotion
                }),
                0.5f),
            0.5f);

    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfEqual(Function, Factor, 0.0f,
            Normal,
            NewMaterialExpressionIfEqual(Function, Size, 0.0f,
                Normal,
                NewMaterialExpressionAdd(Function,
                {
                    Normal,
                    NewMaterialExpressionMultiply(Function, { UVWTangentV, NewMaterialExpressionSubtract(Function, Result2, Result0), Factor }),
                    NewMaterialExpressionMultiply(Function, { UVWTangentU, NewMaterialExpressionSubtract(Function, Result1, Result0), Factor })
                })));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseFlowNoiseTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterSummedFlowNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_summed_flow_noise"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Color Perlin flow noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Phase = NewMaterialExpressionFunctionInput(Function, TEXT("phase"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Levels = NewMaterialExpressionFunctionInput(Function, TEXT("levels"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* AbsoluteNoise = NewMaterialExpressionFunctionInput(Function, TEXT("absolute_noise"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* LevelGain = NewMaterialExpressionFunctionInput(Function, TEXT("level_gain"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* LevelScale = NewMaterialExpressionFunctionInput(Function, TEXT("level_scale"), EFunctionInputType::FunctionInput_Scalar, 2.0f);
    UMaterialExpressionFunctionInput* LevelProgressiveUScale = NewMaterialExpressionFunctionInput(Function, TEXT("level_progressive_u_scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* LevelProgressiveVMotion = NewMaterialExpressionFunctionInput(Function, TEXT("level_progressive_v_motion"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* SummedNoise = NewMaterialExpressionFunctionCall(Function, ImporterSummedFlowNoise, { NewMaterialExpressionDivide(Function, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), Size), Phase, Levels, AbsoluteNoise, LevelGain, LevelScale, LevelProgressiveUScale, LevelProgressiveVMotion });
    UMaterialExpressionIf* Alpha =
        NewMaterialExpressionIfEqual(Function, Size, 0.0f,
            0.0f,
            NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, SummedNoise, 0.5f), 0.5f));
    UMaterialExpressionLinearInterpolate* Tint = NewMaterialExpressionLinearInterpolate(Function, Color1, Color2, Alpha);
    UMaterialExpressionMaterialFunctionCall* Mono = NewMaterialExpressionFunctionCall(Function, MathLuminance, { Tint });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_BaseGlossToRough(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Glossiness = NewMaterialExpressionFunctionInput(Function, TEXT("glossiness"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionOneMinus* Rough = NewMaterialExpressionOneMinus(Function, Glossiness);

    NewMaterialExpressionFunctionOutput(Function, TEXT("rough"), Rough);
}

static void Generator_BasePerlinNoiseBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterApplyNoiseModifications = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_apply_noise_modifications"));
    UMaterialFunction* ImporterSummedPerlinNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_summed_perlin_noise"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Bump-mapping Perlin noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* ApplyMarble = NewMaterialExpressionFunctionInput(Function, TEXT("apply_marble"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* ApplyDent = NewMaterialExpressionFunctionInput(Function, TEXT("apply_dent"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoisePhase = NewMaterialExpressionFunctionInput(Function, TEXT("noise_phase"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseLevels = NewMaterialExpressionFunctionInput(Function, TEXT("noise_levels"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* AbsoluteNoise = NewMaterialExpressionFunctionInput(Function, TEXT("absolute_noise"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* RidgedNoise = NewMaterialExpressionFunctionInput(Function, TEXT("ridged_noise"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoiseDistortion = NewMaterialExpressionFunctionInput(Function, TEXT("noise_distortion"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* NoiseThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_high"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_low"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseBands = NewMaterialExpressionFunctionInput(Function, TEXT("noise_bands"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionDivide* Delta = NewMaterialExpressionDivide(Function, NewMaterialExpressionMultiply(Function, 0.1f, Size), NoiseBands);

    UMaterialExpressionDivide* ScaledPosition0 = NewMaterialExpressionDivide(Function, UVWPosition, Size);
    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterSummedPerlinNoise,{ ScaledPosition0, NoisePhase, NoiseLevels, NoiseDistortion, AbsoluteNoise, RidgedNoise }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition0, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionDivide* ScaledPosition1 = NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Delta, 0.0f, 0.0f })), Size);
    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterSummedPerlinNoise,{ ScaledPosition1, NoisePhase, NoiseLevels, NoiseDistortion, AbsoluteNoise, RidgedNoise }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition1, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionDivide* ScaledPosition2 = NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 0.0f, Delta, 0.0f })), Size);
    UMaterialExpressionMaterialFunctionCall* Result2 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterSummedPerlinNoise,{ ScaledPosition2, NoisePhase, NoiseLevels, NoiseDistortion, AbsoluteNoise, RidgedNoise }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition2, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionDivide* ScaledPosition3 = NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 0.0f, 0.0f, Delta })), Size);
    UMaterialExpressionMaterialFunctionCall* Result3 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterSummedPerlinNoise,{ ScaledPosition3, NoisePhase, NoiseLevels, NoiseDistortion, AbsoluteNoise, RidgedNoise }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition3, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionSubtract* BumpFactor = NewMaterialExpressionNegate(Function, Factor);
    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfEqual(Function, Factor, 0.0f,
            Normal,
            NewMaterialExpressionIfEqual(Function, Size, 0.0f,
                Normal,
                NewMaterialExpressionNormalize(Function,
                    NewMaterialExpressionAdd(Function,
                    {
                        NewMaterialExpressionMultiply(Function,
                            Normal,
                            NewMaterialExpressionAdd(Function,
                                NewMaterialExpressionAbs(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, Result3, Result0), BumpFactor)),
                                1.0f)),
                        NewMaterialExpressionMultiply(Function, { UVWTangentU, NewMaterialExpressionSubtract(Function, Result1, Result0), BumpFactor }),
                        NewMaterialExpressionMultiply(Function, { UVWTangentV, NewMaterialExpressionSubtract(Function, Result2, Result0), BumpFactor })
                    }))));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BasePerlinNoiseTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterApplyNoiseModifications = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_apply_noise_modifications"));
    UMaterialFunction* ImporterSummedPerlinNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_summed_perlin_noise"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Color Perlin noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* ApplyMarble = NewMaterialExpressionFunctionInput(Function, TEXT("apply_marble"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* ApplyDent = NewMaterialExpressionFunctionInput(Function, TEXT("apply_dent"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoisePhase = NewMaterialExpressionFunctionInput(Function, TEXT("noise_phase"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseLevels = NewMaterialExpressionFunctionInput(Function, TEXT("noise_levels"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* AbsoluteNoise = NewMaterialExpressionFunctionInput(Function, TEXT("absolute_noise"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* RidgedNoise = NewMaterialExpressionFunctionInput(Function, TEXT("ridged_noise"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoiseDistortion = NewMaterialExpressionFunctionInput(Function, TEXT("noise_distortion"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* NoiseThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_high"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_low"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseBands = NewMaterialExpressionFunctionInput(Function, TEXT("noise_bands"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionDivide* ScaledPosition = NewMaterialExpressionDivide(Function, UVWPosition, Size);
    UMaterialExpressionIf* Alpha =
        NewMaterialExpressionIfEqual(Function, Size, 0.0f,
            0.0f,
            NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
            {
                NewMaterialExpressionFunctionCall(Function, ImporterSummedPerlinNoise,{ ScaledPosition, NoisePhase, NoiseLevels, NoiseDistortion, AbsoluteNoise, RidgedNoise }),
                NewMaterialExpressionComponentMask(Function, ScaledPosition, 1),
                ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
            }));
    UMaterialExpressionLinearInterpolate* Tint = NewMaterialExpressionLinearInterpolate(Function, Color1, Color2, Alpha);
    UMaterialExpressionMaterialFunctionCall* Mono = NewMaterialExpressionFunctionCall(Function, MathLuminance, { Tint });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_BaseRotationTranslationScale(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* MathMultiplyFloat4x4Float4x4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4x4"));

    Function->Description = TEXT("Construct transformation matrix from Euler rotation, translation and scale.");

    UMaterialExpressionFunctionInput* Rotation = NewMaterialExpressionFunctionInput(Function, TEXT("rotation"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Translation = NewMaterialExpressionFunctionInput(Function, TEXT("translation"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Scaling = NewMaterialExpressionFunctionInput(Function, TEXT("scaling"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });

    // Euler rotation matrix  xyz order
    UMaterialExpressionComponentMask* RX = NewMaterialExpressionComponentMask(Function, Rotation, 1);
    UMaterialExpressionComponentMask* RY = NewMaterialExpressionComponentMask(Function, Rotation, 2);
    UMaterialExpressionComponentMask* RZ = NewMaterialExpressionComponentMask(Function, Rotation, 4);
    UMaterialExpressionSine* SX = NewMaterialExpressionSine(Function, RX);
    UMaterialExpressionSine* SY = NewMaterialExpressionSine(Function, RY);
    UMaterialExpressionSine* SZ = NewMaterialExpressionSine(Function, RZ);
    UMaterialExpressionCosine* CX = NewMaterialExpressionCosine(Function, RX);
    UMaterialExpressionCosine* CY = NewMaterialExpressionCosine(Function, RY);
    UMaterialExpressionCosine* CZ = NewMaterialExpressionCosine(Function, RZ);

    // use the transposed rotation matrix, as we're left-handed here!
    // would need some closer investigations if everything is ordered as expected
    UMaterialExpressionMaterialFunctionCall* Rotate0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionMultiply(Function, CY, CZ),
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function, { SX, SY, CZ }), NewMaterialExpressionMultiply(Function, CX, SZ)),
            NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, { CX, SY, CZ }), NewMaterialExpressionMultiply(Function, SX, SZ)),
            0.0f
        });
    UMaterialExpressionMaterialFunctionCall* Rotate1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionMultiply(Function, CY, SZ),
            NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, { SX, SY, SZ }), NewMaterialExpressionMultiply(Function, CX, CZ)),
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function, { CX, SY, SZ }), NewMaterialExpressionMultiply(Function, SX, CZ)),
            0.0f
        });
    UMaterialExpressionMaterialFunctionCall* Rotate2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionNegate(Function, SY),
            NewMaterialExpressionMultiply(Function, SX, CY),
            NewMaterialExpressionMultiply(Function, CX, CY),
            0.0f
        });
    UMaterialExpressionConstant4Vector* Rotate3 = NewMaterialExpressionConstant(Function, 0.5f, 0.5f, 0.5f, 1.0f);

    UMaterialExpressionMaterialFunctionCall* Scale0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4, { NewMaterialExpressionComponentMask(Function, Scaling, 1), 0.0f, 0.0f, 0.0f });
    UMaterialExpressionMaterialFunctionCall* Scale1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4, { 0.0f, NewMaterialExpressionComponentMask(Function, Scaling, 2), 0.0f, 0.0f });
    UMaterialExpressionMaterialFunctionCall* Scale2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4, { 0.0f, 0.0f, NewMaterialExpressionComponentMask(Function, Scaling, 4), 0.0f });
    UMaterialExpressionMaterialFunctionCall* Scale3 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, Translation, 1), 0.5f),
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, Translation, 2), 0.5f),
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, Translation, 4), 0.5f),
            1.0f
        });

    UMaterialExpressionMaterialFunctionCall* Result = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4, { Rotate0, Rotate1, Rotate2, Rotate3, Scale0, Scale1, Scale2, Scale3 });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), { Result, 0 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), { Result, 1 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), { Result, 2 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_3"), { Result, 3 });
}

static void Generator_BaseSellmeierCoefficientsIOR(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* SellmeierB = NewMaterialExpressionFunctionInput(Function, TEXT("sellmeier_B"), EFunctionInputType::FunctionInput_Vector3, { 1.04f, 0.23f, 1.01f });
    UMaterialExpressionFunctionInput* SellmeierC = NewMaterialExpressionFunctionInput(Function, TEXT("sellmeier_C"), EFunctionInputType::FunctionInput_Vector3, { 0.006f, 0.2f, 103.56f });

    float L2 = 0.5892 * 0.5892;		// simplified to fixed wavelength of 589.2 nm
    UMaterialExpressionSquareRoot* Color =
        NewMaterialExpressionSquareRoot(Function,
            NewMaterialExpressionAdd(Function,
            {
                1.0f,
                NewMaterialExpressionMultiply(Function,
                    NewMaterialExpressionComponentMask(Function, SellmeierB, 1),
                    NewMaterialExpressionDivide(Function,
                        L2,
                        NewMaterialExpressionSubtract(Function, L2, NewMaterialExpressionComponentMask(Function, SellmeierC, 1)))),
                NewMaterialExpressionMultiply(Function,
                    NewMaterialExpressionComponentMask(Function, SellmeierB, 2),
                    NewMaterialExpressionDivide(Function,
                        L2,
                        NewMaterialExpressionSubtract(Function, L2, NewMaterialExpressionComponentMask(Function, SellmeierC, 2)))),
                NewMaterialExpressionMultiply(Function,
                    NewMaterialExpressionComponentMask(Function, SellmeierB, 4),
                    NewMaterialExpressionDivide(Function,
                        L2,
                        NewMaterialExpressionSubtract(Function, L2, NewMaterialExpressionComponentMask(Function, SellmeierC, 4))))
            }));

    UMaterialExpressionMaterialFunctionCall* Result = NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Color, Color, Color });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseTangentSpaceNormalTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* TexLookupFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float3_2d"));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialFunction* WorldAlignedTextureFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_world_aligned_texture_float3"));
#endif

    Function->Description = TEXT("Interprets the color values of a bitmap as a vector in tangent space.");

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultNormal")), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* FlipTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("flip_tangent_u"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* FlipTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("flip_tangent_v"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* Clip = NewMaterialExpressionFunctionInput(Function, TEXT("clip"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnimationStartTime = NewMaterialExpressionFunctionInput(Function, TEXT("animation_start_time"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnimationCrop = NewMaterialExpressionFunctionInput(Function, TEXT("animation_crop"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* AnimationWrap = NewMaterialExpressionFunctionInput(Function, TEXT("animation_wrap"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* AnimationFPS = NewMaterialExpressionFunctionInput(Function, TEXT("animation_fps"), EFunctionInputType::FunctionInput_Scalar, 30.0f);

#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionComponentMask* UVWPositionX = NewMaterialExpressionComponentMask(Function, UVWPosition, 1);
    UMaterialExpressionComponentMask* UVWPositionY = NewMaterialExpressionComponentMask(Function, UVWPosition, 2);

    UMaterialExpressionMaterialFunctionCall* DefaultNormal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

#if 0
    // ignore wrapping for now, as that does not compile without errors!
    // TODO: Needs to be investigated!
    UMaterialExpressionIf* NotFlipTangentU =
        NewMaterialExpressionStaticSwitch(Function, FlipTangentU,
            NewMaterialExpressionStaticBool(Function, false),
            NewMaterialExpressionStaticBool(Function, true));
    UMaterialExpressionIf* PositionXEvenCheck =
        NewMaterialExpressionIfEqual(Function,
            NewMaterialExpressionFmod(Function, NewMaterialExpressionCeil(Function, UVWPositionX), 2.0f),
            0.0f,
            NotFlipTangentU,
            FlipTangentU);
    UMaterialExpressionIf* FlipPositionXLessCheck = NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f, PositionXEvenCheck, FlipTangentU);
    UMaterialExpressionIf* PositionXOddCheck =
        NewMaterialExpressionIfEqual(Function,
            NewMaterialExpressionFmod(Function, NewMaterialExpressionFloor(Function, UVWPositionX), 2.0f),
            1.0f,
            NotFlipTangentU,
            FlipTangentU);
    UMaterialExpressionIf* FlipPositionXGreaterCheck = NewMaterialExpressionIfGreater(Function, UVWPositionX, 0.0f, PositionXOddCheck, FlipPositionXLessCheck);
    UMaterialExpressionIf* PixelFlipU = NewMaterialExpressionIfEqual(Function, WrapU, wrap_mirrored_repeat, FlipPositionXGreaterCheck, FlipTangentU);
    UMaterialExpressionIf* TransformedTangentU = NewMaterialExpressionStaticSwitch(Function, PixelFlipU, NewMaterialExpressionNegate(Function, UVWTangentU), UVWTangentU);

    UMaterialExpressionIf* NotFlipTangentV =
        NewMaterialExpressionStaticSwitch(Function, FlipTangentV,
            NewMaterialExpressionStaticBool(Function, false),
            NewMaterialExpressionStaticBool(Function, true));
    UMaterialExpressionIf* PositionYEvenCheck =
        NewMaterialExpressionIfEqual(Function,
            NewMaterialExpressionFmod(Function, NewMaterialExpressionCeil(Function, UVWPositionY), 2.0f),
            0.0f,
            NotFlipTangentV,
            FlipTangentV);
    UMaterialExpressionIf* FlipPositionYLessCheck = NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f, PositionYEvenCheck, FlipTangentV);
    UMaterialExpressionIf* PositionYOddCheck =
        NewMaterialExpressionIfEqual(Function,
            NewMaterialExpressionFmod(Function, NewMaterialExpressionFloor(Function, UVWPositionY), 2.0f),
            1.0f,
            NotFlipTangentV,
            FlipTangentV);
    UMaterialExpressionIf* FlipPositionYGreaterCheck = NewMaterialExpressionIfGreater(Function, UVWPositionY, 0.0f, PositionYOddCheck, FlipPositionYLessCheck);
    UMaterialExpressionIf* PixelFlipV = NewMaterialExpressionIfEqual(Function, WrapV, wrap_mirrored_repeat, FlipPositionYGreaterCheck, FlipTangentV);
    UMaterialExpressionIf* TransformedTangentV = NewMaterialExpressionStaticSwitch(Function, PixelFlipV, NewMaterialExpressionNegate(Function, UVWTangentV), UVWTangentV);
#else
    UMaterialExpressionIf* TransformedTangentU = NewMaterialExpressionStaticSwitch(Function, FlipTangentU, NewMaterialExpressionNegate(Function, UVWTangentU), UVWTangentU);
    UMaterialExpressionIf* TransformedTangentV = NewMaterialExpressionStaticSwitch(Function, FlipTangentV, NewMaterialExpressionNegate(Function, UVWTangentV), UVWTangentV);
#endif

    // UE4 already applies bias to normal after sampling so no need to do it manually
    UMaterialExpressionMultiply* TangentSpaceNormal =
        NewMaterialExpressionMultiply(Function,
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
            NewMaterialExpressionIfEqual(Function, UseWorldAlignedTexture, 1.0f,
# else
            NewMaterialExpressionStaticSwitch(Function, UseWorldAlignedTexture,
# endif
                NewMaterialExpressionFunctionCall(Function, WorldAlignedTextureFloat3, { Texture, WorldAlignedPositionXY, WorldAlignedPositionXZ, WorldAlignedPositionYZ }),
                NewMaterialExpressionFunctionCall(Function, TexLookupFloat3, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 })),
#else
            NewMaterialExpressionFunctionCall(Function, TexLookupFloat3, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 }),
#endif
            Factor);
    UMaterialExpressionNormalize* UnclippedNormal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionMultiply(Function, TransformedTangentU, NewMaterialExpressionComponentMask(Function, TangentSpaceNormal, 1)),
                NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, TransformedTangentV, NewMaterialExpressionComponentMask(Function, TangentSpaceNormal, 2))),
                NewMaterialExpressionMultiply(Function,
                    DefaultNormal,
                    NewMaterialExpressionAdd(Function,
                        NewMaterialExpressionComponentMask(Function, TangentSpaceNormal, 4),
                        NewMaterialExpressionOneMinus(Function, Factor)))
            }));

    UMaterialExpressionIf* ClipVCheck =
        NewMaterialExpressionIfEqual(Function, WrapV, wrap_clip,
            NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f,
                DefaultNormal,
                NewMaterialExpressionIfGreater(Function, UVWPositionY, 1.0f, DefaultNormal, UnclippedNormal)),
            UnclippedNormal);
    UMaterialExpressionIf* ClipUCheck =
        NewMaterialExpressionIfEqual(Function, WrapU, wrap_clip,
            NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f,
                DefaultNormal,
                NewMaterialExpressionIfGreater(Function, UVWPositionX, 1.0f, DefaultNormal, ClipVCheck)),
            ClipVCheck);
    UMaterialExpressionIf* ClampVCheck =
        NewMaterialExpressionIfEqual(Function, WrapV, wrap_clamp,
            NewMaterialExpressionIfLess(Function, UVWPositionY, 0.0f,
                DefaultNormal,
                NewMaterialExpressionIfGreater(Function, UVWPositionY, 1.0f, DefaultNormal, ClipUCheck)),
            ClipUCheck);
    UMaterialExpressionIf* Normal =
        NewMaterialExpressionStaticSwitch(Function, Clip,
            NewMaterialExpressionIfEqual(Function, WrapU, wrap_clamp,
                NewMaterialExpressionIfLess(Function, UVWPositionX, 0.0f,
                    DefaultNormal,
                    NewMaterialExpressionIfGreater(Function, UVWPositionX, 1.0f, DefaultNormal, ClampVCheck)),
                ClampVCheck),
            ClipUCheck);

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), Normal);
}

static void Generator_BaseTextureCoordinateInfo(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialFunction* StatePosition = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_position"));
#endif
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TangentV = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionMaterialFunctionCall* StatePositionCall = NewMaterialExpressionFunctionCall(Function, StatePosition, {});
    //UMaterialExpressionDivide* TextureCoordinateWorld = NewMaterialExpressionDivide(Function, StatePositionCall, 100.0f);
#endif

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TangentU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), TangentV);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xy"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 3));
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xz"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 5));
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_yz"), NewMaterialExpressionComponentMask(Function, StatePositionCall, 6));
#endif
}

static void Generator_BaseTileBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterEvalTileFunction = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_eval_tile_function"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Bump-mapping tiling generator.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NumberOfRows = NewMaterialExpressionFunctionInput(Function, TEXT("number_of_rows"), EFunctionInputType::FunctionInput_Scalar, 4.0f);
    UMaterialExpressionFunctionInput* NumberOfColumns = NewMaterialExpressionFunctionInput(Function, TEXT("number_of_columns"), EFunctionInputType::FunctionInput_Scalar, 4.0f);
    UMaterialExpressionFunctionInput* GroutWidth = NewMaterialExpressionFunctionInput(Function, TEXT("grout_width"), EFunctionInputType::FunctionInput_Scalar, 0.02f);
    UMaterialExpressionFunctionInput* GroutHeight = NewMaterialExpressionFunctionInput(Function, TEXT("grout_height"), EFunctionInputType::FunctionInput_Scalar, 0.02f);
    UMaterialExpressionFunctionInput* GroutRoughness = NewMaterialExpressionFunctionInput(Function, TEXT("grout_roughness"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MissingTileAmount = NewMaterialExpressionFunctionInput(Function, TEXT("missing_tile_amount"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TileBrightnessVariation = NewMaterialExpressionFunctionInput(Function, TEXT("tile_brightness_variation"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Seed = NewMaterialExpressionFunctionInput(Function, TEXT("seed"), EFunctionInputType::FunctionInput_Scalar, 2.284f);
    UMaterialExpressionFunctionInput* SpecialRowIndex = NewMaterialExpressionFunctionInput(Function, TEXT("special_row_index"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* SpecialRowWidthFactor = NewMaterialExpressionFunctionInput(Function, TEXT("special_row_width_factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* SpecialColumnIndex = NewMaterialExpressionFunctionInput(Function, TEXT("special_column_index"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* SpecialColumnHeightFactor = NewMaterialExpressionFunctionInput(Function, TEXT("special_column_height_factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* OddRowOffset = NewMaterialExpressionFunctionInput(Function, TEXT("odd_row_offset"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* RandomRowOffset = NewMaterialExpressionFunctionInput(Function, TEXT("random_row_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    //!! magic, looks good with this value, has to be dependent on the incoming grout spacing as otherwise problems/aliasing with sampling the bump values
    UMaterialExpressionMultiply* Delta = NewMaterialExpressionMultiply(Function, 0.5f, NewMaterialExpressionAdd(Function, GroutWidth, GroutHeight));

    UMaterialExpressionComponentMask* BasePosition = NewMaterialExpressionComponentMask(Function, UVWPosition, 3);
    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, ImporterEvalTileFunction,
    {
        BasePosition,
        NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f),
        NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f),
        TileBrightnessVariation,
        MissingTileAmount,
        Seed,
        NumberOfRows,
        NumberOfColumns,
        OddRowOffset,
        RandomRowOffset,
        GroutWidth,
        GroutHeight,
        GroutRoughness,
        SpecialColumnIndex,
        SpecialRowIndex,
        SpecialColumnHeightFactor,
        SpecialRowWidthFactor
    });

    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, ImporterEvalTileFunction,
    {
        NewMaterialExpressionAdd(Function, BasePosition, NewMaterialExpressionAppendVector(Function, Delta, 0.0f)),
        NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f),
        NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f),
        TileBrightnessVariation,
        MissingTileAmount,
        Seed,
        NumberOfRows,
        NumberOfColumns,
        OddRowOffset,
        RandomRowOffset,
        GroutWidth,
        GroutHeight,
        GroutRoughness,
        SpecialColumnIndex,
        SpecialRowIndex,
        SpecialColumnHeightFactor,
        SpecialRowWidthFactor
    });

    UMaterialExpressionMaterialFunctionCall* Result2 = NewMaterialExpressionFunctionCall(Function, ImporterEvalTileFunction,
    {
        NewMaterialExpressionAdd(Function, BasePosition, NewMaterialExpressionAppendVector(Function, 0.0f, Delta)),
        NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f),
        NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f),
        TileBrightnessVariation,
        MissingTileAmount,
        Seed,
        NumberOfRows,
        NumberOfColumns,
        OddRowOffset,
        RandomRowOffset,
        GroutWidth,
        GroutHeight,
        GroutRoughness,
        SpecialColumnIndex,
        SpecialRowIndex,
        SpecialColumnHeightFactor,
        SpecialRowWidthFactor
    });

    UMaterialExpressionMultiply* ContrastFactor =
        NewMaterialExpressionMultiply(Function,
            Factor,
            NewMaterialExpressionAbs(Function, NewMaterialExpressionFunctionCall(Function, MathLuminance, { { Result0, 1 } })));

    UMaterialExpressionNormalize* CalculatedNormal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                Normal,
                NewMaterialExpressionMultiply(Function, { UVWTangentU, NewMaterialExpressionSubtract(Function,{ Result1, 0 },{ Result0, 0 }), ContrastFactor }),
                NewMaterialExpressionMultiply(Function, { UVWTangentV, NewMaterialExpressionSubtract(Function,{ Result2, 0 },{ Result0, 0 }), ContrastFactor })
            }));
    UMaterialExpressionIf* Result = NewMaterialExpressionIfEqual(Function, Factor, 0.0f, Normal, CalculatedNormal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), Result);
}

static void Generator_BaseTransformCoordinate(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathMultiplyFloat4x4Float4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Transform a texture coordinate by a matrix.");

    UMaterialExpressionFunctionInput* Transform0 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_0"), EFunctionInputType::FunctionInput_Vector4, { 1.0f, 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Transform1 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_1"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 1.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Transform2 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_2"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 1.0f, 0.0f });
    UMaterialExpressionFunctionInput* Transform3 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_3"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CoordinatePosition = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* CoordinateTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* CoordinateTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif

    UMaterialExpressionAppendVector* ExtendedPosition = NewMaterialExpressionAppendVector(Function, CoordinatePosition, 1.0f);
    UMaterialExpressionComponentMask* Position =
        NewMaterialExpressionComponentMask(Function,
            NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { Transform0, Transform1, Transform2, Transform3, ExtendedPosition }), 7);

    UMaterialExpressionAppendVector* ExtendedTangentU = NewMaterialExpressionAppendVector(Function, CoordinateTangentU, 0.0f);
    UMaterialExpressionNormalize* TangentU =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionComponentMask(Function,
                NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { Transform0, Transform1, Transform2, Transform3, ExtendedTangentU }), 7));

    UMaterialExpressionAppendVector* ExtendedTangentV = NewMaterialExpressionAppendVector(Function, CoordinateTangentV, 0.0f);
    UMaterialExpressionNormalize* TangentV =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionComponentMask(Function,
                NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { Transform0, Transform1, Transform2, Transform3, ExtendedTangentV }), 7));

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionAppendVector* ExtendedPositionXY = NewMaterialExpressionAppendVector(Function, WorldAlignedPositionXY, 0.0f, 1.0f);
    UMaterialExpressionComponentMask* PositionXY =
        NewMaterialExpressionComponentMask(Function,
            NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { Transform0, Transform1, Transform2, Transform3, ExtendedPositionXY }), 3);

    UMaterialExpressionAppendVector* ExtendedPositionXZ = NewMaterialExpressionAppendVector(Function, WorldAlignedPositionXZ, 0.0f, 1.0f);
    UMaterialExpressionComponentMask* PositionXZ =
        NewMaterialExpressionComponentMask(Function,
            NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { Transform0, Transform1, Transform2, Transform3, ExtendedPositionXZ }), 3);

    UMaterialExpressionAppendVector* ExtendedPositionYZ = NewMaterialExpressionAppendVector(Function, WorldAlignedPositionYZ, 0.0f, 1.0f);
    UMaterialExpressionComponentMask* PositionYZ =
        NewMaterialExpressionComponentMask(Function,
            NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { Transform0, Transform1, Transform2, Transform3, ExtendedPositionYZ }), 3);
#endif

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TangentU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), TangentV);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xy"), PositionXY);
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_xz"), PositionXZ);
    NewMaterialExpressionFunctionOutput(Function, TEXT("world_aligned_position_yz"), PositionYZ);
#endif
}

static void Generator_BaseVolumeCoefficient(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathLog = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_log_float3"));

    Function->Description = TEXT("Compute a volume coefficient based on distance and value.");

    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Distance = NewMaterialExpressionFunctionInput(Function, TEXT("distance"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionIf* Coefficient = NewMaterialExpressionIfGreater(Function, Distance, 0.0f,
        NewMaterialExpressionNegate(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionFunctionCall(Function, MathLog, { Tint }), Distance)),
        { 0.0f, 0.0f, 0.0f });

    NewMaterialExpressionFunctionOutput(Function, TEXT("coefficient"), Coefficient);
}

static void Generator_BaseWorleyNoiseBumpTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterApplyNoiseModifications = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_apply_noise_modifications"));
    UMaterialFunction* ImporterWorleyNoiseExt = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_worley_noise_ext"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Bump-mapping Worley noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Metric = NewMaterialExpressionFunctionInput(Function, TEXT("metric"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ApplyMarble = NewMaterialExpressionFunctionInput(Function, TEXT("apply_marble"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* ApplyDent = NewMaterialExpressionFunctionInput(Function, TEXT("apply_dent"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoiseDistortion = NewMaterialExpressionFunctionInput(Function, TEXT("noise_distortion"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* NoiseThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_high"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_low"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseBands = NewMaterialExpressionFunctionInput(Function, TEXT("noise_bands"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* StepThreshold = NewMaterialExpressionFunctionInput(Function, TEXT("step_threshold"), EFunctionInputType::FunctionInput_Scalar, 0.2f);
    UMaterialExpressionFunctionInput* Edge = NewMaterialExpressionFunctionInput(Function, TEXT("edge"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionDivide* Delta = NewMaterialExpressionDivide(Function, NewMaterialExpressionMultiply(Function, 0.1f, Size), NoiseBands);

    UMaterialExpressionDivide* ScaledPosition0 = NewMaterialExpressionDivide(Function, UVWPosition, Size);
    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoiseExt,{ ScaledPosition0, NoiseDistortion, StepThreshold, Mode, Metric,{} }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition0, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionDivide* ScaledPosition1 =
        NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Delta, 0.0f, 0.0f })), Size);
    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoiseExt,{ ScaledPosition1, NoiseDistortion, StepThreshold, Mode, Metric,{} }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition1, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionDivide* ScaledPosition2 =
        NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 0.0f, Delta, 0.0f })), Size);
    UMaterialExpressionMaterialFunctionCall* Result2 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoiseExt,{ ScaledPosition2, NoiseDistortion, StepThreshold, Mode, Metric,{} }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition2, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionDivide* ScaledPosition3 =
        NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, UVWPosition, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 0.0f, 0.0f, Delta })), Size);
    UMaterialExpressionMaterialFunctionCall* Result3 = NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
    {
        NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoiseExt,{ ScaledPosition3, NoiseDistortion, StepThreshold, Mode, Metric,{} }),
        NewMaterialExpressionComponentMask(Function, ScaledPosition3, 1),
        ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
    });

    UMaterialExpressionSubtract* BumpFactor = NewMaterialExpressionNegate(Function, Factor);
    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfEqual(Function, Factor, 0.0f,
            Normal,
            NewMaterialExpressionIfEqual(Function, Size, 0.0f,
                Normal,
                NewMaterialExpressionNormalize(Function,
                    NewMaterialExpressionAdd(Function,
                    {
                        NewMaterialExpressionMultiply(Function,
                            Normal,
                            NewMaterialExpressionAdd(Function,
                                NewMaterialExpressionAbs(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, Result3, Result0), BumpFactor)),
                                1.0f)),
                        NewMaterialExpressionMultiply(Function, { UVWTangentU, NewMaterialExpressionSubtract(Function, Result1, Result0), BumpFactor }),
                        NewMaterialExpressionMultiply(Function, { UVWTangentV, NewMaterialExpressionSubtract(Function, Result2, Result0), BumpFactor })
                    }))));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_BaseWorleyNoiseTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterApplyNoiseModifications = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_apply_noise_modifications"));
    UMaterialFunction* ImporterWorleyNoiseExt = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_worley_noise_ext"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Color Worley noise.");

    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif
    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Metric = NewMaterialExpressionFunctionInput(Function, TEXT("metric"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ApplyMarble = NewMaterialExpressionFunctionInput(Function, TEXT("apply_marble"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* ApplyDent = NewMaterialExpressionFunctionInput(Function, TEXT("apply_dent"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoiseDistortion = NewMaterialExpressionFunctionInput(Function, TEXT("noise_distortion"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* NoiseThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_high"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_low"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseBands = NewMaterialExpressionFunctionInput(Function, TEXT("noise_bands"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* StepThreshold = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold"), EFunctionInputType::FunctionInput_Scalar, 0.2f);
    UMaterialExpressionFunctionInput* Edge = NewMaterialExpressionFunctionInput(Function, TEXT("edge"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionDivide* ScaledPosition = NewMaterialExpressionDivide(Function, UVWPosition, Size);
    UMaterialExpressionIf* Alpha =
        NewMaterialExpressionIfEqual(Function, Size, 0.0f,
            0.0f,
            NewMaterialExpressionFunctionCall(Function, ImporterApplyNoiseModifications,
            {
                NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoiseExt,{ ScaledPosition, NoiseDistortion, StepThreshold, Mode, Metric,{} }),
                NewMaterialExpressionComponentMask(Function, ScaledPosition, 1),
                ApplyMarble, ApplyDent, NoiseThresholdHigh, NoiseThresholdLow, NoiseBands
            }));
    UMaterialExpressionLinearInterpolate* Tint = NewMaterialExpressionLinearInterpolate(Function, Color1, Color2, Alpha);
    UMaterialExpressionMaterialFunctionCall* Mono = NewMaterialExpressionFunctionCall(Function, MathLuminance, { Tint });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), Tint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_DFAnisotropicVDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Volume light distribution with directional bias.");

    UMaterialExpressionFunctionInput* DirectionalBias = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    //UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    // Needs to determine some meaningfull stuff for a VDF !!
    UMaterialExpressionMaterialFunctionCall* VDF = NewMaterialExpressionFunctionCall(Function, MakeFloat4, { DirectionalBias, DirectionalBias, DirectionalBias, DirectionalBias });

    NewMaterialExpressionFunctionOutput(Function, TEXT("vdf"), VDF);
}

static void Generator_DFBackscatteringGlossyReflectionBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Backscattering glossy reflection.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionSquareRoot* Roughness = NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, RoughnessU, RoughnessV), 2.0f));
    UMaterialExpressionTransform* BackscatteringNormal = NewMaterialExpressionTransform(Function, NewMaterialExpressionCameraVectorWS(Function), TRANSFORMSOURCE_World, TRANSFORM_Tangent);

    UMaterialExpressionMakeMaterialAttributes* BSDF = NewMaterialExpressionMakeMaterialAttributes(Function, Tint, 1.0f, 0.0f, Roughness, {}, {}, {}, BackscatteringNormal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFColorFresnelLayer(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterBlendClearCoat = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_blend_clear_coat"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Add an elemental or compound BSDF as a layer on top of another elemental or compound BSDF according to weight and a Fresnel term using a dedicated index of refraction for the layer.The base is weighted with color(1) - (weight*fresnel(ior)).");

    UMaterialExpression* DefaultMaterialAttributes = NewMaterialExpressionMakeMaterialAttributes(Function);

    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Weight = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Layer = NewMaterialExpressionFunctionInput(Function, TEXT("layer"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* ScalarIOR = NewMaterialExpressionFunctionCall(Function, MathLuminance, { IOR });

    // Use Schlick's approximation for the Exponent: https://en.wikipedia.org/wiki/Schlick%27s_approximation
    UMaterialExpressionMultiply* BaseReflectFraction =
        NewMaterialExpressionSquare(Function,
            NewMaterialExpressionDivide(Function, NewMaterialExpressionOneMinus(Function, ScalarIOR), NewMaterialExpressionAdd(Function, 1.0f, ScalarIOR)));

    UMaterialExpressionMultiply* Alpha =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionClamp(Function, NewMaterialExpressionFunctionCall(Function, MathLuminance, { Weight }), 0.0f, 1.0f),
            NewMaterialExpressionFresnel(Function, 5.0f, BaseReflectFraction, NewMaterialExpressionTransform(Function, Normal, TRANSFORMSOURCE_Tangent, TRANSFORM_World)));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, ImporterBlendClearCoat, { Base, Layer, Alpha, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFCustomCurveLayer(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* ImporterBlendClearCoat = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_blend_clear_coat"));

    Function->Description = TEXT("BSDF as a layer on top of another elemental or compound BSDF according to weight and a Schlick-style directional - dependent curve function.The base is weighted with 1 - (weight*curve()).");

    UMaterialExpression* DefaultMaterialAttributes = NewMaterialExpressionMakeMaterialAttributes(Function);

    UMaterialExpressionFunctionInput* NormalReflectivity = NewMaterialExpressionFunctionInput(Function, TEXT("normal_reflectivity"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* GrazingReflectivity = NewMaterialExpressionFunctionInput(Function, TEXT("grazing_reflectivity"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Exponent = NewMaterialExpressionFunctionInput(Function, TEXT("exponent"), EFunctionInputType::FunctionInput_Scalar, 5.0f);
    UMaterialExpressionFunctionInput* Weight = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Layer = NewMaterialExpressionFunctionInput(Function, TEXT("layer"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionClamp* ClampedWeight = NewMaterialExpressionClamp(Function, Weight, 0.0f, 1.0f);

    UMaterialExpressionFresnel* Fresnel = NewMaterialExpressionFresnel(Function, Exponent, 0.0f, NewMaterialExpressionTransform(Function, Normal, TRANSFORMSOURCE_Tangent, TRANSFORM_World));
    UMaterialExpressionLinearInterpolate* Reflectivity = NewMaterialExpressionLinearInterpolate(Function, NormalReflectivity, GrazingReflectivity, Fresnel);
    UMaterialExpressionMultiply* Alpha = NewMaterialExpressionMultiply(Function, ClampedWeight, Reflectivity);

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, ImporterBlendClearCoat, { Base, Layer, Alpha, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFDiffuseEDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Uniform light emission in all directions.");

    UMaterialExpressionMakeMaterialAttributes* EDF = NewMaterialExpressionMakeMaterialAttributes(Function, {}, {}, 0.0f, {}, { 1.0f, 1.0f, 1.0f });

    NewMaterialExpressionFunctionOutput(Function, TEXT("edf"), EDF);
}

static void Generator_DFDiffuseReflectionBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Lambertian reflection extended by the Oren-Nayar microfacet model.");

    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Roughness = NewMaterialExpressionFunctionInput(Function, TEXT("roughness"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMakeMaterialAttributes* BSDF = NewMaterialExpressionMakeMaterialAttributes(Function, Tint, 0.0f, 0.0f, 1.0f, {}, 1.0f, {}, Normal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFDiffuseTransmissionBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Pure diffuse transmission of light through a surface.");

    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMakeMaterialAttributes* BSDF = NewMaterialExpressionMakeMaterialAttributes(Function, Tint, {}, 0.0f, {}, {}, {}, {}, Normal, {}, {}, {}, Tint);

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFDirectionalFactor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Directional modifier.");

    UMaterialExpressionFunctionInput* NormalTint = NewMaterialExpressionFunctionInput(Function, TEXT("normal_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* GrazingTint = NewMaterialExpressionFunctionInput(Function, TEXT("grazing_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Exponent = NewMaterialExpressionFunctionInput(Function, TEXT("exponent"), EFunctionInputType::FunctionInput_Scalar, 5.0f);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes, NewMaterialExpressionMakeMaterialAttributes(Function));
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionLinearInterpolate* NewBaseColor = NewMaterialExpressionLinearInterpolate(Function, NormalTint, GrazingTint, NewMaterialExpressionFresnel(Function, Exponent, {}, {}));
    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_MultiplyBaseColor")), { Base, NewBaseColor,{} });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), BSDF);
}

static void Generator_DFFresnelLayer(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    Function->Description = TEXT("Add an elemental or compound BSDF as a layer on top of another elemental or compound BSDF according to weight and a Fresnel term using a dedicated index of refraction for the layer");

    UMaterialExpression* DefaultMaterialAttributes = NewMaterialExpressionMakeMaterialAttributes(Function);

    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Weight = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Layer = NewMaterialExpressionFunctionInput(Function, TEXT("layer"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    // Use Schlick's approximation for the Exponent: https://en.wikipedia.org/wiki/Schlick%27s_approximation
    UMaterialExpressionMultiply* BaseReflectFraction =
        NewMaterialExpressionSquare(Function,
            NewMaterialExpressionDivide(Function, NewMaterialExpressionOneMinus(Function, IOR), NewMaterialExpressionAdd(Function, 1.0f, IOR)));

    UMaterialExpressionMultiply* Alpha =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionClamp(Function, Weight, 0.0f, 1.0f),
            NewMaterialExpressionFresnel(Function, 5.0f, BaseReflectFraction, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Simple")), { Base, Layer, Alpha });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFLightProfileMaximum(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the maximumintensity in this light profile. A lookup on an invalid light profile reference returns zero.");

    // dummy implementation!
    // we just always return 1.0f
    UMaterialExpressionFunctionInput* Profile = NewMaterialExpressionFunctionInput(Function, TEXT("profile"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));

    NewMaterialExpressionFunctionOutput(Function, TEXT("maximum"), 1.0f);
}

static void Generator_DFLightProfilePower(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the power emitted by this light profile. A lookup on an invalid light profile reference returns zero.");

    // dummy implementation!
    // we just always return 1.0f
    UMaterialExpressionFunctionInput* Profile = NewMaterialExpressionFunctionInput(Function, TEXT("profile"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));

    NewMaterialExpressionFunctionOutput(Function, TEXT("power"), 1.0f);
}

static void Generator_DFMeasuredBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("General isotropic reflection and transmission based on measured data.");

    // a bsdf_measurement essentially is a texture, that needs special accessing functions!
    // 'til its implemented, we simply use a color here, which is totally wrong, but we survive that way
    UMaterialExpressionFunctionInput* Measurement = NewMaterialExpressionFunctionInput(Function, TEXT("measurement"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Multiplier = NewMaterialExpressionFunctionInput(Function, TEXT("multiplier"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMultiply* BaseColor = NewMaterialExpressionMultiply(Function, Measurement, Multiplier);

    UMaterialExpressionFresnel* Fresnel = NewMaterialExpressionFresnel(Function, 5.0f, {}, {});
    UMaterialExpressionIf* Opacity = NewMaterialExpressionSwitch(Function, Mode, { 1.0f, Fresnel, Fresnel });

    UMaterialExpressionMakeMaterialAttributes* BSDF = NewMaterialExpressionMakeMaterialAttributes(Function, BaseColor, {}, {}, {}, {}, Opacity, {}, Normal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFMeasuredEDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Light distribution defined by a profile.");

    // dummy implementation!
    // we just return a uniform light emission in all directions
    UMaterialExpressionFunctionInput* Profile = NewMaterialExpressionFunctionInput(Function, TEXT("profile"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Multiplier = NewMaterialExpressionFunctionInput(Function, TEXT("multiplier"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* GlobalDistribution = NewMaterialExpressionFunctionInput(Function, TEXT("global_distribution"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* GlobalFrame_0 = NewMaterialExpressionFunctionInput(Function, TEXT("global_frame_0"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* GlobalFrame_1 = NewMaterialExpressionFunctionInput(Function, TEXT("global_frame_1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 1.0f, 0.0f });
    UMaterialExpressionFunctionInput* GlobalFrame_2 = NewMaterialExpressionFunctionInput(Function, TEXT("global_frame_2"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));

    UMaterialExpressionMakeMaterialAttributes* EDF = NewMaterialExpressionMakeMaterialAttributes(Function, {}, {}, 0.0f, {}, { 1.0f, 1.0f, 1.0f });

    NewMaterialExpressionFunctionOutput(Function, TEXT("edf"), EDF);
}

static void Generator_DFMeasuredCurveFactor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    check(ArrayInputSizes.Num() > 0 && 0 < ArrayInputSizes[0]);

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Modifier weighting a base BSDF based on a measured reflection curve.");

    TArray<UMaterialExpressionFunctionInput*> Colors;
    int32 ArraySize = ArrayInputSizes[0];
    for (int32 i = 0; i < ArraySize; i++)
    {
        Colors.Add(NewMaterialExpressionFunctionInput(Function, TEXT("color"), EFunctionInputType::FunctionInput_Vector3));
    }
    if (1 < ArraySize)
    {
        for (int32 i = 0; i < ArraySize; i++)
        {
            Colors[i]->InputName += TEXT("_") + FString::FromInt(i);
        }
    }
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("bsdf"), EFunctionInputType::FunctionInput_MaterialAttributes, NewMaterialExpressionMakeMaterialAttributes(Function));
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpression* Color;
    if (1 == ArraySize)
    {
        Color = Colors[0];
    }
    else
    {
        UMaterialExpressionMultiply* Z =
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionArccosine(Function,
                    NewMaterialExpressionClamp(Function,
                        NewMaterialExpressionDotProduct(Function,
                            NewMaterialExpressionTransform(Function, Normal, TRANSFORMSOURCE_Tangent, TRANSFORM_World),
                            NewMaterialExpressionCameraVectorWS(Function)),
                        0.0f,
                        1.0f)),
                ArraySize * 2.0f / PI);		// multiply with the number of curveValues, to get an index

        TArray<FMaterialExpressionConnection> ColorConnections;
        ColorConnections.SetNum(Colors.Num());
        for (int32 i = 0; i < Colors.Num(); i++)
        {
            ColorConnections[i] = { Colors[i] };
        }

        Color = NewMaterialExpressionLinearInterpolate(Function, ColorConnections, Z);
    }

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_MultiplyBaseColor")), { Base, Color,{} });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFMicrofacetBeckmannSmithBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DFSimpleGlossyBSDF = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_df_simple_glossy_bsdf"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Glossy reflection and transmission.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, DFSimpleGlossyBSDF, { RoughnessU, RoughnessV, Tint, MultiScatterTint, TangentU, Mode, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFMicrofacetBeckmannVCavitiesBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DFSimpleGlossyBSDF = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_df_simple_glossy_bsdf"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Glossy reflection and transmission.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, DFSimpleGlossyBSDF, { RoughnessU, RoughnessV, Tint, MultiScatterTint, TangentU, Mode, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFMicrofacetGGXSmithBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DFSimpleGlossyBSDF = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_df_simple_glossy_bsdf"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Glossy reflection and transmission.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, DFSimpleGlossyBSDF, { RoughnessU, RoughnessV, Tint, MultiScatterTint, TangentU, Mode, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFMicrofacetGGXVCavitiesBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DFSimpleGlossyBSDF = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_df_simple_glossy_bsdf"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Glossy reflection and transmission.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, DFSimpleGlossyBSDF, { RoughnessU, RoughnessV, Tint, MultiScatterTint, TangentU, Mode, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static TPair<UMaterialExpression*, UMaterialExpression*> MixAttributesRecursive(UMaterialFunction* Function, UMaterialFunction* MatLayerBlend_Standard,
    const TArray<UMaterialExpressionClamp*>& Weights, const TArray<UMaterialExpressionFunctionInput*>& Components, int32 Begin, int32 End)
{
    check(Begin < End);

    int32 Count = End - Begin;
    if (Count == 1)
    {
        return TPair<UMaterialExpression*, UMaterialExpression*>(Weights[Begin], Components[Begin]);
    }
    else
    {
        int32 Center = (Begin + End) / 2;
        TPair<UMaterialExpression*, UMaterialExpression*> LeftMix = MixAttributesRecursive(Function, MatLayerBlend_Standard, Weights, Components, Begin, Center);
        TPair<UMaterialExpression*, UMaterialExpression*> RightMix = MixAttributesRecursive(Function, MatLayerBlend_Standard, Weights, Components, Center, End);
        UMaterialExpressionAdd* WeightSum = NewMaterialExpressionAdd(Function, LeftMix.Key, RightMix.Key);
        UMaterialExpressionDivide* NormalizedWeight = NewMaterialExpressionDivide(Function, LeftMix.Key, NewMaterialExpressionClamp(Function, WeightSum, SMALL_NUMBER, Count));
        UMaterialExpressionMaterialFunctionCall* ComponentMix = NewMaterialExpressionFunctionCall(Function, MatLayerBlend_Standard, { RightMix.Value, LeftMix.Value, NormalizedWeight });
        return TPair<UMaterialExpression*, UMaterialExpression*>(WeightSum, ComponentMix);
    }
}

static TPair<UMaterialExpressionClamp*, UMaterialExpression*> MixAttributes(UMaterialFunction* Function, UMaterialFunction* MatLayerBlend_Standard,
    const TArray<UMaterialExpressionClamp*>& Weights, const TArray<UMaterialExpressionFunctionInput*>& Components)
{
    TPair<UMaterialExpression*, UMaterialExpression*> Mix = MixAttributesRecursive(Function, MatLayerBlend_Standard, Weights, Components, 0, Weights.Num());
    return TPair<UMaterialExpressionClamp*, UMaterialExpression*>(NewMaterialExpressionClamp(Function, Mix.Key, 0.0f, 1.0f), Mix.Value);
}

static void Generator_DFNormalizedMix(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    check(ArrayInputSizes.Num() > 0 && 0 < ArrayInputSizes[0]);

    UMaterialFunction* MatLayerBlend_Standard = LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Standard"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Mix N elemental or compound distribution functions based on the weights defined in the components. If the sum of the weights exceeds 1.0, they are normalized.");

    TArray<UMaterialExpressionFunctionInput*> Weights, Components;
    int32 ArraySize = ArrayInputSizes[0];
    for (int32 i = 0; i < ArraySize; i++)
    {
        Weights.Add(NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar, 0.0f));
        Components.Add(NewMaterialExpressionFunctionInput(Function, TEXT("component"), EFunctionInputType::FunctionInput_MaterialAttributes, NewMaterialExpressionMakeMaterialAttributes(Function)));
    }
    if (1 < ArraySize)
    {
        for (int32 i = 0; i < ArraySize; i++)
        {
            FString Appendix = TEXT("_") + FString::FromInt(i);
            Weights[i]->InputName += Appendix;
            Components[i]->InputName += Appendix;
        }
    }
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    TArray<UMaterialExpressionClamp*> ClampedWeights;
    for (int32 i = 0; i < ArraySize; i++)
    {
        ClampedWeights.Add(NewMaterialExpressionClamp(Function, Weights[i], 0.0f, 1.0f));
    }

    TPair<UMaterialExpressionClamp*, UMaterialExpression*> Mix = MixAttributes(Function, MatLayerBlend_Standard, ClampedWeights, Components);

    UMaterialExpressionMakeMaterialAttributes* DefaultBSDF = NewMaterialExpressionMakeMaterialAttributes(Function, { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f);
    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, MatLayerBlend_Standard, { DefaultBSDF, Mix.Value, Mix.Key });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFSimpleGlossyBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Glossy reflection and transmission.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    // assume material to be metallic, if the mode is scatter_reflect (opaque!) and the color different from (1,1,1)
    UMaterialExpressionSquareRoot* Roughness = NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, RoughnessU, RoughnessV), 2.0f));

    UMaterialExpressionLinearInterpolate* Fresnel = NewMaterialExpressionLinearInterpolate(Function, 0.1f, 1.0f, NewMaterialExpressionFresnel(Function, 5.0f, {}, {}));
    UMaterialExpressionIf* Opacity = NewMaterialExpressionSwitch(Function, Mode, { 1.0f, Fresnel, Fresnel });

    UMaterialExpressionMakeMaterialAttributes* BSDF = NewMaterialExpressionMakeMaterialAttributes(Function, Tint, 1.0f, 0.0f, Roughness, {}, Opacity, {}, Normal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFSpecularBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Specular reflections and transmissions.");

    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, scatter_reflect);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    // This, using Material->IOR, instead of IOR would be right for BaseReflectionFraction in Fresnel !
    //UMaterialExpressionMultiply* BaseReflectFraction =
    //	NewMaterialExpressionSquare(Function,
    //		NewMaterialExpressionDivide(Function, NewMaterialExpressionOneMinus(Function, IOR), NewMaterialExpressionAdd(Function, 1.0f, IOR)));

    UMaterialExpressionFresnel* Fresnel = NewMaterialExpressionFresnel(Function, 5.0f, {}, {});
    UMaterialExpressionIf* Opacity = NewMaterialExpressionSwitch(Function, Mode, { 1.0f, Fresnel, Fresnel });

    UMaterialExpressionMakeMaterialAttributes* BSDF = NewMaterialExpressionMakeMaterialAttributes(Function, Tint, 1.0f, 0.0f, 0.0f, {}, Opacity, {}, Normal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFSpotEDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Exponentiated cosine weighting for spotlight. The spot light is oriented along the positive z-axis.");

    // dummy implementation!
    // we just return a uniform light emission in all directions
    UMaterialExpressionFunctionInput* Exponent = NewMaterialExpressionFunctionInput(Function, TEXT("exponent"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Spread = NewMaterialExpressionFunctionInput(Function, TEXT("spread"), EFunctionInputType::FunctionInput_Scalar, PI);
    UMaterialExpressionFunctionInput* GlobalDistribution = NewMaterialExpressionFunctionInput(Function, TEXT("global_distribution"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* GlobalFrame_0 = NewMaterialExpressionFunctionInput(Function, TEXT("global_frame_0"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* GlobalFrame_1 = NewMaterialExpressionFunctionInput(Function, TEXT("global_frame_1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 1.0f, 0.0f });
    UMaterialExpressionFunctionInput* GlobalFrame_2 = NewMaterialExpressionFunctionInput(Function, TEXT("global_frame_2"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 1.0f });

    UMaterialExpressionMakeMaterialAttributes* EDF = NewMaterialExpressionMakeMaterialAttributes(Function, {}, {}, 0.0f, {}, { 1.0f, 1.0f, 1.0f });

    NewMaterialExpressionFunctionOutput(Function, TEXT("edf"), EDF);
}

static void Generator_DFThinFilm(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* MathSine3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_sin_float3"));
    UMaterialFunction* Refract = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_refract"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    UMaterialExpressionFunctionInput* Thickness = NewMaterialExpressionFunctionInput(Function, TEXT("thickness"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    // get the refraction vector T, and check for total internal reflection
    UMaterialExpressionCameraVectorWS* ViewDir = NewMaterialExpressionCameraVectorWS(Function);
    UMaterialExpressionMaterialFunctionCall* T0 = NewMaterialExpressionFunctionCall(Function, Refract, { ViewDir, Normal, NewMaterialExpressionComponentMask(Function, IOR, 1) });
    UMaterialExpressionMaterialFunctionCall* T1 = NewMaterialExpressionFunctionCall(Function, Refract, { ViewDir, Normal, NewMaterialExpressionComponentMask(Function, IOR, 2) });
    UMaterialExpressionMaterialFunctionCall* T2 = NewMaterialExpressionFunctionCall(Function, Refract, { ViewDir, Normal, NewMaterialExpressionComponentMask(Function, IOR, 4) });

    // angle between negative stateNormal and refraction vector T
    UMaterialExpressionSubtract* NegativeNormal = NewMaterialExpressionNegate(Function, Normal);
    UMaterialExpressionMaterialFunctionCall* CosTheta =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionDotProduct(Function, NegativeNormal, T0),
            NewMaterialExpressionDotProduct(Function, NegativeNormal, T1),
            NewMaterialExpressionDotProduct(Function, NegativeNormal, T2)
        });

    // wavelength of rgb found at http://en.wikipedia.org/wiki/Visible_spectrum
    UMaterialExpressionConstant3Vector* OneOverLambda = NewMaterialExpressionConstant(Function, 1.0f / 685.0f, 1.0f / 533.0f, 1.0f / 473.0f);

    // optical path difference, as found at https://en.wikipedia.org/wiki/Thin-film_interference in multiples of wavelengths
    UMaterialExpressionMultiply* OPD = NewMaterialExpressionMultiply(Function, { 2.0f, IOR, Thickness, CosTheta, OneOverLambda });

    // adjust for total reflection
    UMaterialExpressionComponentMask* OPDx = NewMaterialExpressionComponentMask(Function, OPD, 1);
    UMaterialExpressionComponentMask* OPDy = NewMaterialExpressionComponentMask(Function, OPD, 2);
    UMaterialExpressionComponentMask* OPDz = NewMaterialExpressionComponentMask(Function, OPD, 4);
    UMaterialExpressionMaterialFunctionCall* M = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
    {
        NewMaterialExpressionIfEqual(Function, OPDx, 0.0f, 0.5f, OPDx),
        NewMaterialExpressionIfEqual(Function, OPDy, 0.0f, 0.5f, OPDy),
        NewMaterialExpressionIfEqual(Function, OPDz, 0.0f, 0.5f, OPDz)
    });

    // with 1 < ior, we have a face shift of 180 degree at the upper boundary of the film, then,
    // with fract(m) == 0.0, we have destructive interference
    // with fract(m) == 0.5, we have constructive interference
    UMaterialExpressionFrac* PD = NewMaterialExpressionFrac(Function, M);																								// range [0.0, 1.0)
    UMaterialExpressionMaterialFunctionCall* Modulate = NewMaterialExpressionFunctionCall(Function, MathSine3, { NewMaterialExpressionMultiply(Function, PI, PD) });	// range [0.0, 1.0), with maximum and PD == 0.5

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Tint")), { Base, Modulate,{} });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFTint(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Tint the result of an elemental or compound distribution function with an additional color.");

    UMaterialExpressionFunctionInput* Color = NewMaterialExpressionFunctionInput(Function, TEXT("color"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Tint")), { Base, Color,{} });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFWardGeislerMoroderBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DFSimpleGlossyBSDF = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_df_simple_glossy_bsdf"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));

    Function->Description = TEXT("Glossy reflection and transmission.");

    UMaterialExpressionFunctionInput* RoughnessU = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RoughnessV = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_v"), EFunctionInputType::FunctionInput_Scalar, RoughnessU);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* MultiScatterTint = NewMaterialExpressionFunctionInput(Function, TEXT("multiscatter_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, DFSimpleGlossyBSDF, { RoughnessU, RoughnessV, Tint, MultiScatterTint, TangentU, scatter_reflect, Normal });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFWeightedLayer(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Add an elemental or compound BSDF as a layer on top of another elemental or compound BSDF according to weight.");

    UMaterialExpression* DefaultMaterialAttributes = NewMaterialExpressionMakeMaterialAttributes(Function);

    UMaterialExpressionFunctionInput* Weight = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Layer = NewMaterialExpressionFunctionInput(Function, TEXT("layer"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes, DefaultMaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionClamp* ClampedWeight = NewMaterialExpressionClamp(Function, Weight, 0.0f, 1.0f);

    UMaterialExpressionMaterialFunctionCall* AdjustedLayer = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_ReplaceNormals")), { Layer, Normal });
    UMaterialExpressionMaterialFunctionCall* BSDF = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Standard")), { Base, AdjustedLayer, ClampedWeight });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_DFChiangHairBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    Function->Description = TEXT("Aggregated reflective and transmissive scattering based on the Chiang et al.");

    UMaterialExpressionFunctionInput* ReflectionWeight = NewMaterialExpressionFunctionInput(Function, TEXT("diffuse_reflection_weight"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ReflectionTint = NewMaterialExpressionFunctionInput(Function, TEXT("diffuse_reflection_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* RoughnessR = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_R"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* RoughnessTT = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_TT"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* RoughnessTRT = NewMaterialExpressionFunctionInput(Function, TEXT("roughness_TRT"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* CuticleAngle = NewMaterialExpressionFunctionInput(Function, TEXT("cuticle_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AbsorptionCoefficient = NewMaterialExpressionFunctionInput(Function, TEXT("absorption_coefficient"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Scalar, 1.55f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionLinearInterpolate* BaseColor = NewMaterialExpressionLinearInterpolate(Function, 
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionOneMinus(Function, AbsorptionCoefficient), NewMaterialExpressionFresnel(Function, IOR)), 
        ReflectionTint, 
        ReflectionWeight);

    UMaterialExpressionMakeMaterialAttributes* HairBSDF = NewMaterialExpressionMakeMaterialAttributes(Function, 
        BaseColor, 
        0.0f, 
        NewMaterialExpressionComponentMask(Function, RoughnessR, 2), 
        NewMaterialExpressionComponentMask(Function, RoughnessR, 1), 
        {}, {}, {}, {}, 
        Normal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("hair_bsdf"), HairBSDF);
}

static void Generator_MathAverage(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the average of the vector elements.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionDivide* Average =
        NewMaterialExpressionDivide(Function,
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionComponentMask(Function, A, 1),
                NewMaterialExpressionComponentMask(Function, A, 2),
                NewMaterialExpressionComponentMask(Function, A, 4)
            }),
            3.0f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("average"), Average);
}

static void Generator_MathCosFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Computes the cosine of an angle in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionCosine* Cosine = NewMaterialExpressionCosine(Function, A);

    NewMaterialExpressionFunctionOutput(Function, TEXT("cos"), Cosine);
}

static void Generator_MathCosFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Computes the cosine of two angles in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMaterialFunctionCall* Cos =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 2))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("cos"), Cos);
}

static void Generator_MathCosFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Computes the cosine of three angles in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Cos =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 4))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("cos"), Cos);
}

static void Generator_MathCosFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Computes the cosine of four angles in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMaterialFunctionCall* Cos =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 4)),
            NewMaterialExpressionCosine(Function, NewMaterialExpressionComponentMask(Function, A, 8))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("cos"), Cos);
}

static void Generator_MathLogFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Computes the natural logarithm.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionMultiply* Ln = NewMaterialExpressionMultiply(Function, NewMaterialExpressionLogarithm2(Function, A), log(2.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("ln"), Ln);
}

static void Generator_MathLogFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Computes the natural logarithm.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMultiply* Ln =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2))
            }),
            log(2.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("ln"), Ln);
}

static void Generator_MathLogFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Computes the natural logarithm.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMultiply* Ln =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 4))
            }),
            log(2.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("ln"), Ln);
}

static void Generator_MathLogFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Computes the natural logarithm.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMultiply* Ln =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 4)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 8))
            }),
            log(2.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("ln"), Ln);
}

static void Generator_MathLog10Float(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Computes the logarithm to the base 10.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionMultiply* Log10 = NewMaterialExpressionMultiply(Function, NewMaterialExpressionLogarithm2(Function, A), log(10.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("log10"), Log10);
}

static void Generator_MathLog10Float2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Computes the logarithm to the base 10.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMultiply* Log10 =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2))
            }),
            log(10.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("log10"), Log10);
}

static void Generator_MathLog10Float3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Computes the logarithm to the base 10.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMultiply* Log10 =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 4))
            }),
            log(10.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("log10"), Log10);
}

static void Generator_MathLog10Float4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Computes the logarithm to the base 10.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMultiply* Log10 =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 4)),
                NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 8))
            }),
            log(10.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("log10"), Log10);
}

static void Generator_MathLog2Float(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Computes the logarithm to the base 2.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionLogarithm2* Log2 = NewMaterialExpressionLogarithm2(Function, A);

    NewMaterialExpressionFunctionOutput(Function, TEXT("log2"), Log2);
}

static void Generator_MathLog2Float2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Computes the logarithm to the base 2.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMaterialFunctionCall* Log2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("log2"), Log2);
}

static void Generator_MathLog2Float3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Computes the logarithm to the base 2.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Log2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 4))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("log2"), Log2);
}

static void Generator_MathLog2Float4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Computes the logarithm to the base 2.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMaterialFunctionCall* Log2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 4)),
            NewMaterialExpressionLogarithm2(Function, NewMaterialExpressionComponentMask(Function, A, 8))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("log2"), Log2);
}

static void Generator_MathLuminance(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("the luminance is equal to 0.212671 * a.x + 0.715160 * a.y + 0.072169 * a.z.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionDotProduct* Luminance = NewMaterialExpressionDotProduct(Function, A, { 0.212677f, 0.715160f, 0.072129f });

    NewMaterialExpressionFunctionOutput(Function, TEXT("luminance"), Luminance);
}

static void Generator_MathMaxValueFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{

    Function->Description = TEXT("Returns the largest value of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMax* MaxValue =
        NewMaterialExpressionMax(Function,
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2));

    NewMaterialExpressionFunctionOutput(Function, TEXT("max_value"), MaxValue);
}

static void Generator_MathMaxValueFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{

    Function->Description = TEXT("Returns the largest value of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMax* MaxValue =
        NewMaterialExpressionMax(Function,
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2),
            NewMaterialExpressionComponentMask(Function, A, 4));

    NewMaterialExpressionFunctionOutput(Function, TEXT("max_value"), MaxValue);
}

static void Generator_MathMaxValueFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{

    Function->Description = TEXT("Returns the largest value of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMax* MaxValue =
        NewMaterialExpressionMax(Function,
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2),
            NewMaterialExpressionComponentMask(Function, A, 4),
            NewMaterialExpressionComponentMask(Function, A, 8));

    NewMaterialExpressionFunctionOutput(Function, TEXT("max_value"), MaxValue);
}

static void Generator_MathMinValueFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{

    Function->Description = TEXT("Returns the smallest value of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMin* MinValue =
        NewMaterialExpressionMin(Function,
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2));

    NewMaterialExpressionFunctionOutput(Function, TEXT("min_value"), MinValue);
}

static void Generator_MathMinValueFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{

    Function->Description = TEXT("Returns the smallest value of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMin* MinValue =
        NewMaterialExpressionMin(Function,
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2),
            NewMaterialExpressionComponentMask(Function, A, 4));

    NewMaterialExpressionFunctionOutput(Function, TEXT("min_value"), MinValue);
}

static void Generator_MathMinValueFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{

    Function->Description = TEXT("Returns the smallest value of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMin* MinValue =
        NewMaterialExpressionMin(Function,
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2),
            NewMaterialExpressionComponentMask(Function, A, 4),
            NewMaterialExpressionComponentMask(Function, A, 8));

    NewMaterialExpressionFunctionOutput(Function, TEXT("min_value"), MinValue);
}

static void Generator_MathMultiplyFloat4x4Float4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Multiplies 4x4-matrix with 4-vector.");

    UMaterialExpressionFunctionInput* Column0 = NewMaterialExpressionFunctionInput(Function, TEXT("column0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Column1 = NewMaterialExpressionFunctionInput(Function, TEXT("column1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Column2 = NewMaterialExpressionFunctionInput(Function, TEXT("column2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Column3 = NewMaterialExpressionFunctionInput(Function, TEXT("column3"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMaterialFunctionCall* Row0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionComponentMask(Function, Column0, 1),
            NewMaterialExpressionComponentMask(Function, Column1, 1),
            NewMaterialExpressionComponentMask(Function, Column2, 1),
            NewMaterialExpressionComponentMask(Function, Column3, 1)
        });
    UMaterialExpressionMaterialFunctionCall* Row1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionComponentMask(Function, Column0, 2),
            NewMaterialExpressionComponentMask(Function, Column1, 2),
            NewMaterialExpressionComponentMask(Function, Column2, 2),
            NewMaterialExpressionComponentMask(Function, Column3, 2)
        });
    UMaterialExpressionMaterialFunctionCall* Row2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionComponentMask(Function, Column0, 4),
            NewMaterialExpressionComponentMask(Function, Column1, 4),
            NewMaterialExpressionComponentMask(Function, Column2, 4),
            NewMaterialExpressionComponentMask(Function, Column3, 4)
        });
    UMaterialExpressionMaterialFunctionCall* Row3 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionComponentMask(Function, Column0, 8),
            NewMaterialExpressionComponentMask(Function, Column1, 8),
            NewMaterialExpressionComponentMask(Function, Column2, 8),
            NewMaterialExpressionComponentMask(Function, Column3, 8)
        });
    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionDotProduct(Function, Row0, Vector),
            NewMaterialExpressionDotProduct(Function, Row1, Vector),
            NewMaterialExpressionDotProduct(Function, Row2, Vector),
            NewMaterialExpressionDotProduct(Function, Row3, Vector)
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_MathMultiplyFloat4Float4x4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Multiplies 4-vector with 4x4-matrix.");

    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Row0 = NewMaterialExpressionFunctionInput(Function, TEXT("row0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Row1 = NewMaterialExpressionFunctionInput(Function, TEXT("row1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Row2 = NewMaterialExpressionFunctionInput(Function, TEXT("row2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Row3 = NewMaterialExpressionFunctionInput(Function, TEXT("row3"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMaterialFunctionCall* Column0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 1),
                NewMaterialExpressionComponentMask(Function, Row1, 1),
                NewMaterialExpressionComponentMask(Function, Row2, 1),
                NewMaterialExpressionComponentMask(Function, Row3, 1)
            });
    UMaterialExpressionMaterialFunctionCall* Column1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 2),
                NewMaterialExpressionComponentMask(Function, Row1, 2),
                NewMaterialExpressionComponentMask(Function, Row2, 2),
                NewMaterialExpressionComponentMask(Function, Row3, 2)
            });
    UMaterialExpressionMaterialFunctionCall* Column2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 4),
                NewMaterialExpressionComponentMask(Function, Row1, 4),
                NewMaterialExpressionComponentMask(Function, Row2, 4),
                NewMaterialExpressionComponentMask(Function, Row3, 4)
            });
    UMaterialExpressionMaterialFunctionCall* Column3 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 8),
                NewMaterialExpressionComponentMask(Function, Row1, 8),
                NewMaterialExpressionComponentMask(Function, Row2, 8),
                NewMaterialExpressionComponentMask(Function, Row3, 8)
            });

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            {
                NewMaterialExpressionDotProduct(Function, Vector, Column0),
                NewMaterialExpressionDotProduct(Function, Vector, Column1),
                NewMaterialExpressionDotProduct(Function, Vector, Column2),
                NewMaterialExpressionDotProduct(Function, Vector, Column3)
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_MathMultiplyFloat4x4Float4x4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Multiplies two 4x4 matrices.");

    UMaterialExpressionFunctionInput* Matrix0Col0 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix0Col1 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix0Col2 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix0Col3 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col3"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix1Col0 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix1Col1 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix1Col2 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Matrix1Col3 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col3"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpression* Row0 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionComponentMask(Function, Matrix0Col0, 1),
        NewMaterialExpressionComponentMask(Function, Matrix0Col1, 1),
        NewMaterialExpressionComponentMask(Function, Matrix0Col2, 1),
        NewMaterialExpressionComponentMask(Function, Matrix0Col3, 1)
    });
    UMaterialExpression* Row1 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionComponentMask(Function, Matrix0Col0, 2),
        NewMaterialExpressionComponentMask(Function, Matrix0Col1, 2),
        NewMaterialExpressionComponentMask(Function, Matrix0Col2, 2),
        NewMaterialExpressionComponentMask(Function, Matrix0Col3, 2)
    });
    UMaterialExpression* Row2 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionComponentMask(Function, Matrix0Col0, 4),
        NewMaterialExpressionComponentMask(Function, Matrix0Col1, 4),
        NewMaterialExpressionComponentMask(Function, Matrix0Col2, 4),
        NewMaterialExpressionComponentMask(Function, Matrix0Col3, 4)
    });
    UMaterialExpression* Row3 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionComponentMask(Function, Matrix0Col0, 8),
        NewMaterialExpressionComponentMask(Function, Matrix0Col1, 8),
        NewMaterialExpressionComponentMask(Function, Matrix0Col2, 8),
        NewMaterialExpressionComponentMask(Function, Matrix0Col3, 8)
    });

    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col0),
        NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col0),
        NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col0),
        NewMaterialExpressionDotProduct(Function, Row3, Matrix1Col0)
    });
    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col1),
        NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col1),
        NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col1),
        NewMaterialExpressionDotProduct(Function, Row3, Matrix1Col1)
    });
    UMaterialExpressionMaterialFunctionCall* Result2 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col2),
        NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col2),
        NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col2),
        NewMaterialExpressionDotProduct(Function, Row3, Matrix1Col2)
    });
    UMaterialExpressionMaterialFunctionCall* Result3 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col3),
        NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col3),
        NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col3),
        NewMaterialExpressionDotProduct(Function, Row3, Matrix1Col3)
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), Result0);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), Result1);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), Result2);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_3"), Result3);
}

static void Generator_MathMultiplyFloat3Float3x3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Multiplies 3-vector with 3x3-matrix.");

    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Row0 = NewMaterialExpressionFunctionInput(Function, TEXT("row0"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Row1 = NewMaterialExpressionFunctionInput(Function, TEXT("row1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Row2 = NewMaterialExpressionFunctionInput(Function, TEXT("row2"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Column0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 1),
                NewMaterialExpressionComponentMask(Function, Row1, 1),
                NewMaterialExpressionComponentMask(Function, Row2, 1)
            });
    UMaterialExpressionMaterialFunctionCall* Column1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 2),
                NewMaterialExpressionComponentMask(Function, Row1, 2),
                NewMaterialExpressionComponentMask(Function, Row2, 2)
            });
    UMaterialExpressionMaterialFunctionCall* Column2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 4),
                NewMaterialExpressionComponentMask(Function, Row1, 4),
                NewMaterialExpressionComponentMask(Function, Row2, 4)
            });

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionDotProduct(Function, Vector, Column0),
                NewMaterialExpressionDotProduct(Function, Vector, Column1),
                NewMaterialExpressionDotProduct(Function, Vector, Column2)
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_MathMultiplyFloat3x3Float3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Multiplies 3x3-matrix with 3-vector.");

    UMaterialExpressionFunctionInput* Column0 = NewMaterialExpressionFunctionInput(Function, TEXT("column0"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Column1 = NewMaterialExpressionFunctionInput(Function, TEXT("column1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Column2 = NewMaterialExpressionFunctionInput(Function, TEXT("column2"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Row0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, Column0, 1),
                NewMaterialExpressionComponentMask(Function, Column1, 1),
                NewMaterialExpressionComponentMask(Function, Column2, 1)
            });
    UMaterialExpressionMaterialFunctionCall* Row1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, Column0, 2),
                NewMaterialExpressionComponentMask(Function, Column1, 2),
                NewMaterialExpressionComponentMask(Function, Column2, 2)
            });
    UMaterialExpressionMaterialFunctionCall* Row2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, Column0, 4),
                NewMaterialExpressionComponentMask(Function, Column1, 4),
                NewMaterialExpressionComponentMask(Function, Column2, 4)
            });

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionDotProduct(Function, Row0, Vector),
                NewMaterialExpressionDotProduct(Function, Row1, Vector),
                NewMaterialExpressionDotProduct(Function, Row2, Vector)
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_MathMultiplyFloat3x3Float3x3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Multiplies two 3x3 matrices.");

    UMaterialExpressionFunctionInput* Matrix0Col0 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col0"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Matrix0Col1 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Matrix0Col2 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col2"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Matrix1Col0 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col0"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Matrix1Col1 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Matrix1Col2 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col2"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpression* Row0 = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionComponentMask(Function, Matrix0Col0, 1),
            NewMaterialExpressionComponentMask(Function, Matrix0Col1, 1),
            NewMaterialExpressionComponentMask(Function, Matrix0Col2, 1)
        });
    UMaterialExpression* Row1 = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionComponentMask(Function, Matrix0Col0, 2),
            NewMaterialExpressionComponentMask(Function, Matrix0Col1, 2),
            NewMaterialExpressionComponentMask(Function, Matrix0Col2, 2)
        });
    UMaterialExpression* Row2 = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionComponentMask(Function, Matrix0Col0, 4),
            NewMaterialExpressionComponentMask(Function, Matrix0Col1, 4),
            NewMaterialExpressionComponentMask(Function, Matrix0Col2, 4)
        });

    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col0),
            NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col0),
            NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col0)
        });
    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col1),
            NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col1),
            NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col1)
        });
    UMaterialExpressionMaterialFunctionCall* Result2 = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col2),
            NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col2),
            NewMaterialExpressionDotProduct(Function, Row2, Matrix1Col2)
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), Result0);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), Result1);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), Result2);
}

static void Generator_MathMultiplyFloat2Float2x2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Multiplies 2-vector with 2x2-matrix.");

    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Row0 = NewMaterialExpressionFunctionInput(Function, TEXT("row0"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Row1 = NewMaterialExpressionFunctionInput(Function, TEXT("row1"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMaterialFunctionCall* Column0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 1),
                NewMaterialExpressionComponentMask(Function, Row1, 1)
            });
    UMaterialExpressionMaterialFunctionCall* Column1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionComponentMask(Function, Row0, 2),
                NewMaterialExpressionComponentMask(Function, Row1, 2)
            });


    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionDotProduct(Function, Vector, Column0),
                NewMaterialExpressionDotProduct(Function, Vector, Column1)
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_MathMultiplyFloat2x2Float2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Multiplies 2x2-matrix with 2-vector.");

    UMaterialExpressionFunctionInput* Column0 = NewMaterialExpressionFunctionInput(Function, TEXT("column0"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Column1 = NewMaterialExpressionFunctionInput(Function, TEXT("column1"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMaterialFunctionCall* Row0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionComponentMask(Function, Column0, 1),
                NewMaterialExpressionComponentMask(Function, Column1, 1)
            });
    UMaterialExpressionMaterialFunctionCall* Row1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionComponentMask(Function, Column0, 2),
                NewMaterialExpressionComponentMask(Function, Column1, 2)
            });

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionDotProduct(Function, Row0, Vector),
                NewMaterialExpressionDotProduct(Function, Row1, Vector)
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_MathMultiplyFloat2x2Float2x2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Multiplies two 2x2 matrices.");

    UMaterialExpressionFunctionInput* Matrix0Col0 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col0"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Matrix0Col1 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix0_col1"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Matrix1Col0 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col0"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Matrix1Col1 = NewMaterialExpressionFunctionInput(Function, TEXT("matrix1_col1"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpression* Row0 = NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionComponentMask(Function, Matrix0Col0, 1),
            NewMaterialExpressionComponentMask(Function, Matrix0Col1, 1)
        });
    UMaterialExpression* Row1 = NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionComponentMask(Function, Matrix0Col0, 2),
            NewMaterialExpressionComponentMask(Function, Matrix0Col1, 2)
        });

    UMaterialExpressionMaterialFunctionCall* Result0 = NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col0),
            NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col0)
        });
    UMaterialExpressionMaterialFunctionCall* Result1 = NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionDotProduct(Function, Row0, Matrix1Col1),
            NewMaterialExpressionDotProduct(Function, Row1, Matrix1Col1)
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), Result0);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), Result1);
}

static void Generator_MathSinFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Computes the sine of an angle in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionSine* Sine = NewMaterialExpressionSine(Function, A);

    NewMaterialExpressionFunctionOutput(Function, TEXT("sin"), Sine);
}

static void Generator_MathSinFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    Function->Description = TEXT("Computes the sine of two angles in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMaterialFunctionCall* Sin =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 2))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("sin"), Sin);
}

static void Generator_MathSinFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Computes the sine of three angles in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Sin =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 4))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("sin"), Sin);
}

static void Generator_MathSinFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));

    Function->Description = TEXT("Computes the sine of four angles in radian.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionMaterialFunctionCall* Sin =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 1)),
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 2)),
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 4)),
            NewMaterialExpressionSine(Function, NewMaterialExpressionComponentMask(Function, A, 8))
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("sin"), Sin);
}

static void Generator_MathSum(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the sum of the elements of a.");

    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionAdd* Sum =
        NewMaterialExpressionAdd(Function,
        {
            NewMaterialExpressionComponentMask(Function, A, 1),
            NewMaterialExpressionComponentMask(Function, A, 2),
            NewMaterialExpressionComponentMask(Function, A, 4)
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("sum"), Sum);
}

static void Generator_NVIDIADCCSupportAD3DSMaxBitmap(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BaseFileTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_file_texture"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));
    UMaterialFunction* DCCSupportMaxRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_max_rotation_translation_scale"));

    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("map_channel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UTile = NewMaterialExpressionFunctionInput(Function, TEXT("U_Tile"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* UMirror = NewMaterialExpressionFunctionInput(Function, TEXT("U_Mirror"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* VTile = NewMaterialExpressionFunctionInput(Function, TEXT("V_Tile"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* VMirror = NewMaterialExpressionFunctionInput(Function, TEXT("V_Mirror"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* UAngle = NewMaterialExpressionFunctionInput(Function, TEXT("U_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* VAngle = NewMaterialExpressionFunctionInput(Function, TEXT("V_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* WAngle = NewMaterialExpressionFunctionInput(Function, TEXT("W_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UOffset = NewMaterialExpressionFunctionInput(Function, TEXT("U_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* VOffset = NewMaterialExpressionFunctionInput(Function, TEXT("V_Offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UTiling = NewMaterialExpressionFunctionInput(Function, TEXT("U_tiling"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* VTiling = NewMaterialExpressionFunctionInput(Function, TEXT("V_tiling"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UVWType = NewMaterialExpressionFunctionInput(Function, TEXT("UVW_Type"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Filename = NewMaterialExpressionFunctionInput(Function, TEXT("filename"), EFunctionInputType::FunctionInput_Texture2D);
    UMaterialExpressionFunctionInput* ClipU = NewMaterialExpressionFunctionInput(Function, TEXT("clipu"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ClipW = NewMaterialExpressionFunctionInput(Function, TEXT("clipw"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* ClipV = NewMaterialExpressionFunctionInput(Function, TEXT("clipv"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ClipH = NewMaterialExpressionFunctionInput(Function, TEXT("clipu"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* BumpAmount = NewMaterialExpressionFunctionInput(Function, TEXT("bump_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Clamp = NewMaterialExpressionFunctionInput(Function, TEXT("clamp"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Invert = NewMaterialExpressionFunctionInput(Function, TEXT("invert"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* AlphaFromRGB = NewMaterialExpressionFunctionInput(Function, TEXT("alphaFromRGB"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* OutputAmount = NewMaterialExpressionFunctionInput(Function, TEXT("output_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBLevel = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_level"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBOffset = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* Offset = NewMaterialExpressionFunctionCall(Function, MakeFloat3, { RGBOffset, RGBOffset, RGBOffset });
    UMaterialExpressionMultiply* ScaleX = NewMaterialExpressionMultiply(Function, RGBLevel, OutputAmount);
    UMaterialExpressionMaterialFunctionCall* Scale = NewMaterialExpressionFunctionCall(Function, MakeFloat3, { ScaleX, ScaleX, ScaleX });

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScaleCall = NewMaterialExpressionFunctionCall(Function, DCCSupportMaxRotationTranslationScale,
    {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat3, {UAngle, VAngle, WAngle}), PI / 180.0f),
        NewMaterialExpressionFunctionCall(Function, MakeFloat3, {UOffset, VOffset, 0.0f}),
        NewMaterialExpressionFunctionCall(Function, MakeFloat3, {UTiling, VTiling, 0.0f}),
        UMirror,
        VMirror
    });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource =
        NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource, { {}, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f)});

    UMaterialExpressionMaterialFunctionCall* TransformCoordinateCall = NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
    {
        { RotationTranslationScaleCall, 0 },
        { RotationTranslationScaleCall, 1 },
        { RotationTranslationScaleCall, 2 },
        { RotationTranslationScaleCall, 3 },
        { CoordinateSource, 0 },
        { CoordinateSource, 1 },
        { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
        , { CoordinateSource, 3 },
        { CoordinateSource, 4 },
        { CoordinateSource, 5 }
#endif
    });

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, BaseFileTexture,
        {
            Filename,
            NewMaterialExpressionStaticSwitch(Function, Invert, NewMaterialExpressionNegate(Function, Offset), Offset),
            NewMaterialExpressionStaticSwitch(Function, Invert, NewMaterialExpressionNegate(Function, Scale), Scale),
            mono_average,
            { TransformCoordinateCall, 0 },
            { TransformCoordinateCall, 1 },
            { TransformCoordinateCall, 2 },
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            { TransformCoordinateCall, 3 },
            { TransformCoordinateCall, 4 },
            { TransformCoordinateCall, 5 },
#endif
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ ClipU, NewMaterialExpressionAdd(Function, ClipW, ClipU) }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ NewMaterialExpressionSubtract(Function, 1.0f, ClipV, ClipH), NewMaterialExpressionOneMinus(Function, ClipV)}),
            NewMaterialExpressionStaticSwitch(Function, UTile, wrap_repeat, NewMaterialExpressionStaticSwitch(Function, UMirror, wrap_mirrored_repeat, wrap_clip)),
            NewMaterialExpressionStaticSwitch(Function, VTile, wrap_repeat, NewMaterialExpressionStaticSwitch(Function, VMirror, wrap_mirrored_repeat, wrap_clip)),
            {},
            {},
            {},
            {},
            {}
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            , UseWorldAlignedTexture
#endif
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), { Result, 0 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), { Result, 1 });
}

static void Generator_NVIDIADCCSupportAD3DSMaxBitmapBump(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BaseFileBumpTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_file_bump_texture"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));
    UMaterialFunction* DCCSupportMaxRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_max_rotation_translation_scale"));

    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("map_channel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UTile = NewMaterialExpressionFunctionInput(Function, TEXT("U_Tile"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* UMirror = NewMaterialExpressionFunctionInput(Function, TEXT("U_Mirror"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* VTile = NewMaterialExpressionFunctionInput(Function, TEXT("V_Tile"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* VMirror = NewMaterialExpressionFunctionInput(Function, TEXT("V_Mirror"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* UAngle = NewMaterialExpressionFunctionInput(Function, TEXT("U_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* VAngle = NewMaterialExpressionFunctionInput(Function, TEXT("V_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* WAngle = NewMaterialExpressionFunctionInput(Function, TEXT("W_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UOffset = NewMaterialExpressionFunctionInput(Function, TEXT("U_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* VOffset = NewMaterialExpressionFunctionInput(Function, TEXT("V_Offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UTiling = NewMaterialExpressionFunctionInput(Function, TEXT("U_tiling"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* VTiling = NewMaterialExpressionFunctionInput(Function, TEXT("V_tiling"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UVWType = NewMaterialExpressionFunctionInput(Function, TEXT("UVW_Type"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Filename = NewMaterialExpressionFunctionInput(Function, TEXT("filename"), EFunctionInputType::FunctionInput_Texture2D);
    UMaterialExpressionFunctionInput* ClipU = NewMaterialExpressionFunctionInput(Function, TEXT("clipu"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ClipW = NewMaterialExpressionFunctionInput(Function, TEXT("clipw"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* ClipV = NewMaterialExpressionFunctionInput(Function, TEXT("clipv"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ClipH = NewMaterialExpressionFunctionInput(Function, TEXT("clipu"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* BumpAmount = NewMaterialExpressionFunctionInput(Function, TEXT("bump_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Clamp = NewMaterialExpressionFunctionInput(Function, TEXT("clamp"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Invert = NewMaterialExpressionFunctionInput(Function, TEXT("invert"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* AlphaFromRGB = NewMaterialExpressionFunctionInput(Function, TEXT("alphaFromRGB"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* OutputAmount = NewMaterialExpressionFunctionInput(Function, TEXT("output_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBLevel = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_level"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBOffset = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScaleCall = NewMaterialExpressionFunctionCall(Function, DCCSupportMaxRotationTranslationScale,
    {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat3,{ UAngle, VAngle, WAngle }), PI / 180.0f),
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,{ UOffset, VOffset, 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,{ UTiling, VTiling, 0.0f }),
        UMirror,
        VMirror
    });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource =
        NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource, { {}, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f) });

    UMaterialExpressionMaterialFunctionCall* TransformCoordinateCall = NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
    {
        { RotationTranslationScaleCall, 0 },
        { RotationTranslationScaleCall, 1 },
        { RotationTranslationScaleCall, 2 },
        { RotationTranslationScaleCall, 3 },
        { CoordinateSource, 0 },
        { CoordinateSource, 1 },
        { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
        , { CoordinateSource, 3 },
        { CoordinateSource, 4 },
        { CoordinateSource, 5 }
#endif
    });

    UMaterialExpressionMultiply* BumpFactor = NewMaterialExpressionMultiply(Function, Factor, BumpAmount);

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, BaseFileBumpTexture,
        {
            Filename,
            NewMaterialExpressionStaticSwitch(Function, Invert, NewMaterialExpressionNegate(Function, BumpFactor), BumpFactor),
            {},
            { TransformCoordinateCall, 0 },
            { TransformCoordinateCall, 1 },
            { TransformCoordinateCall, 2 },
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            { TransformCoordinateCall, 3 },
            { TransformCoordinateCall, 4 },
            { TransformCoordinateCall, 5 },
#endif
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ ClipU, NewMaterialExpressionAdd(Function, ClipW, ClipU) }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ NewMaterialExpressionSubtract(Function, 1.0f, ClipV, ClipH), NewMaterialExpressionOneMinus(Function, ClipV) }),
            NewMaterialExpressionStaticSwitch(Function, UTile, wrap_repeat, NewMaterialExpressionStaticSwitch(Function, UMirror, wrap_mirrored_repeat, wrap_clip)),
            NewMaterialExpressionStaticSwitch(Function, VTile, wrap_repeat, NewMaterialExpressionStaticSwitch(Function, VMirror, wrap_mirrored_repeat, wrap_clip)),
            {},
            {},
            {},
            {},
            {},
            {}
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            , UseWorldAlignedTexture
#endif
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("bump"), Result);
}

static void Generator_NVIDIADCCSupportAD3DSMaxBitmapNormalMap(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BaseTangentSpaceNormalTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_tangent_space_normal_texture"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));
    UMaterialFunction* DCCSupportMaxRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_max_rotation_translation_scale"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("map_channel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UTile = NewMaterialExpressionFunctionInput(Function, TEXT("U_Tile"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* UMirror = NewMaterialExpressionFunctionInput(Function, TEXT("U_Mirror"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* VTile = NewMaterialExpressionFunctionInput(Function, TEXT("V_Tile"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* VMirror = NewMaterialExpressionFunctionInput(Function, TEXT("V_Mirror"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* UAngle = NewMaterialExpressionFunctionInput(Function, TEXT("U_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* VAngle = NewMaterialExpressionFunctionInput(Function, TEXT("V_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* WAngle = NewMaterialExpressionFunctionInput(Function, TEXT("W_angle"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UOffset = NewMaterialExpressionFunctionInput(Function, TEXT("U_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* VOffset = NewMaterialExpressionFunctionInput(Function, TEXT("V_Offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UTiling = NewMaterialExpressionFunctionInput(Function, TEXT("U_tiling"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* VTiling = NewMaterialExpressionFunctionInput(Function, TEXT("V_tiling"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UVWType = NewMaterialExpressionFunctionInput(Function, TEXT("UVW_Type"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Filename = NewMaterialExpressionFunctionInput(Function, TEXT("filename"), EFunctionInputType::FunctionInput_Texture2D);
    UMaterialExpressionFunctionInput* ClipU = NewMaterialExpressionFunctionInput(Function, TEXT("clipu"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ClipW = NewMaterialExpressionFunctionInput(Function, TEXT("clipw"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* ClipV = NewMaterialExpressionFunctionInput(Function, TEXT("clipv"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ClipH = NewMaterialExpressionFunctionInput(Function, TEXT("clipu"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* BumpAmount = NewMaterialExpressionFunctionInput(Function, TEXT("bump_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Clamp = NewMaterialExpressionFunctionInput(Function, TEXT("clamp"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Invert = NewMaterialExpressionFunctionInput(Function, TEXT("invert"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* AlphaFromRGB = NewMaterialExpressionFunctionInput(Function, TEXT("alphaFromRGB"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* OutputAmount = NewMaterialExpressionFunctionInput(Function, TEXT("output_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBLevel = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_level"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBOffset = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* SwapRG = NewMaterialExpressionFunctionInput(Function, TEXT("swap_rg"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* MultSpin = NewMaterialExpressionFunctionInput(Function, TEXT("mult_spin"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Method = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* FlipRed = NewMaterialExpressionFunctionInput(Function, TEXT("flipred"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* FlipGreen = NewMaterialExpressionFunctionInput(Function, TEXT("flipgreen"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* BumpMap = NewMaterialExpressionFunctionInput(Function, TEXT("bump_map"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScaleCall = NewMaterialExpressionFunctionCall(Function, DCCSupportMaxRotationTranslationScale,
    {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat3,{ UAngle, VAngle, WAngle }), PI / 180.0f),
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,{ UOffset, VOffset, 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,{ UTiling, VTiling, 0.0f }),
        UMirror,
        VMirror
    });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource =
        NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource, { {}, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f) });

    UMaterialExpressionMaterialFunctionCall* TransformCoordinateCall = NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
    {
        { RotationTranslationScaleCall, 0 },
        { RotationTranslationScaleCall, 1 },
        { RotationTranslationScaleCall, 2 },
        { RotationTranslationScaleCall, 3 },
        { CoordinateSource, 0 },
        { CoordinateSource, 1 },
        { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
        , { CoordinateSource, 3 },
        { CoordinateSource, 4 },
        { CoordinateSource, 5 }
#endif
    });

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, BaseTangentSpaceNormalTexture,
        {
            Filename,
            NewMaterialExpressionMultiply(Function, { Factor, MultSpin, RGBLevel, OutputAmount}),
            NewMaterialExpressionStaticSwitch(Function, Invert, NewMaterialExpressionStaticSwitch(Function, FlipRed, false, true), FlipRed),
            NewMaterialExpressionStaticSwitch(Function, Invert, FlipGreen, NewMaterialExpressionStaticSwitch(Function, FlipGreen, false, true)),
            { TransformCoordinateCall, 0 },
            { TransformCoordinateCall, 1 },
            { TransformCoordinateCall, 2 },
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            { TransformCoordinateCall, 3 },
            { TransformCoordinateCall, 4 },
            { TransformCoordinateCall, 5 },
#endif
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ ClipU, NewMaterialExpressionAdd(Function, ClipW, ClipU) }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,{ NewMaterialExpressionSubtract(Function, 1.0f, ClipV, ClipH), NewMaterialExpressionOneMinus(Function, ClipV) }),
            NewMaterialExpressionStaticSwitch(Function, UTile, wrap_repeat, NewMaterialExpressionStaticSwitch(Function, UMirror, wrap_mirrored_repeat, wrap_clip)),
            NewMaterialExpressionStaticSwitch(Function, VTile, wrap_repeat, NewMaterialExpressionStaticSwitch(Function, VMirror, wrap_mirrored_repeat, wrap_clip)),
            {},
            {},
            {},
            {},
            {},
            {},
            {}
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            , UseWorldAlignedTexture
#endif
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("normalmap"), Result);
}

static void Generator_NVIDIADCCSupportAD3DSMaxColorCorrection(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum ad_3dsmax_channel
    {
        channel_red,
        channel_green,
        channel_blue,
        channel_alpha,
        channel_inverse_red,
        channel_inverse_green,
        channel_inverse_blue,
        channel_inverse_alpha,
        channel_mono,
        channel_one,
        channel_zero
    };

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));

    UMaterialExpressionFunctionInput* MapTint = NewMaterialExpressionFunctionInput(Function, TEXT("map_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* MapMono = NewMaterialExpressionFunctionInput(Function, TEXT("map_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* RewireR = NewMaterialExpressionFunctionInput(Function, TEXT("rewireR"), EFunctionInputType::FunctionInput_Scalar, channel_red);
    UMaterialExpressionFunctionInput* RewireG = NewMaterialExpressionFunctionInput(Function, TEXT("rewireG"), EFunctionInputType::FunctionInput_Scalar, channel_green);
    UMaterialExpressionFunctionInput* RewireB = NewMaterialExpressionFunctionInput(Function, TEXT("rewireB"), EFunctionInputType::FunctionInput_Scalar, channel_blue);
    UMaterialExpressionFunctionInput* RewireA = NewMaterialExpressionFunctionInput(Function, TEXT("rewireA"), EFunctionInputType::FunctionInput_Scalar, channel_alpha);
    UMaterialExpressionFunctionInput* HueShift = NewMaterialExpressionFunctionInput(Function, TEXT("hueShift"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Saturation = NewMaterialExpressionFunctionInput(Function, TEXT("saturation"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Tint = NewMaterialExpressionFunctionInput(Function, TEXT("tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* TintStrength = NewMaterialExpressionFunctionInput(Function, TEXT("tintStrength"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionComponentMask* R = NewMaterialExpressionComponentMask(Function, MapTint, 1);
    UMaterialExpressionComponentMask* G = NewMaterialExpressionComponentMask(Function, MapTint, 2);
    UMaterialExpressionComponentMask* B = NewMaterialExpressionComponentMask(Function, MapTint, 4);
    UMaterialExpressionOneMinus* InverseR = NewMaterialExpressionOneMinus(Function, R);
    UMaterialExpressionOneMinus* InverseG = NewMaterialExpressionOneMinus(Function, G);
    UMaterialExpressionOneMinus* InverseB = NewMaterialExpressionOneMinus(Function, B);
    UMaterialExpressionOneMinus* InverseMono = NewMaterialExpressionOneMinus(Function, MapMono);
    UMaterialExpressionMaterialFunctionCall* SelectedTint = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
    {
        NewMaterialExpressionSwitch(Function, RewireR,{ R, G, B, MapMono, InverseR, InverseG, InverseB, InverseMono, NewMaterialExpressionFunctionCall(Function, MathLuminance,{ MapTint }), 1.0f, 0.0f }),
        NewMaterialExpressionSwitch(Function, RewireG,{ R, G, B, MapMono, InverseR, InverseG, InverseB, InverseMono, NewMaterialExpressionFunctionCall(Function, MathLuminance,{ MapTint }), 1.0f, 0.0f }),
        NewMaterialExpressionSwitch(Function, RewireB,{ R, G, B, MapMono, InverseR, InverseG, InverseB, InverseMono, NewMaterialExpressionFunctionCall(Function, MathLuminance,{ MapTint }), 1.0f, 0.0f }),
    });
    UMaterialExpressionIf* ResultTint =
        NewMaterialExpressionIfEqual(Function, RewireB, channel_blue,
            NewMaterialExpressionIfEqual(Function, RewireG, channel_green,
                NewMaterialExpressionIfEqual(Function, RewireR, channel_red,
                    NewMaterialExpressionIfEqual(Function, RewireA, channel_alpha,
                        MapTint,
                        SelectedTint),
                    SelectedTint),
                SelectedTint),
            SelectedTint);

    UMaterialExpressionIf* SelectedMono = NewMaterialExpressionSwitch(Function, RewireA, { R, G, B, MapMono, InverseR, InverseG, InverseB, InverseMono, NewMaterialExpressionFunctionCall(Function, MathLuminance,{ MapTint }), 1.0f, 0.0f });
    UMaterialExpressionIf* ResultMono =
        NewMaterialExpressionIfEqual(Function, RewireB, channel_blue,
            NewMaterialExpressionIfEqual(Function, RewireG, channel_green,
                NewMaterialExpressionIfEqual(Function, RewireR, channel_red,
                    NewMaterialExpressionIfEqual(Function, RewireA, channel_alpha,
                        MapMono,
                        SelectedMono),
                    SelectedMono),
                SelectedMono),
            SelectedMono);

    NewMaterialExpressionFunctionOutput(Function, "tint", ResultTint);
    NewMaterialExpressionFunctionOutput(Function, "mono", ResultMono);
}

static void Generator_NVIDIADCCSupportAD3DSMaxCompositeTextureMap(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    TArray<int32> BlendColorLayersArrayInputs = { 1 };
    UMaterialFunction* BaseBlendColorLayers1 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_blend_color_layers"), BlendColorLayersArrayInputs);
    UMaterialFunction* DCCSupportAd3DSMaxMapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_ad_3dsmax_mapmode"));

    UMaterialExpressionFunctionInput* BlendMode1 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode1"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map1Tint = NewMaterialExpressionFunctionInput(Function, TEXT("map1_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Map1Mono = NewMaterialExpressionFunctionInput(Function, TEXT("map1_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Mask1Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask1_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask1Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask1_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity1 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity1"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BlendMode2 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode2"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map2Tint = NewMaterialExpressionFunctionInput(Function, TEXT("map2_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Map2Mono = NewMaterialExpressionFunctionInput(Function, TEXT("map2_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Mask2Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask2_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask2Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask2_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity2 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity2"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BlendMode3 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode3"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map3Tint = NewMaterialExpressionFunctionInput(Function, TEXT("map3_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Map3Mono = NewMaterialExpressionFunctionInput(Function, TEXT("map3_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Mask3Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask3_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask3Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask3_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity3 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity3"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BlendMode4 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode4"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map4Tint = NewMaterialExpressionFunctionInput(Function, TEXT("map4_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Map4Mono = NewMaterialExpressionFunctionInput(Function, TEXT("map4_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Mask4Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask4_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask4Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask4_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity4 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity4"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Layer1 = NewMaterialExpressionFunctionCall(Function, BaseBlendColorLayers1,
    {
        Map1Tint,
        NewMaterialExpressionMultiply(Function, {Opacity1, 0.01f, Mask1Mono}),
        color_layer_blend,
        NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f),
        {}
    });
    UMaterialExpressionIf* T1Tint = NewMaterialExpressionIfGreater(Function, Opacity1, 0.0f, { Layer1, 0 }, NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f));
    UMaterialExpressionIf* T1Mono = NewMaterialExpressionIfGreater(Function, Opacity1, 0.0f, { Layer1, 1 }, 0.0f);

    UMaterialExpressionMultiply* W2 = NewMaterialExpressionMultiply(Function, { Opacity2, 0.01f, Mask2Mono });
    UMaterialExpressionMaterialFunctionCall* Layer2 = NewMaterialExpressionFunctionCall(Function, BaseBlendColorLayers1,
    {
        Map2Tint,
        NewMaterialExpressionIfEqual(Function, BlendMode2, 3, NewMaterialExpressionNegate(Function, W2), W2),
        NewMaterialExpressionFunctionCall(Function, DCCSupportAd3DSMaxMapMode, {BlendMode2}),
        T1Tint,
        {}
    });
    UMaterialExpressionIf* T2Tint = NewMaterialExpressionIfGreater(Function, Opacity2, 0.0f, { Layer2, 0 }, T1Tint);
    UMaterialExpressionIf* T2Mono = NewMaterialExpressionIfGreater(Function, Opacity2, 0.0f, { Layer2, 1 }, T1Mono);

    UMaterialExpressionMultiply* W3 = NewMaterialExpressionMultiply(Function, { Opacity3, 0.01f, Mask3Mono });
    UMaterialExpressionMaterialFunctionCall* Layer3 = NewMaterialExpressionFunctionCall(Function, BaseBlendColorLayers1,
    {
        Map3Tint,
        NewMaterialExpressionIfEqual(Function, BlendMode3, 3, NewMaterialExpressionNegate(Function, W3), W3),
        NewMaterialExpressionFunctionCall(Function, DCCSupportAd3DSMaxMapMode,{ BlendMode3 }),
        T2Tint,
        {}
    });
    UMaterialExpressionIf* T3Tint = NewMaterialExpressionIfGreater(Function, Opacity3, 0.0f, { Layer3, 0 }, T2Tint);
    UMaterialExpressionIf* T3Mono = NewMaterialExpressionIfGreater(Function, Opacity3, 0.0f, { Layer3, 1 }, T2Mono);

    UMaterialExpressionMultiply* W4 = NewMaterialExpressionMultiply(Function, { Opacity4, 0.01f, Mask4Mono });
    UMaterialExpressionMaterialFunctionCall* Layer4 = NewMaterialExpressionFunctionCall(Function, BaseBlendColorLayers1,
    {
        Map4Tint,
        NewMaterialExpressionIfEqual(Function, BlendMode4, 3, NewMaterialExpressionNegate(Function, W4), W4),
        NewMaterialExpressionFunctionCall(Function, DCCSupportAd3DSMaxMapMode,{ BlendMode4 }),
        T3Tint,
        {}
    });
    UMaterialExpressionIf* T4Tint = NewMaterialExpressionIfGreater(Function, Opacity4, 0.0f, { Layer4, 0 }, T3Tint);
    UMaterialExpressionIf* T4Mono = NewMaterialExpressionIfGreater(Function, Opacity4, 0.0f, { Layer4, 1 }, T3Mono);

    NewMaterialExpressionFunctionOutput(Function, "tint", T4Tint);
    NewMaterialExpressionFunctionOutput(Function, "mono", T4Mono);
}

static void Generator_NVIDIADCCSupportAD3DSMaxCompositeTextureMapBump(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* DCCSupportCombineNormals = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_combine_normals"));

    UMaterialExpressionMaterialFunctionCall* Normal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialExpressionFunctionInput* BlendMode1 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode1"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map1 = NewMaterialExpressionFunctionInput(Function, TEXT("map1"), EFunctionInputType::FunctionInput_Vector3, Normal);
    UMaterialExpressionFunctionInput* Mask1Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask1_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask1Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask1_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity1 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity1"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BlendMode2 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode2"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map2 = NewMaterialExpressionFunctionInput(Function, TEXT("map2"), EFunctionInputType::FunctionInput_Vector3, Normal);
    UMaterialExpressionFunctionInput* Mask2Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask2_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask2Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask2_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity2 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity2"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BlendMode3 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode3"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map3 = NewMaterialExpressionFunctionInput(Function, TEXT("map3"), EFunctionInputType::FunctionInput_Vector3, Normal);
    UMaterialExpressionFunctionInput* Mask3Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask3_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask3Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask3_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity3 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity3"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BlendMode4 = NewMaterialExpressionFunctionInput(Function, TEXT("blendMode4"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Map4 = NewMaterialExpressionFunctionInput(Function, TEXT("map4"), EFunctionInputType::FunctionInput_Vector3, Normal);
    UMaterialExpressionFunctionInput* Mask4Tint = NewMaterialExpressionFunctionInput(Function, TEXT("mask4_tint"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Mask4Mono = NewMaterialExpressionFunctionInput(Function, TEXT("mask4_mono"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Opacity4 = NewMaterialExpressionFunctionInput(Function, TEXT("opacity4"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionMultiply* O1 = NewMaterialExpressionMultiply(Function, { Opacity1, 0.01f, Mask1Mono });
    UMaterialExpressionIf* N1 =
        NewMaterialExpressionIfGreater(Function, Opacity1, 0.0f,
            NewMaterialExpressionFunctionCall(Function, DCCSupportCombineNormals,
                { NewMaterialExpressionOneMinus(Function, O1), Normal, NewMaterialExpressionMultiply(Function, O1, Factor), Map1 }),
            Normal);
    UMaterialExpressionMultiply* O2 = NewMaterialExpressionMultiply(Function, { Opacity2, 0.01f, Mask2Mono });
    UMaterialExpressionIf* N2 =
        NewMaterialExpressionIfGreater(Function, Opacity2, 0.0f,
            NewMaterialExpressionFunctionCall(Function, DCCSupportCombineNormals,
                { NewMaterialExpressionOneMinus(Function, O2), N1, NewMaterialExpressionMultiply(Function, O2, Factor), Map2 }),
            N1);
    UMaterialExpressionMultiply* O3 = NewMaterialExpressionMultiply(Function, { Opacity3, 0.01f, Mask3Mono });
    UMaterialExpressionIf* N3 =
        NewMaterialExpressionIfGreater(Function, Opacity3, 0.0f,
            NewMaterialExpressionFunctionCall(Function, DCCSupportCombineNormals,
                { NewMaterialExpressionOneMinus(Function, O3), N2, NewMaterialExpressionMultiply(Function, O3, Factor), Map3 }),
            N2);
    UMaterialExpressionMultiply* O4 = NewMaterialExpressionMultiply(Function, { Opacity4, 0.01f, Mask4Mono });
    UMaterialExpressionIf* N4 =
        NewMaterialExpressionIfGreater(Function, Opacity4, 0.0f,
            NewMaterialExpressionFunctionCall(Function, DCCSupportCombineNormals,
                { NewMaterialExpressionOneMinus(Function, O4), N3, NewMaterialExpressionMultiply(Function, O4, Factor), Map4 }),
            N3);

    NewMaterialExpressionFunctionOutput(Function, "result", N4);
}

static void Generator_NVIDIADCCSupportAD3DSMaxDent(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BasePerlinNoiseTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_perlin_noise_texture"));
    UMaterialFunction* BaseRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_rotation_translation_scale"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));

    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 200.0f);
    UMaterialExpressionFunctionInput* Strength = NewMaterialExpressionFunctionInput(Function, TEXT("strength"), EFunctionInputType::FunctionInput_Scalar, 20.0f);
    UMaterialExpressionFunctionInput* Iterations = NewMaterialExpressionFunctionInput(Function, TEXT("iterations"), EFunctionInputType::FunctionInput_Scalar, 2.0f);
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("tiling"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Angle = NewMaterialExpressionFunctionInput(Function, TEXT("angle"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* BlurOffset = NewMaterialExpressionFunctionInput(Function, TEXT("blur_Offset"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* CoordType = NewMaterialExpressionFunctionInput(Function, TEXT("coordType"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("mapChannel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScale =
        NewMaterialExpressionFunctionCall(Function, BaseRotationTranslationScale, { NewMaterialExpressionMultiply(Function, Angle, PI / 180.0f), Offset, Tiling });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource = NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource,
    {
        NewMaterialExpressionSwitch(Function, CoordType,{ texture_coordinate_object, texture_coordinate_world, texture_coordinate_uvw }),
        NewMaterialExpressionIfLess(Function, CoordType, 2.0f, 0.0f, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f))
    });

    UMaterialExpressionMaterialFunctionCall* TransformCoordinate = NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
    {
        { RotationTranslationScale, 0 },
        { RotationTranslationScale, 1 },
        { RotationTranslationScale, 2 },
        { RotationTranslationScale, 3 },
        { CoordinateSource, 0 },
        { CoordinateSource, 1 },
        { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
        , { CoordinateSource, 3 },
        { CoordinateSource, 4 },
        { CoordinateSource, 5 }
#endif
    });

    UMaterialExpressionMaterialFunctionCall* PerlinNoiseTexture = NewMaterialExpressionFunctionCall(Function, BasePerlinNoiseTexture,
    {
        { TransformCoordinate, 0 },
        { TransformCoordinate, 1 },
        { TransformCoordinate, 2 },
        Color1,
        Color2,
        NewMaterialExpressionDivide(Function, Size, 30.0f),
        {},
        true,
        {},
        Iterations,
        true,
        {},
        {},
        NewMaterialExpressionIfEqual(Function, Strength, 0.0f, 1.0f, NewMaterialExpressionDivide(Function, 1.0f, Strength)),
        {},
        {}
    });

    NewMaterialExpressionFunctionOutput(Function, "tint", { PerlinNoiseTexture, 0 });
    NewMaterialExpressionFunctionOutput(Function, "mono", { PerlinNoiseTexture, 1 });
}

static void Generator_NVIDIADCCSupportAD3DSMaxDentBump(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BasePerlinNoiseBumpTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_perlin_noise_bump_texture"));
    UMaterialFunction* BaseRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_rotation_translation_scale"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));
    UMaterialFunction* DCCSupportCombineNormals = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_combine_normals"));
    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 200.0f);
    UMaterialExpressionFunctionInput* Strength = NewMaterialExpressionFunctionInput(Function, TEXT("strength"), EFunctionInputType::FunctionInput_Scalar, 20.0f);
    UMaterialExpressionFunctionInput* Iterations = NewMaterialExpressionFunctionInput(Function, TEXT("iterations"), EFunctionInputType::FunctionInput_Scalar, 2.0f);
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("tiling"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Angle = NewMaterialExpressionFunctionInput(Function, TEXT("angle"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* BlurOffset = NewMaterialExpressionFunctionInput(Function, TEXT("blur_Offset"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* CoordType = NewMaterialExpressionFunctionInput(Function, TEXT("coordType"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("mapChannel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScale =
        NewMaterialExpressionFunctionCall(Function, BaseRotationTranslationScale, { NewMaterialExpressionMultiply(Function, Angle, PI / 180.0f), Offset, Tiling });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource =
        NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource,
        {
            NewMaterialExpressionSwitch(Function, CoordType,{ texture_coordinate_object, texture_coordinate_world, texture_coordinate_uvw }),
            NewMaterialExpressionIfLess(Function, CoordType, 2.0f, 0.0f, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f))
        });

    UMaterialExpressionMaterialFunctionCall* TransformCoordinate =
        NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
        {
            { RotationTranslationScale, 0 },
            { RotationTranslationScale, 1 },
            { RotationTranslationScale, 2 },
            { RotationTranslationScale, 3 },
            { CoordinateSource, 0 },
            { CoordinateSource, 1 },
            { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
            , { CoordinateSource, 3 },
            { CoordinateSource, 4 },
            { CoordinateSource, 5 }
#endif
        });

    UMaterialExpressionMaterialFunctionCall* PerlinNormal =
        NewMaterialExpressionFunctionCall(Function, BasePerlinNoiseBumpTexture,
        {
            { TransformCoordinate, 0 },
            { TransformCoordinate, 1 },
            { TransformCoordinate, 2 },
            NewMaterialExpressionMultiply(Function,{ 0.01f, Factor, Strength }),
            NewMaterialExpressionDivide(Function, Size, 30.0f),
            {},
            true,
            {},
            Iterations,
            true,
            {},
            {},
            {},
            {},
            {},
            {}
        });

    UMaterialExpressionMultiply* F =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MathAverage, { NewMaterialExpressionSubtract(Function, Color1, Color2) }),
            Factor);

    UMaterialExpressionMaterialFunctionCall* Normal =
        NewMaterialExpressionFunctionCall(Function, DCCSupportCombineNormals,
        {
            F,
            PerlinNormal,
            NewMaterialExpressionOneMinus(Function, F),
            NewMaterialExpressionFunctionCall(Function, StateNormal,{})
        });

    NewMaterialExpressionFunctionOutput(Function, "normal", Normal);
}

static void Generator_NVIDIADCCSupportAD3DSMaxFalloff(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Color1Tint = NewMaterialExpressionFunctionInput(Function, TEXT("color1_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color1Mono = NewMaterialExpressionFunctionInput(Function, TEXT("color1_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Color2Tint = NewMaterialExpressionFunctionInput(Function, TEXT("color2_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2Mono = NewMaterialExpressionFunctionInput(Function, TEXT("color2_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Direction = NewMaterialExpressionFunctionInput(Function, TEXT("direction"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Type = NewMaterialExpressionFunctionInput(Function, TEXT("type"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Node0 = NewMaterialExpressionFunctionInput(Function, TEXT("node_0"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Node1 = NewMaterialExpressionFunctionInput(Function, TEXT("node_1"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Node2 = NewMaterialExpressionFunctionInput(Function, TEXT("node_2"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Node3 = NewMaterialExpressionFunctionInput(Function, TEXT("node_3"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* NearDistance = NewMaterialExpressionFunctionInput(Function, TEXT("nearDistance"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* FarDistance = NewMaterialExpressionFunctionInput(Function, TEXT("farDistance"), EFunctionInputType::FunctionInput_Scalar, 1000000.0f);

    NewMaterialExpressionFunctionOutput(Function, "tint", Color1Tint);
    NewMaterialExpressionFunctionOutput(Function, "mono", Color1Mono);
}

static void Generator_NVIDIADCCSupportAD3DSMaxMapMode(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* M = NewMaterialExpressionFunctionInput(Function, TEXT("m"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionIf* Result =
        NewMaterialExpressionSwitch(Function, M,
        {
            color_layer_blend, color_layer_average, color_layer_add, color_layer_add, color_layer_darken, color_layer_multiply, color_layer_colorburn, color_layer_linearburn,
            color_layer_lighten, color_layer_screen, color_layer_colordodge, color_layer_lineardodge, color_layer_spotlight, color_layer_spotlightblend, color_layer_overlay,
            color_layer_softlight, color_layer_hardlight, color_layer_pinlight, color_layer_hardmix, color_layer_difference, color_layer_exclusion, color_layer_hue,
            color_layer_saturation, color_layer_color, color_layer_brightness, color_layer_blend
        });

    NewMaterialExpressionFunctionOutput(Function, "color_layer_mode", Result);
}

static void Generator_NVIDIADCCSupportAD3DSMaxMix(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Color1Tint = NewMaterialExpressionFunctionInput(Function, TEXT("color1_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color1Mono = NewMaterialExpressionFunctionInput(Function, TEXT("color1_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Color2Tint = NewMaterialExpressionFunctionInput(Function, TEXT("color2_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2Mono = NewMaterialExpressionFunctionInput(Function, TEXT("color2_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Lower = NewMaterialExpressionFunctionInput(Function, TEXT("type"), EFunctionInputType::FunctionInput_Scalar, 0.3f);
    UMaterialExpressionFunctionInput* Upper = NewMaterialExpressionFunctionInput(Function, TEXT("upper"), EFunctionInputType::FunctionInput_Scalar, 0.7f);
    UMaterialExpressionFunctionInput* MixAmount = NewMaterialExpressionFunctionInput(Function, TEXT("mixAmount"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* useCurve = NewMaterialExpressionFunctionInput(Function, TEXT("useCurve"), EFunctionInputType::FunctionInput_Scalar, false);

    UMaterialExpressionMultiply* Alpha = NewMaterialExpressionMultiply(Function, MixAmount, 0.01f);

    NewMaterialExpressionFunctionOutput(Function, "tint", NewMaterialExpressionLinearInterpolate(Function, Color1Tint, Color2Tint, Alpha));
    NewMaterialExpressionFunctionOutput(Function, "mono", NewMaterialExpressionLinearInterpolate(Function, Color1Mono, Color2Mono, Alpha));
}

static void Generator_NVIDIADCCSupportAD3DSMaxNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BasePerlinNoiseTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_perlin_noise_texture"));
    UMaterialFunction* BaseRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_rotation_translation_scale"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));

    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 25.0f);
    UMaterialExpressionFunctionInput* Phase = NewMaterialExpressionFunctionInput(Function, TEXT("phase"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Levels = NewMaterialExpressionFunctionInput(Function, TEXT("levels"), EFunctionInputType::FunctionInput_Scalar, 3.0f);
    UMaterialExpressionFunctionInput* ThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("thresholdLow"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("thresholdHigh"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Type = NewMaterialExpressionFunctionInput(Function, TEXT("type"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("tiling"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Angle = NewMaterialExpressionFunctionInput(Function, TEXT("angle"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* BlurOffset = NewMaterialExpressionFunctionInput(Function, TEXT("blur_Offset"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* CoordType = NewMaterialExpressionFunctionInput(Function, TEXT("coordType"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("mapChannel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* AlphaFromRGB = NewMaterialExpressionFunctionInput(Function, TEXT("alphaFromRGB"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* BumpAmount = NewMaterialExpressionFunctionInput(Function, TEXT("bump_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Clamp = NewMaterialExpressionFunctionInput(Function, TEXT("clamp"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Invert = NewMaterialExpressionFunctionInput(Function, TEXT("invert"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* OutputAmount = NewMaterialExpressionFunctionInput(Function, TEXT("output_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBLevel = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_level"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBOffset = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScale =
        NewMaterialExpressionFunctionCall(Function, BaseRotationTranslationScale, { NewMaterialExpressionMultiply(Function, Angle, PI / 180.0f), Offset, Tiling });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource = NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource,
    {
        NewMaterialExpressionSwitch(Function, CoordType, {texture_coordinate_object, texture_coordinate_world, texture_coordinate_uvw }),
        NewMaterialExpressionIfLess(Function, CoordType, 2.0f, 0.0f, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f))
    });

    UMaterialExpressionMaterialFunctionCall* TransformCoordinate = NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
    {
        { RotationTranslationScale, 0 },
        { RotationTranslationScale, 1 },
        { RotationTranslationScale, 2 },
        { RotationTranslationScale, 3 },
        { CoordinateSource, 0 },
        { CoordinateSource, 1 },
        { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
        , { CoordinateSource, 3 },
        { CoordinateSource, 4 },
        { CoordinateSource, 5 }
#endif
    });

    UMaterialExpressionIf* NoiseLevels = NewMaterialExpressionIfEqual(Function, Type, 0.0f, 1.0f, Levels);

    UMaterialExpressionMaterialFunctionCall* PerlinNoiseTextureWithAbsoluteNoise = NewMaterialExpressionFunctionCall(Function, BasePerlinNoiseTexture,
    {
        { TransformCoordinate, 0 },
        { TransformCoordinate, 1 },
        { TransformCoordinate, 2 },
        Color1,
        Color2,
        Size,
        {},
        {},
        Phase,
        NoiseLevels,
        true,
        {},
        {},
        ThresholdHigh,
        ThresholdLow,
        {}
    });

    UMaterialExpressionMaterialFunctionCall* PerlinNoiseTextureWithoutAbsoluteNoise = NewMaterialExpressionFunctionCall(Function, BasePerlinNoiseTexture,
    {
        { TransformCoordinate, 0 },
        { TransformCoordinate, 1 },
        { TransformCoordinate, 2 },
        Color1,
        Color2,
        Size,
        {},
        {},
        Phase,
        NoiseLevels,
        false,
        {},
        {},
        ThresholdHigh,
        ThresholdLow,
        {}
    });

    UMaterialExpressionIf* ResultTint = NewMaterialExpressionIfEqual(Function, Type, 2.0f, { PerlinNoiseTextureWithAbsoluteNoise, 0 }, { PerlinNoiseTextureWithoutAbsoluteNoise, 0 });
    UMaterialExpressionIf* ResultMono = NewMaterialExpressionIfEqual(Function, Type, 2.0f, { PerlinNoiseTextureWithAbsoluteNoise, 1 }, { PerlinNoiseTextureWithoutAbsoluteNoise, 1 });

    NewMaterialExpressionFunctionOutput(Function, "tint", ResultTint);
    NewMaterialExpressionFunctionOutput(Function, "mono", ResultMono);
}

static void Generator_NVIDIADCCSupportAD3DSMaxNoiseBump(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BasePerlinNoiseBumpTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_perlin_noise_bump_texture"));
    UMaterialFunction* BaseRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_rotation_translation_scale"));
    UMaterialFunction* BaseTransformCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_transform_coordinate"));
    UMaterialFunction* DCCSupportCombineNormals = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_combine_normals"));
    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    UMaterialExpressionFunctionInput* Color1 = NewMaterialExpressionFunctionInput(Function, TEXT("color1"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Color2 = NewMaterialExpressionFunctionInput(Function, TEXT("color2"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Size = NewMaterialExpressionFunctionInput(Function, TEXT("size"), EFunctionInputType::FunctionInput_Scalar, 25.0f);
    UMaterialExpressionFunctionInput* Phase = NewMaterialExpressionFunctionInput(Function, TEXT("phase"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Levels = NewMaterialExpressionFunctionInput(Function, TEXT("levels"), EFunctionInputType::FunctionInput_Scalar, 3.0f);
    UMaterialExpressionFunctionInput* ThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("thresholdLow"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("thresholdHigh"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Type = NewMaterialExpressionFunctionInput(Function, TEXT("type"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("tiling"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* Angle = NewMaterialExpressionFunctionInput(Function, TEXT("angle"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* BlurOffset = NewMaterialExpressionFunctionInput(Function, TEXT("blur_Offset"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* CoordType = NewMaterialExpressionFunctionInput(Function, TEXT("coordType"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MapChannel = NewMaterialExpressionFunctionInput(Function, TEXT("mapChannel"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* AlphaFromRGB = NewMaterialExpressionFunctionInput(Function, TEXT("alphaFromRGB"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* BumpAmount = NewMaterialExpressionFunctionInput(Function, TEXT("bump_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Clamp = NewMaterialExpressionFunctionInput(Function, TEXT("clamp"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Invert = NewMaterialExpressionFunctionInput(Function, TEXT("invert"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* OutputAmount = NewMaterialExpressionFunctionInput(Function, TEXT("output_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBLevel = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_level"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBOffset = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# else
    UMaterialExpressionFunctionInput* UseWorldAlignedTexture = NewMaterialExpressionFunctionInput(Function, TEXT("use_world_aligned_texture"), EFunctionInputType::FunctionInput_Scalar);
# endif
#endif

    UMaterialExpressionMaterialFunctionCall* RotationTranslationScale =
        NewMaterialExpressionFunctionCall(Function, BaseRotationTranslationScale, { NewMaterialExpressionMultiply(Function, Angle, PI / 180.0f), Offset, Tiling });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource = NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource,
    {
        NewMaterialExpressionSwitch(Function, CoordType,{ texture_coordinate_object, texture_coordinate_world, texture_coordinate_uvw }),
        NewMaterialExpressionIfLess(Function, CoordType, 2.0f, 0.0f, NewMaterialExpressionSubtract(Function, MapChannel, 1.0f))
    });

    UMaterialExpressionMaterialFunctionCall* TransformCoordinate = NewMaterialExpressionFunctionCall(Function, BaseTransformCoordinate,
    {
        { RotationTranslationScale, 0 },
        { RotationTranslationScale, 1 },
        { RotationTranslationScale, 2 },
        { RotationTranslationScale, 3 },
        { CoordinateSource, 0 },
        { CoordinateSource, 1 },
        { CoordinateSource, 2 }
#if defined(USE_WORLD_ALIGNED_TEXTURES)
        , { CoordinateSource, 3 },
        { CoordinateSource, 4 },
        { CoordinateSource, 5 }
#endif
    });

    UMaterialExpressionIf* NoiseLevels = NewMaterialExpressionIfEqual(Function, Type, 0.0f, 1.0f, Levels);

    UMaterialExpressionMaterialFunctionCall* PerlinNormalWithAbsoluteNoise =
        NewMaterialExpressionFunctionCall(Function, BasePerlinNoiseBumpTexture,
        {
            { TransformCoordinate, 0 },
            { TransformCoordinate, 1 },
            { TransformCoordinate, 2 },
            Factor,
            Size,
            {},
            {},
            Phase,
            NoiseLevels,
            true,
            {},
            {},
            ThresholdHigh,
            ThresholdLow,
            {},
            {}
        });

    UMaterialExpressionMaterialFunctionCall* PerlinNormalWithoutAbsoluteNoise =
        NewMaterialExpressionFunctionCall(Function, BasePerlinNoiseBumpTexture,
        {
            { TransformCoordinate, 0 },
            { TransformCoordinate, 1 },
            { TransformCoordinate, 2 },
            Factor,
            Size,
            {},
            {},
            Phase,
            NoiseLevels,
            false,
            {},
            {},
            ThresholdHigh,
            ThresholdLow,
            {},
            {}
        });

    UMaterialExpressionIf* PerlinNormal = NewMaterialExpressionIfEqual(Function, Type, 2.0f, PerlinNormalWithAbsoluteNoise, PerlinNormalWithoutAbsoluteNoise);
    UMaterialExpressionMultiply* F =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionFunctionCall(Function, MathAverage, { NewMaterialExpressionSubtract(Function, Color1, Color2) }),
            Factor);

    UMaterialExpressionMaterialFunctionCall* Normal =
        NewMaterialExpressionFunctionCall(Function, DCCSupportCombineNormals,
        {
            F,
            PerlinNormal,
            NewMaterialExpressionOneMinus(Function, F),
            NewMaterialExpressionFunctionCall(Function, StateNormal,{})
        });

    NewMaterialExpressionFunctionOutput(Function, "normal", Normal);
}

static void Generator_NVIDIADCCSupportAD3DSMaxStandardTextureOutput(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* MapTint = NewMaterialExpressionFunctionInput(Function, TEXT("map_tint"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* MapMono = NewMaterialExpressionFunctionInput(Function, TEXT("map_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* BumpAmount = NewMaterialExpressionFunctionInput(Function, TEXT("bump_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Clamp = NewMaterialExpressionFunctionInput(Function, TEXT("clamp"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Invert = NewMaterialExpressionFunctionInput(Function, TEXT("invert"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* AlphaFromRGB = NewMaterialExpressionFunctionInput(Function, TEXT("alphaFromRGB"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* OutputAmount = NewMaterialExpressionFunctionInput(Function, TEXT("output_amount"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBLevel = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_level"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* RGBOffset = NewMaterialExpressionFunctionInput(Function, TEXT("rgb_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionAdd* Tint = NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, { MapTint, RGBLevel, RGBOffset }), RGBOffset);
    UMaterialExpressionAdd* Alpha = NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, { MapMono, RGBLevel, RGBOffset }), RGBOffset);
    //UMaterialExpressionIf* ResultTint = NewMaterialExpressionStaticSwitch(Function, Invert,
    //	NewMaterialExpressionSubtract(Function, NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f), Tint),
    //	Tint);
    UMaterialExpressionIf* ResultTint = NewMaterialExpressionStaticSwitch(Function, Invert, NewMaterialExpressionOneMinus(Function, Tint), Tint);
    UMaterialExpressionIf* ResultMono = NewMaterialExpressionStaticSwitch(Function, Invert, NewMaterialExpressionOneMinus(Function, Alpha), Alpha);

    NewMaterialExpressionFunctionOutput(Function, "tint", ResultTint);
    NewMaterialExpressionFunctionOutput(Function, "mono", ResultMono);
}

static void Generator_NVIDIADCCSupportCombineNormals(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DCCSupportTransformInternalToTangent = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_transform_internal_to_tangent"));
    UMaterialFunction* DCCSupportTransformTangentToInternal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_nvidia_DCC_support_transform_tangent_to_internal"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Returns a normal as a weighted combination of two normals.");

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* N1 = NewMaterialExpressionFunctionInput(Function, TEXT("n1"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* N2 = NewMaterialExpressionFunctionInput(Function, TEXT("n2"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionConstant3Vector* ZAxis = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f);
    UMaterialExpressionAdd* N1Temp = NewMaterialExpressionAdd(Function,
        NewMaterialExpressionLinearInterpolate(Function,
            ZAxis,
            NewMaterialExpressionFunctionCall(Function, DCCSupportTransformInternalToTangent, { N1 }),
            W1),
        ZAxis);
    UMaterialExpressionMultiply* N2Temp = NewMaterialExpressionMultiply(Function,
        NewMaterialExpressionLinearInterpolate(Function,
            ZAxis,
            NewMaterialExpressionFunctionCall(Function, DCCSupportTransformInternalToTangent, { N2 }),
            W2),
        NewMaterialExpressionConstant(Function, -1.0f, -1.0f, 1.0f));

    UMaterialExpressionSubtract* N = NewMaterialExpressionSubtract(Function,
        NewMaterialExpressionDivide(Function,
            NewMaterialExpressionMultiply(Function, N1Temp, NewMaterialExpressionDotProduct(Function, N1Temp, N2Temp)),
            NewMaterialExpressionComponentMask(Function, N1Temp, 4)),
        N2Temp);

    UMaterialExpressionNormalize* Result = NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, DCCSupportTransformTangentToInternal, { N }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("combined_normal"), Result);
}

static void Generator_NVIDIADCCSupportMaxRotationTranslationScale(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* MathMultiplyFloat4x4Float4x4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4x4"));

    Function->Description = TEXT("Construct transformation matrix from Euler rotation, translation and scale.");

    UMaterialExpressionFunctionInput* Rotation = NewMaterialExpressionFunctionInput(Function, TEXT("rotation"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Translation = NewMaterialExpressionFunctionInput(Function, TEXT("translation"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Scaling = NewMaterialExpressionFunctionInput(Function, TEXT("scaling"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* UMirror = NewMaterialExpressionFunctionInput(Function, TEXT("u_mirror"), EFunctionInputType::FunctionInput_Scalar, { false });
    UMaterialExpressionFunctionInput* VMirror = NewMaterialExpressionFunctionInput(Function, TEXT("v_mirror"), EFunctionInputType::FunctionInput_Scalar, { false });

    UMaterialExpressionComponentMask* ScaleX = NewMaterialExpressionComponentMask(Function, Scaling, 1);
    UMaterialExpressionComponentMask* ScaleY = NewMaterialExpressionComponentMask(Function, Scaling, 2);
    UMaterialExpressionComponentMask* ScaleZ = NewMaterialExpressionComponentMask(Function, Scaling, 4);

    UMaterialExpressionMaterialFunctionCall* Scale0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            { NewMaterialExpressionStaticSwitch(Function, UMirror, NewMaterialExpressionMultiply(Function, ScaleX, 2.0f), ScaleX), 0.0f, 0.0f, 0.0f });
    UMaterialExpressionMaterialFunctionCall* Scale1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            { 0.0f, NewMaterialExpressionStaticSwitch(Function, VMirror, NewMaterialExpressionMultiply(Function, ScaleY, 2.0f), ScaleY), 0.0f, 0.0f });
    UMaterialExpressionMaterialFunctionCall* Scale2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4, { 0.0f, 0.0f, ScaleZ, 0.0f });
    UMaterialExpressionMaterialFunctionCall* Scale3 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
            { NewMaterialExpressionStaticSwitch(Function, UMirror, 1.0f, 0.5f), NewMaterialExpressionStaticSwitch(Function, VMirror, 1.0f, 0.5f), 0.5f, 1.0f });

    UMaterialExpressionConstant4Vector* Translate0 = NewMaterialExpressionConstant(Function, 1.0f, 0.0f, 0.0f, 0.0f);
    UMaterialExpressionConstant4Vector* Translate1 = NewMaterialExpressionConstant(Function, 0.0f, 1.0f, 0.0f, 0.0f);
    UMaterialExpressionConstant4Vector* Translate2 = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f, 0.0f);
    UMaterialExpressionMaterialFunctionCall* Translate3 = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionSubtract(Function, -0.5f, NewMaterialExpressionComponentMask(Function, Translation, 1)),
        NewMaterialExpressionSubtract(Function, -0.5f, NewMaterialExpressionComponentMask(Function, Translation, 2)),
        NewMaterialExpressionSubtract(Function, -0.5f, NewMaterialExpressionComponentMask(Function, Translation, 4)),
        1.0f
    });

    // Euler rotation matrix  xyz order
    UMaterialExpressionComponentMask* RX = NewMaterialExpressionComponentMask(Function, Rotation, 1);
    UMaterialExpressionComponentMask* RY = NewMaterialExpressionComponentMask(Function, Rotation, 2);
    UMaterialExpressionComponentMask* RZ = NewMaterialExpressionComponentMask(Function, Rotation, 4);
    UMaterialExpressionSine* SX = NewMaterialExpressionSine(Function, RX);
    UMaterialExpressionSine* SY = NewMaterialExpressionSine(Function, RY);
    UMaterialExpressionSine* SZ = NewMaterialExpressionSine(Function, RZ);
    UMaterialExpressionCosine* CX = NewMaterialExpressionCosine(Function, RX);
    UMaterialExpressionCosine* CY = NewMaterialExpressionCosine(Function, RY);
    UMaterialExpressionCosine* CZ = NewMaterialExpressionCosine(Function, RZ);

    UMaterialExpressionMaterialFunctionCall* Rotate0 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionMultiply(Function, CY, CZ),
            NewMaterialExpressionMultiply(Function, CY, SZ),
            NewMaterialExpressionNegate(Function, SY),
            0.0f
        });
    UMaterialExpressionMaterialFunctionCall* Rotate1 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function,{ SX, SY, CZ }), NewMaterialExpressionMultiply(Function, CX, SZ)),
            NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function,{ SX, SY, SZ }), NewMaterialExpressionMultiply(Function, CX, CZ)),
            NewMaterialExpressionMultiply(Function, SX, CY),
            0.0f
        });
    UMaterialExpressionMaterialFunctionCall* Rotate2 =
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,
        {
            NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function,{ CX, SY, CZ }), NewMaterialExpressionMultiply(Function, SX, SZ)),
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function,{ CX, SY, SZ }), NewMaterialExpressionMultiply(Function, SX, CZ)),
            NewMaterialExpressionMultiply(Function, CX, CY),
            0.0f
        });
    UMaterialExpressionConstant4Vector* Rotate3 = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f, 1.0f);

    UMaterialExpressionMaterialFunctionCall* ScaleByRotate =
        NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4, { Scale0, Scale1, Scale2, Scale3, Rotate0, Rotate1, Rotate2, Rotate3 });
    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4, { {ScaleByRotate, 0}, {ScaleByRotate, 1}, {ScaleByRotate, 2}, {ScaleByRotate, 3}, Translate0, Translate1, Translate2, Translate3 });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), { Result, 0 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), { Result, 1 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), { Result, 2 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_3"), { Result, 3 });
}

static void Generator_NVIDIADCCSupportTransformInternalToTangent(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Returns the normal n in tangent space, given n is in internal space.");

    UMaterialExpressionMaterialFunctionCall* Normal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});
    UMaterialExpressionMaterialFunctionCall* TextureTangentU = NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 });
    UMaterialExpressionMaterialFunctionCall* TextureTangentV = NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 });

    UMaterialExpressionFunctionInput* N = NewMaterialExpressionFunctionInput(Function, TEXT("n"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionAdd* Result = NewMaterialExpressionAdd(Function,
    {
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionComponentMask(Function, N, 1),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, TextureTangentU, 1),
                NewMaterialExpressionComponentMask(Function, TextureTangentV, 1),
                NewMaterialExpressionComponentMask(Function, Normal, 1)
            })),
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionComponentMask(Function, N, 2),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, TextureTangentU, 2),
                NewMaterialExpressionComponentMask(Function, TextureTangentV, 2),
                NewMaterialExpressionComponentMask(Function, Normal, 2)
            })),
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionComponentMask(Function, N, 4),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionComponentMask(Function, TextureTangentU, 4),
                NewMaterialExpressionComponentMask(Function, TextureTangentV, 4),
                NewMaterialExpressionComponentMask(Function, Normal, 4)
            }))
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_space_normal"), Result);
}

static void Generator_NVIDIADCCSupportTransformTangentToInternal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Returns the normal n in internal space, given n is in tangent space.");

    UMaterialExpressionFunctionInput* N = NewMaterialExpressionFunctionInput(Function, TEXT("n"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionAdd* Result = NewMaterialExpressionAdd(Function,
    {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }), NewMaterialExpressionComponentMask(Function, N, 1)),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }), NewMaterialExpressionComponentMask(Function, N, 2)),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, StateNormal, {}), NewMaterialExpressionComponentMask(Function, N, 4)),
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("internal_space_normal"), Result);
}

static void Generator_NVIDIADistillingSupportAddDetailNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Returns a normal by adding a detail normal to a global normal.");

    UMaterialExpressionFunctionInput* ND = NewMaterialExpressionFunctionInput(Function, TEXT("nd"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionFunctionInput* N = NewMaterialExpressionFunctionInput(Function, TEXT("n"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionAdd* NT = NewMaterialExpressionAdd(Function, N, NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f));
    UMaterialExpressionMultiply* NDT = NewMaterialExpressionMultiply(Function, ND, NewMaterialExpressionConstant(Function, -1.0f, -1.0f, 1.0f));
    UMaterialExpressionNormalize* Result = NewMaterialExpressionNormalize(Function,
        NewMaterialExpressionSubtract(Function,
            NewMaterialExpressionMultiply(Function,
                NT,
                NewMaterialExpressionDivide(Function, NewMaterialExpressionDotProduct(Function, NT, NDT), NewMaterialExpressionComponentMask(Function, NT, 4))),
            NDT));

    NewMaterialExpressionFunctionOutput(Function, TEXT("n"), Result);
}

static void Generator_NVIDIADistillingSupportAverageFloatFloatFloatFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the weighted and re-normalized average of r1 and r2. Returns 0 if w1 + w2 is too small.");

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R1 = NewMaterialExpressionFunctionInput(Function, TEXT("r1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R2 = NewMaterialExpressionFunctionInput(Function, TEXT("r2"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionAdd* Sum = NewMaterialExpressionAdd(Function, W1, W2);
    UMaterialExpressionIf* Result = NewMaterialExpressionIfGreater(Function,
        NewMaterialExpressionAbs(Function, Sum),
        NewMaterialExpressionConstant(Function, 0.00001f),
        NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, W1, R1), NewMaterialExpressionMultiply(Function, W2, R2)), Sum),
        NewMaterialExpressionConstant(Function, 0.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportAverageFloatFloatFloatFloatFloatFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the weighted and re-normalized average of r1, r2 and r3. Returns 0 if w1 + w2 + w3 is too small.");

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R1 = NewMaterialExpressionFunctionInput(Function, TEXT("r1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R2 = NewMaterialExpressionFunctionInput(Function, TEXT("r2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* W3 = NewMaterialExpressionFunctionInput(Function, TEXT("w3"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R3 = NewMaterialExpressionFunctionInput(Function, TEXT("r3"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionAdd* Sum = NewMaterialExpressionAdd(Function, { W1, W2, W3 });
    UMaterialExpressionIf* Result = NewMaterialExpressionIfGreater(Function,
        NewMaterialExpressionAbs(Function, Sum),
        NewMaterialExpressionConstant(Function, 0.00001f),
        NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, { NewMaterialExpressionMultiply(Function, W1, R1), NewMaterialExpressionMultiply(Function, W2, R2), NewMaterialExpressionMultiply(Function, W3, R3) } ), Sum),
        NewMaterialExpressionConstant(Function, 0.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportAverageFloatColorFloatColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the weighted and re-normalized average of r1 and r2. Returns (0,0,0) if w1 + w2 is too small.");

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R1 = NewMaterialExpressionFunctionInput(Function, TEXT("r1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R2 = NewMaterialExpressionFunctionInput(Function, TEXT("r2"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionAdd* Sum = NewMaterialExpressionAdd(Function, W1, W2);
    UMaterialExpressionDivide* Yes = NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, W1, R1), NewMaterialExpressionMultiply(Function, W2, R2)), Sum);
    UMaterialExpressionIf *Result = NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, Sum), NewMaterialExpressionConstant(Function, 0.00001f), Yes, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 0.0f, 0.0f, 0.0f }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportAverageFloatColorFloatColorFloatColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the weighted and re-normalized average of r1, r2 and r3. Returns (0,0,0) if w1 + w2 + w3 is too small.");

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R1 = NewMaterialExpressionFunctionInput(Function, TEXT("r1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R2 = NewMaterialExpressionFunctionInput(Function, TEXT("r2"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* W3 = NewMaterialExpressionFunctionInput(Function, TEXT("w3"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R3 = NewMaterialExpressionFunctionInput(Function, TEXT("r3"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionAdd* Sum = NewMaterialExpressionAdd(Function, {W1, W2, W3} );
    UMaterialExpressionDivide* Yes = NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, { NewMaterialExpressionMultiply(Function, W1, R1), NewMaterialExpressionMultiply(Function, W2, R2), NewMaterialExpressionMultiply(Function, W3, R3) }), Sum);
    UMaterialExpressionIf *Result = NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, Sum), NewMaterialExpressionConstant(Function, 0.00001f), Yes, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 0.0f, 0.0f, 0.0f }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportCombineAnisotropicRoughness(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Combines the two anisotropic roughness values into one. The current heuristic chooses the lower roughness. Just averaging leads to too dull looking materials.");

    UMaterialExpressionFunctionInput* R1 = NewMaterialExpressionFunctionInput(Function, TEXT("r1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* R2 = NewMaterialExpressionFunctionInput(Function, TEXT("r2"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionMin* Result = NewMaterialExpressionMin(Function, R1, R2);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportAffineNormalSumFloatFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns a normal for a weighted combination of n normals.");

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialExpressionMaterialFunctionCall* N = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* N1 = NewMaterialExpressionFunctionInput(Function, TEXT("n1"), EFunctionInputType::FunctionInput_Vector3, N);
    UMaterialExpressionMultiply* A = NewMaterialExpressionMultiply(Function,
        NewMaterialExpressionSubtract(Function, N1, N), NewMaterialExpressionFunctionCall(Function, MakeFloat3, { W1, W1, W1 }));
    UMaterialExpressionNormalize* Result = NewMaterialExpressionNormalize(Function, NewMaterialExpressionAdd(Function, A, N));

    NewMaterialExpressionFunctionOutput(Function, TEXT("n"), Result);
}


static void Generator_NVIDIADistillingSupportAffineNormalSumFloatFloat3FloatFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns a normal for a weighted combination of n normals.");

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialExpressionMaterialFunctionCall* N = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* N1 =
        NewMaterialExpressionFunctionInput(Function, TEXT("n1"), EFunctionInputType::FunctionInput_Vector3, N);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* N2 =
        NewMaterialExpressionFunctionInput(Function, TEXT("n2"), EFunctionInputType::FunctionInput_Vector3, N);

    UMaterialExpressionNormalize* Result = NewMaterialExpressionNormalize(Function, NewMaterialExpressionAdd(Function, {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, N1, N), NewMaterialExpressionFunctionCall(Function, MakeFloat3, { W1, W1, W1 })),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, N2, N), NewMaterialExpressionFunctionCall(Function, MakeFloat3, { W2, W2, W2 })),
        N }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("n"), Result);
}

static void Generator_NVIDIADistillingSupportAffineNormalSumFloatFloat3FloatFloat3FloatFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns a normal for a weighted combination of n normals.");

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialExpressionMaterialFunctionCall* N = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* N1 =
        NewMaterialExpressionFunctionInput(Function, TEXT("n1"), EFunctionInputType::FunctionInput_Vector3, N);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* N2 =
        NewMaterialExpressionFunctionInput(Function, TEXT("n2"), EFunctionInputType::FunctionInput_Vector3, N);
    UMaterialExpressionFunctionInput* W3 = NewMaterialExpressionFunctionInput(Function, TEXT("w3"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* N3 =
        NewMaterialExpressionFunctionInput(Function, TEXT("n3"), EFunctionInputType::FunctionInput_Vector3, N);

    UMaterialExpressionNormalize* Result = NewMaterialExpressionNormalize(Function, NewMaterialExpressionAdd(Function, {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, N1, N), NewMaterialExpressionFunctionCall(Function, MakeFloat3, { W1, W1, W1 })),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, N2, N), NewMaterialExpressionFunctionCall(Function, MakeFloat3, { W2, W2, W2 })),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, N3, N), NewMaterialExpressionFunctionCall(Function, MakeFloat3, { W3, W3, W3 })),
        N }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("n"), Result);
}

static void Generator_NVIDIADistillingSupportCombineNormals(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    //alternative way of combining the 2 normals
    //float3 n = float3(n1_t.x + n2_t.x,n1_t.y + n2_t.y, n1_t.z*n2_t.z);

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    Function->Description = TEXT("Returns a normal as a weighted combination of two normals.");

    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionConstant(Function, 1.0f));
    UMaterialExpressionFunctionInput* N1 = NewMaterialExpressionFunctionInput(Function, TEXT("n1"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionConstant(Function, 1.0f));
    UMaterialExpressionFunctionInput* N2 = NewMaterialExpressionFunctionInput(Function, TEXT("n2"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionConstant3Vector* Z = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f);
    UMaterialExpressionAdd* N1T = NewMaterialExpressionAdd(Function, NewMaterialExpressionLinearInterpolate(Function, Z, N1, W1), Z);
    UMaterialExpressionMultiply* N2T = NewMaterialExpressionMultiply(Function,
        NewMaterialExpressionLinearInterpolate(Function, Z, N2, W2),
        NewMaterialExpressionConstant(Function, -1.0f, -1.0f, 1.0f));
    UMaterialExpressionNormalize* N = NewMaterialExpressionNormalize(Function,
        NewMaterialExpressionSubtract(Function,
            NewMaterialExpressionMultiply(Function,
                N1T,
                NewMaterialExpressionDivide(Function, NewMaterialExpressionDotProduct(Function, N1T, N2T), NewMaterialExpressionComponentMask(Function, N1T, 4))),
            N2T));

    NewMaterialExpressionFunctionOutput(Function, TEXT("n"), N);
}

static void Generator_NVIDIADistillingSupportDirectionalColoring(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("A helper function, marked as \"noinline\", to communicate directional dependent coloring information to be picked up by later passes and integration code. Note: this cannot be expressed fully functional in MDL itself, it targets a different model.");

    UMaterialExpressionFunctionInput* C_0 = NewMaterialExpressionFunctionInput(Function, TEXT("c_0"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* C_90 = NewMaterialExpressionFunctionInput(Function, TEXT("c_90"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* E = NewMaterialExpressionFunctionInput(Function, TEXT("e"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpression* Fresnel = NewMaterialExpressionFresnel(Function, 5.0f);
    UMaterialExpression* Result = NewMaterialExpressionLinearInterpolate(Function, { C_0, C_90 }, Fresnel);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportDirectionalWeighting(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("A helper function, marked as \"noinline\", to communicate directional dependent coloring information to be picked up by later passes and integration code. Note: this cannot be expressed fully functional in MDL itself, it targets a different model.");

    UMaterialExpressionFunctionInput* W_0 = NewMaterialExpressionFunctionInput(Function, TEXT("w_0"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* W_90 = NewMaterialExpressionFunctionInput(Function, TEXT("w_90"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* E = NewMaterialExpressionFunctionInput(Function, TEXT("e"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpression* Fresnel = NewMaterialExpressionFresnel(Function, 5.0f);
    UMaterialExpression* Result = NewMaterialExpressionLinearInterpolate(Function, { W_0, W_90 }, Fresnel);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportPartNormalized(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* A = NewMaterialExpressionFunctionInput(Function, TEXT("a"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* W1 = NewMaterialExpressionFunctionInput(Function, TEXT("w1"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* W2 = NewMaterialExpressionFunctionInput(Function, TEXT("w2"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionAdd* Sum = NewMaterialExpressionAdd(Function, W1, W2);
    UMaterialExpressionIf* Result = NewMaterialExpressionIfGreater(Function,
        NewMaterialExpressionAbs(Function, Sum),
        NewMaterialExpressionConstant(Function, 0.00001f),
        NewMaterialExpressionDivide(Function, A, Sum),
        NewMaterialExpressionConstant(Function, 0.0f));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_NVIDIADistillingSupportReflFromIORColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns a normal incidence direction reflectivity value for a given IOR.");

    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionDivide* RootR = NewMaterialExpressionDivide(Function,
        NewMaterialExpressionAdd(Function, IOR, -1),
        NewMaterialExpressionAdd(Function, IOR, 1));
    UMaterialExpressionMultiply* Refl = NewMaterialExpressionSquare(Function, RootR);

    NewMaterialExpressionFunctionOutput(Function, TEXT("refl"), Refl);
}

static void Generator_NVIDIADistillingSupportReflFromIORFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns a normal incidence direction reflectivity value for a given IOR.");

    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionDivide* RootR = NewMaterialExpressionDivide(Function,
        NewMaterialExpressionAdd(Function, IOR, -1),
        NewMaterialExpressionAdd(Function, IOR, 1));
    UMaterialExpressionMultiply* Refl = NewMaterialExpressionSquare(Function, RootR);

    NewMaterialExpressionFunctionOutput(Function, TEXT("refl"), Refl);
}

static void Generator_NVIDIADistillingSupportReflFromIORKColorColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns a normal incidence direction reflectivity value for a given complex IOR.");

    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("ior"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* K = NewMaterialExpressionFunctionInput(Function, TEXT("k"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMultiply* K2 = NewMaterialExpressionSquare(Function, K);
    UMaterialExpressionMultiply* IOR2 = NewMaterialExpressionSquare(Function, IOR);
    UMaterialExpressionAdd* RS_Num = NewMaterialExpressionAdd(Function, NewMaterialExpressionSubtract(Function, NewMaterialExpressionAdd(Function, K2, IOR2), NewMaterialExpressionMultiply(Function, IOR, 2.0f)), 1.0f);
    UMaterialExpressionAdd* RS_Den = NewMaterialExpressionAdd(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionAdd(Function, K2, IOR2), NewMaterialExpressionMultiply(Function, IOR, 2.0f)), 1.0f);

    UMaterialExpressionDivide* RS = NewMaterialExpressionDivide(Function, RS_Num, RS_Den);

    UMaterialExpressionSaturate* Refl = NewMaterialExpressionSaturate(Function, RS);

    NewMaterialExpressionFunctionOutput(Function, TEXT("refl"), Refl);
}

static void Generator_StateAnimationTime(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The time of the current sample in seconds, including the time within a shutter interval.");

    UMaterialExpression* Time = NewMaterialExpression<UMaterialExpressionTime>(Function);

    NewMaterialExpressionFunctionOutput(Function, TEXT("animation_time"), Time);
}

static void Generator_StateDirection(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Lookup direction in the context of an environment lookup and float3(0.0) in all other contexts.");

    UMaterialExpressionFunctionInput* CustomWorldNormal = NewMaterialExpressionFunctionInput(Function, TEXT("CustomWorldNormal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpression<UMaterialExpressionPixelNormalWS>(Function));

    UMaterialExpressionReflectionVectorWS* Direction = NewMaterialExpressionReflectionVectorWS(Function, CustomWorldNormal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("direction"), Direction);
}

static void Generator_StateGeometryNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The true geometric surface normal for the current geometry as a unit - length vector.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_normal"), { 0.0f, 0.0f, 1.0f });
}

static void Generator_StateGeometryNormalX(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The true geometric surface normal for the current geometry as a unit - length vector.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_normal"), 0.0f);
}

static void Generator_StateGeometryNormalY(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The true geometric surface normal for the current geometry as a unit - length vector.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_normal"), 0.0f);
}

static void Generator_StateGeometryNormalZ(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The true geometric surface normal for the current geometry as a unit - length vector.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_normal"), 1.0f);
}

static void Generator_StateGeometryTangentU(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry tangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_u"), { 1.0f, 0.0f, 0.0f });
}

static void Generator_StateGeometryTangentUX(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry tangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_u"), 1.0f);
}

static void Generator_StateGeometryTangentUY(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry tangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_u"), 0.0f);
}

static void Generator_StateGeometryTangentUZ(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry tangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_u"), 0.0f);
}

static void Generator_StateGeometryTangentV(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry bitangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    // As UE4 is left handed, the v-tangent is (0, -1, 0) !
    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_v"), { 0.0f, -1.0f, 0.0f });
}

static void Generator_StateGeometryTangentVX(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry bitangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    // As UE4 is left handed, the v-tangent is (0, -1, 0) !
    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_v"), 0.0f);
}

static void Generator_StateGeometryTangentVY(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry bitangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    // As UE4 is left handed, the v-tangent is (0, -1, 0) !
    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_v"), -1.0f);
}

static void Generator_StateGeometryTangentVZ(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Array of geometry bitangents.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    // As UE4 is left handed, the v-tangent is (0, -1, 0) !
    NewMaterialExpressionFunctionOutput(Function, TEXT("geometry_tangent_v"), 0.0f);
}

static void Generator_StateMetersPerSceneUnit(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the distance of one scene unit in meters.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("meters_per_scene_unit"), 0.01f);
}

static void Generator_StateNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The shading surface normal as a unit-length vector.");

    // DAR HACK Need to evaluate the state normal in fragment space to get the clear coat bottom normal working.
    UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/blue_1x1")));

    // Sampling a 1x1 blue texture with constant coordinate.
    UMaterialExpressionTextureSample* NormalSampler = NewMaterialExpressionTextureSample(Function, Texture, {});
    NormalSampler->SamplerType = SAMPLERTYPE_LinearColor;

    //UMaterialExpressionMultiply* Normal = NewMaterialExpressionMultiply(Function, NormalSampler, NewMaterialExpressionTwoSidedSign(Function));

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), { NormalSampler });
}

static void Generator_StateObjectId(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the object ID provided in a scene, and zero if none was given or for the environment.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("object_id"), 0.0f);
}

static void Generator_StatePosition(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The intersection point on the surface or the sample point in the volume.");

    UMaterialExpressionWorldPosition* Position = NewMaterialExpression<UMaterialExpressionWorldPosition>(Function);
    //UMaterialExpressionDivide* PositionInMeter = NewMaterialExpressionDivide(Function, Position, 100.0f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
}

static void Generator_StateSceneUnitsPerMeter(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Returns the distance of one meter in scene units.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("scene_units_per_meter"), 100.0f);
}

static void Generator_StateTangentSpace(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("The array of tangent space matrices for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionMaterialFunctionCall* X = NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { Index });
    UMaterialExpressionMaterialFunctionCall* Y = NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { Index });
    UMaterialExpressionMaterialFunctionCall* Z = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_u"), X);
    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_v"), Y);
    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), Z);

}

static void Generator_StateTextureCoordinate(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The texture space at the given index.");

    UMaterialFunction* BreakFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("BreakFloat2Components.BreakFloat2Components"));

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);
    
    UMaterialExpressionIf* TextureCoordinate = NewMaterialExpressionIfEqual(Function, Index, 0, NewMaterialExpressionTextureCoordinate(Function, 0), 
                                                NewMaterialExpressionIfEqual(Function, Index, 1, NewMaterialExpressionTextureCoordinate(Function, 1), 
                                                NewMaterialExpressionIfEqual(Function, Index, 2, NewMaterialExpressionTextureCoordinate(Function, 2),
                                                NewMaterialExpressionTextureCoordinate(Function, 3))));

    // we need to OneMinus the y-component of the texture coordinate to get into the same orientation as mdl expects it
    // -> later on, in the TexLookupN functions, the y-component is OneMinus'ed back again, as UE turns the textures upside down as well
    UMaterialExpressionMaterialFunctionCall* TextureCoordinatesBreak = NewMaterialExpressionFunctionCall(Function, BreakFloat2, { TextureCoordinate });
    UMaterialExpressionAppendVector* FinalTextureCoordinate =
        NewMaterialExpressionAppendVector(Function, { TextureCoordinatesBreak, 0 }, NewMaterialExpressionOneMinus(Function, { TextureCoordinatesBreak, 1 }), 0.0f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_coordinate"), FinalTextureCoordinate);
}

static void Generator_StateTextureSpaceMax(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The maximal number of texture spaces available.");

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_space_max"), 1.0f);
}

static void Generator_StateTextureTangentU(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of tangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_u"), { 1.0f, 0.0f, 0.0f });
}

static void Generator_StateTextureTangentUX(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of tangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_u"), 1.0f);
}

static void Generator_StateTextureTangentUY(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of tangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_u"), 0.0f);
}

static void Generator_StateTextureTangentUZ(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of tangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_u"), 0.0f);
}

static void Generator_StateTextureTangentV(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of bitangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_v"), { 0.0f, 1.0f, 0.0f });
}

static void Generator_StateTextureTangentVX(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of bitangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_v"), 0.0f);
}

static void Generator_StateTextureTangentVY(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of bitangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_v"), 1.0f);
}

static void Generator_StateTextureTangentVZ(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("The array of bitangent vectors for each texture space.");

    UMaterialExpressionFunctionInput* Index = NewMaterialExpressionFunctionInput(Function, TEXT("index"), EFunctionInputType::FunctionInput_Scalar);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texture_tangent_v"), 0.0f);
}

static void Generator_StateTransformPoint(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // for points, internal space is world space !
    Function->Description = TEXT("Transforms a point from one coordinate system to another.");

    UMaterialExpressionFunctionInput* From = NewMaterialExpressionFunctionInput(Function, TEXT("from"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* To = NewMaterialExpressionFunctionInput(Function, TEXT("to"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Point = NewMaterialExpressionFunctionInput(Function, TEXT("point"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionTransformPosition* ObjectToWorld = NewMaterialExpressionTransformPosition(Function, Point, TRANSFORMPOSSOURCE_Local, TRANSFORMPOSSOURCE_World);
    UMaterialExpressionTransformPosition* WorldToObject = NewMaterialExpressionTransformPosition(Function, Point, TRANSFORMPOSSOURCE_World, TRANSFORMPOSSOURCE_Local);

    // for points, internal space equals world space
    UMaterialExpressionIf* ToInternal = NewMaterialExpressionIf(Function, From, coordinate_object, Point, ObjectToWorld, Point);
    UMaterialExpressionIf* ToObject = NewMaterialExpressionIf(Function, From, coordinate_object, WorldToObject, Point, WorldToObject);
    UMaterialExpressionIf* ToWorld = NewMaterialExpressionIf(Function, From, coordinate_object, Point, ObjectToWorld, Point);

    UMaterialExpressionIf* Transformed = NewMaterialExpressionIf(Function, To, coordinate_object, ToInternal, ToObject, ToWorld);

    NewMaterialExpressionFunctionOutput(Function, TEXT("transformed"), Transformed);
}

static void Generator_StateTransformVector(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    Function->Description = TEXT("Transforms a vector from one coordinate system to another.");

    UMaterialExpressionFunctionInput* From = NewMaterialExpressionFunctionInput(Function, TEXT("from"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* To = NewMaterialExpressionFunctionInput(Function, TEXT("to"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionTransform* WorldToObject = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_World, TRANSFORM_Local);
    UMaterialExpressionTransform* ObjectToWorld = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_Local, TRANSFORM_World);
    UMaterialExpressionTransform* WorldToTangent = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_World, TRANSFORM_Tangent);
    UMaterialExpressionTransform* TangentToWorld = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_Tangent, TRANSFORM_World);
    UMaterialExpressionTransform* ObjectToTangent = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_Local, TRANSFORM_Tangent);
    UMaterialExpressionTransform* TangentToObject = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_Tangent, TRANSFORM_Local);

    // for vectors, internal space equals world space
    UMaterialExpressionIf* ToInternal = NewMaterialExpressionIfEqual(Function, From, coordinate_internal, Vector, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_object, ObjectToWorld, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_world, Vector,
                                                TangentToWorld)));

    UMaterialExpressionIf* ToObject = NewMaterialExpressionIfEqual(Function, From, coordinate_internal, WorldToObject, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_object, Vector, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_world, WorldToObject,
                                                TangentToObject)));

    UMaterialExpressionIf* ToWorld = NewMaterialExpressionIfEqual(Function, From, coordinate_internal, Vector, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_object, ObjectToWorld, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_world, Vector,
                                                TangentToWorld)));
    // add tangent space internally
    UMaterialExpressionIf* ToTangent = NewMaterialExpressionIfEqual(Function, From, coordinate_internal, WorldToTangent, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_object, ObjectToTangent, 
                                                NewMaterialExpressionIfEqual(Function, From, coordinate_world, WorldToTangent,
                                                Vector)));

    UMaterialExpressionIf* Transformed = NewMaterialExpressionIfEqual(Function, To, coordinate_internal, ToInternal, 
                                                NewMaterialExpressionIfEqual(Function, To, coordinate_object, ToObject, 
                                                NewMaterialExpressionIfEqual(Function, To, coordinate_world, ToWorld,
                                                ToTangent)));

    NewMaterialExpressionFunctionOutput(Function, TEXT("transformed"), Transformed);
}

static void Generator_Tex2DLookupFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texture2d_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), { Sample, 1 });
}

static void Generator_Tex2DLookupFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texture2d_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, { Sample, 1 }, { Sample, 2 }));
}

static void Generator_Tex2DLookupFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texture2d_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Sample);
}

static void Generator_Tex2DLookupFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texture2d_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, { Sample, 0 }, { Sample, 4 }));
}

static void Generator_TexNormalLookupFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float_2d"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV, Frame });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionMultiply(Function, NewMaterialExpressionAdd(Function, Sample, 1.0f), 0.5f));
}

static void Generator_TexNormalLookupFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float2_2d"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV, Frame });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionMultiply(Function, NewMaterialExpressionAdd(Function, Sample, 1.0f), 0.5f));
}

static void Generator_TexNormalLookupFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float3_2d"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV, Frame });
    UMaterialExpressionComponentMask* MaskRG = NewMaterialExpressionComponentMask(Function, Sample, 3);
    UMaterialExpressionMultiply* PackNormal = NewMaterialExpressionMultiply(Function, NewMaterialExpressionAdd(Function, MaskRG, 1.0f), 0.5f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, PackNormal, 1.0f));
}

static void Generator_TexNormalLookupFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* Frame = NewMaterialExpressionFunctionInput(Function, TEXT("frame"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord, WrapU, WrapV, CropU, CropV, Frame });
    UMaterialExpressionComponentMask* MaskRG = NewMaterialExpressionComponentMask(Function, Sample, 3);
    UMaterialExpressionMultiply* PackNormal = NewMaterialExpressionMultiply(Function, NewMaterialExpressionAdd(Function, MaskRG, 1.0f), 0.5f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, PackNormal, NewMaterialExpressionConstant(Function, 1.0f, 1.0f)));
}

static void Generator_TexCubeLookupFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texturecube_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_TextureCube, NewMaterialExpressionTextureObject(Function, LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), { Sample, 1 });
}

static void Generator_TexCubeLookupFloat2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texturecube_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_TextureCube, NewMaterialExpressionTextureObject(Function, LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, { Sample, 1 }, { Sample, 2 }));
}

static void Generator_TexCubeLookupFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texturecube_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_TextureCube, NewMaterialExpressionTextureObject(Function, LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Sample);
}

static void Generator_TexCubeLookupFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTextureSample = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texturecube_sample"));

    Function->Description = TEXT("Returns the sampled texture value for the twodimensional coordinates coord given in normalized texture space in the range [0, 1)^2, where the wrap"
        "modes define the behavior for coordinates outside of that range. The crop parameters further define a sub - range on the texture that is actually"
        "used and that defines the normalized texture space in the range [0, 1)^2. The crop parameter defaults float2(0.0, 1.0) corresponds to the whole texture"
        "in the corresponding axis. A lookup on an invalid texture reference returns zero.");

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_TextureCube, NewMaterialExpressionTextureObject(Function, LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Sample = NewMaterialExpressionFunctionCall(Function, ImporterTextureSample, { Tex, Coord });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, { Sample, 0 }, { Sample, 4 }));
}

static void Generator_ImporterAddDetailNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Detail = NewMaterialExpressionFunctionInput(Function, TEXT("detail"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionAdd* TangentSpaceNormal =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionTransform(Function, Normal, TRANSFORMSOURCE_Local, TRANSFORM_Tangent),
            NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f));
    UMaterialExpressionMultiply* TangentSpaceDetail =
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionTransform(Function, Detail, TRANSFORMSOURCE_Local, TRANSFORM_Tangent),
            NewMaterialExpressionConstant(Function, -1.0f, -1.0f, 1.0f));
    UMaterialExpressionNormalize* DetailNormal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionTransform(Function,
                NewMaterialExpressionSubtract(Function,
                    NewMaterialExpressionMultiply(Function,
                        TangentSpaceNormal,
                        NewMaterialExpressionDivide(Function,
                            NewMaterialExpressionDotProduct(Function, TangentSpaceNormal, TangentSpaceDetail),
                            NewMaterialExpressionComponentMask(Function, TangentSpaceNormal, 4))),
                    TangentSpaceDetail),
                TRANSFORMSOURCE_Tangent, TRANSFORM_Local));

    NewMaterialExpressionFunctionOutput(Function, TEXT("detail_normal"), DetailNormal);
}

static void Generator_ImporterApplyNoiseModifications(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("value"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("position"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* ApplyMarble = NewMaterialExpressionFunctionInput(Function, TEXT("apply_marble"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* ApplyDent = NewMaterialExpressionFunctionInput(Function, TEXT("apply_dent"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* NoiseThresholdHigh = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_high"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* NoiseThresholdLow = NewMaterialExpressionFunctionInput(Function, TEXT("noise_threshold_low"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* NoiseBands = NewMaterialExpressionFunctionInput(Function, TEXT("noise_bands"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionIf* ApplyMarbleCheck =
        NewMaterialExpressionStaticSwitch(Function, ApplyMarble,
            NewMaterialExpressionCosine(Function, NewMaterialExpressionAdd(Function, Position, NewMaterialExpressionMultiply(Function, Value, 5.0f))),
            Value);

    UMaterialExpressionIf* ApplyDentCheck =
        NewMaterialExpressionStaticSwitch(Function, ApplyDent,
            NewMaterialExpressionMultiply(Function, { ApplyMarbleCheck, ApplyMarbleCheck, ApplyMarbleCheck }),
            ApplyMarbleCheck);

    UMaterialExpressionPower* NoiseBandsResult =
        NewMaterialExpressionPower(Function,
            NewMaterialExpressionOneMinus(Function, NewMaterialExpressionFrac(Function, NewMaterialExpressionMultiply(Function, ApplyDentCheck, NoiseBands))),
            20.0f);
    UMaterialExpressionIf* NoiseBandsCheck = NewMaterialExpressionIfEqual(Function, NoiseBands, 1.0f, ApplyDentCheck, NoiseBandsResult);

    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfGreater(Function, NoiseThresholdHigh, NoiseThresholdLow,
            NewMaterialExpressionClamp(Function,
                NewMaterialExpressionDivide(Function,
                    NewMaterialExpressionSubtract(Function, NoiseBandsCheck, NoiseThresholdLow),
                    NewMaterialExpressionSubtract(Function, NoiseThresholdHigh, NoiseThresholdLow)),
                0.0f,
                1.0f),
            NoiseBandsCheck);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterBlendClearCoat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Base = NewMaterialExpressionFunctionInput(Function, TEXT("base"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* Layer = NewMaterialExpressionFunctionInput(Function, TEXT("layer"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* Alpha = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* LayerNormal = NewMaterialExpressionFunctionInput(Function, TEXT("layer_normal"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* AdjustedLayer = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_ReplaceNormals")), { Layer, LayerNormal });
    UMaterialExpressionMaterialFunctionCall* Blend = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Standard")), { Base, AdjustedLayer, Alpha });

    UMaterialExpressionBreakMaterialAttributes* BaseBreak = NewMaterialExpressionBreakMaterialAttributes(Function, Base);
    UMaterialExpressionBreakMaterialAttributes* LayerBreak = NewMaterialExpressionBreakMaterialAttributes(Function, AdjustedLayer);
    UMaterialExpressionBreakMaterialAttributes* BlendBreak = NewMaterialExpressionBreakMaterialAttributes(Function, Blend);

    // use ClearCoat as some kind of flag: if it's set, functions getting this as input know, there has been a "clearcoatable" layer
    UMaterialExpressionConstant* ClearCoat = NewMaterialExpressionConstant(Function, 1.0f);

    // blend the ClearCoatRoughness out of the Roughness and the ClearCoatRoughness of the Layer, using its ClearCoat as the weight
    UMaterialExpressionLinearInterpolate* ClearCoatRoughness = NewMaterialExpressionLinearInterpolate(Function, { LayerBreak, MAI_Roughness }, { LayerBreak, MAI_CustomData1 }, { LayerBreak, MAI_CustomData0 });

    UMaterialExpressionMakeMaterialAttributes* BSDF =
        NewMaterialExpressionMakeMaterialAttributes(Function,
            { BaseBreak, MAI_BaseColor },		// BaseColor purely from the base
            { BlendBreak, MAI_Metallic },		// Metallic
            { BlendBreak, MAI_Specular },		// Specular
            { LayerBreak, MAI_Roughness },		// Roughness purely from the layer
            { BlendBreak, MAI_EmissiveColor },		// EmissiveColor
            { BlendBreak, MAI_Opacity },		// Opacity,
            { BlendBreak, MAI_OpacityMask },		// OpacityMask
            { LayerBreak, MAI_Normal },		// Normal purely from the layer
            { BlendBreak, MAI_WorldPositionOffset },		// WorldPositionOffset,
            { BlendBreak, MAI_WorldDisplacement },		// WorldDisplacement
            { BlendBreak, MAI_TessellationMultiplier },		// TessellationMultiplier
            { BlendBreak, MAI_SubsurfaceColor },		// SubsurfaceColor,
            ClearCoat,				// ClearCoat
            ClearCoatRoughness,		// ClearCoatRoughness
            { BlendBreak, MAI_AmbientOcclusion },		// AmbientOcclusion,
            { BlendBreak, MAI_Refraction },		// Refraction
            { BlendBreak, MAI_CustomizedUVs0 },		// CustomizedUVs0
            { BlendBreak, MAI_CustomizedUVs1 },		// CustomizedUVs1
            { BlendBreak, MAI_CustomizedUVs2 },		// CustomizedUVs2
            { BlendBreak, MAI_CustomizedUVs3 },		// CustomizedUVs3
            { BlendBreak, MAI_CustomizedUVs4 },		// CustomizedUVs4
            { BlendBreak, MAI_CustomizedUVs5 },		// CustomizedUVs5
            { BlendBreak, MAI_CustomizedUVs6 },		// CustomizedUVs6
            { BlendBreak, MAI_CustomizedUVs7 },		// CustomizedUVs7
            { BlendBreak, MAI_PixelDepthOffset });	// PixelDepthOffset

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_ImporterBlendColors(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    Function->Description = TEXT("Helper function for texture blending");

    UMaterialFunction* ImporterCalculateHue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_calculate_hue"));
    UMaterialFunction* ImporterCalculateSaturation = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_calculate_saturation"));
    UMaterialFunction* ImporterHSVToRGB = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_hsv_to_rgb"));

    UMaterialExpressionFunctionInput* Top = NewMaterialExpressionFunctionInput(Function, TEXT("top"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Bottom = NewMaterialExpressionFunctionInput(Function, TEXT("bottom"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Weight = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionComponentMask* TopX = NewMaterialExpressionComponentMask(Function, Top, 1);
    UMaterialExpressionComponentMask* TopY = NewMaterialExpressionComponentMask(Function, Top, 2);
    UMaterialExpressionComponentMask* TopZ = NewMaterialExpressionComponentMask(Function, Top, 4);
    UMaterialExpressionComponentMask* BottomX = NewMaterialExpressionComponentMask(Function, Bottom, 1);
    UMaterialExpressionComponentMask* BottomY = NewMaterialExpressionComponentMask(Function, Bottom, 2);
    UMaterialExpressionComponentMask* BottomZ = NewMaterialExpressionComponentMask(Function, Bottom, 4);

    UMaterialExpression* Blend = Top;
    UMaterialExpressionAdd* Add = NewMaterialExpressionAdd(Function, Top, Bottom);
    UMaterialExpressionMultiply* Multiply = NewMaterialExpressionMultiply(Function, Top, Bottom);
    UMaterialExpressionMaterialFunctionCall* Screen = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_Screen")), { Top, Bottom });
    UMaterialExpressionMaterialFunctionCall* Overlay = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_Overlay")), { Top, Bottom });
    UMaterialExpressionMax* BrightnessTop = NewMaterialExpressionMax(Function, TopX, TopY, TopZ);
    UMaterialExpressionMax* BrightnessBottom = NewMaterialExpressionMax(Function, BottomX, BottomY, BottomZ);
    UMaterialExpressionIf* Brightness =
        NewMaterialExpressionIfEqual(Function, BrightnessBottom, 0.0f,
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { BrightnessTop, BrightnessTop, BrightnessTop }),
            NewMaterialExpressionMultiply(Function, Bottom, NewMaterialExpressionDivide(Function, BrightnessTop, BrightnessBottom)));
    UMaterialExpressionIf* Color =
        NewMaterialExpressionIfEqual(Function, BrightnessTop, 0.0f,
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { BrightnessBottom, BrightnessBottom, BrightnessBottom }),
            NewMaterialExpressionMultiply(Function, Top, NewMaterialExpressionDivide(Function, BrightnessBottom, BrightnessTop)));
    UMaterialExpressionMaterialFunctionCall* Exclusion = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_Exclusion")), { Top, Bottom });
    UMaterialExpressionMultiply* Average = NewMaterialExpressionMultiply(Function, Add, 0.5f);
    UMaterialExpressionMaterialFunctionCall* Lighten = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_Lighten")), { Top, Bottom });
    UMaterialExpressionMaterialFunctionCall* Darken = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_Darken")), { Top, Bottom });
    UMaterialExpressionSubtract* Sub = NewMaterialExpressionSubtract(Function, Add, 1.0f);
    UMaterialExpressionOneMinus* Negation = NewMaterialExpressionOneMinus(Function, NewMaterialExpressionAbs(Function, NewMaterialExpressionOneMinus(Function, Add)));
    UMaterialExpressionMaterialFunctionCall* Difference = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_Difference")), { Top, Bottom });

    // Note: the build-in Function /Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_SoftLight make something different!!
    UMaterialExpressionMultiply* LessThanPointFive =
        NewMaterialExpressionMultiply(Function,
            2.0f,
            NewMaterialExpressionAdd(Function,
                NewMaterialExpressionMultiply(Function, Top, Bottom),
                NewMaterialExpressionMultiply(Function, { Bottom, Bottom, NewMaterialExpressionSubtract(Function, 0.5f, Top) })));
    UMaterialExpressionMaterialFunctionCall* SquareRootBottom =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionComponentMask(Function, Bottom, 1)),
            NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionComponentMask(Function, Bottom, 2)),
            NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionComponentMask(Function, Bottom, 4))
        });
    UMaterialExpressionMultiply* GreaterThanPointFive =
        NewMaterialExpressionMultiply(Function,
            2.0f,
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionMultiply(Function, SquareRootBottom, NewMaterialExpressionSubtract(Function, Top, 0.5f)), Bottom),
                NewMaterialExpressionMultiply(Function, Top, Bottom)));
    UMaterialExpressionMaterialFunctionCall* SoftLight = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
    {
        NewMaterialExpressionIfLess(Function, TopX, 0.5f, NewMaterialExpressionComponentMask(Function, LessThanPointFive, 1), NewMaterialExpressionComponentMask(Function, GreaterThanPointFive, 1)),
        NewMaterialExpressionIfLess(Function, TopY, 0.5f, NewMaterialExpressionComponentMask(Function, LessThanPointFive, 2), NewMaterialExpressionComponentMask(Function, GreaterThanPointFive, 2)),
        NewMaterialExpressionIfLess(Function, TopZ, 0.5f, NewMaterialExpressionComponentMask(Function, LessThanPointFive, 4), NewMaterialExpressionComponentMask(Function, GreaterThanPointFive, 4))
    });

    UMaterialExpressionMaterialFunctionCall* ColorDodge = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_ColorDodge")), { Top, Bottom });
    UMaterialExpressionMin* ReflectTemp =
        NewMaterialExpressionMin(Function,
            NewMaterialExpressionMultiply(Function, Bottom, NewMaterialExpressionDivide(Function, Bottom, NewMaterialExpressionOneMinus(Function, Top))),
            1.0f);
    UMaterialExpression* Reflect =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionIfEqual(Function, TopX, 1.0f, 1.0f, NewMaterialExpressionComponentMask(Function, ReflectTemp, 1)),
            NewMaterialExpressionIfEqual(Function, TopY, 1.0f, 1.0f, NewMaterialExpressionComponentMask(Function, ReflectTemp, 2)),
            NewMaterialExpressionIfEqual(Function, TopZ, 1.0f, 1.0f, NewMaterialExpressionComponentMask(Function, ReflectTemp, 4))
        });
    UMaterialExpressionMaterialFunctionCall* ColorBurn = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_ColorBurn")), { Top, Bottom });
    UMaterialExpressionAdd* Phoenix =
        NewMaterialExpressionAdd(Function,
            NewMaterialExpressionMin(Function, Top, Bottom),
            NewMaterialExpressionOneMinus(Function, NewMaterialExpressionMax(Function, Top, Bottom)));
    UMaterialExpressionMaterialFunctionCall* HardLight = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_HardLight")), { Top, Bottom });
    UMaterialExpressionMaterialFunctionCall* PinLight = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_PinLight")), { Top, Bottom });
    UMaterialExpression* HardMix =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionIfGreater(Function, NewMaterialExpressionComponentMask(Function, Add, 1), 1.0f, 1.0f, 0.0f),
            NewMaterialExpressionIfGreater(Function, NewMaterialExpressionComponentMask(Function, Add, 2), 1.0f, 1.0f, 0.0f),
            NewMaterialExpressionIfGreater(Function, NewMaterialExpressionComponentMask(Function, Add, 4), 1.0f, 1.0f, 0.0f)
        });
    UMaterialExpressionMaterialFunctionCall* LinearDodge = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_LinearDodge")), { Top, Bottom });
    UMaterialExpressionMaterialFunctionCall* LinearBurn = NewMaterialExpressionFunctionCall(Function, LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions03/Blends"), TEXT("Blend_LinearBurn")), { Top, Bottom });
    UMaterialExpressionMultiply* SpotLight = NewMaterialExpressionMultiply(Function, Multiply, 2.0f);
    UMaterialExpressionAdd* SpotLightBlend = NewMaterialExpressionAdd(Function, Multiply, Bottom);
    UMaterialExpression* HueArgument =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionFunctionCall(Function, ImporterCalculateHue,{ Top }),
            NewMaterialExpressionFunctionCall(Function, ImporterCalculateSaturation,{ Bottom }),
            NewMaterialExpressionMax(Function, BottomX, BottomY, BottomZ)
        });
    UMaterialExpressionMaterialFunctionCall* Hue = NewMaterialExpressionFunctionCall(Function, ImporterHSVToRGB, { HueArgument });
    UMaterialExpression* SaturationArgument =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionFunctionCall(Function, ImporterCalculateHue,{ Bottom }),
            NewMaterialExpressionFunctionCall(Function, ImporterCalculateSaturation,{ Top }),
            NewMaterialExpressionMax(Function, BottomX, BottomY, BottomZ)
        });
    UMaterialExpressionMaterialFunctionCall* Saturation = NewMaterialExpressionFunctionCall(Function, ImporterHSVToRGB, { SaturationArgument });
    UMaterialExpressionIf* ColorBlend = NewMaterialExpressionSwitch(Function, Mode,
    {
        Blend, Add, Multiply, Screen, Overlay, Brightness, Color, Exclusion, Average, Lighten, Darken, Sub, Negation, Difference, SoftLight, ColorDodge,
        Reflect, ColorBurn, Phoenix, HardLight, PinLight, HardMix, LinearDodge, LinearBurn, SpotLight, SpotLightBlend, Hue, Saturation
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionLinearInterpolate(Function, Bottom, ColorBlend, Weight));
}

static void Generator_ImporterCalculateHue(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));
    UMaterialFunction* MathMinValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_min_value"));

    UMaterialExpressionFunctionInput* RGB = NewMaterialExpressionFunctionInput(Function, TEXT("rgb"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionComponentMask* RGBX = NewMaterialExpressionComponentMask(Function, RGB, 1);
    UMaterialExpressionComponentMask* RGBY = NewMaterialExpressionComponentMask(Function, RGB, 2);
    UMaterialExpressionComponentMask* RGBZ = NewMaterialExpressionComponentMask(Function, RGB, 4);

    UMaterialExpressionMaterialFunctionCall* Max = NewMaterialExpressionFunctionCall(Function, MathMaxValue, { RGB });
    UMaterialExpressionMaterialFunctionCall* Min = NewMaterialExpressionFunctionCall(Function, MathMinValue, { RGB });
    UMaterialExpressionSubtract* Range = NewMaterialExpressionSubtract(Function, Max, Min);

    UMaterialExpressionIf* Hue =
        NewMaterialExpressionIfEqual(Function, Range, 0.0f,
            0.0f,
            NewMaterialExpressionMultiply(Function,
                1.0f / 6.0f,
                NewMaterialExpressionIfEqual(Function, Max, RGBX,
                    NewMaterialExpressionDivide(Function,
                        NewMaterialExpressionSubtract(Function, RGBY, RGBZ),
                        Range),
                    NewMaterialExpressionIfEqual(Function, Max, RGBY,
                        NewMaterialExpressionAdd(Function,
                            2.0f,
                            NewMaterialExpressionDivide(Function,
                                NewMaterialExpressionSubtract(Function, RGBZ, RGBX),
                                Range)),
                        NewMaterialExpressionAdd(Function,
                            4.0f,
                            NewMaterialExpressionDivide(Function,
                                NewMaterialExpressionSubtract(Function, RGBX, RGBY),
                                Range))))));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfLess(Function, Hue, 0.0f, NewMaterialExpressionAdd(Function, Hue, 1.0f), Hue));
}

static void Generator_ImporterCalculateSaturation(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));
    UMaterialFunction* MathMinValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_min_value"));

    UMaterialExpressionFunctionInput* RGB = NewMaterialExpressionFunctionInput(Function, TEXT("rgb"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Max = NewMaterialExpressionFunctionCall(Function, MathMaxValue, { RGB });
    UMaterialExpressionMaterialFunctionCall* Min = NewMaterialExpressionFunctionCall(Function, MathMinValue, { RGB });
    UMaterialExpressionOneMinus* Saturation = NewMaterialExpressionOneMinus(Function, NewMaterialExpressionDivide(Function, Min, Max));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfEqual(Function, Max, 0.0f, 0.0f, Saturation));
}

static void Generator_ImporterComputeCubicTransform(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathMultiplyFloat4x4Float4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4"));

    UMaterialExpressionFunctionInput* ProjectionTransform0 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform1 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform2 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform3 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_3"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* TransformedPosition = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, NewMaterialExpressionAppendVector(Function, Position, 1.0f) });
    UMaterialExpressionMaterialFunctionCall* TempNormal = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, NewMaterialExpressionAppendVector(Function, Normal, 0.0f) });
    UMaterialExpressionIf* TransformedNormal = NewMaterialExpressionIfLess(Function, NewMaterialExpressionDotProduct(Function, NewMaterialExpressionComponentMask(Function, TransformedPosition, 7), NewMaterialExpressionComponentMask(Function, TempNormal, 7)), 0.0f, NewMaterialExpressionNegate(Function, TempNormal), TempNormal);
    UMaterialExpressionComponentMask* NX = NewMaterialExpressionComponentMask(Function, TransformedNormal, 1);
    UMaterialExpressionComponentMask* NY = NewMaterialExpressionComponentMask(Function, TransformedNormal, 2);
    UMaterialExpressionComponentMask* NZ = NewMaterialExpressionComponentMask(Function, TransformedNormal, 4);
    UMaterialExpressionAbs* AbsNX = NewMaterialExpressionAbs(Function, NX);
    UMaterialExpressionAbs* AbsNY = NewMaterialExpressionAbs(Function, NY);
    UMaterialExpressionAbs* AbsNZ = NewMaterialExpressionAbs(Function, NZ);

    UMaterialExpressionSubtract* NegativeProjectionTransform0 = NewMaterialExpressionNegate(Function, ProjectionTransform0);
    UMaterialExpressionSubtract* NegativeProjectionTransform1 = NewMaterialExpressionNegate(Function, ProjectionTransform1);

    // Find out on which cube face is the intersection point
    // transform is then planar, but based on a rotated/flipped coordsys
    UMaterialExpressionIf* ZCheck0 =
        NewMaterialExpressionIfLess(Function, AbsNX, AbsNZ,
            NewMaterialExpressionIfLess(Function, AbsNY, AbsNZ,
                NewMaterialExpressionIfLess(Function, NZ, 0.0f, NegativeProjectionTransform0, ProjectionTransform0),
                ProjectionTransform0),
            ProjectionTransform0);
    UMaterialExpressionIf* YCheck0 =
        NewMaterialExpressionIfLess(Function, AbsNX, AbsNY,
            NewMaterialExpressionIfLess(Function, AbsNZ, AbsNY,
                NewMaterialExpressionIfGreater(Function, NY, 0.0f, NegativeProjectionTransform0, ProjectionTransform0),
                ZCheck0),
            ZCheck0);
    UMaterialExpressionIf* Result0 =
        NewMaterialExpressionIfLess(Function, AbsNY, AbsNX,
            NewMaterialExpressionIfLess(Function, AbsNZ, AbsNX,
                NewMaterialExpressionIfGreater(Function, NX, 0.0f, ProjectionTransform1, NegativeProjectionTransform1),
                YCheck0),
            YCheck0);

    UMaterialExpressionIf* YCheck1 =
        NewMaterialExpressionIfLess(Function, AbsNX, AbsNY,
            NewMaterialExpressionIfLess(Function, AbsNZ, AbsNY, ProjectionTransform2, ProjectionTransform1),
            ProjectionTransform1);
    UMaterialExpressionIf* Result1 =
        NewMaterialExpressionIfLess(Function, AbsNY, AbsNX,
            NewMaterialExpressionIfLess(Function, AbsNZ, AbsNX,
                ProjectionTransform2,
                YCheck1),
            YCheck1);

    UMaterialExpressionIf* YCheck2 =
        NewMaterialExpressionIfLess(Function, AbsNX, AbsNY,
            NewMaterialExpressionIfLess(Function, AbsNZ, AbsNY, ProjectionTransform1, ProjectionTransform2),
            ProjectionTransform2);
    UMaterialExpressionIf* Result2 =
        NewMaterialExpressionIfLess(Function, AbsNY, AbsNX,
            NewMaterialExpressionIfLess(Function, AbsNZ, AbsNX,
                ProjectionTransform0,
                YCheck2),
            YCheck2);

    UMaterialExpression* Result3 = ProjectionTransform3;

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), Result0);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), Result1);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), Result2);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_3"), Result3);
}

static void Generator_ImporterComputeCylindricTransform(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* MathMultiplyFloat4x4Float4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4"));
    UMaterialFunction* MathMultiplyFloat4x4Float4x4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4x4"));

    UMaterialExpressionFunctionInput* ProjectionTransform0 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform1 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform2 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform3 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_3"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Infinite = NewMaterialExpressionFunctionInput(Function, TEXT("infinite"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));
    UMaterialExpressionFunctionInput* Normalized = NewMaterialExpressionFunctionInput(Function, TEXT("normalized"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));

    UMaterialExpressionMaterialFunctionCall* TransformedPosition = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, NewMaterialExpressionAppendVector(Function, Position, 1.0f) });
    UMaterialExpressionComponentMask* TransformedPositionX = NewMaterialExpressionComponentMask(Function, TransformedPosition, 1);
    UMaterialExpressionComponentMask* TransformedPositionY = NewMaterialExpressionComponentMask(Function, TransformedPosition, 2);
    UMaterialExpressionComponentMask* TransformedPositionZ = NewMaterialExpressionComponentMask(Function, TransformedPosition, 4);

    UMaterialExpressionMaterialFunctionCall* TempNormal = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, NewMaterialExpressionAppendVector(Function, Normal, 0.0f) });
    UMaterialExpressionIf* TransformedNormal =
        NewMaterialExpressionIfLess(Function, NewMaterialExpressionDotProduct(Function, TransformedPosition, TempNormal), 0.0f,
            NewMaterialExpressionNegate(Function, TempNormal), TempNormal);
    UMaterialExpressionComponentMask* TransformedNormalX = NewMaterialExpressionComponentMask(Function, TransformedNormal, 1);
    UMaterialExpressionComponentMask* TransformedNormalY = NewMaterialExpressionComponentMask(Function, TransformedNormal, 2);
    UMaterialExpressionComponentMask* TransformedNormalZ = NewMaterialExpressionComponentMask(Function, TransformedNormal, 4);
    UMaterialExpressionAbs* AbsNX = NewMaterialExpressionAbs(Function, TransformedNormalX);
    UMaterialExpressionAbs* AbsNY = NewMaterialExpressionAbs(Function, TransformedNormalY);
    UMaterialExpressionAbs* AbsNZ = NewMaterialExpressionAbs(Function, TransformedNormalZ);

    UMaterialExpressionSquareRoot* Length = NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionSquare(Function, TransformedPositionX), NewMaterialExpressionSquare(Function, TransformedPositionY)));
    UMaterialExpressionIf* OrgDist = NewMaterialExpressionStaticSwitch(Function, Normalized, 1.0f / PI, Length);
    UMaterialExpressionNormalize* BaseZ = NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { TransformedPositionX, TransformedPositionY, 0.0f }));
    UMaterialExpressionConstant3Vector* BaseY = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f);
    UMaterialExpressionNormalize* BaseX = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, BaseY, BaseZ));

    UMaterialExpressionMaterialFunctionCall* Offset2TimesBase = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4,
    {
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 1.0f, 0.0f, 0.0f, NewMaterialExpressionMultiply(Function, OrgDist, NewMaterialExpressionArctangent2(Function, TransformedPositionY, TransformedPositionX)) }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 0.0f, 1.0f, 0.0f, TransformedPositionZ }),
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        NewMaterialExpressionAppendVector(Function, BaseX, 0.0f),
        NewMaterialExpressionAppendVector(Function, BaseY, 0.0f),
        NewMaterialExpressionAppendVector(Function, BaseZ, 0.0f),
        { 0.0f, 0.0f, 0.0f, 1.0f }
    });
    UMaterialExpressionMaterialFunctionCall* Result = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4,
    {
        { Offset2TimesBase, 0 },
        { Offset2TimesBase, 1 },
        { Offset2TimesBase, 2 },
        { Offset2TimesBase, 3 },
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 1.0f, 0.0f, 0.0f, NewMaterialExpressionNegate(Function, NewMaterialExpressionComponentMask(Function, Position, 1)) }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 0.0f, 1.0f, 0.0f, NewMaterialExpressionNegate(Function, NewMaterialExpressionComponentMask(Function, Position, 2)) }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 0.0f, 0.0f, 1.0f, NewMaterialExpressionNegate(Function, NewMaterialExpressionComponentMask(Function, Position, 4)) }),
        { 0.0f, 0.0f, 0.0f, 1.0f },
    });

    UMaterialExpressionIf* Result0 =
        NewMaterialExpressionStaticSwitch(Function, Infinite,
        { Result, 0 },
            NewMaterialExpressionIfLess(Function, AbsNX, AbsNZ,
                NewMaterialExpressionIfLess(Function, AbsNY, AbsNZ,
                    NewMaterialExpressionIfLess(Function, TransformedNormalZ, 0.0f,
                        NewMaterialExpressionNegate(Function, ProjectionTransform0),
                        ProjectionTransform0),
                        { Result, 0 }),
                        { Result, 0 }));

    UMaterialExpressionIf* Result1 =
        NewMaterialExpressionStaticSwitch(Function, Infinite,
        { Result, 1 },
            NewMaterialExpressionIfLess(Function, AbsNX, AbsNZ, NewMaterialExpressionIfLess(Function, AbsNY, AbsNZ, ProjectionTransform1, { Result, 1 }), { Result, 1 }));

    UMaterialExpressionIf* Result2 =
        NewMaterialExpressionStaticSwitch(Function, Infinite,
        { Result, 2 },
            NewMaterialExpressionIfLess(Function, AbsNX, AbsNZ, NewMaterialExpressionIfLess(Function, AbsNY, AbsNZ, ProjectionTransform2, { Result, 2 }), { Result, 2 }));

    UMaterialExpressionIf* Result3 =
        NewMaterialExpressionStaticSwitch(Function, Infinite,
        { Result, 3 },
            NewMaterialExpressionIfLess(Function, AbsNX, AbsNZ, NewMaterialExpressionIfLess(Function, AbsNY, AbsNZ, ProjectionTransform3, { Result, 3 }), { Result, 3 }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), Result0);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), Result1);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), Result2);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_3"), Result3);
}

static void Generator_ImporterComputeSphericProjection(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* Dir = NewMaterialExpressionFunctionInput(Function, TEXT("dir"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Normalized = NewMaterialExpressionFunctionInput(Function, TEXT("normalized"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));

    UMaterialExpressionComponentMask* DirX = NewMaterialExpressionComponentMask(Function, Dir, 1);
    UMaterialExpressionComponentMask* DirY = NewMaterialExpressionComponentMask(Function, Dir, 2);
    UMaterialExpressionComponentMask* DirZ = NewMaterialExpressionComponentMask(Function, Dir, 4);

    UMaterialExpressionAdd* XY = NewMaterialExpressionAdd(Function, NewMaterialExpressionSquare(Function, DirX), NewMaterialExpressionSquare(Function, DirY));
    UMaterialExpressionIf* OrgDist =
        NewMaterialExpressionStaticSwitch(Function, Normalized,
            1.0f / PI,
            NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionAdd(Function, XY, NewMaterialExpressionSquare(Function, DirZ))));

    UMaterialExpressionMaterialFunctionCall* UVCoordXY =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            NewMaterialExpressionMultiply(Function, OrgDist, NewMaterialExpressionArctangent2(Function, DirY, DirX)),
            NewMaterialExpressionMultiply(Function, OrgDist, NewMaterialExpressionArctangent2(Function, DirZ, NewMaterialExpressionSquareRoot(Function, XY))),
            OrgDist
        });
    UMaterialExpressionMaterialFunctionCall* UVCoordZ =
        NewMaterialExpressionFunctionCall(Function, MakeFloat3,
        {
            0.0f,
            NewMaterialExpressionMultiply(Function, OrgDist, NewMaterialExpressionArctangent2(Function, DirZ, NewMaterialExpressionSquareRoot(Function, XY))),
            OrgDist
        });

    UMaterialExpressionIf* UVCoord =
        NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, DirX), 0.0f,
            UVCoordXY,
            NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, DirY), 0.0f,
                UVCoordXY,
                NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, DirZ), 0.0f,
                    UVCoordZ,
                    NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f))));

    NewMaterialExpressionFunctionOutput(Function, TEXT("uv_coord"), UVCoord);
}

static void Generator_ImporterComputeSphericTransform(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterComputeSphericProjection = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_compute_spheric_projection"));
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* MathMultiplyFloat4x4Float4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4"));
    UMaterialFunction* MathMultiplyFloat4x4Float4x4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4x4"));

    UMaterialExpressionFunctionInput* ProjectionTransform0 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_0"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform1 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_1"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform2 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_2"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* ProjectionTransform3 = NewMaterialExpressionFunctionInput(Function, TEXT("projection_transform_3"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Normalized = NewMaterialExpressionFunctionInput(Function, TEXT("normalized"), EFunctionInputType::FunctionInput_Scalar, NewMaterialExpressionStaticBool(Function, false));

    UMaterialExpressionMaterialFunctionCall* TransformedPosition = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, { ProjectionTransform0, ProjectionTransform1, ProjectionTransform2, ProjectionTransform3, NewMaterialExpressionAppendVector(Function, Position, 1.0f) });
    UMaterialExpressionComponentMask* TransformedPositionX = NewMaterialExpressionComponentMask(Function, TransformedPosition, 1);
    UMaterialExpressionComponentMask* TransformedPositionY = NewMaterialExpressionComponentMask(Function, TransformedPosition, 2);
    UMaterialExpressionComponentMask* TransformedPositionZ = NewMaterialExpressionComponentMask(Function, TransformedPosition, 4);

    UMaterialExpressionMaterialFunctionCall* UVCoord = NewMaterialExpressionFunctionCall(Function, ImporterComputeSphericProjection, { TransformedPosition, Normalized });
    UMaterialExpressionNormalize* BaseZ = NewMaterialExpressionNormalize(Function, NewMaterialExpressionComponentMask(Function, TransformedPosition, 7));
    UMaterialExpressionNormalize* BaseX = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f), BaseZ));
    UMaterialExpressionNormalize* BaseY = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, BaseZ, BaseX));

    UMaterialExpressionConstant4Vector* Vec1 = NewMaterialExpressionConstant(Function, 1.0f, 0.0f, 0.0f, 0.0f);
    UMaterialExpressionConstant4Vector* Vec2 = NewMaterialExpressionConstant(Function, 0.0f, 1.0f, 0.0f, 0.0f);
    UMaterialExpressionConstant4Vector* Vec3 = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 1.0f, 0.0f);

    UMaterialExpressionMaterialFunctionCall* Offset2TimesBase = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4,
    {
        Vec1,
        Vec2,
        Vec3,
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ NewMaterialExpressionComponentMask(Function, UVCoord, 1), NewMaterialExpressionComponentMask(Function, UVCoord, 2), 0.0f, 1.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ NewMaterialExpressionComponentMask(Function, BaseX, 1), NewMaterialExpressionComponentMask(Function, BaseY, 1), NewMaterialExpressionComponentMask(Function, BaseZ, 1), 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ NewMaterialExpressionComponentMask(Function, BaseX, 2), NewMaterialExpressionComponentMask(Function, BaseY, 2), NewMaterialExpressionComponentMask(Function, BaseZ, 2), 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ NewMaterialExpressionComponentMask(Function, BaseX, 4), NewMaterialExpressionComponentMask(Function, BaseY, 4), NewMaterialExpressionComponentMask(Function, BaseZ, 4), 0.0f }),
        NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f, 1.0f)
    });
    UMaterialExpressionMaterialFunctionCall* Result = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4x4,
    {
        { Offset2TimesBase, 0 },
        { Offset2TimesBase, 1 },
        { Offset2TimesBase, 2 },
        { Offset2TimesBase, 3 },
        Vec1,
        Vec2,
        Vec3,
        NewMaterialExpressionAppendVector(Function, NewMaterialExpressionNegate(Function, Position), 1.0f)
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result_0"), { Result, 0 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_1"), { Result, 1 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_2"), { Result, 2 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result_3"), { Result, 3 });
}

static void Generator_ImporterComputeTangents(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTransformVector = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_transform_vector"));

    UMaterialExpressionFunctionInput* Space = NewMaterialExpressionFunctionInput(Function, TEXT("space"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionMaterialFunctionCall* Position = NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 });

    UMaterialExpressionMaterialFunctionCall* Normal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});
    UMaterialExpressionNormalize* UDir = NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, StateTransformVector, { Space, coordinate_tangent,{ 1.0f, 0.0f, 0.0f } }));
    UMaterialExpressionNormalize* VDir = NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, StateTransformVector, { Space, coordinate_tangent,{ 0.0f, 1.0f, 0.0f } }));
    UMaterialExpressionAbs* NormalDotUDir = NewMaterialExpressionAbs(Function, NewMaterialExpressionDotProduct(Function, Normal, UDir));
    UMaterialExpressionAbs* NormalDotVDir = NewMaterialExpressionAbs(Function, NewMaterialExpressionDotProduct(Function, Normal, VDir));

    UMaterialExpressionMaterialFunctionCall* ShadingNormal = NewMaterialExpressionFunctionCall(Function, StateTransformVector, { coordinate_tangent, Space, Normal });
    UMaterialExpressionComponentMask* ShadingNormalX = NewMaterialExpressionComponentMask(Function, ShadingNormal, 1);
    UMaterialExpressionComponentMask* ShadingNormalY = NewMaterialExpressionComponentMask(Function, ShadingNormal, 2);
    UMaterialExpressionComponentMask* ShadingNormalZ = NewMaterialExpressionComponentMask(Function, ShadingNormal, 4);
    UMaterialExpressionSubtract* YZ = NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, ShadingNormalY, ShadingNormalZ));
    UMaterialExpressionIf* TangentVPreliminary =
        NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, ShadingNormalZ), 0.99999f,
            NewMaterialExpressionNormalize(Function,
                NewMaterialExpressionFunctionCall(Function, MakeFloat3,
                {
                    NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, ShadingNormalX, ShadingNormalY)),
                    NewMaterialExpressionOneMinus(Function, NewMaterialExpressionSquare(Function, ShadingNormalY)),
                    YZ
                })),
            NewMaterialExpressionNormalize(Function,
                NewMaterialExpressionFunctionCall(Function, MakeFloat3,
                {
                    NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, ShadingNormalX, ShadingNormalZ)),
                    YZ,
                    NewMaterialExpressionOneMinus(Function, NewMaterialExpressionSquare(Function, ShadingNormalZ))
                })));
    UMaterialExpressionNormalize* TangentUComplexPreliminary = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, TangentVPreliminary, ShadingNormal));
    UMaterialExpressionNormalize* TangentUComplex =
        NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, StateTransformVector, { Space, coordinate_tangent, TangentUComplexPreliminary }));
    UMaterialExpressionNormalize* TangentUSimplePreliminary = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, VDir, Normal));
    UMaterialExpressionIf* TangentUSimple =
        NewMaterialExpressionIfLess(Function, NewMaterialExpressionDotProduct(Function, TangentUSimplePreliminary, UDir), 0.0f,
            NewMaterialExpressionNegate(Function, TangentUSimplePreliminary),
            TangentUSimplePreliminary);
    UMaterialExpressionIf* TangentUUDirCheck = NewMaterialExpressionIfGreater(Function, NormalDotUDir, 0.999f, TangentUComplex, TangentUSimple);
    UMaterialExpressionIf* TangentU = NewMaterialExpressionIfGreater(Function, NormalDotVDir, 0.999f, TangentUComplex, TangentUUDirCheck);

    UMaterialExpressionNormalize* TangentVComplex =
        NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, StateTransformVector, { Space, coordinate_tangent, TangentVPreliminary }));
    UMaterialExpressionNormalize* TangentVSimple = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, Normal, TangentUSimplePreliminary));
    UMaterialExpressionIf* TangentVUDirCheck = NewMaterialExpressionIfGreater(Function, NormalDotUDir, 0.999f, TangentVComplex, TangentVSimple);
    // As UE4 is left-handed, negate the resulting v-tangent !
    UMaterialExpressionSubtract* TangentV = NewMaterialExpressionNegate(Function, NewMaterialExpressionIfGreater(Function, NormalDotVDir, 0.999f, TangentVComplex, TangentVUDirCheck));

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TangentU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), TangentV);
}

static void Generator_ImporterComputeTangentsTransformed(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTransformVector = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_transform_vector"));

    UMaterialExpressionFunctionInput* CoordinateSystem = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate_system"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Transform0 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_0"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Transform1 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_1"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Transform2 = NewMaterialExpressionFunctionInput(Function, TEXT("transform_2"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* Position = NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 });

    UMaterialExpressionIf* Space = NewMaterialExpressionIfEqual(Function, CoordinateSystem, texture_coordinate_world, coordinate_world, coordinate_object);
    UMaterialExpressionNormalize* UDir =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionFunctionCall(Function, StateTransformVector,
            {
                Space,
                coordinate_tangent,
                NewMaterialExpressionFunctionCall(Function, MakeFloat3,
                {
                    NewMaterialExpressionComponentMask(Function, Transform0, 1),
                    NewMaterialExpressionComponentMask(Function, Transform1, 1),
                    NewMaterialExpressionComponentMask(Function, Transform2, 1)
                })
            }));
    UMaterialExpressionNormalize* VDir =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionFunctionCall(Function, StateTransformVector,
            {
                Space,
                coordinate_tangent,
                NewMaterialExpressionFunctionCall(Function, MakeFloat3,
                {
                    NewMaterialExpressionComponentMask(Function, Transform0, 2),
                    NewMaterialExpressionComponentMask(Function, Transform1, 2),
                    NewMaterialExpressionComponentMask(Function, Transform2, 2)
                })
            }));

    UMaterialExpressionMaterialFunctionCall* Normal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});
    UMaterialExpressionAbs* NormalDotUDir = NewMaterialExpressionAbs(Function, NewMaterialExpressionDotProduct(Function, Normal, UDir));
    UMaterialExpressionAbs* NormalDotVDir = NewMaterialExpressionAbs(Function, NewMaterialExpressionDotProduct(Function, Normal, VDir));

    UMaterialExpressionMaterialFunctionCall* ShadingNormal = NewMaterialExpressionFunctionCall(Function, StateTransformVector, { coordinate_tangent, Space, Normal });
    UMaterialExpressionComponentMask* ShadingNormalX = NewMaterialExpressionComponentMask(Function, ShadingNormal, 1);
    UMaterialExpressionComponentMask* ShadingNormalY = NewMaterialExpressionComponentMask(Function, ShadingNormal, 2);
    UMaterialExpressionComponentMask* ShadingNormalZ = NewMaterialExpressionComponentMask(Function, ShadingNormal, 4);
    UMaterialExpressionSubtract* YZ = NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, ShadingNormalY, ShadingNormalZ));
    UMaterialExpressionIf* TangentVPreliminary =
        NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAbs(Function, ShadingNormalZ), 0.99999f,
            NewMaterialExpressionNormalize(Function,
                NewMaterialExpressionFunctionCall(Function, MakeFloat3,
                {
                    NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, ShadingNormalX, ShadingNormalY)),
                    NewMaterialExpressionOneMinus(Function, NewMaterialExpressionSquare(Function, ShadingNormalY)),
                    YZ
                })),
            NewMaterialExpressionNormalize(Function,
                NewMaterialExpressionFunctionCall(Function, MakeFloat3,
                {
                    NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function, ShadingNormalX, ShadingNormalZ)),
                    YZ,
                    NewMaterialExpressionOneMinus(Function, NewMaterialExpressionSquare(Function, ShadingNormalZ))
                })));
    UMaterialExpressionNormalize* TangentUComplexPreliminary =
        NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, TangentVPreliminary, ShadingNormal));
    UMaterialExpressionNormalize* TangentUComplex =
        NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, StateTransformVector, { Space, coordinate_tangent, TangentUComplexPreliminary }));
    UMaterialExpressionNormalize* TangentUSimplePreliminary = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, VDir, Normal));
    UMaterialExpressionIf* TangentUSimple =
        NewMaterialExpressionIfLess(Function, NewMaterialExpressionDotProduct(Function, TangentUSimplePreliminary, UDir), 0.0f,
            NewMaterialExpressionNegate(Function, TangentUSimplePreliminary),
            TangentUSimplePreliminary);
    UMaterialExpressionIf* TangentUUDirCheck = NewMaterialExpressionIfGreater(Function, NormalDotUDir, 0.999f, TangentUComplex, TangentUSimple);
    UMaterialExpressionIf* TangentU = NewMaterialExpressionIfGreater(Function, NormalDotVDir, 0.999f, TangentUComplex, TangentUUDirCheck);

    UMaterialExpressionNormalize* TangentVComplex =
        NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, StateTransformVector, { Space, coordinate_tangent, TangentVPreliminary }));
    UMaterialExpressionNormalize* TangentVSimplePreliminary = NewMaterialExpressionNormalize(Function, NewMaterialExpressionCrossProduct(Function, Normal, TangentUSimplePreliminary));
    UMaterialExpressionIf* TangentVSimple =
        NewMaterialExpressionIfLess(Function, NewMaterialExpressionDotProduct(Function, TangentVSimplePreliminary, VDir), 0.0f,
            NewMaterialExpressionNegate(Function, TangentVSimplePreliminary),
            TangentVSimplePreliminary);
    UMaterialExpressionIf* TangentVUDirCheck = NewMaterialExpressionIfGreater(Function, NormalDotUDir, 0.999f, TangentVComplex, TangentVSimple);
    // As UE4 is left-handed, negate the resulting v-tangent !
    UMaterialExpressionSubtract* TangentV = NewMaterialExpressionNegate(Function, NewMaterialExpressionIfGreater(Function, NormalDotVDir, 0.999f, TangentVComplex, TangentVUDirCheck));

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), Position);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TangentU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), TangentV);
}

static void Generator_ImporterEvalChecker(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("position"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* CheckerPosition = NewMaterialExpressionFunctionInput(Function, TEXT("checker_position"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Blur = NewMaterialExpressionFunctionInput(Function, TEXT("blur"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionFrac* Tex = NewMaterialExpressionFrac(Function, NewMaterialExpressionAdd(Function,
        Position,
        NewMaterialExpressionMultiply(Function,
            NewMaterialExpressionMin(Function,
                CheckerPosition,
                NewMaterialExpressionOneMinus(Function, CheckerPosition)),
            0.5f)));

    UMaterialExpressionFmod * InColor1 =
        NewMaterialExpressionFmod(Function,
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionIfLess(Function, NewMaterialExpressionComponentMask(Function, Tex, 1), CheckerPosition, 1.0f, 0.0f),
                NewMaterialExpressionIfLess(Function, NewMaterialExpressionComponentMask(Function, Tex, 2), CheckerPosition, 1.0f, 0.0f),
                NewMaterialExpressionIfLess(Function, NewMaterialExpressionComponentMask(Function, Tex, 4), CheckerPosition, 1.0f, 0.0f)
            }),
            2.0f);

    UMaterialExpressionMin* EdgeDist =
        NewMaterialExpressionMin(Function,
            Tex,
            NewMaterialExpressionMin(Function,
                NewMaterialExpressionOneMinus(Function, Tex),
                NewMaterialExpressionAbs(Function,
                    NewMaterialExpressionSubtract(Function,
                        Tex,
                        NewMaterialExpressionFunctionCall(Function, MakeFloat3, { CheckerPosition, CheckerPosition, CheckerPosition })))));
    UMaterialExpressionComponentMask* EdgeDistX = NewMaterialExpressionComponentMask(Function, EdgeDist, 1);
    UMaterialExpressionComponentMask* EdgeDistY = NewMaterialExpressionComponentMask(Function, EdgeDist, 2);
    UMaterialExpressionComponentMask* EdgeDistZ = NewMaterialExpressionComponentMask(Function, EdgeDist, 2);

    UMaterialExpressionIf* BlendAmountX =
        NewMaterialExpressionIfGreater(Function, Blur, EdgeDistX,
            NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, EdgeDistX, Blur), 0.5f),
            0.5f);
    UMaterialExpressionIf* BlendAmountY =
        NewMaterialExpressionIfGreater(Function, Blur, EdgeDistY,
            NewMaterialExpressionMultiply(Function,
                BlendAmountX,
                NewMaterialExpressionDivide(Function, EdgeDistY, Blur)),
            BlendAmountX);
    UMaterialExpressionIf* BlendAmount =
        NewMaterialExpressionIfGreater(Function, Blur, EdgeDistZ,
            NewMaterialExpressionMultiply(Function,
                BlendAmountY,
                NewMaterialExpressionDivide(Function, EdgeDistZ, Blur)),
            BlendAmountY);

    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfEqual(Function, InColor1, 1.0f,
            NewMaterialExpressionAdd(Function, 0.5f, BlendAmount),
            NewMaterialExpressionSubtract(Function, 0.5f, BlendAmount));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterEvalTileFunction(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* PerlinNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_perlin_noise"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* TileColor = NewMaterialExpressionFunctionInput(Function, TEXT("tile_color"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* GroutColor = NewMaterialExpressionFunctionInput(Function, TEXT("grout_color"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* TileBrightnessVariation = NewMaterialExpressionFunctionInput(Function, TEXT("tile_brightness_variation"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TileHoleAmount = NewMaterialExpressionFunctionInput(Function, TEXT("tile_hole_amount"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Seed = NewMaterialExpressionFunctionInput(Function, TEXT("seed"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* NumberOfRows = NewMaterialExpressionFunctionInput(Function, TEXT("number_of_rows"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* NumberOfColumns = NewMaterialExpressionFunctionInput(Function, TEXT("number_of_columns"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* OddRowOffset = NewMaterialExpressionFunctionInput(Function, TEXT("odd_row_offset"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RandomRowOffset = NewMaterialExpressionFunctionInput(Function, TEXT("random_row_offset"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TileGroutWidth = NewMaterialExpressionFunctionInput(Function, TEXT("tile_grout_width"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TileGroutHeight = NewMaterialExpressionFunctionInput(Function, TEXT("tile_grout_height"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TileGroutRoughness = NewMaterialExpressionFunctionInput(Function, TEXT("tile_grout_roughness"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* SpecialColumnIndex = NewMaterialExpressionFunctionInput(Function, TEXT("special_column_index"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* SpecialRowIndex = NewMaterialExpressionFunctionInput(Function, TEXT("special_row_index"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* SpecialColumnHeightFactor = NewMaterialExpressionFunctionInput(Function, TEXT("special_column_height_factor"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* SpecialRowWidthFactor = NewMaterialExpressionFunctionInput(Function, TEXT("special_row_width_factor"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionComponentMask* TexX = NewMaterialExpressionComponentMask(Function, Tex, 1);
    UMaterialExpressionComponentMask* TexY = NewMaterialExpressionComponentMask(Function, Tex, 2);
    UMaterialExpressionMaterialFunctionCall* XYCR = NewMaterialExpressionFunctionCall(Function, MakeFloat4,
    {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, TexX, NewMaterialExpressionFloor(Function, TexX)), NumberOfColumns),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, TexY, NewMaterialExpressionFloor(Function, TexY)), NumberOfRows),
        NumberOfColumns,
        NumberOfRows
    });

    UMaterialExpressionIf* ColIndexCheck =
        NewMaterialExpressionIfEqual(Function,
            NewMaterialExpressionFmod(Function,
                NewMaterialExpressionAdd(Function, NewMaterialExpressionFloor(Function, NewMaterialExpressionComponentMask(Function, XYCR, 1)), 1.0f), SpecialColumnIndex),
            0.0f,
            NewMaterialExpressionMultiply(Function, XYCR, NewMaterialExpressionFunctionCall(Function, MakeFloat4, { SpecialColumnHeightFactor, SpecialRowWidthFactor, SpecialColumnHeightFactor, SpecialRowWidthFactor })),
            XYCR);
    UMaterialExpressionIf* RowIndexCheck =
        NewMaterialExpressionIfEqual(Function,
            NewMaterialExpressionFmod(Function,
                NewMaterialExpressionAdd(Function, NewMaterialExpressionFloor(Function, NewMaterialExpressionComponentMask(Function, XYCR, 2)), 1.0f), SpecialRowIndex),
            0.0f,
            ColIndexCheck,
            XYCR);
    UMaterialExpressionComponentMask* X0 = NewMaterialExpressionComponentMask(Function, RowIndexCheck, 1);
    UMaterialExpressionComponentMask* Y0 = NewMaterialExpressionComponentMask(Function, RowIndexCheck, 2);
    UMaterialExpressionFloor* RowIndex = NewMaterialExpressionFloor(Function, Y0);
    UMaterialExpressionIf* X1 =
        NewMaterialExpressionIfEqual(Function, NewMaterialExpressionFmod(Function, RowIndex, 2.0f), 0.0f, X0, NewMaterialExpressionAdd(Function, X0, OddRowOffset));
    UMaterialExpressionIf* X2 =
        NewMaterialExpressionIfGreater(Function, RandomRowOffset, 0.0f,
            NewMaterialExpressionAdd(Function,
                X1,
                NewMaterialExpressionMultiply(Function, RandomRowOffset, NewMaterialExpressionFunctionCall(Function, PerlinNoise, { NewMaterialExpressionMultiply(Function, RowIndex, Seed) }))),
            X1);
    UMaterialExpressionFloor* ColIndex = NewMaterialExpressionFloor(Function, X2);
    UMaterialExpressionSubtract* X = NewMaterialExpressionSubtract(Function, X2, ColIndex);
    UMaterialExpressionSubtract* Y = NewMaterialExpressionSubtract(Function, Y0, RowIndex);

    UMaterialExpressionComponentMask* NumColumns = NewMaterialExpressionComponentMask(Function, RowIndexCheck, 4);
    UMaterialExpressionComponentMask* NumRows = NewMaterialExpressionComponentMask(Function, RowIndexCheck, 8);
    UMaterialExpressionMultiply* NoiseScaleY = NewMaterialExpressionMultiply(Function, 10.0f, NewMaterialExpressionAdd(Function, NumRows, NumColumns));
    UMaterialExpressionMultiply* NoiseScaleX = NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NumColumns, NumRows), NoiseScaleY);

    UMaterialExpressionMultiply* NoisePos0 = NewMaterialExpressionMultiply(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat2, { ColIndex, RowIndex }), Seed);
    UMaterialExpressionMultiply* NoisePos1 = NewMaterialExpressionMultiply(Function, Tex, NewMaterialExpressionFunctionCall(Function, MakeFloat2, { NoiseScaleX, NoiseScaleY }));
    UMaterialExpressionMultiply* NoisePos2 = NewMaterialExpressionMultiply(Function, Tex, NewMaterialExpressionFunctionCall(Function, MakeFloat2, { NoiseScaleY, NoiseScaleX }));
    UMaterialExpressionMultiply* NoisePos3 = NewMaterialExpressionMultiply(Function, NoisePos0, 0.5f);

    UMaterialExpressionMaterialFunctionCall* Noise0 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { NewMaterialExpressionAppendVector(Function, NoisePos0, 0.0f) });
    UMaterialExpressionMaterialFunctionCall* Noise1 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { NewMaterialExpressionAppendVector(Function, NoisePos1, 0.0f) });
    UMaterialExpressionMaterialFunctionCall* Noise2 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { NewMaterialExpressionAppendVector(Function, NoisePos2, 0.0f) });
    UMaterialExpressionMaterialFunctionCall* Noise3 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { NewMaterialExpressionAppendVector(Function, NoisePos3, 0.0f) });

    UMaterialExpressionMultiply* GroutWidthBase = NewMaterialExpressionMultiply(Function, TileGroutWidth, NumColumns);
    UMaterialExpressionMultiply* GroutHeightBase = NewMaterialExpressionMultiply(Function, TileGroutHeight, NumRows);
    UMaterialExpressionIf* GroutRoughnessCheck =
        NewMaterialExpressionIfGreater(Function, TileGroutRoughness, 0.0f,
            NewMaterialExpressionFunctionCall(Function, MakeFloat2,
            {
                NewMaterialExpressionAdd(Function,
                GroutWidthBase,
                NewMaterialExpressionMultiply(Function, { NewMaterialExpressionDivide(Function, NumColumns, NumberOfColumns), TileGroutRoughness, Noise1 })),
                NewMaterialExpressionAdd(Function,
                GroutHeightBase,
                NewMaterialExpressionMultiply(Function,
                {
                    NewMaterialExpressionDivide(Function, NewMaterialExpressionSquare(Function, NumRows), NewMaterialExpressionMultiply(Function, NumColumns, NumberOfRows)),
                    TileGroutRoughness,
                    Noise2
                }))
            }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat2, { GroutWidthBase, GroutHeightBase }));
    UMaterialExpressionComponentMask* GroutWidth = NewMaterialExpressionComponentMask(Function, GroutRoughnessCheck, 1);
    UMaterialExpressionComponentMask* GroutHeight = NewMaterialExpressionComponentMask(Function, GroutRoughnessCheck, 2);

    UMaterialExpressionAppendVector* GroutReturn = NewMaterialExpressionAppendVector(Function, GroutColor, 0.0f);

    UMaterialExpressionIf* TileBrightnessVariationCheck =
        NewMaterialExpressionIfGreater(Function, TileBrightnessVariation, 0.0f,
            NewMaterialExpressionSaturate(Function,
                NewMaterialExpressionAdd(Function, TileColor, NewMaterialExpressionMultiply(Function, { TileColor, TileBrightnessVariation, Noise3 }))),
            TileColor);
    UMaterialExpressionAppendVector* TileReturn = NewMaterialExpressionAppendVector(Function, TileBrightnessVariationCheck, 1.0f);

    UMaterialExpressionIf* InTileCheck4 = NewMaterialExpressionIfGreater(Function, Y, NewMaterialExpressionOneMinus(Function, GroutHeight), GroutReturn, TileReturn);
    UMaterialExpressionIf* InTileCheck3 = NewMaterialExpressionIfLess(Function, Y, GroutHeight, GroutReturn, InTileCheck4);
    UMaterialExpressionIf* InTileCheck2 = NewMaterialExpressionIfGreater(Function, X, NewMaterialExpressionOneMinus(Function, GroutWidth), GroutReturn, InTileCheck3);
    UMaterialExpressionIf* InTileCheck1 = NewMaterialExpressionIfLess(Function, X, GroutWidth, GroutReturn, InTileCheck2);

    UMaterialExpressionIf* TileHoleAmountCheck =
        NewMaterialExpressionIfGreater(Function, TileHoleAmount, NewMaterialExpressionIfLess(Function, Noise0, 0.0f, NewMaterialExpressionAdd(Function, Noise0, 1.0f), Noise0),
            GroutReturn,
            InTileCheck1);
    UMaterialExpressionIf* TileHoleAmountPositiveCheck = NewMaterialExpressionIfGreater(Function, TileHoleAmount, 0.0f, TileHoleAmountCheck, InTileCheck1);

    UMaterialExpressionComponentMask* IsInTile = NewMaterialExpressionComponentMask(Function, TileHoleAmountPositiveCheck, 8);
    UMaterialExpressionComponentMask* ColorResult = NewMaterialExpressionComponentMask(Function, TileHoleAmountPositiveCheck, 7);

    NewMaterialExpressionFunctionOutput(Function, TEXT("is_in_tile"), IsInTile);
    NewMaterialExpressionFunctionOutput(Function, TEXT("color_result"), ColorResult);
}

static void Generator_ImporterFlowNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterGradFlow = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_grad_flow"));
    UMaterialFunction* ImporterPermuteFlow = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_permute_flow"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* P = NewMaterialExpressionFunctionInput(Function, TEXT("p"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Rot = NewMaterialExpressionFunctionInput(Function, TEXT("rot"), EFunctionInputType::FunctionInput_Scalar);

    float F0 = 0.5f - sqrt(3.0f) / 6.0f;
    float F1 = -0.5f - sqrt(3.0f) / 6.0f;
    UMaterialExpressionFloor* Pi = NewMaterialExpressionFloor(Function,
        NewMaterialExpressionAdd(Function,
            P,
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionComponentMask(Function, P, 1),
                    NewMaterialExpressionComponentMask(Function, P, 2)),
                0.5f * sqrt(3.0f) - 0.5f)));
    UMaterialExpressionComponentMask* PiX = NewMaterialExpressionComponentMask(Function, Pi, 1);
    UMaterialExpressionComponentMask* PiY = NewMaterialExpressionComponentMask(Function, Pi, 2);
    UMaterialExpressionAdd* V0 = NewMaterialExpressionAdd(Function,
        NewMaterialExpressionSubtract(Function, P, Pi),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionAdd(Function, PiX, PiY), F0));
    UMaterialExpressionComponentMask* V0X = NewMaterialExpressionComponentMask(Function, V0, 1);
    UMaterialExpressionComponentMask* V0Y = NewMaterialExpressionComponentMask(Function, V0, 2);
    UMaterialExpressionAdd* V1 = NewMaterialExpressionAdd(Function,
        V0,
        NewMaterialExpressionIfLess(Function, V0X, V0Y,
            NewMaterialExpressionFunctionCall(Function, MakeFloat2, { F0, F1 }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat2, { F1, F0 })));
    UMaterialExpressionSubtract* V2 = NewMaterialExpressionSubtract(Function, V0, 1.0f / sqrt(3.0f));
    UMaterialExpressionMax* T = NewMaterialExpressionMax(Function,
        NewMaterialExpressionSubtract(Function,
        { 0.5f, 0.5f, 0.5f },
            NewMaterialExpressionFunctionCall(Function, MakeFloat3,
            {
                NewMaterialExpressionDotProduct(Function, V0, V0),
                NewMaterialExpressionDotProduct(Function, V1, V1),
                NewMaterialExpressionDotProduct(Function, V2, V2)
            })),
            { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionAdd* TmpP0 = NewMaterialExpressionAdd(Function, NewMaterialExpressionFunctionCall(Function, ImporterPermuteFlow, { PiX }), PiY);
    UMaterialExpressionAdd* TmpP2 = NewMaterialExpressionAdd(Function, NewMaterialExpressionFunctionCall(Function, ImporterPermuteFlow, { NewMaterialExpressionAdd(Function, PiX, 1.0f) }), PiY);
    UMaterialExpressionAdd* TmpP1 = NewMaterialExpressionAdd(Function,
        NewMaterialExpressionIfLess(Function, V0X, V0Y, TmpP0, TmpP2),
        NewMaterialExpressionIfLess(Function, V0X, V0Y, 1.0f, 0.0f));
    UMaterialExpressionMaterialFunctionCall* G0 = NewMaterialExpressionFunctionCall(Function, ImporterGradFlow, { NewMaterialExpressionFunctionCall(Function, ImporterPermuteFlow,{ TmpP0 }), Rot });
    UMaterialExpressionMaterialFunctionCall* G1 = NewMaterialExpressionFunctionCall(Function, ImporterGradFlow, { NewMaterialExpressionFunctionCall(Function, ImporterPermuteFlow,{ TmpP1 }), Rot });
    UMaterialExpressionMaterialFunctionCall* G2 = NewMaterialExpressionFunctionCall(Function, ImporterGradFlow, { NewMaterialExpressionFunctionCall(Function, ImporterPermuteFlow,{ TmpP2 }), Rot });
    UMaterialExpressionMaterialFunctionCall* GV = NewMaterialExpressionFunctionCall(Function, MakeFloat3,
    {
        NewMaterialExpressionDotProduct(Function, G0, V0),
        NewMaterialExpressionDotProduct(Function, G1, V1),
        NewMaterialExpressionDotProduct(Function, G2, V2)
    });
    UMaterialExpressionMultiply* T2 = NewMaterialExpressionSquare(Function, T);
    UMaterialExpressionMultiply* T4 = NewMaterialExpressionSquare(Function, T2);
    UMaterialExpressionMultiply* Result = NewMaterialExpressionMultiply(Function, 40.0f, NewMaterialExpressionDotProduct(Function, T4, GV));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterGradFlow(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* P = NewMaterialExpressionFunctionInput(Function, TEXT("p"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Rot = NewMaterialExpressionFunctionInput(Function, TEXT("rot"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionAdd* Magic = NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, P, 1.0f / 41.0f), Rot);
    UMaterialExpressionSubtract* U = NewMaterialExpressionSubtract(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, Magic, NewMaterialExpressionFloor(Function, Magic)), 4.0f),
        2.0f);
    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionSubtract(Function, NewMaterialExpressionAbs(Function, U), 1.0f),
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionAbs(Function,
                    NewMaterialExpressionSubtract(Function, NewMaterialExpressionAbs(Function, NewMaterialExpressionAdd(Function, U, 1.0f)), 2.0f)),
                1.0f)
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterHSVToRGB(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* HSV = NewMaterialExpressionFunctionInput(Function, TEXT("hsv"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionComponentMask* HSVX = NewMaterialExpressionComponentMask(Function, HSV, 1);
    UMaterialExpressionComponentMask* HSVY = NewMaterialExpressionComponentMask(Function, HSV, 2);
    UMaterialExpressionComponentMask* HSVZ = NewMaterialExpressionComponentMask(Function, HSV, 4);

    UMaterialExpressionIf* HPrime = NewMaterialExpressionIfEqual(Function, HSVX, 1.0f, 0.0f, NewMaterialExpressionMultiply(Function, HSVX, 6.0f));
    UMaterialExpressionFloor* HFloor = NewMaterialExpressionFloor(Function, HPrime);
    UMaterialExpressionSubtract* F = NewMaterialExpressionSubtract(Function, HPrime, HFloor);
    UMaterialExpressionMultiply* ZY = NewMaterialExpressionMultiply(Function, HSVZ, HSVY);
    UMaterialExpressionSubtract* A = NewMaterialExpressionSubtract(Function, HSVZ, ZY);
    UMaterialExpressionSubtract* B = NewMaterialExpressionSubtract(Function, HSVZ, NewMaterialExpressionMultiply(Function, ZY, F));
    UMaterialExpressionAdd* C = NewMaterialExpressionAdd(Function, A, NewMaterialExpressionMultiply(Function, ZY, F));

    UMaterialExpressionIf* Check1 =
        NewMaterialExpressionIf(Function, HFloor, 1.0f,
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { HSVZ, C, A }),		// 0
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { B, HSVZ, A }),		// 1
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { A, HSVZ, C }));		// 2
    UMaterialExpressionIf* Check4 =
        NewMaterialExpressionIf(Function, HFloor, 4.0f,
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { A, B, HSVZ }),		// 3
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { C, A, HSVZ }),		// 4
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { HSVZ, A, B }));		// 5

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIf(Function, HFloor, 2.5f, Check1, {}, Check4));
}

static void Generator_ImporterMonoMode(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));

    UMaterialExpressionFunctionInput* Sample = NewMaterialExpressionFunctionInput(Function, TEXT("sample"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, mono_average);

    UMaterialExpressionComponentMask* Color = NewMaterialExpressionComponentMask(Function, Sample, 7);
    UMaterialExpressionComponentMask* Alpha = NewMaterialExpressionComponentMask(Function, Sample, 8);

    UMaterialExpressionIf* Result = NewMaterialExpressionSwitch(Function, Mode,
    {
        Alpha,
        NewMaterialExpressionFunctionCall(Function, MathAverage,{ Color }),
        NewMaterialExpressionFunctionCall(Function, MathLuminance,{ Color }),
        NewMaterialExpressionFunctionCall(Function, MathMaxValue,{ Color })
    });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterMiNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* XYZ = NewMaterialExpressionFunctionInput(Function, TEXT("xyz"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionVectorNoise* Noise = NewMaterialExpressionVectorNoise(Function, XYZ, VNF_GradientALU, 4);
    UMaterialExpressionComponentMask* Grad = NewMaterialExpressionComponentMask(Function, Noise, 7);
    UMaterialExpressionComponentMask* Value = NewMaterialExpressionComponentMask(Function, Noise, 8);

    NewMaterialExpressionFunctionOutput(Function, TEXT("grad"), Grad);
    NewMaterialExpressionFunctionOutput(Function, TEXT("value"), Value);
}

static void Generator_ImporterPerlinNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Pos = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector4);

    UMaterialExpressionNoise* Noise = NewMaterialExpressionNoise(Function, NewMaterialExpressionComponentMask(Function, Pos, 7), {}, 2);

    NewMaterialExpressionFunctionOutput(Function, TEXT("noise"), NewMaterialExpressionComponentMask(Function, Noise, 1));
}

static void Generator_ImporterPermuteFlow(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* X = NewMaterialExpressionFunctionInput(Function, TEXT("x"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionFmod* Result = NewMaterialExpressionFmod(Function,
        NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, { X, X, 34.0f }), X),
        289.0f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterRefract(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // The IncidentVector and the NormalVector are supposed to be normalized !
    UMaterialExpressionFunctionInput* IncidentVector = NewMaterialExpressionFunctionInput(Function, TEXT("incident_vector"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* NormalVector = NewMaterialExpressionFunctionInput(Function, TEXT("normal_vector"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Eta = NewMaterialExpressionFunctionInput(Function, TEXT("eta"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionDotProduct* DotNI = NewMaterialExpressionDotProduct(Function, NormalVector, IncidentVector);
    UMaterialExpressionOneMinus* K =
        NewMaterialExpressionOneMinus(Function, NewMaterialExpressionMultiply(Function, { Eta, Eta, NewMaterialExpressionOneMinus(Function, DotNI), DotNI }));
    UMaterialExpressionIf* Result =
        NewMaterialExpressionIfLess(Function, K, 0.0f,
            NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f),
            NewMaterialExpressionSubtract(Function,
                NewMaterialExpressionMultiply(Function, Eta, IncidentVector),
                NewMaterialExpressionMultiply(Function,
                    NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, Eta, DotNI), NewMaterialExpressionSquareRoot(Function, K)),
                    NormalVector)));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterSelectBSDF(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("Value"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* True = NewMaterialExpressionFunctionInput(Function, TEXT("True"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* False = NewMaterialExpressionFunctionInput(Function, TEXT("False"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionBreakMaterialAttributes* BreakTrue = NewMaterialExpressionBreakMaterialAttributes(Function, True);
    UMaterialExpressionBreakMaterialAttributes* BreakFalse = NewMaterialExpressionBreakMaterialAttributes(Function, False);

    UMaterialExpressionMakeMaterialAttributes* BSDF =
        NewMaterialExpressionMakeMaterialAttributes(Function,
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_BaseColor }, { BreakTrue,  MAI_BaseColor }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_Metallic }, { BreakTrue,  MAI_Metallic }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_Specular }, { BreakTrue,  MAI_Specular }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_Roughness }, { BreakTrue,  MAI_Roughness }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_EmissiveColor }, { BreakTrue,  MAI_EmissiveColor }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_Opacity }, { BreakTrue,  MAI_Opacity }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_OpacityMask }, { BreakTrue,  MAI_OpacityMask }),
            Normal,
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_WorldPositionOffset }, { BreakTrue,  MAI_WorldPositionOffset }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse,  MAI_WorldDisplacement }, { BreakTrue,  MAI_WorldDisplacement }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_TessellationMultiplier }, { BreakTrue, MAI_TessellationMultiplier }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_SubsurfaceColor }, { BreakTrue, MAI_SubsurfaceColor }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomData0 }, { BreakTrue, MAI_CustomData0 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomData1 }, { BreakTrue, MAI_CustomData1 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_AmbientOcclusion }, { BreakTrue, MAI_AmbientOcclusion }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_Refraction }, { BreakTrue, MAI_Refraction }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs0 }, { BreakTrue, MAI_CustomizedUVs0 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs1 }, { BreakTrue, MAI_CustomizedUVs1 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs2 }, { BreakTrue, MAI_CustomizedUVs2 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs3 }, { BreakTrue, MAI_CustomizedUVs3 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs4 }, { BreakTrue, MAI_CustomizedUVs4 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs5 }, { BreakTrue, MAI_CustomizedUVs5 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs6 }, { BreakTrue, MAI_CustomizedUVs6 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_CustomizedUVs7 }, { BreakTrue, MAI_CustomizedUVs7 }),
            NewMaterialExpressionIfEqual(Function, Value, 0.0f, { BreakFalse, MAI_PixelDepthOffset }, { BreakTrue, MAI_PixelDepthOffset }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

#if defined(ADD_CLIP_MASK)
static void Generator_ImporterSetClipMask(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ClipMask = LoadFunction(FMDLImporterUtility::GetContentPath(TEXT("/MDL/Materials/ClipSphere")), TEXT("ClipMask.ClipMask"));

    UMaterialExpressionFunctionInput* Material = NewMaterialExpressionFunctionInput(Function, TEXT("material"), EFunctionInputType::FunctionInput_MaterialAttributes);

    UMaterialExpressionBreakMaterialAttributes* Break = NewMaterialExpressionBreakMaterialAttributes(Function, Material);
    UMaterialExpressionMaterialFunctionCall* Factor = NewMaterialExpressionFunctionCall(Function, ClipMask, {});

    UMaterialExpressionMultiply* Opacity = NewMaterialExpressionMultiply(Function, { Break, 5 }, Factor);
    UMaterialExpressionMultiply* OpacityMask = NewMaterialExpressionMultiply(Function, { Break, 6 }, Factor);

    UMaterialExpressionMakeMaterialAttributes* BSDF =
        NewMaterialExpressionMakeMaterialAttributes(Function,
        { Break, MAI_BaseColor },		// BaseColor
        { Break, MAI_Metallic },		// Metallic
        { Break, MAI_Specular },		// Specular
        { Break, MAI_Roughness },		// Roughness
        { Break, MAI_EmissiveColor },		// EmissiveColor
        Opacity,			// Opacity
        OpacityMask,		// OpacityMask
        { Break, MAI_Normal },		// Normal
        { Break, MAI_WorldPositionOffset },		// WorldPositionOffset
        { Break, MAI_WorldDisplacement },		// WorldDisplacement
        { Break, MAI_TessellationMultiplier },		// TessellationMultiplier
        { Break, MAI_SubsurfaceColor },		// SubsurfaceColor
        { Break, MAI_CustomData0 },		// ClearCoat
        { Break, MAI_CustomData1 },		// ClearCoatRoughness
        { Break, MAI_AmbientOcclusion },		// AmbientOcclusion
        { Break, MAI_Refraction },		// Refraction
        { Break, MAI_CustomizedUVs0 },		// CustomizedUVs0
        { Break, MAI_CustomizedUVs1 },		// CustomizedUVs1
        { Break, MAI_CustomizedUVs2 },		// CustomizedUVs2
        { Break, MAI_CustomizedUVs3 },		// CustomizedUVs3
        { Break, MAI_CustomizedUVs4 },		// CustomizedUVs4
        { Break, MAI_CustomizedUVs5 },		// CustomizedUVs5
        { Break, MAI_CustomizedUVs6 },		// CustomizedUVs6
        { Break, MAI_CustomizedUVs7 },		// CustomizedUVs7
        { Break, MAI_PixelDepthOffset });		// PixelDepthOffset

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}
#endif

static void Generator_ImporterSetRefraction(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Material = NewMaterialExpressionFunctionInput(Function, TEXT("material"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* IOR = NewMaterialExpressionFunctionInput(Function, TEXT("IOR"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionLinearInterpolate* Refraction =
        NewMaterialExpressionLinearInterpolate(Function, NewMaterialExpressionConstant(Function, 1.0f), IOR, NewMaterialExpressionFresnel(Function));

    UMaterialExpressionBreakMaterialAttributes* MaterialBreak = NewMaterialExpressionBreakMaterialAttributes(Function, Material);
    UMaterialExpressionMakeMaterialAttributes* BSDF =
        NewMaterialExpressionMakeMaterialAttributes(Function,
        { MaterialBreak, MAI_BaseColor },		// BaseColor
        { MaterialBreak, MAI_Metallic },		// Metallic
        { MaterialBreak, MAI_Specular },		// Specular
        { MaterialBreak, MAI_Roughness },		// Roughness
        { MaterialBreak, MAI_EmissiveColor },		// EmissiveColor
        { MaterialBreak, MAI_Opacity },		// Opacity,
        { MaterialBreak, MAI_OpacityMask },		// OpacityMask
        { MaterialBreak, MAI_Normal },		// Normal
        { MaterialBreak, MAI_WorldPositionOffset },		// WorldPositionOffset
        { MaterialBreak, MAI_WorldDisplacement },		// WorldDisplacement
        { MaterialBreak, MAI_TessellationMultiplier },		// TessellationMultiplier
        { MaterialBreak, MAI_SubsurfaceColor },		// SubsurfaceColor
        { MaterialBreak, MAI_CustomData0 },		// ClearCoat
        { MaterialBreak, MAI_CustomData1 },		// ClearCoatRoughness
        { MaterialBreak, MAI_AmbientOcclusion },		// AmbientOcclusion
        Refraction,					// Refraction
        { MaterialBreak, MAI_CustomizedUVs0 },		// CustomizedUVs0
        { MaterialBreak, MAI_CustomizedUVs1 },		// CustomizedUVs1
        { MaterialBreak, MAI_CustomizedUVs2 },		// CustomizedUVs2
        { MaterialBreak, MAI_CustomizedUVs3 },		// CustomizedUVs3
        { MaterialBreak, MAI_CustomizedUVs4 },		// CustomizedUVs4
        { MaterialBreak, MAI_CustomizedUVs5 },		// CustomizedUVs5
        { MaterialBreak, MAI_CustomizedUVs6 },		// CustomizedUVs6
        { MaterialBreak, MAI_CustomizedUVs7 },		// CustomizedUVs7
        { MaterialBreak, MAI_PixelDepthOffset });		// PixelDepthOffset

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_ImporterSetSubsurfaceColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Material = NewMaterialExpressionFunctionInput(Function, TEXT("material"), EFunctionInputType::FunctionInput_MaterialAttributes);
    UMaterialExpressionFunctionInput* SubsurfaceColor = NewMaterialExpressionFunctionInput(Function, TEXT("subsurface_color"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionBreakMaterialAttributes* MaterialBreak = NewMaterialExpressionBreakMaterialAttributes(Function, Material);

    UMaterialExpressionMakeMaterialAttributes* BSDF =
        NewMaterialExpressionMakeMaterialAttributes(Function,
        { MaterialBreak, MAI_BaseColor },		// BaseColor
        { MaterialBreak, MAI_Metallic },		// Metallic
        { MaterialBreak, MAI_Specular },		// Specular
        { MaterialBreak, MAI_Roughness },		// Roughness
        { MaterialBreak, MAI_EmissiveColor },		// EmissiveColor
        { MaterialBreak, MAI_Opacity },		// Opacity,
        { MaterialBreak, MAI_OpacityMask },		// OpacityMask
        { MaterialBreak, MAI_Normal },		// Normal
        { MaterialBreak, MAI_WorldPositionOffset },		// WorldPositionOffset
        { MaterialBreak, MAI_WorldDisplacement },		// WorldDisplacement
        { MaterialBreak, MAI_TessellationMultiplier },		// TessellationMultiplier
        SubsurfaceColor,			// SubsurfaceColor
        { MaterialBreak, MAI_CustomData0 },		// ClearCoat
        { MaterialBreak, MAI_CustomData1 },		// ClearCoatRoughness
        { MaterialBreak, MAI_AmbientOcclusion },		// AmbientOcclusion
        { MaterialBreak, MAI_Refraction },		// Refraction
        { MaterialBreak, MAI_CustomizedUVs0 },		// CustomizedUVs0
        { MaterialBreak, MAI_CustomizedUVs1 },		// CustomizedUVs1
        { MaterialBreak, MAI_CustomizedUVs2 },		// CustomizedUVs2
        { MaterialBreak, MAI_CustomizedUVs3 },		// CustomizedUVs3
        { MaterialBreak, MAI_CustomizedUVs4 },		// CustomizedUVs4
        { MaterialBreak, MAI_CustomizedUVs5 },		// CustomizedUVs5
        { MaterialBreak, MAI_CustomizedUVs6 },		// CustomizedUVs6
        { MaterialBreak, MAI_CustomizedUVs7 },		// CustomizedUVs7
        { MaterialBreak, MAI_PixelDepthOffset });		// PixelDepthOffset

    NewMaterialExpressionFunctionOutput(Function, TEXT("bsdf"), BSDF);
}

static void Generator_ImporterSummedFlowNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterFlowNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_flow_noise"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* Pos = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Time = NewMaterialExpressionFunctionInput(Function, TEXT("time"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Iterations = NewMaterialExpressionFunctionInput(Function, TEXT("iterations"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* AbsNoise = NewMaterialExpressionFunctionInput(Function, TEXT("abs_noise"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* WeightFactor = NewMaterialExpressionFunctionInput(Function, TEXT("weight_factor"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* PosFactor = NewMaterialExpressionFunctionInput(Function, TEXT("pos_factor"), EFunctionInputType::FunctionInput_Scalar, 2.0f);
    UMaterialExpressionFunctionInput* UProgressiveScale = NewMaterialExpressionFunctionInput(Function, TEXT("u_progressive_scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* VProgressiveOffset = NewMaterialExpressionFunctionInput(Function, TEXT("v_progressive_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    // i == 0
    UMaterialExpressionAppendVector* P0 = NewMaterialExpressionAppendVector(Function, Pos, Time);
    UMaterialExpressionIf* LerpPos0 = NewMaterialExpressionIfGreater(Function, Iterations, 1.0f, 0.0f, 1.0f);
    UMaterialExpressionMaterialFunctionCall* Noise0 = NewMaterialExpressionFunctionCall(Function, ImporterFlowNoise,
    {
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionComponentMask(Function, P0, 1),
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionOneMinus(Function, LerpPos0),
                    NewMaterialExpressionMultiply(Function, LerpPos0, UProgressiveScale))),
            NewMaterialExpressionAdd(Function,
                NewMaterialExpressionComponentMask(Function, P0, 2),
                NewMaterialExpressionMultiply(Function, LerpPos0, VProgressiveOffset))
        }),
        NewMaterialExpressionComponentMask(Function, P0, 4)
    });
    UMaterialExpressionIf* Sum1 = NewMaterialExpressionStaticSwitch(Function, AbsNoise, NewMaterialExpressionAbs(Function, Noise0), Noise0);

    // i == 1
    UMaterialExpressionDivide* InvIterations = NewMaterialExpressionDivide(Function, 1.0f, NewMaterialExpressionSubtract(Function, Iterations, 1.0f));
    UMaterialExpressionMultiply* P1 = NewMaterialExpressionMultiply(Function, P0, PosFactor);
    UMaterialExpressionIf* LerpPos1 = NewMaterialExpressionIfGreater(Function, Iterations, 1.0f, NewMaterialExpressionMultiply(Function, 1.0f, InvIterations), 1.0f);
    UMaterialExpressionMaterialFunctionCall* Noise1 = NewMaterialExpressionFunctionCall(Function, ImporterFlowNoise,
    {
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionComponentMask(Function, P1, 1),
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionOneMinus(Function, LerpPos1),
                    NewMaterialExpressionMultiply(Function, LerpPos1, UProgressiveScale))),
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionComponentMask(Function, P1, 2),
                NewMaterialExpressionMultiply(Function, LerpPos1, VProgressiveOffset),
                7.0f
            })
        }),
        NewMaterialExpressionComponentMask(Function, P1, 4)
    });
    UMaterialExpressionAdd* Sum2 = NewMaterialExpressionAdd(Function,
        Sum1,
        NewMaterialExpressionMultiply(Function, WeightFactor, NewMaterialExpressionStaticSwitch(Function, AbsNoise, NewMaterialExpressionAbs(Function, Noise1), Noise1)));

    // i == 2
    UMaterialExpressionMultiply* Weight2 = NewMaterialExpressionMultiply(Function, WeightFactor, WeightFactor);
    UMaterialExpressionMultiply* P2 = NewMaterialExpressionMultiply(Function, P1, PosFactor);
    UMaterialExpressionIf* LerpPos2 = NewMaterialExpressionIfGreater(Function, Iterations, 1.0f, NewMaterialExpressionMultiply(Function, 2.0f, InvIterations), 1.0f);
    UMaterialExpressionMaterialFunctionCall* Noise2 = NewMaterialExpressionFunctionCall(Function, ImporterFlowNoise,
    {
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionComponentMask(Function, P2, 1),
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionOneMinus(Function, LerpPos2),
                    NewMaterialExpressionMultiply(Function, LerpPos2, UProgressiveScale))),
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionComponentMask(Function, P2, 2),
                NewMaterialExpressionMultiply(Function, LerpPos2, VProgressiveOffset),
                14.0f
            })
        }),
        NewMaterialExpressionComponentMask(Function, P2, 4)
    });
    UMaterialExpressionAdd* Sum3 = NewMaterialExpressionAdd(Function,
        Sum2,
        NewMaterialExpressionMultiply(Function, Weight2, NewMaterialExpressionStaticSwitch(Function, AbsNoise, NewMaterialExpressionAbs(Function, Noise2), Noise2)));

    // i == 3
    UMaterialExpressionMultiply* Weight3 = NewMaterialExpressionMultiply(Function, Weight2, WeightFactor);
    UMaterialExpressionMultiply* P3 = NewMaterialExpressionMultiply(Function, P2, PosFactor);
    UMaterialExpressionIf* LerpPos3 = NewMaterialExpressionIfGreater(Function, Iterations, 1.0f, NewMaterialExpressionMultiply(Function, 3.0f, InvIterations), 1.0f);
    UMaterialExpressionMaterialFunctionCall* Noise3 = NewMaterialExpressionFunctionCall(Function, ImporterFlowNoise,
    {
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionComponentMask(Function, P3, 1),
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionOneMinus(Function, LerpPos3),
                    NewMaterialExpressionMultiply(Function, LerpPos3, UProgressiveScale))),
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionComponentMask(Function, P3, 2),
                NewMaterialExpressionMultiply(Function, LerpPos3, VProgressiveOffset),
                21.0f
            })
        }),
        NewMaterialExpressionComponentMask(Function, P3, 4)
    });
    UMaterialExpressionAdd* Sum4 = NewMaterialExpressionAdd(Function,
        Sum3,
        NewMaterialExpressionMultiply(Function, Weight3, NewMaterialExpressionStaticSwitch(Function, AbsNoise, NewMaterialExpressionAbs(Function, Noise3), Noise3)));

    // i == 4
    UMaterialExpressionMultiply* Weight4 = NewMaterialExpressionMultiply(Function, Weight3, WeightFactor);
    UMaterialExpressionMultiply* P4 = NewMaterialExpressionMultiply(Function, P3, PosFactor);
    UMaterialExpressionIf* LerpPos4 = NewMaterialExpressionIfGreater(Function, Iterations, 1.0f, NewMaterialExpressionMultiply(Function, 4.0f, InvIterations), 1.0f);
    UMaterialExpressionMaterialFunctionCall* Noise4 = NewMaterialExpressionFunctionCall(Function, ImporterFlowNoise,
    {
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionMultiply(Function,
                NewMaterialExpressionComponentMask(Function, P4, 1),
                NewMaterialExpressionAdd(Function,
                    NewMaterialExpressionOneMinus(Function, LerpPos4),
                    NewMaterialExpressionMultiply(Function, LerpPos4, UProgressiveScale))),
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionComponentMask(Function, P4, 2),
                NewMaterialExpressionMultiply(Function, LerpPos4, VProgressiveOffset),
                28.0f
            })
        }),
        NewMaterialExpressionComponentMask(Function, P4, 4)
    });
    UMaterialExpressionAdd* Sum5 = NewMaterialExpressionAdd(Function,
        Sum4,
        NewMaterialExpressionMultiply(Function, Weight4, NewMaterialExpressionStaticSwitch(Function, AbsNoise, NewMaterialExpressionAbs(Function, Noise4), Noise4)));

    UMaterialExpressionIf* SumGreaterTwo = NewMaterialExpressionIf(Function, Iterations, 4.0f, Sum3, Sum4, Sum5);
    UMaterialExpressionIf* Sum = NewMaterialExpressionIf(Function, Iterations, 2.0f, Sum1, Sum2, SumGreaterTwo);

    NewMaterialExpressionFunctionOutput(Function, TEXT("sum"), Sum);
}

static void Generator_ImporterSummedPerlinNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* PerlinNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_perlin_noise"));

    UMaterialExpressionFunctionInput* Pos = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Time = NewMaterialExpressionFunctionInput(Function, TEXT("time"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Terms = NewMaterialExpressionFunctionInput(Function, TEXT("terms"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TurbulenceWeight = NewMaterialExpressionFunctionInput(Function, TEXT("turbulence_weight"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* AbsNoise = NewMaterialExpressionFunctionInput(Function, TEXT("abs_noise"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Ridged = NewMaterialExpressionFunctionInput(Function, TEXT("ridged"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionAppendVector* P1 = NewMaterialExpressionAppendVector(Function, Pos, Time);
    UMaterialExpressionIf* Weight1 = NewMaterialExpressionStaticSwitch(Function, Ridged, 0.625f, 1.0f);
    UMaterialExpressionMaterialFunctionCall* Noise1 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { P1 });
    UMaterialExpressionAbs* AbsNoise1 = NewMaterialExpressionAbs(Function, Noise1);
    UMaterialExpressionIf* N21 =
        NewMaterialExpressionStaticSwitch(Function, Ridged,
            NewMaterialExpressionSquare(Function, NewMaterialExpressionOneMinus(Function, AbsNoise1)),
            NewMaterialExpressionStaticSwitch(Function, AbsNoise, AbsNoise1, Noise1));
    UMaterialExpressionMultiply* Sum1 = NewMaterialExpressionMultiply(Function, Weight1, N21);

    UMaterialExpressionAdd* P2 = NewMaterialExpressionAdd(Function, P1, P1);
    UMaterialExpressionIf* Prev2 = NewMaterialExpressionStaticSwitch(Function, Ridged, N21, 1.0f);
    UMaterialExpressionMultiply* Weight2 = NewMaterialExpressionMultiply(Function, Weight1, 0.5f);
    UMaterialExpressionMaterialFunctionCall* Noise2 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { P2 });
    UMaterialExpressionAbs* AbsNoise2 = NewMaterialExpressionAbs(Function, Noise2);
    UMaterialExpressionIf* N22 =
        NewMaterialExpressionStaticSwitch(Function, Ridged,
            NewMaterialExpressionSquare(Function, NewMaterialExpressionOneMinus(Function, AbsNoise2)),
            NewMaterialExpressionStaticSwitch(Function, AbsNoise, AbsNoise2, Noise2));
    UMaterialExpressionAdd* Sum2 = NewMaterialExpressionAdd(Function, Sum1, NewMaterialExpressionMultiply(Function, { Weight2, Prev2, N22 }));

    UMaterialExpressionAdd* P3 = NewMaterialExpressionAdd(Function, P2, P2);
    UMaterialExpressionIf* Prev3 = NewMaterialExpressionStaticSwitch(Function, Ridged, N22, 1.0f);
    UMaterialExpressionMultiply* Weight3 = NewMaterialExpressionMultiply(Function, Weight2, 0.5f);
    UMaterialExpressionMaterialFunctionCall* Noise3 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { P3 });
    UMaterialExpressionAbs* AbsNoise3 = NewMaterialExpressionAbs(Function, Noise3);
    UMaterialExpressionIf* N23 =
        NewMaterialExpressionStaticSwitch(Function, Ridged,
            NewMaterialExpressionSquare(Function, NewMaterialExpressionOneMinus(Function, AbsNoise3)),
            NewMaterialExpressionStaticSwitch(Function, AbsNoise, AbsNoise3, Noise3));
    UMaterialExpressionAdd* Sum3 = NewMaterialExpressionAdd(Function, Sum2, NewMaterialExpressionMultiply(Function, { Weight3, Prev3, N23 }));

    UMaterialExpressionAdd* P4 = NewMaterialExpressionAdd(Function, P3, P3);
    UMaterialExpressionIf* Prev4 = NewMaterialExpressionStaticSwitch(Function, Ridged, N23, 1.0f);
    UMaterialExpressionMultiply* Weight4 = NewMaterialExpressionMultiply(Function, Weight3, 0.5f);
    UMaterialExpressionMaterialFunctionCall* Noise4 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { P4 });
    UMaterialExpressionAbs* AbsNoise4 = NewMaterialExpressionAbs(Function, Noise4);
    UMaterialExpressionIf* N24 =
        NewMaterialExpressionStaticSwitch(Function, Ridged,
            NewMaterialExpressionSquare(Function, NewMaterialExpressionOneMinus(Function, AbsNoise4)),
            NewMaterialExpressionStaticSwitch(Function, AbsNoise, AbsNoise4, Noise4));
    UMaterialExpressionAdd* Sum4 = NewMaterialExpressionAdd(Function, Sum3, NewMaterialExpressionMultiply(Function, { Weight4, Prev4, N24 }));

    UMaterialExpressionAdd* P5 = NewMaterialExpressionAdd(Function, P4, P4);
    UMaterialExpressionIf* Prev5 = NewMaterialExpressionStaticSwitch(Function, Ridged, N24, 1.0f);
    UMaterialExpressionMultiply* Weight5 = NewMaterialExpressionMultiply(Function, Weight4, 0.5f);
    UMaterialExpressionMaterialFunctionCall* Noise5 = NewMaterialExpressionFunctionCall(Function, PerlinNoise, { P5 });
    UMaterialExpressionAbs* AbsNoise5 = NewMaterialExpressionAbs(Function, Noise5);
    UMaterialExpressionIf* N25 =
        NewMaterialExpressionStaticSwitch(Function, Ridged,
            NewMaterialExpressionSquare(Function, NewMaterialExpressionOneMinus(Function, AbsNoise5)),
            NewMaterialExpressionStaticSwitch(Function, AbsNoise, AbsNoise5, Noise5));
    UMaterialExpressionAdd* Sum5 = NewMaterialExpressionAdd(Function, Sum4, NewMaterialExpressionMultiply(Function, { Weight5, Prev5, N25 }));

    UMaterialExpressionIf* Sum345 = NewMaterialExpressionIf(Function, Terms, 4.0f, Sum3, Sum4, Sum5);
    UMaterialExpressionIf* SumAll = NewMaterialExpressionIf(Function, Terms, 2.0f, Sum1, Sum2, Sum345);

    UMaterialExpressionIf* Noise =
        NewMaterialExpressionIfEqual(Function, TurbulenceWeight, { 0.0f, 0.0f, 0.0f },
            SumAll,
            NewMaterialExpressionSine(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionDotProduct(Function, Pos, TurbulenceWeight), SumAll)));
    UMaterialExpressionIf* RidgedCheck =
        NewMaterialExpressionStaticSwitch(Function, Ridged, Noise, NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, Noise, 0.5f), 0.5f));
    UMaterialExpressionIf* Sum = NewMaterialExpressionStaticSwitch(Function, AbsNoise, Noise, RidgedCheck);

    NewMaterialExpressionFunctionOutput(Function, TEXT("sum"), Sum);
}

static void Generator_ImporterTexremapu1(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* TexRes = NewMaterialExpressionFunctionInput(Function, TEXT("texres"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* CropOffset = NewMaterialExpressionFunctionInput(Function, TEXT("crop_ofs"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TexiIn = NewMaterialExpressionFunctionInput(Function, TEXT("texi_in"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Wrap = NewMaterialExpressionFunctionInput(Function, TEXT("wrap"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionIf* S = NewMaterialExpressionIfLess(Function, TexiIn, 0.0f, 1.0f, 0.0f);
    UMaterialExpressionFloor* D = NewMaterialExpressionFloor(Function, NewMaterialExpressionDivide(Function, TexiIn, TexRes));
    UMaterialExpressionFmod* Texi0 = NewMaterialExpressionFmod(Function, TexiIn, TexRes);
    UMaterialExpressionIf* Alternate =
        NewMaterialExpressionIfEqual(Function, Wrap, wrap_mirrored_repeat,
            NewMaterialExpressionIfEqual(Function, NewMaterialExpressionFmod(Function, D, 2.0f), S, 0.0f, 1.0f),
            0.0f);
    UMaterialExpressionIf* Texi1 = NewMaterialExpressionIfEqual(Function, Alternate, 1.0f, NewMaterialExpressionNegate(Function, Texi0), Texi0);
    UMaterialExpressionIf* Texi2 = NewMaterialExpressionIfEqual(Function, S, Alternate,
        Texi1,
        NewMaterialExpressionAdd(Function, { Texi1, TexRes, -1.0f }));
    UMaterialExpressionIf* WrapCheck =
        NewMaterialExpressionIfEqual(Function, Wrap, wrap_clamp,
            NewMaterialExpressionClamp(Function, TexiIn, 0.0f, NewMaterialExpressionSubtract(Function, TexRes, 1.0f)),
            Texi2);
    UMaterialExpressionAdd* Texi = NewMaterialExpressionAdd(Function, WrapCheck, CropOffset);

    NewMaterialExpressionFunctionOutput(Function, TEXT("texi"), Texi);
}

static void Generator_ImporterTexremapu2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterTexremapu1 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_texremapu1"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* TexRes = NewMaterialExpressionFunctionInput(Function, TEXT("texres"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* CropOffset = NewMaterialExpressionFunctionInput(Function, TEXT("crop_ofs"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionMaterialFunctionCall* Result =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionFunctionCall(Function, ImporterTexremapu1,
            {
                NewMaterialExpressionComponentMask(Function, TexRes, 1),
                NewMaterialExpressionComponentMask(Function, CropOffset, 1),
                NewMaterialExpressionComponentMask(Function, Tex, 1),
                WrapU
            }),
            NewMaterialExpressionFunctionCall(Function, ImporterTexremapu1,
            {
                NewMaterialExpressionComponentMask(Function, TexRes, 2),
                NewMaterialExpressionComponentMask(Function, CropOffset, 2),
                NewMaterialExpressionComponentMask(Function, Tex, 2),
                WrapV
            })
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ImporterTexture2DSample(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BreakFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("BreakFloat2Components.BreakFloat2Components"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2, NewMaterialExpressionTextureCoordinate(Function, 0));
    // NOTE: UE4 material system can't do separated UV address
    // Mirror TODO?
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });

    UMaterialExpressionMaterialFunctionCall* UpperLeftCorner =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionComponentMask(Function, CropU, 1),
            NewMaterialExpressionComponentMask(Function, CropV, 1)
        });
    UMaterialExpressionMaterialFunctionCall* LowerRightCorner =
        NewMaterialExpressionFunctionCall(Function, MakeFloat2,
        {
            NewMaterialExpressionComponentMask(Function, CropU, 2),
            NewMaterialExpressionComponentMask(Function, CropV, 2)
        });

    // we need to OneMinus the y-component of the texture coordinate, to get the correct texture access
    UMaterialExpressionMaterialFunctionCall* CoordBreak = NewMaterialExpressionFunctionCall(Function, BreakFloat2, { Coord });
    UMaterialExpressionAppendVector* AdjustedCoord = NewMaterialExpressionAppendVector(Function, { CoordBreak, 0 }, NewMaterialExpressionOneMinus(Function, { CoordBreak, 1 }));
    UMaterialExpressionDivide* CroppedUVs = NewMaterialExpressionDivide(Function,
        NewMaterialExpressionSubtract(Function, AdjustedCoord, UpperLeftCorner),
        NewMaterialExpressionSubtract(Function, LowerRightCorner, UpperLeftCorner));

    // If uv has the different address mode, prefer using the addressV mode.
    UMaterialExpressionTextureSample* ClampSample = NewMaterialExpressionTextureSample(Function, Tex, CroppedUVs, SSM_Clamp_WorldGroupSettings);
    UMaterialExpressionTextureSample* WrapSample = NewMaterialExpressionTextureSample(Function, Tex, CroppedUVs, SSM_Wrap_WorldGroupSettings);
    UMaterialExpressionConstant3Vector* Black = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f);
    UMaterialExpressionConstant* BlackA = NewMaterialExpressionConstant(Function, 0.0f);

    UMaterialExpressionMaterialFunctionCall* FinalCoordBreak = NewMaterialExpressionFunctionCall(Function, BreakFloat2, { CroppedUVs });

    UMaterialExpressionIf* ClipRGB = NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 0}, 1.0f, 
        NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 0}, 0.0f, Black, 
            NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 1}, 1.0f, 
                NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 1}, 0.0f, Black, 
                    {ClampSample, 0}), 
                Black)), 
        Black);

    UMaterialExpressionIf* ClipA = NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 0}, 1.0f, 
        NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 0}, 0.0f, BlackA, 
            NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 1}, 1.0f, 
                NewMaterialExpressionIfLess(Function, {FinalCoordBreak, 1}, 0.0f, BlackA, 
                    {ClampSample, 4}), 
                BlackA)), 
        BlackA);

    UMaterialExpressionIf* RGB = NewMaterialExpressionSwitch(Function, WrapV, 
        {
            {ClampSample, 0}, 
            {WrapSample, 0},
            {WrapSample, 0}, //Mirror
            ClipRGB
        });
    UMaterialExpressionIf* R = NewMaterialExpressionSwitch(Function, WrapV, 
        {
            {ClampSample, 1}, 
            {WrapSample, 1},
            {WrapSample, 1}, //Mirror
            NewMaterialExpressionComponentMask(Function, ClipRGB, 1)
        });
    UMaterialExpressionIf* G = NewMaterialExpressionSwitch(Function, WrapV, 
        {
            {ClampSample, 2}, 
            {WrapSample, 2},
            {WrapSample, 2}, //Mirror
            NewMaterialExpressionComponentMask(Function, ClipRGB, 2)
        });
    UMaterialExpressionIf* B = NewMaterialExpressionSwitch(Function, WrapV, 
        {
            {ClampSample, 3}, 
            {WrapSample, 3},
            {WrapSample, 3}, //Mirror
            NewMaterialExpressionComponentMask(Function, ClipRGB, 4)
        });
    UMaterialExpressionIf* A = NewMaterialExpressionSwitch(Function, WrapV, 
        {
            {ClampSample, 4}, 
            {WrapSample, 4},
            {WrapSample, 4}, //Mirror
            ClipA
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("rgb"), RGB);
    NewMaterialExpressionFunctionOutput(Function, TEXT("r"), R);
    NewMaterialExpressionFunctionOutput(Function, TEXT("g"), G);
    NewMaterialExpressionFunctionOutput(Function, TEXT("b"), B);
    NewMaterialExpressionFunctionOutput(Function, TEXT("a"), A);
}

static void Generator_ImporterTextureCubeSample(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* BreakFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("BreakFloat2Components.BreakFloat2Components"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_TextureCube, NewMaterialExpressionTextureObject(Function, LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionTextureSample* Sample = NewMaterialExpressionTextureSample(Function, Tex, Coord);

    NewMaterialExpressionFunctionOutput(Function, TEXT("rgb"), { Sample, 0 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("r"), { Sample, 1 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("g"), { Sample, 2 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("b"), { Sample, 3 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("a"), { Sample, 4 });
}

#if defined(USE_WORLD_ALIGNED_TEXTURES)
static void Generator_ImporterWorldAlignedTextureFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* CheapContrast = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment"), TEXT("CheapContrast.CheapContrast"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionTextureSample* RGSample = NewMaterialExpressionTextureSample(Function, Tex, WorldAlignedPositionXY);
    UMaterialExpressionTextureSample* RBSample = NewMaterialExpressionTextureSample(Function, Tex, WorldAlignedPositionXZ);
    UMaterialExpressionTextureSample* GBSample = NewMaterialExpressionTextureSample(Function, Tex, WorldAlignedPositionYZ);

    UMaterialExpressionVertexNormalWS* Normal = NewMaterialExpression<UMaterialExpressionVertexNormalWS>(Function);
    UMaterialExpressionMaterialFunctionCall* ContrastR =
        NewMaterialExpressionFunctionCall(Function, CheapContrast, { NewMaterialExpressionAbs(Function, NewMaterialExpressionComponentMask(Function, Normal, 1)), 1.0f });
    UMaterialExpressionMaterialFunctionCall* ContrastB =
        NewMaterialExpressionFunctionCall(Function, CheapContrast, { NewMaterialExpressionAbs(Function, NewMaterialExpressionComponentMask(Function, Normal, 4)), 1.0f });

    UMaterialExpressionLinearInterpolate* Lerp1 = NewMaterialExpressionLinearInterpolate(Function, RBSample, GBSample, ContrastR);
    UMaterialExpressionLinearInterpolate* Lerp2 = NewMaterialExpressionLinearInterpolate(Function, Lerp1, RGSample, ContrastB);

    NewMaterialExpressionFunctionOutput(Function, TEXT("xyz texture"), Lerp2);
}

static void Generator_ImporterWorldAlignedTextureFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* CheapContrast = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment"), TEXT("CheapContrast.CheapContrast"));

    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionTextureSample* RGSample = NewMaterialExpressionTextureSample(Function, Tex, WorldAlignedPositionXY);
    UMaterialExpressionTextureSample* RBSample = NewMaterialExpressionTextureSample(Function, Tex, WorldAlignedPositionXZ);
    UMaterialExpressionTextureSample* GBSample = NewMaterialExpressionTextureSample(Function, Tex, WorldAlignedPositionYZ);

    UMaterialExpressionVertexNormalWS* Normal = NewMaterialExpression<UMaterialExpressionVertexNormalWS>(Function);
    UMaterialExpressionMaterialFunctionCall* ContrastR =
        NewMaterialExpressionFunctionCall(Function, CheapContrast, { NewMaterialExpressionAbs(Function, NewMaterialExpressionComponentMask(Function, Normal, 1)), 1.0f });
    UMaterialExpressionMaterialFunctionCall* ContrastB =
        NewMaterialExpressionFunctionCall(Function, CheapContrast, { NewMaterialExpressionAbs(Function, NewMaterialExpressionComponentMask(Function, Normal, 4)), 1.0f });

    UMaterialExpressionLinearInterpolate* Lerp1 =
        NewMaterialExpressionLinearInterpolate(Function,
            NewMaterialExpressionAppendVector(Function, { RBSample, 0 }, { RBSample, 4 }),
            NewMaterialExpressionAppendVector(Function, { GBSample, 0 }, { GBSample, 4 }),
            ContrastR);
    UMaterialExpressionLinearInterpolate* Lerp2 =
        NewMaterialExpressionLinearInterpolate(Function, Lerp1, NewMaterialExpressionAppendVector(Function, { RGSample, 0 }, { RGSample, 4 }), ContrastB);

    NewMaterialExpressionFunctionOutput(Function, TEXT("xyz texture"), Lerp2);
}
#endif

static void Generator_ImporterWorleyNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* Pos = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Jitter = NewMaterialExpressionFunctionInput(Function, TEXT("jitter"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Metric = NewMaterialExpressionFunctionInput(Function, TEXT("metric"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionVectorNoise* Noise = NewMaterialExpressionVectorNoise(Function, Pos, VNF_VoronoiALU, 4);
    UMaterialExpressionComponentMask* NearestPos0 = NewMaterialExpressionComponentMask(Function, Noise, 7);
    UMaterialExpressionComponentMask* Value = NewMaterialExpressionComponentMask(Function, Noise, 8);
    UMaterialExpressionMaterialFunctionCall* Val = NewMaterialExpressionFunctionCall(Function, MakeFloat2, { Value, Value });		// Val.y should correspond to NearestPos1 !!

    NewMaterialExpressionFunctionOutput(Function, TEXT("nearest_pos_0"), NearestPos0);
    NewMaterialExpressionFunctionOutput(Function, TEXT("nearest_pos_1"), NearestPos0);	// should be the second nearest position !!
    NewMaterialExpressionFunctionOutput(Function, TEXT("val"), Val);
}

static void Generator_ImporterWorleyNoiseExt(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* ImporterWorleyNoise = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_worley_noise"));
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));
    UMaterialFunction* MathSum = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_sum"));

    UMaterialExpressionFunctionInput* Pos = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* TurbulenceWeight = NewMaterialExpressionFunctionInput(Function, TEXT("turbulence_weight"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* StepThreshold = NewMaterialExpressionFunctionInput(Function, TEXT("step_threshold"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Metric = NewMaterialExpressionFunctionInput(Function, TEXT("metric"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Jitter = NewMaterialExpressionFunctionInput(Function, TEXT("jitter"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionMaterialFunctionCall* WorleyNoise = NewMaterialExpressionFunctionCall(Function, ImporterWorleyNoise, { Pos, Jitter, Metric });
    UMaterialExpressionSubtract* PosDiff = NewMaterialExpressionSubtract(Function, { WorleyNoise, 0 }, { WorleyNoise, 1 });

    UMaterialExpressionComponentMask* F1F2X = NewMaterialExpressionComponentMask(Function, { WorleyNoise, 2 }, 1);
    UMaterialExpressionComponentMask* F1F2Y = NewMaterialExpressionComponentMask(Function, { WorleyNoise, 2 }, 2);
    UMaterialExpressionSubtract* Diff = NewMaterialExpressionSubtract(Function, F1F2Y, F1F2X);
    UMaterialExpressionAdd* Sum = NewMaterialExpressionAdd(Function, F1F2Y, F1F2X);
    UMaterialExpression* Simple0 = F1F2X;
    UMaterialExpressionMultiply* Simple1 = NewMaterialExpressionSquare(Function, F1F2X);
    UMaterialExpression* Cell = F1F2X;		// would require some random/xor -> just do something here, for now!
    UMaterialExpressionIf* Step0 = NewMaterialExpressionIfLess(Function, Diff, StepThreshold, 0.0f, 1.0f);
    UMaterialExpression* Step1 = Diff;
    UMaterialExpressionDivide* Step2 = NewMaterialExpressionDivide(Function, Diff, Sum);
    UMaterialExpressionMultiply* Mul = NewMaterialExpressionMultiply(Function, F1F2X, F1F2Y);
    UMaterialExpression* Add = Sum;
    UMaterialExpression* Simple2 = F1F2Y;
    UMaterialExpressionMultiply* Simple3 = NewMaterialExpressionSquare(Function, F1F2Y);
    UMaterialExpressionMaterialFunctionCall* Manhattan = NewMaterialExpressionFunctionCall(Function, MathSum, { PosDiff });
    UMaterialExpressionMaterialFunctionCall* Chebyshev = NewMaterialExpressionFunctionCall(Function, MathMaxValue, { NewMaterialExpressionAbs(Function, PosDiff) });

    UMaterialExpressionIf* ModeResult = NewMaterialExpressionSwitch(Function, Mode,
    {
        Simple0, Simple1, Cell, Step0, Step1, Step2, Mul, Add, Simple2, Simple3, Manhattan, Chebyshev
    });

    UMaterialExpressionIf* Result = NewMaterialExpressionIfEqual(Function, TurbulenceWeight, { 0.0f, 0.0f, 0.0f },
        ModeResult,
        NewMaterialExpressionSine(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionDotProduct(Function, Pos, TurbulenceWeight), ModeResult)));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_UE4TangentSpaceNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Interprets the color values as a vector in tangent space.");

    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TangentV = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionMaterialFunctionCall* DefaultNormal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialExpressionNormalize* UnclippedNormal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
            {
                NewMaterialExpressionMultiply(Function, TangentU, NewMaterialExpressionComponentMask(Function, Normal, 1)),
                NewMaterialExpressionMultiply(Function, TangentV, NewMaterialExpressionComponentMask(Function, Normal, 2)),
                NewMaterialExpressionMultiply(Function,
                    DefaultNormal,
                    NewMaterialExpressionComponentMask(Function, Normal, 4)
                )
            }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), UnclippedNormal);
}

static void Generator_UE4WorldSpaceNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    Function->Description = TEXT("Interprets the color values as a vector in world space.");

    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TangentV = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), Normal);
}

static void Generator_UE4TextureLookup(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // Do nothing for this function, because UE4 did in the Common.ush
    UMaterialExpressionFunctionInput* TextureSample = NewMaterialExpressionFunctionInput(Function, TEXT("texture_sample"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 1.0f });

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), TextureSample);
}

static void Generator_UE4PixelNormalWS(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionPixelNormalWS* Normal = NewMaterialExpression<UMaterialExpressionPixelNormalWS>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Normal);
}

static void Generator_UE4VertexNormalWS(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionVertexNormalWS* Normal = NewMaterialExpression<UMaterialExpressionVertexNormalWS>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Normal);
}

static void Generator_UE4Fresnel(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Exponent = NewMaterialExpressionFunctionInput(Function, TEXT("exponent"), EFunctionInputType::FunctionInput_Scalar, 5.0f);
    UMaterialExpressionFunctionInput* BaseReflectRraction = NewMaterialExpressionFunctionInput(Function, TEXT("base_reflect_fraction"), EFunctionInputType::FunctionInput_Scalar, 0.04f);
    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpression<UMaterialExpressionPixelNormalWS>(Function));

    UMaterialExpressionFresnel* Fresnel = NewMaterialExpressionFresnel(Function, Exponent, BaseReflectRraction, Normal);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Fresnel);
}

static void Generator_UE4FresnelFunction(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* NormalVector = NewMaterialExpressionFunctionInput(Function, TEXT("normal_vector"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpression<UMaterialExpressionVertexNormalWS>(Function));
    UMaterialExpressionFunctionInput* CameraVector = NewMaterialExpressionFunctionInput(Function, TEXT("camera_vector"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpression<UMaterialExpressionCameraVectorWS>(Function));
    UMaterialExpressionFunctionInput* InvertFresnel = NewMaterialExpressionFunctionInput(Function, TEXT("invert_fresnel"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* Power = NewMaterialExpressionFunctionInput(Function, TEXT("power"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* UseCheapContrast = NewMaterialExpressionFunctionInput(Function, TEXT("use_cheap_contrast"), EFunctionInputType::FunctionInput_Scalar, false);
    UMaterialExpressionFunctionInput* CheapContrastDark = NewMaterialExpressionFunctionInput(Function, TEXT("cheap_contrast_dark"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* CheapContrastBright = NewMaterialExpressionFunctionInput(Function, TEXT("cheap_contrast_bright"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* ClampFresnelDotProduct = NewMaterialExpressionFunctionInput(Function, TEXT("clamp_fresnel_dot_product"), EFunctionInputType::FunctionInput_Scalar, true);

    // UE4 fresnel function used the static bool input which can't be compatible with the scalar input, creating new fresnel function instead
    //UMaterialFunction* FresnelFunction = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02"), TEXT("Fresnel_Function.Fresnel_Function"));
    //UMaterialExpressionMaterialFunctionCall* FunctionCall = NewMaterialExpressionFunctionCall(Function, FresnelFunction, {NormalVector, CameraVector, InvertFresnel, Power, UseCheapContrast, CheapContrastDark, CheapContrastBright, ClampFresnelDotProduct});
    UMaterialExpressionDotProduct* NormalCameraDot = NewMaterialExpressionDotProduct(Function, NormalVector, CameraVector);
    UMaterialExpressionIf* ClampFresnelDotProductSwitch = NewMaterialExpressionStaticSwitch(Function, ClampFresnelDotProduct, NewMaterialExpressionClamp(Function, NormalCameraDot, 0.0f, 1.0f), NormalCameraDot);
    UMaterialExpressionIf* InvertFresnelSwitch = NewMaterialExpressionStaticSwitch(Function, InvertFresnel, ClampFresnelDotProductSwitch, NewMaterialExpressionOneMinus(Function, ClampFresnelDotProductSwitch));
    UMaterialExpressionIf* UseCheapContrastSwitch = NewMaterialExpressionStaticSwitch(Function, UseCheapContrast, NewMaterialExpressionLinearInterpolate(Function, CheapContrastDark, CheapContrastBright, InvertFresnelSwitch), NewMaterialExpressionPower(Function, InvertFresnelSwitch, Power));

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), UseCheapContrastSwitch);
}

static void Generator_UE4CameraVector(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionCameraVectorWS* CameraVector = NewMaterialExpression<UMaterialExpressionCameraVectorWS>(Function);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), CameraVector);
}

static void Generator_UE4PixelDepth(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionPixelDepth* PixelDepth = NewMaterialExpression<UMaterialExpressionPixelDepth>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), PixelDepth);
}

static void Generator_UE4VertexColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionVertexColor* VertexColor = NewMaterialExpression<UMaterialExpressionVertexColor>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, { VertexColor, 0 }, { VertexColor, 4 }));
}

static void Generator_UE4VertexColor2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* CoordinateIndex = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate_index"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    UMaterialExpressionVertexColor* VertexColor = NewMaterialExpression<UMaterialExpressionVertexColor>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, { VertexColor, 0 }, { VertexColor, 4 }));
}

static void Generator_UE4CameraPosition(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionCameraPositionWS* CameraPosition = NewMaterialExpression<UMaterialExpressionCameraPositionWS>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), CameraPosition);
}

static void Generator_UE4TransformTangentToWorld(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector3, {});
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TangentV = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionTransform* TransformedVector = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_Tangent, TRANSFORM_World);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), TransformedVector);
}

static void Generator_UE4TransformWorldToTangent(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* Vector = NewMaterialExpressionFunctionInput(Function, TEXT("vector"), EFunctionInputType::FunctionInput_Vector3, {});
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* TangentU = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TangentV = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionTransform* TransformedVector = NewMaterialExpressionTransform(Function, Vector, TRANSFORMSOURCE_World, TRANSFORM_Tangent);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), TransformedVector);
}

static void Generator_UE4EmissiveMultiplier(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // Emissive multiplier is always 1.0 in UE4
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionConstant(Function, 2560.0f));
}

static void Generator_UE4ReflectionVector(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionReflectionVectorWS(Function));
}

static void Generator_UE4DitherTemporalAA(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* DitherTemporalAAFunction = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("DitherTemporalAA.DitherTemporalAA"));

    UMaterialExpressionFunctionInput* AlphaThreshold = NewMaterialExpressionFunctionInput(Function, TEXT("AlphaThreshold"), EFunctionInputType::FunctionInput_Scalar, 0.5f);
    UMaterialExpressionFunctionInput* Random = NewMaterialExpressionFunctionInput(Function, TEXT("Random"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionMaterialFunctionCall* FunctionCall = NewMaterialExpressionFunctionCall(Function, DitherTemporalAAFunction, {AlphaThreshold, Random});

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), FunctionCall);
}

static void Generator_TexTextureIsValid(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // in UE4, a texture is always valid, have to check the texture size to decide if the texture is valid. We used 1x1 by default.
    UMaterialExpressionFunctionInput* Tex = NewMaterialExpressionFunctionInput(Function, TEXT("tex"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionTextureProperty* TexProperty = NewMaterialExpressionTextureProperty(Function, Tex, TMTM_TextureSize);
    UMaterialExpressionIf* CheckX = NewMaterialExpressionIfGreater(Function, NewMaterialExpressionComponentMask(Function, TexProperty, 1), 1.0f, 1.0f, 0.0f);
    UMaterialExpressionIf* CheckY = NewMaterialExpressionIfGreater(Function, NewMaterialExpressionComponentMask(Function, TexProperty, 2), 1.0f, 1.0f, 0.0f);

    UMaterialExpressionIf* Result = NewMaterialExpressionIfGreater(Function, NewMaterialExpressionAdd(Function, CheckX, CheckY), 0.0f, 1.0f, 0.0f);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_SceneDataLookupFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Name = NewMaterialExpressionFunctionInput(Function, TEXT("name"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* DefaultValue = NewMaterialExpressionFunctionInput(Function, TEXT("default_value"), EFunctionInputType::FunctionInput_Scalar, 0.0);

    // TODO: add more primvar
    //UMaterialExpressionIf* Data = NewMaterialExpressionSwitch(Function, Name, 
    //	{
    //		NewMaterialExpressionComponentMask(Function, NewMaterialExpression<UMaterialExpressionVertexColor>(Function), 7)
    //	});

    UMaterialExpressionVertexColor* VertexColor = NewMaterialExpression<UMaterialExpressionVertexColor>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfLess(Function, Name, 0.0, DefaultValue, { VertexColor, 4 }));
}

static void Generator_SceneDataLookupFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Name = NewMaterialExpressionFunctionInput(Function, TEXT("name"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* DefaultValue = NewMaterialExpressionFunctionInput(Function, TEXT("default_value"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });

    // TODO: add more primvar
    //UMaterialExpressionIf* Data = NewMaterialExpressionSwitch(Function, Name, 
    //	{
    //		NewMaterialExpressionComponentMask(Function, NewMaterialExpression<UMaterialExpressionVertexColor>(Function), 7)
    //	});

    UMaterialExpressionVertexColor* VertexColor = NewMaterialExpression<UMaterialExpressionVertexColor>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfLess(Function, Name, 0.0, DefaultValue, VertexColor));
}

static void Generator_SceneDataLookupFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Name = NewMaterialExpressionFunctionInput(Function, TEXT("name"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* DefaultValue = NewMaterialExpressionFunctionInput(Function, TEXT("default_value"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 0.0f });

    // TODO: add more primvar
    //UMaterialExpressionIf* Data = NewMaterialExpressionSwitch(Function, Name, 
    //	{
    //		NewMaterialExpression<UMaterialExpressionVertexColor>(Function)
    //	});

    UMaterialExpressionVertexColor* VertexColor = NewMaterialExpression<UMaterialExpressionVertexColor>(Function);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfLess(Function, Name, 0.0, DefaultValue, NewMaterialExpressionAppendVector(Function, { VertexColor, 0 }, { VertexColor, 4 })));
}

static void Generator_UE4Noise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* NoiseGradientTexture = NewMaterialExpressionFunctionInput(Function, TEXT("NoiseGradientTexture"), EFunctionInputType::FunctionInput_Texture2D);
    UMaterialExpressionFunctionInput* Noise3DTextrue = NewMaterialExpressionFunctionInput(Function, TEXT("Noise3DTextrue"), EFunctionInputType::FunctionInput_VolumeTexture);
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("Position"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("Scale"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Quality = NewMaterialExpressionFunctionInput(Function, TEXT("Quality"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* FunctionInput = NewMaterialExpressionFunctionInput(Function, TEXT("Function"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Turbulence = NewMaterialExpressionFunctionInput(Function, TEXT("Turbulence"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Levels = NewMaterialExpressionFunctionInput(Function, TEXT("Levels"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* OutputMin = NewMaterialExpressionFunctionInput(Function, TEXT("OutputMin"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* OutputMax = NewMaterialExpressionFunctionInput(Function, TEXT("OutputMax"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* LevelScale = NewMaterialExpressionFunctionInput(Function, TEXT("LevelScale"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* FilterWidth = NewMaterialExpressionFunctionInput(Function, TEXT("FilterWidth"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("Tiling"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* RepeatSize = NewMaterialExpressionFunctionInput(Function, TEXT("RepeatSize"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionNoise* Result = NewMaterialExpressionNoise(Function, Position, FilterWidth, 2);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionComponentMask(Function, Result, 1));
}

static void Generator_UE4VectorNoise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("Position"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Quality = NewMaterialExpressionFunctionInput(Function, TEXT("Quality"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* FunctionInput = NewMaterialExpressionFunctionInput(Function, TEXT("Function"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("Tiling"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TileSize = NewMaterialExpressionFunctionInput(Function, TEXT("TileSize"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionVectorNoise* Result = NewMaterialExpressionVectorNoise(Function, Position, VNF_GradientALU, 4);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionComponentMask(Function, Result, 7));
}

static void Generator_UE4Vector4Noise(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Position = NewMaterialExpressionFunctionInput(Function, TEXT("Position"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Quality = NewMaterialExpressionFunctionInput(Function, TEXT("Quality"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* FunctionInput = NewMaterialExpressionFunctionInput(Function, TEXT("Function"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Tiling = NewMaterialExpressionFunctionInput(Function, TEXT("Tiling"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* TileSize = NewMaterialExpressionFunctionInput(Function, TEXT("TileSize"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionVectorNoise* Result = NewMaterialExpressionVectorNoise(Function, Position, VNF_GradientALU, 4);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionComponentMask(Function, Result, 15));
}

static void Generator_UE4TranslucentGetTint(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* BaseColor = NewMaterialExpressionFunctionInput(Function, TEXT("base_color"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Opacity = NewMaterialExpressionFunctionInput(Function, TEXT("opacity"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), BaseColor);
}

static void Generator_UE4SubsurfaceWeight(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    NewMaterialExpressionFunctionOutput(Function, TEXT("weight"), 0.0f);
}

static void Generator_UE4SubsurfaceColor(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Color = NewMaterialExpressionFunctionInput(Function, TEXT("color"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    NewMaterialExpressionFunctionOutput(Function, TEXT("color"), Color);
}

static void Generator_UE4SubsurfaceOpacity(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Opacity = NewMaterialExpressionFunctionInput(Function, TEXT("opacity"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    NewMaterialExpressionFunctionOutput(Function, TEXT("opacity"), Opacity);
}

static void Generator_UE4Smoothstep(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Min = NewMaterialExpressionFunctionInput(Function, TEXT("min"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Max = NewMaterialExpressionFunctionInput(Function, TEXT("max"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Alpha = NewMaterialExpressionFunctionInput(Function, TEXT("alpha"), EFunctionInputType::FunctionInput_Scalar, 0.5f);

    UMaterialFunction* SmoothstepFunction = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02"), TEXT("SmoothStep.SmoothStep"));

    UMaterialExpressionMaterialFunctionCall* FunctionCall = NewMaterialExpressionFunctionCall(Function, SmoothstepFunction, { Alpha, Min, Max });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), FunctionCall);
}

static void Generator_UE4OpacityWeight(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // always return 1
    UMaterialExpressionFunctionInput* Opacity = NewMaterialExpressionFunctionInput(Function, TEXT("opacity"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    NewMaterialExpressionFunctionOutput(Function, TEXT("weight"), 1.0f);
}

static void Generator_ApertureRawFileTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif	
    
    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);

    UMaterialExpressionMaterialFunctionCall* FunctionCall = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, { Texture, NewMaterialExpressionComponentMask(Function, UVWPosition, 3), WrapU, WrapV, CropU, CropV, 0 });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), FunctionCall);
}

static void Generator_UE4BlackBody(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Temp = NewMaterialExpressionFunctionInput(Function, TEXT("temp"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionBlackBody* BlackBody = NewMaterialExpressionBlackBody(Function, Temp);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), BlackBody);
}

static void Generator_UE4PerInstanceRandom(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* NoiseGradientTexture = NewMaterialExpressionFunctionInput(Function, TEXT("NoiseGradientTexture"), EFunctionInputType::FunctionInput_Texture2D);
    UMaterialExpressionFunctionInput* NumInstances = NewMaterialExpressionFunctionInput(Function, TEXT("NumInstances"), EFunctionInputType::FunctionInput_Scalar, 0.0f);

    // NOTE: MaterialExpressionPerInstanceRandom header need to be changed, so using custom expression instead
    UMaterialExpressionCustom* PerInstanceRandom = NewMaterialExpression<UMaterialExpressionCustom>(Function);
    PerInstanceRandom->Description = TEXT("PerInstanceRandom");
    PerInstanceRandom->OutputType = ECustomMaterialOutputType::CMOT_Float1;
    PerInstanceRandom->Code = TEXT("return GetPerInstanceRandom(Parameters);");
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), PerInstanceRandom);
}

static void Generator_UE4ConvertToLH(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Vec3 = NewMaterialExpressionFunctionInput(Function, TEXT("vec3"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    UMaterialExpressionFunctionInput* IsPosition = NewMaterialExpressionFunctionInput(Function, TEXT("is_position"), EFunctionInputType::FunctionInput_Scalar, false);

    // Do nothing to convert LH
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Vec3);
}

static void Generator_UE4UnpackNormalmap(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* TextureSample = NewMaterialExpressionFunctionInput(Function, TEXT("texture_sample"), EFunctionInputType::FunctionInput_Vector4, { 0.0f, 0.0f, 0.0f, 1.0f });

    UMaterialExpressionComponentMask* MaskRG = NewMaterialExpressionComponentMask(Function, TextureSample, 3);
    UMaterialExpressionSubtract* UnpackNormal = NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function, MaskRG, 2.0f), 1.f);
    UMaterialExpressionSquareRoot* Z = NewMaterialExpressionSquareRoot(Function, NewMaterialExpressionSaturate(Function, NewMaterialExpressionOneMinus(Function, NewMaterialExpressionDotProduct(Function, UnpackNormal, UnpackNormal))));
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionAppendVector(Function, NewMaterialExpressionAppendVector(Function, UnpackNormal, Z), 1.0f));
}

static void Generator_UE4UnpackIntegerAsFloat(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* N = NewMaterialExpressionFunctionInput(Function, TEXT("n"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialFunction* UnpackIntegerAsFloatFunction = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02"), TEXT("ms_PivotPainter2_UnpackIntegerAsFloat.ms_PivotPainter2_UnpackIntegerAsFloat"));
    UMaterialExpressionMaterialFunctionCall* FunctionCall = NewMaterialExpressionFunctionCall(Function, UnpackIntegerAsFloatFunction, { N });
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), FunctionCall);
}

static void Generator_UE4LocalObjectBoundsMin(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionLocalObjectBoundsMin(Function));
}

static void Generator_UE4LocalObjectBoundsMax(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionLocalObjectBoundsMax(Function));
}

static void Generator_UE4ObjectBounds(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionObjectBounds(Function));
}

static void Generator_UE4ObjectRadius(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionObjectRadius(Function));
}

static void Generator_UE4ObjectWorldPosition(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* UpZ = NewMaterialExpressionFunctionInput(Function, TEXT("up_z"), EFunctionInputType::FunctionInput_Scalar, true);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionObjectPositionWS(Function));
}

static void Generator_UE4ObjectOrientation(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionObjectOrientation(Function));
}

//------------------------------ OmniSurface ------------------------------

static void Generator_OmniImageNativeWrapMode(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_black,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_file,
        image_wrap_missing,
    };

    UMaterialExpressionFunctionInput* Wrap = NewMaterialExpressionFunctionInput(Function, TEXT("wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);

    UMaterialExpressionIf* NativeWrap = NewMaterialExpressionSwitch(Function, Wrap, 
        {
            wrap_repeat,
            wrap_clip,
            wrap_clamp,
            wrap_mirrored_repeat,
            wrap_clip, //TODO
            wrap_clip //TODO
        }
    );

    NewMaterialExpressionFunctionOutput(Function, TEXT("native_wrap"), NativeWrap);
}

static void Generator_OmniImageComputeLatitudeLongitudeProjection(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* Pos = NewMaterialExpressionFunctionInput(Function, TEXT("pos"), EFunctionInputType::FunctionInput_Vector3, { 0,0,0 });
    UMaterialExpressionFunctionInput* Rotation = NewMaterialExpressionFunctionInput(Function, TEXT("rotation"), EFunctionInputType::FunctionInput_Vector3, { 0,0,0 });

    UMaterialExpressionNormalize* NormalizedPos = NewMaterialExpressionNormalize(Function, Pos);
    UMaterialExpressionComponentMask* X = NewMaterialExpressionComponentMask(Function, NormalizedPos, 1);
    UMaterialExpressionComponentMask* Y = NewMaterialExpressionComponentMask(Function, NormalizedPos, 2);
    UMaterialExpressionComponentMask* Z = NewMaterialExpressionComponentMask(Function, NormalizedPos, 4);

    UMaterialExpressionArccosine* ArcNegOne = NewMaterialExpressionArccosine(Function, { -1.0f });

    UMaterialExpressionMultiply* ThetaX = NewMaterialExpressionMultiply(Function, {
        NewMaterialExpressionComponentMask(Function, Rotation, 1),
        ArcNegOne,
        2.0f });

    UMaterialExpressionAdd* RotateX_Y = NewMaterialExpressionAdd(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionCosine(Function, ThetaX), Y),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSine(Function, ThetaX), Z));

    UMaterialExpressionSubtract* RotateX_Z = NewMaterialExpressionSubtract(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionCosine(Function, ThetaX), Z),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSine(Function, ThetaX), Y));

    UMaterialExpressionMultiply* ThetaY = NewMaterialExpressionMultiply(Function, {
        NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, Rotation, 2), -0.25f),
        ArcNegOne,
        2.0f });

    UMaterialExpressionAdd* RotateY_X = NewMaterialExpressionAdd(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionCosine(Function, ThetaY), X),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSine(Function, ThetaY), RotateX_Y));

    UMaterialExpressionSubtract* RotateY_Y = NewMaterialExpressionSubtract(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionCosine(Function, ThetaY), RotateX_Y),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSine(Function, ThetaY), X));

    UMaterialExpressionMultiply* ThetaZ = NewMaterialExpressionMultiply(Function, {
        NewMaterialExpressionComponentMask(Function, Rotation, 4),
        ArcNegOne,
        2.0f });

    UMaterialExpressionSubtract* RotateZ_X = NewMaterialExpressionNegate(Function, NewMaterialExpressionAdd(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionCosine(Function, ThetaZ), RotateY_X),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSine(Function, ThetaZ), RotateX_Z)));

    UMaterialExpressionSubtract* RotateZ_Z = NewMaterialExpressionSubtract(Function,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionCosine(Function, ThetaZ), RotateX_Z),
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSine(Function, ThetaZ), RotateY_X));

    //UMaterialExpressionAbs* AbsX = NewMaterialExpressionAbs(Function, RotateZ_X);
    //UMaterialExpressionAbs* AbsY = NewMaterialExpressionAbs(Function, RotateY_Y);
    //UMaterialExpressionAbs* AbsZ = NewMaterialExpressionAbs(Function, RotateZ_Z);

    //UMaterialExpressionMaterialFunctionCall* Seg_0 = NewMaterialExpressionFunctionCall(Function, MakeFloat2, 
    //	{
    //		NewMaterialExpressionIfGreater(Function, RotateZ_Z, 0.0f, {2.0}, {0.0}),
    //		0.0
    //	});

    //UMaterialExpressionMaterialFunctionCall* UV_0 = NewMaterialExpressionFunctionCall(Function, MakeFloat2,
    //	{
    //		NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionNegate(Function, RotateZ_X), RotateZ_Z), 0.5f), 0.5f),
    //		NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionNegate(Function, RotateY_Y), AbsZ), 0.5f), 0.5f)
    //	});

    //UMaterialExpressionIf* UV_1 = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionIfLess(Function, AbsX, AbsY, 1.0f, 0.0f), NewMaterialExpressionIfLess(Function, AbsZ, AbsY, 1.0f, 0.0f)), 1.0f,
    //	NewMaterialExpressionFunctionCall(Function, MakeFloat2,
    //		{
    //			NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionNegate(Function, RotateZ_X), RotateY_Y), 0.5f), 0.5f),
    //			NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, RotateZ_Z, AbsY), 0.5f), 0.5f)
    //		})
    //	, UV_0);

    //UMaterialExpressionIf* Seg_1 = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionIfLess(Function, AbsX, AbsY, 1.0f, 0.0f), NewMaterialExpressionIfLess(Function, AbsZ, AbsY, 1.0f, 0.0f)), 1.0f,
    //	NewMaterialExpressionFunctionCall(Function, MakeFloat2,
    //		{
    //			NewMaterialExpressionIfGreater(Function, RotateY_Y, 0.0f, {0.0}, {2.0}),
    //			1.0f
    //		})
    //	, Seg_0);

    //UMaterialExpressionIf* UV_2 = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionIfGreater(Function, AbsX, AbsY, 1.0f, 0.0f), NewMaterialExpressionIfLess(Function, AbsZ, AbsX, 1.0f, 0.0f)), 1.0f,
    //	NewMaterialExpressionFunctionCall(Function, MakeFloat2,
    //		{
    //			NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionIfGreater(Function, RotateZ_X, 0.0, RotateZ_Z, NewMaterialExpressionNegate(Function, RotateY_Y)), AbsX), 0.5f), 0.5f),
    //			NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionIfGreater(Function, RotateZ_X, 0.0, NewMaterialExpressionNegate(Function, RotateY_Y), RotateZ_Z), AbsX), 0.5f), 0.5f)
    //		})
    //	, UV_1);

    //UMaterialExpressionIf* Seg_2 = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionIfGreater(Function, AbsX, AbsY, 1.0f, 0.0f), NewMaterialExpressionIfLess(Function, AbsZ, AbsY, 1.0f, 0.0f)), 1.0f,
    //	NewMaterialExpressionFunctionCall(Function, MakeFloat2,
    //		{
    //			1.0f,
    //			NewMaterialExpressionIfGreater(Function, RotateZ_X, 0.0f, {0.0}, {1.0})
    //		})
    //	, Seg_1);

    //UMaterialExpressionAdd* UV_3 = NewMaterialExpressionAdd(Function, 
    //	NewMaterialExpressionDivide(Function, NewMaterialExpressionMultiply(Function, 
    //		NewMaterialExpressionArctangent(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function, UV_2, 2.0f), 1.0f), 0.997f)), 2.0f), ArcNegOne), 0.5f);

    UMaterialExpressionNormalize* NormalizedXYZ = NewMaterialExpressionNormalize(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat3, { RotateZ_X , NewMaterialExpressionNegate(Function, RotateZ_Z), RotateY_Y }));
    UMaterialExpressionComponentMask* NormalizedX = NewMaterialExpressionComponentMask(Function, NormalizedXYZ, 1);
    UMaterialExpressionComponentMask* NormalizedY = NewMaterialExpressionComponentMask(Function, NormalizedXYZ, 2);
    UMaterialExpressionComponentMask* NormalizedZ = NewMaterialExpressionComponentMask(Function, NormalizedXYZ, 4);

    UMaterialExpressionAdd* FinalUV = NewMaterialExpressionAdd(Function, NewMaterialExpressionFunctionCall(Function, MakeFloat2, {
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionDivide(Function, NewMaterialExpressionArctangent2(Function, NormalizedX, NewMaterialExpressionNegate(Function, NormalizedZ)), ArcNegOne), 0.5f),
        NewMaterialExpressionDivide(Function, NewMaterialExpressionArcsine(Function, NewMaterialExpressionNegate(Function, NormalizedY)), ArcNegOne)
        }), 0.5f);

    NewMaterialExpressionFunctionOutput(Function, TEXT("projection"), NewMaterialExpressionAppendVector(Function, FinalUV, RotateZ_Z));
}

static void Generator_OmniImageComputeTextureCoordinate(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    // NOTE: this is different from the projection mode in base module
    enum projection_mode
    {
        projection_invalid,
        projection_cubic,
        projection_spherical,
        projection_cylindrical,
        projection_infinite_cylindrical,
        projection_planar,
        projection_spherical_normalized,
        projection_cylindrical_normalized,
        projection_infinite_cylindrical_normalized,
        projection_tri_planar,
        projection_latitude_longitude,
    };

    UMaterialFunction* BaseCoordinateSource = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_source"));
    UMaterialFunction* BaseCoordinateProjection = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_coordinate_projection"));
    UMaterialFunction* BaseRotationTranslationScale = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_rotation_translation_scale"));
    UMaterialFunction* MathMultiplyFloat4x4Float4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_multiply_float4x4_float4"));
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));
    UMaterialFunction* MakeFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4"));
    UMaterialFunction* BreakFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("BreakFloat3Components.BreakFloat3Components"));
    UMaterialFunction* BreakFloat4 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("BreakFloat4Components.BreakFloat4Components"));
    UMaterialFunction* ComputeProjection = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniSurface_OmniImage_compute_latitude_longitude_projection"));

    UMaterialExpressionFunctionInput* CoordinateSystem = NewMaterialExpressionFunctionInput(Function, TEXT("coordinate_system"), EFunctionInputType::FunctionInput_Scalar, texture_coordinate_world);
    UMaterialExpressionFunctionInput* UVSet = NewMaterialExpressionFunctionInput(Function, TEXT("uv_set"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* SOffset = NewMaterialExpressionFunctionInput(Function, TEXT("s_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TOffset = NewMaterialExpressionFunctionInput(Function, TEXT("t_offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* SScale = NewMaterialExpressionFunctionInput(Function, TEXT("s_scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* TScale = NewMaterialExpressionFunctionInput(Function, TEXT("t_scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* SFlip = NewMaterialExpressionFunctionInput(Function, TEXT("s_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TFlip = NewMaterialExpressionFunctionInput(Function, TEXT("t_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* SwapST = NewMaterialExpressionFunctionInput(Function, TEXT("swap_st"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* ExtendedProjectionMode = NewMaterialExpressionFunctionInput(Function, TEXT("extended_projection_mode"), EFunctionInputType::FunctionInput_Scalar, projection_cubic);
    UMaterialExpressionFunctionInput* ProjectionTranslate = NewMaterialExpressionFunctionInput(Function, TEXT("projection_translate"), EFunctionInputType::FunctionInput_Vector3, { 0,0,0 });
    UMaterialExpressionFunctionInput* ProjectionRotate = NewMaterialExpressionFunctionInput(Function, TEXT("projection_rotate"), EFunctionInputType::FunctionInput_Vector3, { 0,0,0 });
    UMaterialExpressionFunctionInput* ProjectionScale = NewMaterialExpressionFunctionInput(Function, TEXT("projection_scale"), EFunctionInputType::FunctionInput_Vector3, { 1.0f,1.0f,1.0f });

    UMaterialExpressionMaterialFunctionCall* CoordinateSourceUVW = NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource, { texture_coordinate_uvw, UVSet });

    UMaterialExpressionMaterialFunctionCall* CoordinateSource =
        NewMaterialExpressionFunctionCall(Function, BaseCoordinateSource, { CoordinateSystem, UVSet });

    UMaterialExpressionMaterialFunctionCall* LatLongProjectPosition = NewMaterialExpressionFunctionCall(Function, ComputeProjection, { { CoordinateSource, 0 }, ProjectionRotate });

    UMaterialExpressionMaterialFunctionCall* RTS = NewMaterialExpressionFunctionCall(Function, BaseRotationTranslationScale, { ProjectionRotate, ProjectionTranslate, ProjectionScale });
    UMaterialExpressionIf* NativeProjectionMode = NewMaterialExpressionIfEqual(Function, ExtendedProjectionMode, projection_latitude_longitude, projection_planar, ExtendedProjectionMode);
    UMaterialExpressionMaterialFunctionCall* CoordinateProjection = NewMaterialExpressionFunctionCall(Function, BaseCoordinateProjection, { CoordinateSystem, UVSet, NativeProjectionMode,
            {RTS, 0},
            {RTS, 1},
            {RTS, 2},
            {RTS, 3} });

    UMaterialExpressionIf* ProjectionInfoPosition = NewMaterialExpressionIfEqual(Function, ExtendedProjectionMode, projection_latitude_longitude,
        LatLongProjectPosition,
        { CoordinateProjection, 0 });
    UMaterialExpressionIf* ProjectionInfoTangentU = NewMaterialExpressionIfEqual(Function, ExtendedProjectionMode, projection_latitude_longitude,
        { CoordinateSource, 1 },
        { CoordinateProjection, 1 });
    UMaterialExpressionIf* ProjectionInfoTangentV = NewMaterialExpressionIfEqual(Function, ExtendedProjectionMode, projection_latitude_longitude,
        { CoordinateSource, 2 },
        { CoordinateProjection, 2 });

    UMaterialExpressionIf* TextureCoordInfoPosition = NewMaterialExpressionIfEqual(Function, CoordinateSystem, texture_coordinate_uvw,
        { CoordinateSourceUVW, 0 },
        ProjectionInfoPosition);
    UMaterialExpressionIf* TextureCoordInfoTangentU = NewMaterialExpressionIfEqual(Function, CoordinateSystem, texture_coordinate_uvw,
        { CoordinateSourceUVW, 1 },
        ProjectionInfoTangentU);
    UMaterialExpressionIf* TextureCoordInfoTangentV = NewMaterialExpressionIfEqual(Function, CoordinateSystem, texture_coordinate_uvw,
        { CoordinateSourceUVW, 2 },
        ProjectionInfoTangentV);

    UMaterialExpressionMaterialFunctionCall* PositionBreak = NewMaterialExpressionFunctionCall(Function, BreakFloat3, { TextureCoordInfoPosition });

    UMaterialExpressionMaterialFunctionCall* InPos = NewMaterialExpressionFunctionCall(Function, MakeFloat4, 
        {
        NewMaterialExpressionStaticSwitch(Function, SwapST, 
            NewMaterialExpressionAdd(Function, {PositionBreak, 1}, SOffset),
            NewMaterialExpressionAdd(Function, {PositionBreak, 0}, SOffset)),
        NewMaterialExpressionStaticSwitch(Function, SwapST, 
            NewMaterialExpressionAdd(Function, {PositionBreak, 0}, TOffset),
            NewMaterialExpressionAdd(Function, {PositionBreak, 1}, TOffset)),
        {PositionBreak, 2},
        1.0f
        });

    UMaterialExpressionMaterialFunctionCall* TransformPosition = NewMaterialExpressionFunctionCall(Function, MathMultiplyFloat4x4Float4, {
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ SScale, 0.0f, 0.0f, 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 0.0f, TScale, 0.0f, 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 0.0f, 0.0f, 1.0f, 0.0f }),
        NewMaterialExpressionFunctionCall(Function, MakeFloat4,{ 0.0f, 0.0f, 0.0f, 1.0f }),
        InPos });

    UMaterialExpressionMaterialFunctionCall* TransformPositionBreak = NewMaterialExpressionFunctionCall(Function, BreakFloat4, { TransformPosition });

    UMaterialExpressionMaterialFunctionCall* FinalPosition = NewMaterialExpressionFunctionCall(Function, MakeFloat3, 
        {
            NewMaterialExpressionStaticSwitch(Function, SFlip, NewMaterialExpressionMultiply(Function, {TransformPositionBreak, 0}, -1.0f), {TransformPositionBreak, 0}),
            NewMaterialExpressionStaticSwitch(Function, TFlip, NewMaterialExpressionMultiply(Function, {TransformPositionBreak, 1}, -1.0f), {TransformPositionBreak, 1}),
            {TransformPositionBreak, 2}
        }
    );

    NewMaterialExpressionFunctionOutput(Function, TEXT("position"), FinalPosition);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_u"), TextureCoordInfoTangentU);
    NewMaterialExpressionFunctionOutput(Function, TEXT("tangent_v"), TextureCoordInfoTangentV);
}

static void Generator_OmniImageTextureLookup(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_black,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_file,
        image_wrap_missing,
    };

    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));
    UMaterialFunction* NativeWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImage_native_wrap_mode"));
    UMaterialFunction* TextureIsValid = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_texture_isvalid"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* IgnoreMissingTexture = NewMaterialExpressionFunctionInput(Function, TEXT("ignore_missing_texture"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MissingColor = NewMaterialExpressionFunctionInput(Function, TEXT("missing_color"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* UseUVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("use_uv_coords"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("uv_coords"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* SWrap = NewMaterialExpressionFunctionInput(Function, TEXT("s_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TWrap = NewMaterialExpressionFunctionInput(Function, TEXT("t_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TextureCoordinateInfoPosition = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionAppendVector* Error = NewMaterialExpressionAppendVector(Function, MissingColor, 0.0f);

    UMaterialExpressionMaterialFunctionCall* TextureLookup = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, 
        { 
            Texture,
            NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 3),
            NewMaterialExpressionFunctionCall(Function, NativeWrapMode, {SWrap}),
            NewMaterialExpressionFunctionCall(Function, NativeWrapMode, {TWrap}),
            NewMaterialExpressionConstant(Function, 0.0f, 1.0f),
            NewMaterialExpressionConstant(Function, 0.0f, 1.0f),
            0.0f 
        });

    UMaterialExpressionIf* PositionX = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 1),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 1));

    UMaterialExpressionIf* PositionY = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 2),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 2));

    UMaterialExpressionIf* PositionXCheck = NewMaterialExpressionIf(Function, PositionX, 0.0f, Error, TextureLookup, NewMaterialExpressionIf(Function, PositionX, 1.0f, TextureLookup, TextureLookup, Error));
    UMaterialExpressionIf* PositionYCheck = NewMaterialExpressionIf(Function, PositionY, 0.0f, Error, TextureLookup, NewMaterialExpressionIf(Function, PositionY, 1.0f, TextureLookup, TextureLookup, Error));

    UMaterialExpressionIf* WrapMissingCheck = NewMaterialExpressionIfEqual(Function, SWrap, image_wrap_missing, PositionXCheck,
        NewMaterialExpressionIfEqual(Function, TWrap, image_wrap_missing, PositionYCheck, TextureLookup));

    UMaterialExpressionIf* Result = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionMultiply(Function, IgnoreMissingTexture, NewMaterialExpressionOneMinus(Function, NewMaterialExpressionFunctionCall(Function, TextureIsValid, { Texture }))), 1.0f,
        Error, WrapMissingCheck);

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), NewMaterialExpressionComponentMask(Function, Result, 7));
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), NewMaterialExpressionComponentMask(Function, Result, 8));
}

static void Generator_OmniImageTextureLookupNN(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* TexSize = NewMaterialExpressionFunctionInput(Function, TEXT("tex_size"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coords"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionDivide* Quant = NewMaterialExpressionDivide(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionFloor(Function, NewMaterialExpressionMultiply(Function, Coord, TexSize)), 0.5f), TexSize);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Quant);
}

static void Generator_OmniImageTextureLookupWrap(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_missing,
    };

    UMaterialExpressionFunctionInput* Wrap = NewMaterialExpressionFunctionInput(Function, TEXT("Wrap"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("Coord"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionMultiply* FracCoord = NewMaterialExpressionMultiply(Function, NewMaterialExpressionFrac(Function, NewMaterialExpressionMultiply(Function, Coord, 0.5f)), 2.0);

    UMaterialExpressionIf* Switch = NewMaterialExpressionSwitch(Function, Wrap, 
        {
            NewMaterialExpressionFrac(Function, Coord),
            NewMaterialExpressionClamp(Function, Coord, 0.0f, 0.999999f),
            NewMaterialExpressionIfLess(Function, FracCoord, 1.0, FracCoord, NewMaterialExpressionClamp(Function, NewMaterialExpressionSubtract(Function, 2.0f, FracCoord), 0.0, 0.999999)),
            Coord
        });
    NewMaterialExpressionFunctionOutput(Function, TEXT("OutCoord"), Switch); 
}

static void Generator_OmniImageTextureLookup2Smooth(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_missing,
    };

    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));
    UMaterialFunction* NativeWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImage_native_wrap_mode"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* MissingColor = NewMaterialExpressionFunctionInput(Function, TEXT("missing_color"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* SWrap = NewMaterialExpressionFunctionInput(Function, TEXT("s_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TWrap = NewMaterialExpressionFunctionInput(Function, TEXT("t_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TextureCoordinateInfoPosition = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionAppendVector* Error = NewMaterialExpressionAppendVector(Function, MissingColor, 0.0f);
    UMaterialExpressionConstant3Vector* ErrorNormal = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f);

    UMaterialExpressionMaterialFunctionCall* TextureLookup = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, 
        { 
            Texture,
            NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 3),
            NewMaterialExpressionFunctionCall(Function, NativeWrapMode, {SWrap}),
            NewMaterialExpressionFunctionCall(Function, NativeWrapMode, {TWrap}),
            NewMaterialExpressionConstant(Function, 0.0f, 1.0f),
            NewMaterialExpressionConstant(Function, 0.0f, 1.0f),
            0.0f 
        });

    UMaterialExpressionComponentMask* PositionX = NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 1);
    UMaterialExpressionComponentMask* PositionY = NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 2);

    UMaterialExpressionIf* PositionXCheck = NewMaterialExpressionIf(Function, PositionX, 0.0f, Error, TextureLookup, NewMaterialExpressionIf(Function, PositionX, 1.0f, TextureLookup, TextureLookup, Error));
    UMaterialExpressionIf* PositionYCheck = NewMaterialExpressionIf(Function, PositionY, 0.0f, Error, TextureLookup, NewMaterialExpressionIf(Function, PositionY, 1.0f, TextureLookup, TextureLookup, Error));

    UMaterialExpressionIf* WrapMissingCheck = NewMaterialExpressionIfEqual(Function, SWrap, image_wrap_missing, PositionXCheck,
        NewMaterialExpressionIfEqual(Function, TWrap, image_wrap_missing, PositionYCheck, TextureLookup));
    
    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), NewMaterialExpressionComponentMask(Function, WrapMissingCheck, 7));
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), NewMaterialExpressionComponentMask(Function, WrapMissingCheck, 8));
    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), ErrorNormal);
}

static void Generator_OmniImageTextureLookup2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_missing,
    };

    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));
    UMaterialFunction* NativeWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImage_native_wrap_mode"));
    UMaterialFunction* TextureIsValid = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_texture_isvalid"));
    UMaterialFunction* LookupSmooth = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImageNN_texture_lookup_2_smooth"));
    UMaterialFunction* LookupNN = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImageNN_texture_lookup_nn"));
    UMaterialFunction* LookupWrap = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImageNN_texture_lookup_wrap"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* IgnoreMissingTexture = NewMaterialExpressionFunctionInput(Function, TEXT("ignore_missing_texture"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MissingColor = NewMaterialExpressionFunctionInput(Function, TEXT("missing_color"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* UseUVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("use_uv_coords"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("uv_coords"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* SWrap = NewMaterialExpressionFunctionInput(Function, TEXT("s_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TWrap = NewMaterialExpressionFunctionInput(Function, TEXT("t_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* FilteredTextures = NewMaterialExpressionFunctionInput(Function, TEXT("filtered_textures"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TextureCoordinateInfoPosition = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionAppendVector* Error = NewMaterialExpressionAppendVector(Function, MissingColor, 0.0f);
    UMaterialExpressionConstant3Vector* ErrorNormal = NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f);

    UMaterialExpressionIf* PositionX = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 1),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 1));

    UMaterialExpressionIf* PositionY = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 2),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 2));

    UMaterialExpressionTextureProperty* TexProperty = NewMaterialExpressionTextureProperty(Function, Texture, TMTM_TextureSize);

    UMaterialExpressionMaterialFunctionCall* TextureLookupSmooth = NewMaterialExpressionFunctionCall(Function, LookupSmooth,
        {
            Texture,
            MissingColor,
            SWrap,
            TWrap,
            NewMaterialExpressionStaticSwitch(Function, UseUVCoords, NewMaterialExpressionAppendVector(Function, UVCoords, NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 4)), TextureCoordinateInfoPosition),
            TextureCoordinateInfoTangentV
        });

    UMaterialExpressionAppendVector* LookupSmooth4 = NewMaterialExpressionAppendVector(Function, {TextureLookupSmooth, 0}, {TextureLookupSmooth, 1});

    UMaterialExpressionMaterialFunctionCall* TextureLookupWrapX = NewMaterialExpressionFunctionCall(Function, LookupWrap, {SWrap, PositionX});
    UMaterialExpressionMaterialFunctionCall* TextureLookupWrapY = NewMaterialExpressionFunctionCall(Function, LookupWrap, {TWrap, PositionY});
    
    UMaterialExpressionMaterialFunctionCall* PositionNN = NewMaterialExpressionFunctionCall(Function, LookupNN, {TexProperty, NewMaterialExpressionFunctionCall(Function, MakeFloat2, {TextureLookupWrapX, TextureLookupWrapY})});
    UMaterialExpressionCustom* TexelLookup = NewMaterialExpressionTexelLookup(Function, Texture, NewMaterialExpressionAppendVector(Function, NewMaterialExpressionMultiply(Function, PositionNN, TexProperty), 0));

    UMaterialExpressionIf* PositionXCheck = NewMaterialExpressionIf(Function, TextureLookupWrapX, 0.0f, Error, TexelLookup, NewMaterialExpressionIf(Function, TextureLookupWrapX, 1.0f, TexelLookup, Error, Error));
    UMaterialExpressionIf* PositionYCheck = NewMaterialExpressionIf(Function, TextureLookupWrapY, 0.0f, Error, PositionXCheck, NewMaterialExpressionIf(Function, TextureLookupWrapY, 1.0f, PositionXCheck, Error, Error));

    UMaterialExpressionIf* Result = NewMaterialExpressionIfEqual(Function, FilteredTextures, 1.0, LookupSmooth4, PositionYCheck, true);

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), NewMaterialExpressionComponentMask(Function, Result, 7));
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), NewMaterialExpressionComponentMask(Function, Result, 8));
    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), ErrorNormal);
}

static void Generator_OmniImageNormalLookup2(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_missing,
    };

    UMaterialFunction* TextureIsValid = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_texture_isvalid"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* NativeWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImage_native_wrap_mode"));
    UMaterialFunction* TexLookupFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float3_2d"));
    UMaterialFunction* LookupNN = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImageNN_texture_lookup_nn"));
    UMaterialFunction* LookupWrap = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImageNN_texture_lookup_wrap"));
    UMaterialFunction* MakeFloat2 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultNormal")), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* IgnoreMissingTexture = NewMaterialExpressionFunctionInput(Function, TEXT("ignore_missing_texture"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* MissingColor = NewMaterialExpressionFunctionInput(Function, TEXT("missing_color"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* UseUVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("use_uv_coords"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("uv_coords"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* SWrap = NewMaterialExpressionFunctionInput(Function, TEXT("s_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TWrap = NewMaterialExpressionFunctionInput(Function, TEXT("t_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* SFlip = NewMaterialExpressionFunctionInput(Function, TEXT("s_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TFlip = NewMaterialExpressionFunctionInput(Function, TEXT("t_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFactor = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipRChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_r_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipGChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_g_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipBChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_b_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* FilteredTextures = NewMaterialExpressionFunctionInput(Function, TEXT("filtered_textures"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TextureCoordinateInfoPosition = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionAppendVector* ErrorColor = NewMaterialExpressionAppendVector(Function, MissingColor, 0.0f);
    UMaterialExpressionMaterialFunctionCall* DefaultNormal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialExpressionIf* PositionX = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 1),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 1));

    UMaterialExpressionIf* PositionY = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 2),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 2));

    UMaterialExpressionTextureProperty* TexProperty = NewMaterialExpressionTextureProperty(Function, Texture, TMTM_TextureSize);
    UMaterialExpressionMaterialFunctionCall* TextureLookupWrapX = NewMaterialExpressionFunctionCall(Function, LookupWrap, {SWrap, PositionX});
    UMaterialExpressionMaterialFunctionCall* TextureLookupWrapY = NewMaterialExpressionFunctionCall(Function, LookupWrap, {TWrap, PositionY});
    
    UMaterialExpressionMaterialFunctionCall* PositionNN = NewMaterialExpressionFunctionCall(Function, LookupNN, {TexProperty, NewMaterialExpressionFunctionCall(Function, MakeFloat2, {TextureLookupWrapX, TextureLookupWrapY})});

    UMaterialExpressionCustom* NormalTexelLookup = NewMaterialExpressionNormalTexelLookup(Function, Texture, NewMaterialExpressionAppendVector(Function, NewMaterialExpressionMultiply(Function, PositionNN, TexProperty), 0));

    UMaterialExpressionMultiply* FlipNormalX = NewMaterialExpressionMultiply(Function, NewMaterialExpressionStaticSwitch(Function, TangentSpaceFlipRChannel, NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, NormalTexelLookup, 1), -1.0f), NewMaterialExpressionComponentMask(Function, NormalTexelLookup, 1)), TangentSpaceFactor);
    UMaterialExpressionMultiply* FlipNormalY = NewMaterialExpressionMultiply(Function, NewMaterialExpressionStaticSwitch(Function, TangentSpaceFlipGChannel, NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, NormalTexelLookup, 2), -1.0f), NewMaterialExpressionComponentMask(Function, NormalTexelLookup, 2)), TangentSpaceFactor);
    UMaterialExpressionMultiply* FlipNormalZ = NewMaterialExpressionMultiply(Function, NewMaterialExpressionStaticSwitch(Function, TangentSpaceFlipBChannel, NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, NormalTexelLookup, 4), -1.0f), NewMaterialExpressionComponentMask(Function, NormalTexelLookup, 4)), TangentSpaceFactor);

    UMaterialExpressionNormalize* Normal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
                {
                    NewMaterialExpressionMultiply(Function, 
                        NewMaterialExpressionStaticSwitch(Function, SFlip, NewMaterialExpressionMultiply(Function, TextureCoordinateInfoTangentU, -1.0f), TextureCoordinateInfoTangentU), FlipNormalX),
                    NewMaterialExpressionNegate(Function, NewMaterialExpressionMultiply(Function,
                        NewMaterialExpressionStaticSwitch(Function, TFlip, NewMaterialExpressionMultiply(Function, TextureCoordinateInfoTangentV, -1.0f), TextureCoordinateInfoTangentV), FlipNormalY)),
                    NewMaterialExpressionMultiply(Function,
                        DefaultNormal, NewMaterialExpressionAdd(Function,
                            FlipNormalZ,
                            NewMaterialExpressionOneMinus(Function, TangentSpaceFactor)))
                }));

    UMaterialExpressionIf* PositionXCheck = NewMaterialExpressionIf(Function, TextureLookupWrapX, 0.0f, DefaultNormal, Normal, NewMaterialExpressionIf(Function, TextureLookupWrapX, 1.0f, Normal, DefaultNormal, DefaultNormal));
    UMaterialExpressionIf* PositionYCheck = NewMaterialExpressionIf(Function, TextureLookupWrapY, 0.0f, DefaultNormal, PositionXCheck, NewMaterialExpressionIf(Function, TextureLookupWrapY, 1.0f, PositionXCheck, DefaultNormal, DefaultNormal));

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), NewMaterialExpressionComponentMask(Function, ErrorColor, 7));
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), NewMaterialExpressionComponentMask(Function, ErrorColor, 8));
    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), PositionYCheck);
}

static void Generator_OmniImageTangentSpaceNormalLookup(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_black,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_file,
        image_wrap_missing,
    };

    UMaterialFunction* TextureIsValid = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_texture_isvalid"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* NativeWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniImage_native_wrap_mode"));
    UMaterialFunction* TexLookupFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float3_2d"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultNormal")), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* IgnoreMissingTexture = NewMaterialExpressionFunctionInput(Function, TEXT("ignore_missing_texture"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UseUVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("use_uv_coords"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("uv_coords"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* SWrap = NewMaterialExpressionFunctionInput(Function, TEXT("s_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TWrap = NewMaterialExpressionFunctionInput(Function, TEXT("t_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* SFlip = NewMaterialExpressionFunctionInput(Function, TEXT("s_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TFlip = NewMaterialExpressionFunctionInput(Function, TEXT("t_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFactor = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipRChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_r_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipGChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_g_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipBChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_b_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TextureCoordinateInfoPosition = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionMaterialFunctionCall* DefaultNormal = NewMaterialExpressionFunctionCall(Function, StateNormal, {});

    UMaterialExpressionIf* PositionX = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 1),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 1));

    UMaterialExpressionIf* PositionY = NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
        NewMaterialExpressionComponentMask(Function, UVCoords, 2),
        NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 2));

    UMaterialExpressionMaterialFunctionCall* TextureNormal = NewMaterialExpressionFunctionCall(Function, TexLookupFloat3,
        {
            Texture,
            NewMaterialExpressionStaticSwitch(Function, UseUVCoords,
                NewMaterialExpressionComponentMask(Function, UVCoords, 3),
                NewMaterialExpressionComponentMask(Function, TextureCoordinateInfoPosition, 3)),
            NewMaterialExpressionFunctionCall(Function, NativeWrapMode, {SWrap}),
            NewMaterialExpressionFunctionCall(Function, NativeWrapMode, {TWrap}),
            NewMaterialExpressionConstant(Function, 0.0f, 1.0f),
            NewMaterialExpressionConstant(Function, 0.0f, 1.0f),
            0.0f
        });

    UMaterialExpressionMultiply* FlipNormalX = NewMaterialExpressionMultiply(Function, NewMaterialExpressionStaticSwitch(Function, TangentSpaceFlipRChannel, NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, TextureNormal, 1), -1.0f), NewMaterialExpressionComponentMask(Function, TextureNormal, 1)), TangentSpaceFactor);
    UMaterialExpressionMultiply* FlipNormalY = NewMaterialExpressionMultiply(Function, NewMaterialExpressionStaticSwitch(Function, TangentSpaceFlipGChannel, NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, TextureNormal, 2), -1.0f), NewMaterialExpressionComponentMask(Function, TextureNormal, 2)), TangentSpaceFactor);
    UMaterialExpressionMultiply* FlipNormalZ = NewMaterialExpressionMultiply(Function, NewMaterialExpressionStaticSwitch(Function, TangentSpaceFlipBChannel, NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, TextureNormal, 4), -1.0f), NewMaterialExpressionComponentMask(Function, TextureNormal, 4)), TangentSpaceFactor);

    UMaterialExpressionNormalize* Normal =
        NewMaterialExpressionNormalize(Function,
            NewMaterialExpressionAdd(Function,
                {
                    NewMaterialExpressionMultiply(Function, 
                        NewMaterialExpressionStaticSwitch(Function, SFlip, NewMaterialExpressionMultiply(Function, TextureCoordinateInfoTangentU, -1.0f), TextureCoordinateInfoTangentU), FlipNormalX),
                    NewMaterialExpressionMultiply(Function,
                        NewMaterialExpressionStaticSwitch(Function, TFlip, NewMaterialExpressionMultiply(Function, TextureCoordinateInfoTangentV, -1.0f), TextureCoordinateInfoTangentV), FlipNormalY),
                    NewMaterialExpressionMultiply(Function,
                        DefaultNormal, NewMaterialExpressionAdd(Function,
                            FlipNormalZ,
                            NewMaterialExpressionOneMinus(Function, TangentSpaceFactor)))
                }));

    UMaterialExpressionIf* PositionXCheck = NewMaterialExpressionIf(Function, PositionX, 0.0f, DefaultNormal, Normal, NewMaterialExpressionIf(Function, PositionX, 1.0f, Normal, Normal, DefaultNormal));
    UMaterialExpressionIf* PositionYCheck = NewMaterialExpressionIf(Function, PositionY, 0.0f, DefaultNormal, Normal, NewMaterialExpressionIf(Function, PositionY, 1.0f, Normal, Normal, DefaultNormal));

    UMaterialExpressionIf* WrapMissingCheck = NewMaterialExpressionIfEqual(Function, SWrap, image_wrap_missing, PositionXCheck,
        NewMaterialExpressionIfEqual(Function, TWrap, image_wrap_missing, PositionYCheck, Normal));

    UMaterialExpressionIf* FinalNormal = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionMultiply(Function, IgnoreMissingTexture, NewMaterialExpressionOneMinus(Function, NewMaterialExpressionFunctionCall(Function, TextureIsValid, { Texture }))), 1.0f,
        DefaultNormal, WrapMissingCheck);

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), FinalNormal);
}

static void Generator_OmniImageNormalMapping(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum tangent_bitangent_mapping
    {
        red_green = 1,
        blue_green = 9
    };

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));

    UMaterialExpressionFunctionInput* Normal = NewMaterialExpressionFunctionInput(Function, TEXT("normal"), EFunctionInputType::FunctionInput_Vector3, { 0.0f, 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* TangentBitangentMapping = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_bitangent_mapping"), EFunctionInputType::FunctionInput_Scalar, red_green);
    UMaterialExpressionFunctionInput* NormalCamera = NewMaterialExpressionFunctionInput(Function, TEXT("normal_camera"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateNormal, {}));

    UMaterialExpressionIf* Result = NewMaterialExpressionIf(Function, NewMaterialExpressionDistance(Function, Normal, {0.0f, 0.0f, 0.0f}), 0, NormalCamera, NormalCamera, NewMaterialExpressionNormalMapping(Function, Normal, TangentBitangentMapping));
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_OmniImageTextureAlphaChannelRemap(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum alpha_mode
    {
        alpha_default,
        alpha_red,
        alpha_green,
        alpha_blue,
        alpha_white,
        alpha_black,
        alpha_luminance,
        alpha_average
    };

    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));
    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));

    UMaterialExpressionFunctionInput* TextureDataTint = NewMaterialExpressionFunctionInput(Function, TEXT("texture_data_tint"), EFunctionInputType::FunctionInput_Vector3, {0.0f, 0.0f, 0.0f});
    UMaterialExpressionFunctionInput* TextureDataMono = NewMaterialExpressionFunctionInput(Function, TEXT("texture_data_mono"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AlphaSource = NewMaterialExpressionFunctionInput(Function, TEXT("alpha_source"), EFunctionInputType::FunctionInput_Scalar, alpha_default);

    UMaterialExpressionIf* Mono = NewMaterialExpressionSwitch(Function, AlphaSource, 
        {
            TextureDataMono,
            NewMaterialExpressionComponentMask(Function, TextureDataTint, 1),
            NewMaterialExpressionComponentMask(Function, TextureDataTint, 2),
            NewMaterialExpressionComponentMask(Function, TextureDataTint, 4),
            1.0f,
            0.0f,
            NewMaterialExpressionFunctionCall(Function, MathLuminance, { TextureDataTint }),
            NewMaterialExpressionFunctionCall(Function, MathAverage, { TextureDataTint })
        }
    );

    NewMaterialExpressionFunctionOutput(Function, TEXT("tint"), TextureDataTint);
    NewMaterialExpressionFunctionOutput(Function, TEXT("mono"), Mono);
}

static void Generator_OmniSharedIorPresetToValue(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum ior_presets
    {
        ior_acrylic_glass = 0,
        ior_air,
        ior_crystal,
        ior_diamond,
        ior_emerald,
        ior_ethanol,
        ior_flint_glass,
        ior_glass,
        ior_honey_21p_water,
        ior_human_eye_aqueous_humor,
        ior_human_eye_cornea,
        ior_human_eye_lens,
        ior_human_eye_vitreous_humor,
        ior_human_skin,
        ior_human_hair,
        ior_human_wet_hair,
        ior_ice,
        ior_milk,
        ior_olive_oil,
        ior_pearl,
        ior_plastic,
        ior_sapphire,
        ior_soap_bubble,
        ior_vacuum,
        ior_water_0c,
        ior_water_35c,
        ior_water_100c,
        ior_custom = 99
    };


    UMaterialExpressionFunctionInput* Preset = NewMaterialExpressionFunctionInput(Function, TEXT("preset"), EFunctionInputType::FunctionInput_Scalar, ior_acrylic_glass);
    UMaterialExpressionFunctionInput* Custom = NewMaterialExpressionFunctionInput(Function, TEXT("custom"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionIf* Value = NewMaterialExpressionIfEqual(Function, Preset, ior_custom, Custom,
        NewMaterialExpressionSwitch(Function, Preset,
            {
                1.49f,
                1.0f,
                2.0f,
                2.42f,
                1.56f,
                1.36f,
                1.66f,
                1.5f,
                1.484f,
                1.33f,
                1.37f,
                1.41f,
                1.34f,
                1.4f,
                1.55f,
                1.85f,
                1.31f,
                1.35f,
                1.47f,
                1.53f,
                1.55f,
                1.77f,
                1.4f,
                1.0f,
                1.333f,
                1.325f,
                1.318f
            },
            true), true);
                                    
    NewMaterialExpressionFunctionOutput(Function, TEXT("value"), Value);
}

static void Generator_OmniSharedMelaninConcentrationPresetToValue(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum melanin_concentration_presets
    {
        melanin_concentration_white = 0,
        melanin_concentration_platinum_blonde,
        melanin_concentration_light_blonde,
        melanin_concentration_blonde,
        melanin_concentration_dark_blonde,
        melanin_concentration_dark_brown,
        melanin_concentration_black,
        melanin_concentration_custom
    };
    UMaterialExpressionFunctionInput* Preset = NewMaterialExpressionFunctionInput(Function, TEXT("preset"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* MelaninConcentration = NewMaterialExpressionFunctionInput(Function, TEXT("melanin_concentration_value"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionIf* Value = NewMaterialExpressionIfEqual(Function, Preset, melanin_concentration_custom, MelaninConcentration,
        NewMaterialExpressionSwitch(Function, Preset,
            {
                0.0f,
                0.0025f,
                0.10f,
                0.175f,
                0.30f,
                0.65f,
                1.0f
            },
            true), true);
                                    
    NewMaterialExpressionFunctionOutput(Function, TEXT("value"), Value);
}

static void Generator_OmniSharedScatteringColorsPresetToValue(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum scattering_colors_presets
    {
        scattering_colors_apple,
        scattering_colors_chicken,
        scattering_colors_cream,
        scattering_colors_ketchup,
        scattering_colors_marble,
        scattering_colors_potato,
        scattering_colors_skim_milk,
        scattering_colors_whole_milk,
        scattering_colors_skin_1,
        scattering_colors_skin_2,
        scattering_colors_skin_3,
        scattering_colors_skin_4,
        scattering_colors_custom,
    };

    UMaterialExpressionFunctionInput* Preset = NewMaterialExpressionFunctionInput(Function, TEXT("preset"), EFunctionInputType::FunctionInput_Scalar, scattering_colors_apple);
    UMaterialExpressionFunctionInput* ScatteringTransmissionColor = NewMaterialExpressionFunctionInput(Function, TEXT("scattering_transmission_color"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* ScatteringScatteringColor = NewMaterialExpressionFunctionInput(Function, TEXT("scattering_scattering_color"), EFunctionInputType::FunctionInput_Vector3, { 1.0f, 1.0f, 1.0f });

    UMaterialExpressionIf* OutTransmissionColor = NewMaterialExpressionSwitch(Function, Preset,
        {
            NewMaterialExpressionConstant(Function, 0.430f, 0.210f, 0.170f),
            NewMaterialExpressionConstant(Function, 0.440f, 0.220f, 0.140f),
            NewMaterialExpressionConstant(Function, 0.990f, 0.940f, 0.830f),
            NewMaterialExpressionConstant(Function, 0.220f, 0.010f, 0.001f),
            NewMaterialExpressionConstant(Function, 0.930f, 0.910f, 0.880f),
            NewMaterialExpressionConstant(Function, 0.860f, 0.740f, 0.290f),
            NewMaterialExpressionConstant(Function, 0.890f, 0.890f, 0.800f),
            NewMaterialExpressionConstant(Function, 0.950f, 0.930f, 0.850f),
            NewMaterialExpressionConstant(Function, 0.570f, 0.310f, 0.170f),
            NewMaterialExpressionConstant(Function, 0.750f, 0.570f, 0.470f),
            NewMaterialExpressionConstant(Function, 0.999f, 0.615f, 0.521f),
            NewMaterialExpressionConstant(Function, 0.078f, 0.043f, 0.025f),
            ScatteringTransmissionColor
        }, true);
    UMaterialExpressionIf* OutScatteringColor = NewMaterialExpressionSwitch(Function, Preset,
        {
            NewMaterialExpressionConstant(Function, 11.610f, 3.880f, 1.750f),
            NewMaterialExpressionConstant(Function, 9.440f, 3.350f, 1.790f),
            NewMaterialExpressionConstant(Function, 15.030f, 4.660f, 2.540f),
            NewMaterialExpressionConstant(Function, 4.760f, 0.570f, 0.390f),
            NewMaterialExpressionConstant(Function, 8.510f, 5.570f, 3.950f),
            NewMaterialExpressionConstant(Function, 14.270f, 7.230f, 2.040f),
            NewMaterialExpressionConstant(Function, 18.420f, 10.440f, 3.500f),
            NewMaterialExpressionConstant(Function, 10.900f, 6.580f, 2.510f),
            NewMaterialExpressionConstant(Function, 3.670f, 1.370f, 0.680f),
            NewMaterialExpressionConstant(Function, 4.820f, 1.690f, 1.090f),
            NewMaterialExpressionConstant(Function, 1.000f, 0.300f, 0.100f),
            NewMaterialExpressionConstant(Function, 0.723f, 0.264f, 0.127f),
            ScatteringScatteringColor
        }, true);

    NewMaterialExpressionFunctionOutput(Function, TEXT("scattering_transmission_color"), OutTransmissionColor);
    NewMaterialExpressionFunctionOutput(Function, TEXT("scattering_scattering_color"), OutScatteringColor);
}

static void Generator_OmniSharedDisplacementAdjustment(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum displacement_mode
    {
        displacement_height,
        displacement_vector_tangent_space,
        displacement_vector_object_space,
        displacement_vector_world_space
    };

    UMaterialFunction* StateTransformPoint = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_transform_point"));
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* StatePosition = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_position"));

    UMaterialExpressionFunctionInput* Displacement = NewMaterialExpressionFunctionInput(Function, TEXT("displacement"), EFunctionInputType::FunctionInput_Vector3, {0.0f, 0.0f, 0.0f});
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar, displacement_height);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);

    UMaterialExpressionIf* Result = NewMaterialExpressionSwitch(Function, Mode,
        {
            NewMaterialExpressionMultiply(Function, {NewMaterialExpressionFunctionCall(Function, StateNormal, {}), Displacement, Scale}),
            NewMaterialExpressionAdd(Function,
                {
                    NewMaterialExpressionMultiply(Function,
                        {
                            NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, {0}),
                            NewMaterialExpressionComponentMask(Function, Displacement, 1),
                            Scale
                        }),
                    NewMaterialExpressionMultiply(Function,
                        {
                            NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, {0}),
                            NewMaterialExpressionComponentMask(Function, Displacement, 4),
                            Scale
                        }),
                    NewMaterialExpressionMultiply(Function,
                        {
                            NewMaterialExpressionFunctionCall(Function, StateNormal, {}),
                            NewMaterialExpressionComponentMask(Function, Displacement, 2),
                            Scale
                        })
                }),
            NewMaterialExpressionFunctionCall(Function, StateTransformPoint,{ coordinate_internal, coordinate_object, NewMaterialExpressionMultiply(Function, Displacement, Scale) }),
            NewMaterialExpressionFunctionCall(Function, StateTransformPoint,{ coordinate_internal, coordinate_world, NewMaterialExpressionMultiply(Function, Displacement, Scale) }),
            NewMaterialExpressionFunctionCall(Function, StatePosition, {})		
        }
    );

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);

}

static void Generator_ReallusionHue(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));
    UMaterialFunction* MathMinValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_min_value"));

    UMaterialExpressionFunctionInput* RGB = NewMaterialExpressionFunctionInput(Function, TEXT("rgb"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionMaterialFunctionCall* MaxValue = NewMaterialExpressionFunctionCall(Function, MathMaxValue, {RGB});
    UMaterialExpressionMaterialFunctionCall* MinValue = NewMaterialExpressionFunctionCall(Function, MathMinValue, {RGB});
    
    UMaterialExpressionSubtract* Range = NewMaterialExpressionSubtract(Function, MaxValue, MinValue);
    UMaterialExpressionDivide* InvRange = NewMaterialExpressionDivide(Function, 1.0f, Range);
    UMaterialExpressionComponentMask* R = NewMaterialExpressionComponentMask(Function, RGB, 1);
    UMaterialExpressionComponentMask* G = NewMaterialExpressionComponentMask(Function, RGB, 2);
    UMaterialExpressionComponentMask* B = NewMaterialExpressionComponentMask(Function, RGB, 4);

    UMaterialExpressionIf* MaxRGB = NewMaterialExpressionIfEqual(Function, NewMaterialExpressionSubtract(Function, MaxValue, R), 0.0f,
        NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, G, B), InvRange),
        NewMaterialExpressionIfEqual(Function, NewMaterialExpressionSubtract(Function, MaxValue, G), 0.0f, 
        NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, B, R), InvRange), 2.0f),
        NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, NewMaterialExpressionSubtract(Function, R, G), InvRange), 4.0f)
        ));

    UMaterialExpressionIf* Hue = NewMaterialExpressionIfEqual(Function, Range, 0.0f, 0.0f, NewMaterialExpressionDivide(Function, MaxRGB, 6.0f));
    UMaterialExpressionIf* Result = NewMaterialExpressionIfLess(Function, Hue, 0.0f, NewMaterialExpressionAdd(Function, Hue, 1.0f), Hue);
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ReallusionSaturation(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathMaxValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_max_value"));
    UMaterialFunction* MathMinValue = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_min_value"));

    UMaterialExpressionFunctionInput* RGB = NewMaterialExpressionFunctionInput(Function, TEXT("rgb"), EFunctionInputType::FunctionInput_Vector3);

    UMaterialExpressionMaterialFunctionCall* MaxValue = NewMaterialExpressionFunctionCall(Function, MathMaxValue, {RGB});
    UMaterialExpressionMaterialFunctionCall* MinValue = NewMaterialExpressionFunctionCall(Function, MathMinValue, {RGB});

    UMaterialExpressionIf* Result = NewMaterialExpressionIfEqual(Function, MaxValue, 0.0f, 0.0f, NewMaterialExpressionOneMinus(Function, NewMaterialExpressionDivide(Function, MinValue, MaxValue)));
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ReallusionHSVToRGB(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* HSV = NewMaterialExpressionFunctionInput(Function, TEXT("hsv"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionComponentMask* X = NewMaterialExpressionComponentMask(Function, HSV, 1);
    UMaterialExpressionComponentMask* Y = NewMaterialExpressionComponentMask(Function, HSV, 2);
    UMaterialExpressionComponentMask* Z = NewMaterialExpressionComponentMask(Function, HSV, 4);


    UMaterialExpressionIf* HPrime = NewMaterialExpressionIfEqual(Function, X, 1.0f, 0.0f, NewMaterialExpressionMultiply(Function, X, 6.0f));
    UMaterialExpressionFloor* HFloor = NewMaterialExpressionFloor(Function, HPrime);
    UMaterialExpressionSubtract* PF = NewMaterialExpressionSubtract(Function, HPrime, HFloor);
    UMaterialExpressionMultiply* ZY = NewMaterialExpressionMultiply(Function, Z, Y);
    UMaterialExpressionSubtract* A = NewMaterialExpressionSubtract(Function, Z, ZY);
    UMaterialExpressionMultiply* ZYPF = NewMaterialExpressionMultiply(Function, ZY, PF);
    UMaterialExpressionSubtract* B = NewMaterialExpressionSubtract(Function, Z, ZYPF);
    UMaterialExpressionAdd* C = NewMaterialExpressionAdd(Function, A, ZYPF);

    UMaterialExpressionIf* Result = NewMaterialExpressionSwitch(Function, HFloor, 
        {
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Z, C, A }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { B, Z, A }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { A, Z, C }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { A, B, Z }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { C, A, Z }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Z, A, B }),
            NewMaterialExpressionFunctionCall(Function, MakeFloat3, { Z, C, A }),
        }, true);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ReallusionBlendMapByMode(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum RLBlendMode
    {
        blendmode_multiply,
        blendmode_add,
        blendmode_overlay,
    };

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* BlendColor = NewMaterialExpressionFunctionInput(Function, TEXT("blend_color"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* BaseColor = NewMaterialExpressionFunctionInput(Function, TEXT("base_color"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* Weight = NewMaterialExpressionFunctionInput(Function, TEXT("weight"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionMultiply* MultiColor = NewMaterialExpressionMultiply(Function, BlendColor, BaseColor);
    UMaterialExpressionAdd* AddColor = NewMaterialExpressionAdd(Function, BlendColor, BaseColor);

    UMaterialExpressionComponentMask* MultiColorX = NewMaterialExpressionComponentMask(Function, BlendColor, 1);
    UMaterialExpressionComponentMask* MultiColorY = NewMaterialExpressionComponentMask(Function, BlendColor, 2);
    UMaterialExpressionComponentMask* MultiColorZ = NewMaterialExpressionComponentMask(Function, BlendColor, 4);

    UMaterialExpressionIf* MultiX = NewMaterialExpressionIfLess(Function, NewMaterialExpressionComponentMask(Function, BaseColor, 1), 0.5f, MultiColorX, NewMaterialExpressionSubtract(Function, NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, AddColor, 1), MultiColorX), 0.5f));
    UMaterialExpressionIf* MultiY = NewMaterialExpressionIfLess(Function, NewMaterialExpressionComponentMask(Function, BaseColor, 2), 0.5f, MultiColorY, NewMaterialExpressionSubtract(Function, NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, AddColor, 2), MultiColorY), 0.5f));
    UMaterialExpressionIf* MultiZ = NewMaterialExpressionIfLess(Function, NewMaterialExpressionComponentMask(Function, BaseColor, 4), 0.5f, MultiColorZ, NewMaterialExpressionSubtract(Function, NewMaterialExpressionSubtract(Function, NewMaterialExpressionComponentMask(Function, AddColor, 4), MultiColorZ), 0.5f));

    UMaterialExpressionMaterialFunctionCall* NewOp = NewMaterialExpressionFunctionCall(Function, MakeFloat3, { MultiX, MultiY, MultiZ });

    UMaterialExpressionIf* OpResult = NewMaterialExpressionSwitch(Function, Mode, 
        {
            MultiColor,
            AddColor,
            NewMaterialExpressionAdd(Function, NewOp, NewOp)
        }, true);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionLinearInterpolate(Function, BaseColor, OpResult, Weight));
}

static void Generator_ReallusionBaseColorMake(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum RLBlendMode
    {
        blendmode_multiply,
        blendmode_add,
        blendmode_overlay,
    };

    UMaterialFunction* MakeFloat3 = LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3"));

    UMaterialExpressionFunctionInput* BaseColorMakeDataBaseColor = NewMaterialExpressionFunctionInput(Function, TEXT("data.base_color"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* BaseColorMakeDataBlendColor = NewMaterialExpressionFunctionInput(Function, TEXT("data.blend"), EFunctionInputType::FunctionInput_Vector3);
    UMaterialExpressionFunctionInput* BaseColorMakeDataBlendStrength = NewMaterialExpressionFunctionInput(Function, TEXT("data.blend_strength"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* BaseColorMakeDataBlendMode = NewMaterialExpressionFunctionInput(Function, TEXT("data.blend_mode"), EFunctionInputType::FunctionInput_Scalar);


    UMaterialExpressionLinearInterpolate* B = NewMaterialExpressionLinearInterpolate(Function, NewMaterialExpressionConstant(Function, 1.0f, 1.0f, 1.0f), BaseColorMakeDataBlendColor, BaseColorMakeDataBlendStrength);
    UMaterialExpressionMultiply* VA = NewMaterialExpressionMultiply(Function, 2.0f, NewMaterialExpressionMultiply(Function, BaseColorMakeDataBaseColor, B));
    UMaterialExpressionOneMinus* VB = NewMaterialExpressionOneMinus(Function, NewMaterialExpressionMultiply(Function, 2.0f, NewMaterialExpressionMultiply(Function, NewMaterialExpressionOneMinus(Function, BaseColorMakeDataBaseColor), NewMaterialExpressionOneMinus(Function, B))));

    UMaterialExpressionComponentMask* VA_X = NewMaterialExpressionComponentMask(Function, VA, 1);
    UMaterialExpressionComponentMask* VA_Y = NewMaterialExpressionComponentMask(Function, VA, 2);
    UMaterialExpressionComponentMask* VA_Z = NewMaterialExpressionComponentMask(Function, VA, 4);

    UMaterialExpressionComponentMask* VB_X = NewMaterialExpressionComponentMask(Function, VB, 1);
    UMaterialExpressionComponentMask* VB_Y = NewMaterialExpressionComponentMask(Function, VB, 2);
    UMaterialExpressionComponentMask* VB_Z = NewMaterialExpressionComponentMask(Function, VB, 4);

    UMaterialExpressionComponentMask* A_X = NewMaterialExpressionComponentMask(Function, BaseColorMakeDataBaseColor, 1);
    UMaterialExpressionComponentMask* A_Y = NewMaterialExpressionComponentMask(Function, BaseColorMakeDataBaseColor, 2);
    UMaterialExpressionComponentMask* A_Z = NewMaterialExpressionComponentMask(Function, BaseColorMakeDataBaseColor, 4);

    UMaterialExpressionMaterialFunctionCall* OverLay = NewMaterialExpressionFunctionCall(Function, MakeFloat3, { 
        NewMaterialExpressionIfLess(Function, A_X, 0.5f, VA_X, VB_X),
        NewMaterialExpressionIfLess(Function, A_Y, 0.5f, VA_Y, VB_Y),
        NewMaterialExpressionIfLess(Function, A_Z, 0.5f, VA_Z, VB_Z)  
        });

    UMaterialExpressionIf* Result = NewMaterialExpressionSwitch(Function, BaseColorMakeDataBlendMode, 
        {
            NewMaterialExpressionMultiply(Function, BaseColorMakeDataBaseColor, B),
            NewMaterialExpressionAdd(Function, BaseColorMakeDataBaseColor, NewMaterialExpressionLinearInterpolate(Function, NewMaterialExpressionConstant(Function, 0.0f, 0.0f, 0.0f), BaseColorMakeDataBlendColor, BaseColorMakeDataBlendStrength)),
            OverLay
        }, true);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), Result);
}

static void Generator_ReallusionAdjustOpacity(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum alpha_mode
    {
        alpha_default,
        alpha_red,
        alpha_green,
        alpha_blue,
        alpha_white,
        alpha_black,
        alpha_luminance,
        alpha_average
    };

    UMaterialFunction* MathAverage = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_average"));
    UMaterialFunction* MathLuminance = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_luminance"));

    UMaterialExpressionFunctionInput* RGBA = NewMaterialExpressionFunctionInput(Function, TEXT("rgba"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* AlphaMode = NewMaterialExpressionFunctionInput(Function, TEXT("alpha_mode"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* AlphaGain = NewMaterialExpressionFunctionInput(Function, TEXT("alpha_gain"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionIf* Result = NewMaterialExpressionSwitch(Function, AlphaMode, 
    {
        NewMaterialExpressionComponentMask(Function, RGBA, 8), //default
        NewMaterialExpressionComponentMask(Function, RGBA, 1), //red
        NewMaterialExpressionComponentMask(Function, RGBA, 2), //green
        NewMaterialExpressionComponentMask(Function, RGBA, 4), //blue
        1.0f, //white
        0.0f, //black
        NewMaterialExpressionFunctionCall(Function, MathLuminance, {NewMaterialExpressionComponentMask(Function, RGBA, 7)}), //luminance
        NewMaterialExpressionFunctionCall(Function, MathAverage, {NewMaterialExpressionComponentMask(Function, RGBA, 7)}) //average
    }, true);
    
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionMultiply(Function, Result, AlphaGain));
}

static void Generator_GltfPrepareTexFloat4(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Valid = NewMaterialExpressionFunctionInput(Function, TEXT("valid"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("value"), EFunctionInputType::FunctionInput_Vector4);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfEqual(Function, Valid, 1, Value, {1.0, 1.0, 1.0, 1.0}));
}

static void Generator_GltfPrepareTexFloat3(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Valid = NewMaterialExpressionFunctionInput(Function, TEXT("valid"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("value"), EFunctionInputType::FunctionInput_Vector4);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfEqual(Function, Valid, 1, NewMaterialExpressionComponentMask(Function, Value, 7), {1.0, 1.0, 1.0}));
}

static void Generator_GltfPrepareOcclusion(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialExpressionFunctionInput* Valid = NewMaterialExpressionFunctionInput(Function, TEXT("valid"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("value"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* OcclusionStrength = NewMaterialExpressionFunctionInput(Function, TEXT("occlusion_strength"), EFunctionInputType::FunctionInput_Scalar);
    
    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionLinearInterpolate(Function, 1.0, NewMaterialExpressionIfEqual(Function, Valid, 1, NewMaterialExpressionComponentMask(Function, Value, 1), 1.0f), OcclusionStrength));
}

static void Generator_GltfPrepareEmission(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* TexFloat3 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_gltf_pbr_gltf_prepare_tex_float3"));
    UMaterialExpressionFunctionInput* Valid = NewMaterialExpressionFunctionInput(Function, TEXT("valid"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("value"), EFunctionInputType::FunctionInput_Vector4);
    UMaterialExpressionFunctionInput* EmissionFactor = NewMaterialExpressionFunctionInput(Function, TEXT("emissive_factor"), EFunctionInputType::FunctionInput_Vector3);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionMultiply(Function, EmissionFactor, NewMaterialExpressionFunctionCall(Function, TexFloat3, {Valid, Value})));
}

static void Generator_GltfPrepareNormal(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialExpressionFunctionInput* Valid = NewMaterialExpressionFunctionInput(Function, TEXT("valid"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Value = NewMaterialExpressionFunctionInput(Function, TEXT("value"), EFunctionInputType::FunctionInput_Vector3);

    NewMaterialExpressionFunctionOutput(Function, TEXT("result"), NewMaterialExpressionIfEqual(Function, Valid, 1, Value, NewMaterialExpressionFunctionCall(Function, StateNormal, {})));
}

static void Generator_GltfTextureTransformApply(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* MathSin = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_sin_float"));
    UMaterialFunction* MathCos = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_math_cos_float"));

    UMaterialExpressionFunctionInput* Coord = NewMaterialExpressionFunctionInput(Function, TEXT("coord"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* Rotation = NewMaterialExpressionFunctionInput(Function, TEXT("rotation"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Vector2);

    UMaterialExpressionMaterialFunctionCall* SinRot = NewMaterialExpressionFunctionCall(Function, MathSin, {Rotation});
    UMaterialExpressionMaterialFunctionCall* CosRot = NewMaterialExpressionFunctionCall(Function, MathCos, {Rotation});

    UMaterialExpressionMultiply* CoordX = NewMaterialExpressionMultiply(Function, NewMaterialExpressionComponentMask(Function, Coord, 1), NewMaterialExpressionComponentMask(Function, Scale, 1));
    UMaterialExpressionMultiply* CoordY = NewMaterialExpressionMultiply(Function, NewMaterialExpressionOneMinus(Function, NewMaterialExpressionComponentMask(Function, Coord, 2)), NewMaterialExpressionComponentMask(Function, Scale, 2));

    UMaterialExpressionAdd* FinalCoordX = NewMaterialExpressionAdd(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionMultiply(Function, CosRot, CoordX), NewMaterialExpressionMultiply(Function, SinRot, CoordY)), NewMaterialExpressionComponentMask(Function, Offset, 1));
    UMaterialExpressionOneMinus* FinalCoordY = NewMaterialExpressionOneMinus(Function, NewMaterialExpressionAdd(Function, NewMaterialExpressionSubtract(Function, NewMaterialExpressionMultiply(Function, CosRot, CoordY), NewMaterialExpressionMultiply(Function, SinRot, CoordX)), NewMaterialExpressionComponentMask(Function, Offset, 2)));

    NewMaterialExpressionFunctionOutput(Function, TEXT("coord"), NewMaterialExpressionAppendVector(Function, FinalCoordX, FinalCoordY));
}

static void Generator_GltfConvertWrapMode(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum gltf_wrapping_mode
    {
        clamp_to_edge = 33071,
        mirrored_repeat = 33648,
        repeat = 10497
    };

    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar);
    NewMaterialExpressionFunctionOutput(Function, TEXT("wrap_mode"), NewMaterialExpressionIfEqual(Function, Mode, clamp_to_edge, wrap_clamp, NewMaterialExpressionIfEqual(Function, Mode, mirrored_repeat, wrap_mirrored_repeat, wrap_repeat)));
}

static void Generator_GltfTextureLookup(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum gltf_wrapping_mode
    {
        clamp_to_edge = 33071,
        mirrored_repeat = 33648,
        repeat = 10497
    };

    UMaterialFunction* ConvertWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_gltf_pbr_convert_wrap_mode"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* TextureIsValid = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_texture_isvalid"));
    UMaterialFunction* TransformApply = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_gltf_pbr_khr_texture_transform_apply"));
    UMaterialFunction* TexLookupFloat4 = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_lookup_float4_2d"));


    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, nullptr));
    UMaterialExpressionFunctionInput* TexCoordIndex = NewMaterialExpressionFunctionInput(Function, TEXT("tex_coord_index"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Rotation = NewMaterialExpressionFunctionInput(Function, TEXT("rotation"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Vector2, { 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapS = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_s"), EFunctionInputType::FunctionInput_Scalar, repeat);
    UMaterialExpressionFunctionInput* WrapT = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_t"), EFunctionInputType::FunctionInput_Scalar, repeat);

    UMaterialExpressionMaterialFunctionCall* IsValid = NewMaterialExpressionFunctionCall(Function, TextureIsValid, {Texture});
    UMaterialExpressionMaterialFunctionCall* TexCoord = NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, {TexCoordIndex});

    UMaterialExpressionMaterialFunctionCall* Value = NewMaterialExpressionFunctionCall(Function, TexLookupFloat4, {Texture, NewMaterialExpressionFunctionCall(Function, TransformApply, {NewMaterialExpressionComponentMask(Function, TexCoord, 3), Offset, Rotation, Scale})
        , NewMaterialExpressionFunctionCall(Function, ConvertWrapMode, {WrapS}), NewMaterialExpressionFunctionCall(Function, ConvertWrapMode, {WrapT})});

    NewMaterialExpressionFunctionOutput(Function, TEXT("valid"), IsValid);
    NewMaterialExpressionFunctionOutput(Function, TEXT("value"), NewMaterialExpressionIfEqual(Function, IsValid, 1.0f, Value, {0.0f, 0.0f, 0.0f, 1.0f}));
}

static void Generator_GltfNormalTextureLookup(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum gltf_wrapping_mode
    {
        clamp_to_edge = 33071,
        mirrored_repeat = 33648,
        repeat = 10497
    };

    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* TextureIsValid = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_tex_texture_isvalid"));
    UMaterialFunction* ConvertWrapMode = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_gltf_pbr_convert_wrap_mode"));
    UMaterialFunction* TransformApply = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_gltf_pbr_khr_texture_transform_apply"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));
    UMaterialFunction* BaseTangentSpaceNormalTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_tangent_space_normal_texture"));
    UMaterialFunction* StatePosition = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_position"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultNormal")), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* NormalScaleFactor = NewMaterialExpressionFunctionInput(Function, TEXT("normal_scale_factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* TexCoordIndex = NewMaterialExpressionFunctionInput(Function, TEXT("tex_coord_index"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TexTangentIndex = NewMaterialExpressionFunctionInput(Function, TEXT("tex_tangent_index"), EFunctionInputType::FunctionInput_Scalar, 0.0f);	
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* Rotation = NewMaterialExpressionFunctionInput(Function, TEXT("rotation"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Vector2, { 1.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapS = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_s"), EFunctionInputType::FunctionInput_Scalar, repeat);
    UMaterialExpressionFunctionInput* WrapT = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_t"), EFunctionInputType::FunctionInput_Scalar, repeat);


    UMaterialExpressionMaterialFunctionCall* IsValid = NewMaterialExpressionFunctionCall(Function, TextureIsValid, {Texture});
    UMaterialExpressionMaterialFunctionCall* TexCoord = NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, {TexCoordIndex});
    UMaterialExpressionMaterialFunctionCall* StatePositionCall = NewMaterialExpressionFunctionCall(Function, StatePosition, {});

    UMaterialExpressionMaterialFunctionCall* Value = NewMaterialExpressionFunctionCall(Function, BaseTangentSpaceNormalTexture, 
        {
            Texture, 
            NormalScaleFactor, 
            0.0f,
            0.0f,
            NewMaterialExpressionAppendVector(Function, NewMaterialExpressionFunctionCall(Function, TransformApply, {NewMaterialExpressionComponentMask(Function, TexCoord, 3), Offset, Rotation, Scale}), NewMaterialExpressionComponentMask(Function, TexCoord, 4)),
            NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, {TexTangentIndex}), NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, {TexTangentIndex}),	
#if defined(USE_WORLD_ALIGNED_TEXTURES)		
            NewMaterialExpressionComponentMask(Function, StatePositionCall, 3),
            NewMaterialExpressionComponentMask(Function, StatePositionCall, 5),
            NewMaterialExpressionComponentMask(Function, StatePositionCall, 6),
#endif		
            {},
            {},
            NewMaterialExpressionFunctionCall(Function, ConvertWrapMode, {WrapS}), 
            NewMaterialExpressionFunctionCall(Function, ConvertWrapMode, {WrapT}),
            0.0f,
            {},
            {},
            {},
            {},
            {},
            {}
#if defined(USE_WORLD_ALIGNED_TEXTURES)	
            ,0.0f
#endif
        });


    NewMaterialExpressionFunctionOutput(Function, TEXT("valid"), IsValid);
    NewMaterialExpressionFunctionOutput(Function, TEXT("value"), NewMaterialExpressionIfEqual(Function, IsValid, 1.0f, Value, {0.0f, 0.0f, 0.0f}));
}

static void Generator_GltfComputeOpacity(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum gltf_alpha_mode
    {
        opaque,
        mask,
        blend
    };

    UMaterialExpressionFunctionInput* Mode = NewMaterialExpressionFunctionInput(Function, TEXT("mode"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Alpha = NewMaterialExpressionFunctionInput(Function, TEXT("alpha"), EFunctionInputType::FunctionInput_Scalar);
    UMaterialExpressionFunctionInput* Cutoff = NewMaterialExpressionFunctionInput(Function, TEXT("cutoff"), EFunctionInputType::FunctionInput_Scalar);

    UMaterialExpressionIf* Opacity = NewMaterialExpressionSwitch(Function, Mode, 
        {
        1.0f,
        NewMaterialExpressionIfLess(Function, Alpha, Cutoff, 0.0f, 1.0f),
        Alpha
        });

    NewMaterialExpressionFunctionOutput(Function, TEXT("opacity"), Opacity);
}

static void Generator_OmniPBRBaseTangentspaceNormalRoughnessTexture(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/DefaultNormal"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* Factor = NewMaterialExpressionFunctionInput(Function, TEXT("factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* FlipTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("flip_tangent_u"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* FlipTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("flip_tangent_v"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UVWPosition = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* UVWTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("uvw.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    UMaterialExpressionFunctionInput* WorldAlignedPositionXY = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xy"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionXZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_xz"), EFunctionInputType::FunctionInput_Vector2);
    UMaterialExpressionFunctionInput* WorldAlignedPositionYZ = NewMaterialExpressionFunctionInput(Function, TEXT("world_aligned_position_yz"), EFunctionInputType::FunctionInput_Vector2);
#endif

    UMaterialExpressionFunctionInput* CropU = NewMaterialExpressionFunctionInput(Function, TEXT("crop_u"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* CropV = NewMaterialExpressionFunctionInput(Function, TEXT("crop_v"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 1.0f });
    UMaterialExpressionFunctionInput* WrapU = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_u"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* WrapV = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_v"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* Clip = NewMaterialExpressionFunctionInput(Function, TEXT("clip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* Scale = NewMaterialExpressionFunctionInput(Function, TEXT("scale"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* Offset = NewMaterialExpressionFunctionInput(Function, TEXT("offset"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* AnimationStartTime = NewMaterialExpressionFunctionInput(Function, TEXT("animation_start_time"), EFunctionInputType::FunctionInput_Scalar, 0);
    UMaterialExpressionFunctionInput* AnimationCrop = NewMaterialExpressionFunctionInput(Function, TEXT("animation_crop"), EFunctionInputType::FunctionInput_Vector2, { 0, 0 });
    UMaterialExpressionFunctionInput* WrapMode = NewMaterialExpressionFunctionInput(Function, TEXT("wrap_mode"), EFunctionInputType::FunctionInput_Scalar, wrap_repeat);
    UMaterialExpressionFunctionInput* AnimationFps = NewMaterialExpressionFunctionInput(Function, TEXT("animation_fps"), EFunctionInputType::FunctionInput_Scalar, 30);

    UMaterialFunction* BaseTangentSpaceNormalTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_base_tangent_space_normal_texture"));

    UMaterialExpressionMaterialFunctionCall* Normal =
        NewMaterialExpressionFunctionCall(Function, BaseTangentSpaceNormalTexture,
            {
                Texture,
                Factor,
                FlipTangentU,
                FlipTangentV,
                UVWPosition,
                UVWTangentU,
                UVWTangentV,
#if defined(USE_WORLD_ALIGNED_TEXTURES)
                WorldAlignedPositionXY,
                WorldAlignedPositionXZ,
                WorldAlignedPositionYZ,
#endif
                CropU,
                CropV,
                WrapU,
                WrapV,
                Clip,
                Scale,
                Offset,
                AnimationStartTime,
                AnimationCrop,
                WrapMode,
                AnimationFps
#if defined(USE_WORLD_ALIGNED_TEXTURES)	
                ,0.0f
#endif
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), Normal);
    NewMaterialExpressionFunctionOutput(Function, TEXT("roughness"), { 0.0f, 0.0f });
}

static void Generator_OmniImageTangentspaceNormalRoughnessLookup(UMaterialFunction* Function, const TArray<int32>& ArrayInputSizes)
{
    enum image_wrap_mode
    {
        image_wrap_periodic,
        image_wrap_black,
        image_wrap_clamp,
        image_wrap_mirror,
        image_wrap_file,
        image_wrap_missing,
    };

    UMaterialFunction* StateNormal = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal"));
    UMaterialFunction* StateTextureCoordinate = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_coordinate"));
    UMaterialFunction* StateTextureTangentU = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_u"));
    UMaterialFunction* StateTextureTangentV = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_texture_tangent_v"));

    UMaterialFunction* TangentSpaceNormalTexture = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_OmniSurface_OmniImage_tangent_space_normal_lookup_2"));

    UMaterialExpressionFunctionInput* Texture = NewMaterialExpressionFunctionInput(Function, TEXT("texture"), EFunctionInputType::FunctionInput_Texture2D, NewMaterialExpressionTextureObject(Function, LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/DefaultNormal"), nullptr, LOAD_None, nullptr)));
    UMaterialExpressionFunctionInput* IgnoreMissingTexture = NewMaterialExpressionFunctionInput(Function, TEXT("ignore_missing_texture"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UseUVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("use_uv_coords"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* UVCoords = NewMaterialExpressionFunctionInput(Function, TEXT("uv_coords"), EFunctionInputType::FunctionInput_Vector2, { 0.0f, 0.0f });
    UMaterialExpressionFunctionInput* SWrap = NewMaterialExpressionFunctionInput(Function, TEXT("s_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* TWrap = NewMaterialExpressionFunctionInput(Function, TEXT("t_wrap"), EFunctionInputType::FunctionInput_Scalar, image_wrap_periodic);
    UMaterialExpressionFunctionInput* SFlip = NewMaterialExpressionFunctionInput(Function, TEXT("s_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TFlip = NewMaterialExpressionFunctionInput(Function, TEXT("t_flip"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFactor = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_factor"), EFunctionInputType::FunctionInput_Scalar, 1.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipRChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_r_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipGChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_g_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TangentSpaceFlipBChannel = NewMaterialExpressionFunctionInput(Function, TEXT("tangent_space_flip_b_channel"), EFunctionInputType::FunctionInput_Scalar, 0.0f);
    UMaterialExpressionFunctionInput* TextureCoordinateInfoPosition = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.position"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureCoordinate, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentU = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_u"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentU, { 0 }));
    UMaterialExpressionFunctionInput* TextureCoordinateInfoTangentV = NewMaterialExpressionFunctionInput(Function, TEXT("texture_coordinate_info.tangent_v"), EFunctionInputType::FunctionInput_Vector3, NewMaterialExpressionFunctionCall(Function, StateTextureTangentV, { 0 }));

    UMaterialExpressionMaterialFunctionCall* Normal =
        NewMaterialExpressionFunctionCall(Function, TangentSpaceNormalTexture,
            {
                Texture,
                IgnoreMissingTexture,
                UseUVCoords,
                UVCoords,
                SWrap,
                TWrap,
                SFlip,
                TFlip,
                TangentSpaceFactor,
                TangentSpaceFlipRChannel,
                TangentSpaceFlipGChannel,
                TangentSpaceFlipBChannel,
                TextureCoordinateInfoPosition,
                TextureCoordinateInfoTangentU,
                TextureCoordinateInfoTangentV,
            });

    NewMaterialExpressionFunctionOutput(Function, TEXT("normal"), Normal);
    NewMaterialExpressionFunctionOutput(Function, TEXT("roughness"), { 0.0f, 0.0f });
}

struct FunctionGenerationData
{
    void(*Generator)(UMaterialFunction*, const TArray<int32>& ArrayInputSizes);
    int32	Version;
};

class FMDLFunctionGenerator
{
public:
    FMDLFunctionGenerator();
    virtual ~FMDLFunctionGenerator();

    UMaterialFunction*	GenerateFunction(const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes);
    void UpdateFunction(UMaterialFunction* Function, const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes);
    int32 GetVersion(const FString& AssetName);

private:
    UMaterialFunctionFactoryNew*			FunctionFactory;
    TMap<FString, FunctionGenerationData>	FunctionGenerators;
};

FMDLFunctionGenerator::FMDLFunctionGenerator()
    : FunctionFactory(NewObject<UMaterialFunctionFactoryNew>())
{
    FunctionFactory->AddToRoot();		// prevent garbage collection of this object

    FunctionGenerators.Add(TEXT("mdl_base_abbe_number_ior"), { Generator_BaseAbbeNumberIOR, 1 });
    FunctionGenerators.Add(TEXT("mdl_base_anisotropy_conversion"), { Generator_BaseAnisotropyConversion, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_architectural_gloss_to_rough"), { Generator_BaseArchitecturalGlossToRough, 1 });
    FunctionGenerators.Add(TEXT("mdl_base_blend_color_layers"), { Generator_BaseBlendColorLayers, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_checker_bump_texture"), { Generator_BaseCheckerBumpTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_checker_texture"), { Generator_BaseCheckerTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_coordinate_projection"), { Generator_BaseCoordinateProjection, 6 });
    FunctionGenerators.Add(TEXT("mdl_base_coordinate_source"), { Generator_BaseCoordinateSource, 4 });
    FunctionGenerators.Add(TEXT("mdl_base_file_bump_texture"), { Generator_BaseFileBumpTexture, 9 });
    FunctionGenerators.Add(TEXT("mdl_base_file_texture"), { Generator_BaseFileTexture, 7 });
    FunctionGenerators.Add(TEXT("mdl_base_file_texture_normal"), { Generator_BaseFileNormalTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_flake_noise_bump_texture"), { Generator_BaseFlakeNoiseBumpTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_flake_noise_texture"), { Generator_BaseFlakeNoiseTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_flow_noise_bump_texture"), { Generator_BaseFlowNoiseBumpTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_flow_noise_texture"), { Generator_BaseFlowNoiseTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_gloss_to_rough"), { Generator_BaseGlossToRough, 1 });
    FunctionGenerators.Add(TEXT("mdl_base_perlin_noise_bump_texture"), { Generator_BasePerlinNoiseBumpTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_perlin_noise_texture"), { Generator_BasePerlinNoiseTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_rotation_translation_scale"), { Generator_BaseRotationTranslationScale, 1 });
    FunctionGenerators.Add(TEXT("mdl_base_sellmeier_coefficients_ior"), { Generator_BaseSellmeierCoefficientsIOR, 1 });
    FunctionGenerators.Add(TEXT("mdl_base_tangent_space_normal_texture"), { Generator_BaseTangentSpaceNormalTexture, 9 });
    FunctionGenerators.Add(TEXT("mdl_base_texture_coordinate_info"), { Generator_BaseTextureCoordinateInfo, 4 });
    FunctionGenerators.Add(TEXT("mdl_base_tile_bump_texture"), { Generator_BaseTileBumpTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_transform_coordinate"), { Generator_BaseTransformCoordinate, 3 });
    FunctionGenerators.Add(TEXT("mdl_base_volume_coefficient"), { Generator_BaseVolumeCoefficient, 1 });
    FunctionGenerators.Add(TEXT("mdl_base_worley_noise_bump_texture"), { Generator_BaseWorleyNoiseBumpTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_base_worley_noise_texture"), { Generator_BaseWorleyNoiseTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_anisotropic_vdf"), { Generator_DFAnisotropicVDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_backscattering_glossy_reflection_bsdf"), { Generator_DFBackscatteringGlossyReflectionBSDF, 3 });
    FunctionGenerators.Add(TEXT("mdl_df_color_fresnel_layer"), { Generator_DFColorFresnelLayer, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_custom_curve_layer"), { Generator_DFCustomCurveLayer, 4 });
    FunctionGenerators.Add(TEXT("mdl_df_diffuse_edf"), { Generator_DFDiffuseEDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_diffuse_reflection_bsdf"), { Generator_DFDiffuseReflectionBSDF, 3 });
    FunctionGenerators.Add(TEXT("mdl_df_diffuse_transmission_bsdf"), { Generator_DFDiffuseTransmissionBSDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_directional_factor"), { Generator_DFDirectionalFactor, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_fresnel_layer"), { Generator_DFFresnelLayer, 8 });
    FunctionGenerators.Add(TEXT("mdl_df_light_profile_maximum"), { Generator_DFLightProfileMaximum, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_light_profile_power"), { Generator_DFLightProfilePower, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_measured_bsdf"), { Generator_DFMeasuredBSDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_measured_edf"), { Generator_DFMeasuredEDF, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_measured_curve_factor"), { Generator_DFMeasuredCurveFactor, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_microfacet_beckmann_smith_bsdf"), { Generator_DFMicrofacetBeckmannSmithBSDF, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_microfacet_beckmann_vcavities_bsdf"), { Generator_DFMicrofacetBeckmannVCavitiesBSDF, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_microfacet_ggx_smith_bsdf"), { Generator_DFMicrofacetGGXSmithBSDF, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_microfacet_ggx_vcavities_bsdf"), { Generator_DFMicrofacetGGXVCavitiesBSDF, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_normalized_mix"), { Generator_DFNormalizedMix, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_simple_glossy_bsdf"), { Generator_DFSimpleGlossyBSDF, 3 });
    FunctionGenerators.Add(TEXT("mdl_df_specular_bsdf"), { Generator_DFSpecularBSDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_spot_edf"), { Generator_DFSpotEDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_thin_film"), { Generator_DFThinFilm, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_tint"), { Generator_DFTint, 1 });
    FunctionGenerators.Add(TEXT("mdl_df_ward_geisler_moroder_bsdf"), { Generator_DFWardGeislerMoroderBSDF, 2 });
    FunctionGenerators.Add(TEXT("mdl_df_weighted_layer"), { Generator_DFWeightedLayer, 4 });
    FunctionGenerators.Add(TEXT("mdl_df_chiang_hair_bsdf"), { Generator_DFChiangHairBSDF, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_average"), { Generator_MathAverage, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_cos_float"), { Generator_MathCosFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_cos_float2"), { Generator_MathCosFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_cos_float3"), { Generator_MathCosFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_cos_float4"), { Generator_MathCosFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log_float"), { Generator_MathLogFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log_float2"), { Generator_MathLogFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log_float3"), { Generator_MathLogFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log_float4"), { Generator_MathLogFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log10_float"), { Generator_MathLog10Float, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log10_float2"), { Generator_MathLog10Float2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log10_float3"), { Generator_MathLog10Float3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log10_float4"), { Generator_MathLog10Float4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log2_float"), { Generator_MathLog2Float, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log2_float2"), { Generator_MathLog2Float2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log2_float3"), { Generator_MathLog2Float3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_log2_float4"), { Generator_MathLog2Float4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_luminance"), { Generator_MathLuminance, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_max_value"), { Generator_MathMaxValueFloat3, 1 }); // legacy
    FunctionGenerators.Add(TEXT("mdl_math_max_value_float2"), { Generator_MathMaxValueFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_max_value_float3"), { Generator_MathMaxValueFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_max_value_float4"), { Generator_MathMaxValueFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_min_value"), { Generator_MathMinValueFloat3, 1 }); // legacy
    FunctionGenerators.Add(TEXT("mdl_math_min_value_float2"), { Generator_MathMinValueFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_min_value_float3"), { Generator_MathMinValueFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_min_value_float4"), { Generator_MathMinValueFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float4_float4x4"), { Generator_MathMultiplyFloat4Float4x4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float4x4_float4"), { Generator_MathMultiplyFloat4x4Float4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float4x4_float4x4"), { Generator_MathMultiplyFloat4x4Float4x4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float3_float3x3"), { Generator_MathMultiplyFloat3Float3x3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float3x3_float3"), { Generator_MathMultiplyFloat3x3Float3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float3x3_float3x3"), { Generator_MathMultiplyFloat3x3Float3x3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float2_float2x2"), { Generator_MathMultiplyFloat2Float2x2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float2x2_float2"), { Generator_MathMultiplyFloat2x2Float2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_multiply_float2x2_float2x2"), { Generator_MathMultiplyFloat2x2Float2x2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_sin_float"), { Generator_MathSinFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_sin_float2"), { Generator_MathSinFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_sin_float3"), { Generator_MathSinFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_sin_float4"), { Generator_MathSinFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_math_sum"), { Generator_MathSum, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_bitmap"), { Generator_NVIDIADCCSupportAD3DSMaxBitmap, 4 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_bitmap_bump"), { Generator_NVIDIADCCSupportAD3DSMaxBitmapBump, 3 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_bitmap_normalmap"), { Generator_NVIDIADCCSupportAD3DSMaxBitmapNormalMap, 3 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_color_correction"), { Generator_NVIDIADCCSupportAD3DSMaxColorCorrection, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_CompositeTexturemap"), { Generator_NVIDIADCCSupportAD3DSMaxCompositeTextureMap, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_CompositeTexturemap_bump"), { Generator_NVIDIADCCSupportAD3DSMaxCompositeTextureMapBump, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_dent"), { Generator_NVIDIADCCSupportAD3DSMaxDent, 2 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_dent_bump"), { Generator_NVIDIADCCSupportAD3DSMaxDentBump, 2 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_falloff"), { Generator_NVIDIADCCSupportAD3DSMaxFalloff, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_mapmode"), { Generator_NVIDIADCCSupportAD3DSMaxMapMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_Mix"), { Generator_NVIDIADCCSupportAD3DSMaxMix, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_noise"), { Generator_NVIDIADCCSupportAD3DSMaxNoise, 2 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_noise_bump"), { Generator_NVIDIADCCSupportAD3DSMaxNoiseBump, 2 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_ad_3dsmax_StandardTextureOutput"), { Generator_NVIDIADCCSupportAD3DSMaxStandardTextureOutput, 2 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_combine_normals"), { Generator_NVIDIADCCSupportCombineNormals, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_max_rotation_translation_scale"), { Generator_NVIDIADCCSupportMaxRotationTranslationScale, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_transform_internal_to_tangent"), { Generator_NVIDIADCCSupportTransformInternalToTangent, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_DCC_support_transform_tangent_to_internal"), { Generator_NVIDIADCCSupportTransformTangentToInternal, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_add_detail_normal_float3_float3"), { Generator_NVIDIADistillingSupportAddDetailNormal, 3 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_average_float_float_float_float"), { Generator_NVIDIADistillingSupportAverageFloatFloatFloatFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_average_float_color_float_color"), { Generator_NVIDIADistillingSupportAverageFloatColorFloatColor, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_average_float_float_float_float_float_float"), { Generator_NVIDIADistillingSupportAverageFloatFloatFloatFloatFloatFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_average_float_color_float_color_float_color"), { Generator_NVIDIADistillingSupportAverageFloatColorFloatColorFloatColor, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_combine_anisotropic_roughness_float_float"), { Generator_NVIDIADistillingSupportCombineAnisotropicRoughness, 2 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_combine_normals_float_float3_float_float3"), { Generator_NVIDIADistillingSupportCombineNormals, 3 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_affine_normal_sum_float_float3"), { Generator_NVIDIADistillingSupportAffineNormalSumFloatFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_affine_normal_sum_float_float3_float_float3"), { Generator_NVIDIADistillingSupportAffineNormalSumFloatFloat3FloatFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_affine_normal_sum_float_float3_float_float3_float_float3"), { Generator_NVIDIADistillingSupportAffineNormalSumFloatFloat3FloatFloat3FloatFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_directional_coloring_color_color_float"), { Generator_NVIDIADistillingSupportDirectionalColoring, 3 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_directional_weighting_float_float_float"), { Generator_NVIDIADistillingSupportDirectionalWeighting, 3 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_part_normalized_float_float_float"), { Generator_NVIDIADistillingSupportPartNormalized, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_refl_from_ior_color"), { Generator_NVIDIADistillingSupportReflFromIORColor, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_refl_from_ior_float"), { Generator_NVIDIADistillingSupportReflFromIORFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_nvidia_distilling_support_refl_from_ior_k_color_color"), { Generator_NVIDIADistillingSupportReflFromIORKColorColor, 2 });
    FunctionGenerators.Add(TEXT("mdl_state_animation_time"), { Generator_StateAnimationTime, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_direction"), { Generator_StateDirection, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_normal"), { Generator_StateGeometryNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_normal_x"), { Generator_StateGeometryNormalX, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_normal_y"), { Generator_StateGeometryNormalY, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_normal_z"), { Generator_StateGeometryNormalZ, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_u"), { Generator_StateGeometryTangentU, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_u_x"), { Generator_StateGeometryTangentUX, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_u_y"), { Generator_StateGeometryTangentUY, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_u_z"), { Generator_StateGeometryTangentUZ, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_v"), { Generator_StateGeometryTangentV, 2 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_v_x"), { Generator_StateGeometryTangentVX, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_v_y"), { Generator_StateGeometryTangentVY, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_geometry_tangent_v_z"), { Generator_StateGeometryTangentVZ, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_meters_per_scene_unit"), { Generator_StateMetersPerSceneUnit, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_normal"), { Generator_StateNormal, 3 });
    FunctionGenerators.Add(TEXT("mdl_state_object_id"), { Generator_StateObjectId, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_position"), { Generator_StatePosition, 2 });
    FunctionGenerators.Add(TEXT("mdl_state_scene_units_per_meter"), { Generator_StateSceneUnitsPerMeter, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_tangent_space"), { Generator_StateTangentSpace, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_coordinate"), { Generator_StateTextureCoordinate, 4 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_space_max"), { Generator_StateTextureSpaceMax, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_u"), { Generator_StateTextureTangentU, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_u_x"), { Generator_StateTextureTangentUX, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_u_y"), { Generator_StateTextureTangentUY, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_u_z"), { Generator_StateTextureTangentUZ, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_v"), { Generator_StateTextureTangentV, 3 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_v_x"), { Generator_StateTextureTangentVX, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_v_y"), { Generator_StateTextureTangentVY, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_texture_tangent_v_z"), { Generator_StateTextureTangentVZ, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_transform_point"), { Generator_StateTransformPoint, 1 });
    FunctionGenerators.Add(TEXT("mdl_state_transform_vector"), { Generator_StateTransformVector, 2 });
    FunctionGenerators.Add(TEXT("mdl_state_transform_normal"), { Generator_StateTransformVector, 1 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_color_2d"), { Generator_Tex2DLookupFloat3, 8 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float_2d"), { Generator_Tex2DLookupFloat, 3 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float2_2d"), { Generator_Tex2DLookupFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float3_2d"), { Generator_Tex2DLookupFloat3, 8 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float4_2d"), { Generator_Tex2DLookupFloat4, 8 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_color_normal"), { Generator_TexNormalLookupFloat3, 2 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float_normal"), { Generator_TexNormalLookupFloat, 2 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float2_normal"), { Generator_TexNormalLookupFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float3_normal"), { Generator_TexNormalLookupFloat3, 2 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float4_normal"), { Generator_TexNormalLookupFloat4, 2 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_color_cube"), { Generator_TexCubeLookupFloat3, 7 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float_cube"), { Generator_TexCubeLookupFloat, 2 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float2_cube"), { Generator_TexCubeLookupFloat2, 1 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float3_cube"), { Generator_TexCubeLookupFloat3, 7 });
    FunctionGenerators.Add(TEXT("mdl_tex_lookup_float4_cube"), { Generator_TexCubeLookupFloat4, 7 });
    FunctionGenerators.Add(TEXT("mdl_tex_texture_isvalid"), { Generator_TexTextureIsValid, 1 });
    FunctionGenerators.Add(TEXT("mdl_scene_data_lookup_float"), { Generator_SceneDataLookupFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_scene_data_lookup_float3"), { Generator_SceneDataLookupFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_scene_data_lookup_float4"), { Generator_SceneDataLookupFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_scene_data_lookup_color"), { Generator_SceneDataLookupFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_add_detail_normal"), { Generator_ImporterAddDetailNormal, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_apply_noise_modifications"), { Generator_ImporterApplyNoiseModifications, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_blend_clear_coat"), { Generator_ImporterBlendClearCoat, 4 });
    FunctionGenerators.Add(TEXT("mdlimporter_blend_colors"), { Generator_ImporterBlendColors, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_calculate_hue"), { Generator_ImporterCalculateHue, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_calculate_saturation"), { Generator_ImporterCalculateSaturation, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_compute_cubic_transform"), { Generator_ImporterComputeCubicTransform, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_compute_cylindric_transform"), { Generator_ImporterComputeCylindricTransform, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_compute_spheric_projection"), { Generator_ImporterComputeSphericProjection, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_compute_spheric_transform"), { Generator_ImporterComputeSphericTransform, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_compute_tangents"), { Generator_ImporterComputeTangents, 3 });
    FunctionGenerators.Add(TEXT("mdlimporter_compute_tangents_transformed"), { Generator_ImporterComputeTangentsTransformed, 3 });
    FunctionGenerators.Add(TEXT("mdlimporter_eval_checker"), { Generator_ImporterEvalChecker, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_eval_tile_function"), { Generator_ImporterEvalTileFunction, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_flow_noise"), { Generator_ImporterFlowNoise, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_grad_flow"), { Generator_ImporterGradFlow, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_hsv_to_rgb"), { Generator_ImporterHSVToRGB, 1 });
    //FunctionGenerators.Add(TEXT("mdlimporter_interpolate_texspace"), { Generator_ImporterInterpolateTexspace, 3 });
    FunctionGenerators.Add(TEXT("mdlimporter_mono_mode"), { Generator_ImporterMonoMode, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_mi_noise"), { Generator_ImporterMiNoise, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_perlin_noise"), { Generator_ImporterPerlinNoise, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_permute_flow"), { Generator_ImporterPermuteFlow, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_refract"), { Generator_ImporterRefract, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_select_bsdf"), { Generator_ImporterSelectBSDF, 2 });
#if defined(ADD_CLIP_MASK)
    FunctionGenerators.Add(TEXT("mdlimporter_set_clip_mask"), { Generator_ImporterSetClipMask, 1 });
#endif
    FunctionGenerators.Add(TEXT("mdlimporter_set_refraction"), { Generator_ImporterSetRefraction, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_set_subsurface_color"), { Generator_ImporterSetSubsurfaceColor, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_summed_flow_noise"), { Generator_ImporterSummedFlowNoise, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_summed_perlin_noise"), { Generator_ImporterSummedPerlinNoise, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_texremapu1"), { Generator_ImporterTexremapu1, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_texremapu2"), { Generator_ImporterTexremapu2, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_texture2d_sample"), { Generator_ImporterTexture2DSample, 2 });
    FunctionGenerators.Add(TEXT("mdlimporter_texturecube_sample"), { Generator_ImporterTextureCubeSample, 1 });
#if defined(USE_WORLD_ALIGNED_TEXTURES)
    FunctionGenerators.Add(TEXT("mdlimporter_world_aligned_texture_float3"), { Generator_ImporterWorldAlignedTextureFloat3, 3 });
    FunctionGenerators.Add(TEXT("mdlimporter_world_aligned_texture_float4"), { Generator_ImporterWorldAlignedTextureFloat4, 3 });
#endif
    FunctionGenerators.Add(TEXT("mdlimporter_worley_noise"), { Generator_ImporterWorleyNoise, 1 });
    FunctionGenerators.Add(TEXT("mdlimporter_worley_noise_ext"), { Generator_ImporterWorleyNoiseExt, 1 });

    // -------------------------------- For legacy mdl ----------------------------------------------------------
    FunctionGenerators.Add(TEXT("mdl_Ue4basedMDL_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Translucent_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Subsurface_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_unpack_normal_map"), { Generator_UE4TextureLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_greyscale_texture_lookup"), { Generator_UE4TextureLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_pixel_normal_world_space"), { Generator_UE4PixelNormalWS, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_vertex_normal_world_space"), { Generator_UE4VertexNormalWS, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_fresnel"), { Generator_UE4Fresnel, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_fresnel_function"), { Generator_UE4FresnelFunction, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_camera_vector"), { Generator_UE4CameraVector, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_transform_vector_from_tangent_to_world"), { Generator_UE4TransformTangentToWorld, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_transform_vector_from_world_to_tangent"), { Generator_UE4TransformWorldToTangent, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_pixel_depth"), { Generator_UE4PixelDepth, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_vertex_color"), { Generator_UE4VertexColor, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_camera_position"), { Generator_UE4CameraPosition, 1 });
    FunctionGenerators.Add(TEXT("mdl_Ue4basedMDL_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Translucent_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Unlit_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Subsurface_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_reflection_vector"), { Generator_UE4ReflectionVector, 2 });
    FunctionGenerators.Add(TEXT("mdl_Ue4Function_dither_temporalAA"), { Generator_UE4DitherTemporalAA, 1 });
    // ------------------------------------- deprecated ---------------------------------------------------------- 
    FunctionGenerators.Add(TEXT("mdl_Ue4basedMDL_unpack_normal_map"), { Generator_UE4TextureLookup, 1 });

    // -----------------------------------------------------------------------------------------------------------

    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_unpack_normal_map"), { Generator_UE4UnpackNormalmap, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_greyscale_texture_lookup"), { Generator_UE4TextureLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_pixel_normal_world_space"), { Generator_UE4PixelNormalWS, 2 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_vertex_normal_world_space"), { Generator_UE4VertexNormalWS, 2 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_fresnel"), { Generator_UE4Fresnel, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_fresnel_function"), { Generator_UE4FresnelFunction, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_camera_vector"), { Generator_UE4CameraVector, 2 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_transform_vector_from_tangent_to_world"), { Generator_UE4TransformTangentToWorld, 3 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_transform_vector_from_world_to_tangent"), { Generator_UE4TransformWorldToTangent, 3 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_pixel_depth"), { Generator_UE4PixelDepth, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_vertex_color"), { Generator_UE4VertexColor, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_camera_position"), { Generator_UE4CameraPosition, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_reflection_vector"), { Generator_UE4ReflectionVector, 2 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_dither_temporalAA"), { Generator_UE4DitherTemporalAA, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_vertex_color_from_coordinate"), { Generator_UE4VertexColor2, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_smoothstep"), { Generator_UE4Smoothstep, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_noise"), { Generator_UE4Noise, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_vector3_noise"), { Generator_UE4VectorNoise, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_vector4_noise"), { Generator_UE4Vector4Noise, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_black_body"), { Generator_UE4BlackBody, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_per_instance_random"), { Generator_UE4PerInstanceRandom, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_convert_to_left_hand"), { Generator_UE4ConvertToLH, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4FunctionExtension17_unpack_integer_as_float"), { Generator_UE4UnpackIntegerAsFloat, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_local_object_bounds_min"), { Generator_UE4LocalObjectBoundsMin, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_local_object_bounds_max"), { Generator_UE4LocalObjectBoundsMax, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_object_bounds"), { Generator_UE4ObjectBounds, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_object_radius"), { Generator_UE4ObjectRadius, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_object_world_position"), { Generator_UE4ObjectWorldPosition, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Function_object_orientation"), { Generator_UE4ObjectOrientation, 1 });

    FunctionGenerators.Add(TEXT("mdl_OmniUe4Base_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Base_world_space_normal"), { Generator_UE4WorldSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Base_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 1 });

    FunctionGenerators.Add(TEXT("mdl_OmniUe4Translucent_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Translucent_world_space_normal"), { Generator_UE4WorldSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Translucent_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Translucent_get_translucent_tint"), { Generator_UE4TranslucentGetTint, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Translucent_get_emissive_intensity"), { Generator_UE4TranslucentGetTint, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Translucent_get_translucent_opacity"), { Generator_UE4OpacityWeight, 1 });

    FunctionGenerators.Add(TEXT("mdl_OmniUe4Subsurface_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Subsurface_world_space_normal"), { Generator_UE4WorldSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Subsurface_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Subsurface_get_subsurface_weight"), { Generator_UE4SubsurfaceWeight, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Subsurface_get_subsurface_color"), { Generator_UE4SubsurfaceColor, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniUe4Subsurface_get_subsurface_opacity"), { Generator_UE4SubsurfaceOpacity, 1 });

    FunctionGenerators.Add(TEXT("mdl_OmniUe4Unlit_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 1 });
    // -------------------------------- For copying function from ue4 mdl ----------------------------------------------------------
    FunctionGenerators.Add(TEXT("mdl_tangent_space_normal"), { Generator_UE4TangentSpaceNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_emissive_multiplier"), { Generator_UE4EmissiveMultiplier, 1 });
    FunctionGenerators.Add(TEXT("mdl_vertex_color_from_coordinate"), { Generator_UE4VertexColor2, 1 });

    //--------------------------------- OmniPBRBase ---------------------------------------------------------------
    FunctionGenerators.Add(TEXT("mdl_OmniPBRBase_tangent_space_normal_roughness_texture"), { Generator_OmniPBRBaseTangentspaceNormalRoughnessTexture, 1 });

    //--------------------------------- OmniSurface ---------------------------------------------------------------
    FunctionGenerators.Add(TEXT("mdl_OmniImage_native_wrap_mode"), { Generator_OmniImageNativeWrapMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_compute_texture_coordinate_2"), { Generator_OmniImageComputeTextureCoordinate, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_texture_lookup_2"), { Generator_OmniImageTextureLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_texture_alpha_channel_remap_2"), { Generator_OmniImageTextureAlphaChannelRemap, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_tangent_space_normal_lookup_2"), { Generator_OmniImageTangentSpaceNormalLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_normal_mapping"), { Generator_OmniImageNormalMapping, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_compute_latitude_longitude_projection"), { Generator_OmniImageComputeLatitudeLongitudeProjection, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniImage_tangent_space_normal_roughness_lookup_2"), { Generator_OmniImageTangentspaceNormalRoughnessLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniShared_ior_preset_to_value"), { Generator_OmniSharedIorPresetToValue, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniShared_scattering_colors_preset_to_value"), { Generator_OmniSharedScatteringColorsPresetToValue, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniShared_displacement_adjustment"), { Generator_OmniSharedDisplacementAdjustment, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniSurface_OmniShared_melanin_concentration_preset_to_value"), { Generator_OmniSharedMelaninConcentrationPresetToValue, 1 });
    
    FunctionGenerators.Add(TEXT("mdl_OmniImageNN_texture_lookup_2"), { Generator_OmniImageTextureLookup2, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniImageNN_normal_lookup_2"), { Generator_OmniImageNormalLookup2, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniImageNN_texture_lookup_2_smooth"), { Generator_OmniImageTextureLookup2Smooth, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniImageNN_texture_lookup_wrap"), {Generator_OmniImageTextureLookupWrap, 1});
    FunctionGenerators.Add(TEXT("mdl_OmniImageNN_texture_lookup_nn"), {Generator_OmniImageTextureLookupNN, 1});
    FunctionGenerators.Add(TEXT("mdl_OmniImageNN_normal_mapping"), {Generator_OmniImageNormalMapping, 1});

    // -------------------------------- For Aperture mdl ----------------------------------------------------------
    FunctionGenerators.Add(TEXT("mdl_AperturePBR_Opacity_raw_file_texture"), { Generator_ApertureRawFileTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_AperturePBR_ThinOpaque_raw_file_texture"), { Generator_ApertureRawFileTexture, 2 });
    FunctionGenerators.Add(TEXT("mdl_AperturePBR_ThinTranslucent_raw_file_texture"), { Generator_ApertureRawFileTexture, 2 });

    // -------------------------------- OmniReallusion mdl ---------------------------------------------------------
    FunctionGenerators.Add(TEXT("mdl_OmniRL_PBR_hue"), { Generator_ReallusionHue, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_PBR_hue"), { Generator_ReallusionHue, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniRL_PBR_saturation"), { Generator_ReallusionSaturation, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_PBR_saturation"), { Generator_ReallusionSaturation, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniRL_PBR_hsv_to_rgb"), { Generator_ReallusionHSVToRGB, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_PBR_hsv_to_rgb"), { Generator_ReallusionHSVToRGB, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniRL_PBR_BlendMapByMode"), { Generator_ReallusionBlendMapByMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_PBR_BlendMapByMode"), { Generator_ReallusionBlendMapByMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniRL_SSS_BlendMapByMode"), { Generator_ReallusionBlendMapByMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_SSS_BlendMapByMode"), { Generator_ReallusionBlendMapByMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniRL_SSS_adjust_opacity_RL"), { Generator_ReallusionAdjustOpacity, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_SSS_adjust_opacity_RL"), { Generator_ReallusionAdjustOpacity, 1 });	
    FunctionGenerators.Add(TEXT("mdl_OmniRL_DHHead_BlendMapByMode"), { Generator_ReallusionBlendMapByMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusion_DHHead_BlendMapByMode"), { Generator_ReallusionBlendMapByMode, 1 });	
    FunctionGenerators.Add(TEXT("mdl_OmniRLEye_BaseColorMake"), { Generator_ReallusionBaseColorMake, 1 });
    FunctionGenerators.Add(TEXT("mdl_OmniReallusionEye_BaseColorMake"), { Generator_ReallusionBaseColorMake, 1 });	

    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_prepare_tex_float4"), { Generator_GltfPrepareTexFloat4, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_prepare_tex_float3"), { Generator_GltfPrepareTexFloat3, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_prepare_normal"), { Generator_GltfPrepareNormal, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_prepare_occlusion"), { Generator_GltfPrepareOcclusion, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_perpare_emission"), { Generator_GltfPrepareEmission, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_khr_texture_transform_apply"), { Generator_GltfTextureTransformApply, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_convert_wrap_mode"), { Generator_GltfConvertWrapMode, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_texture_lookup"), { Generator_GltfTextureLookup, 1 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_gltf_normal_texture_lookup"), { Generator_GltfNormalTextureLookup, 2 });
    FunctionGenerators.Add(TEXT("mdl_gltf_pbr_compute_opacity"), { Generator_GltfComputeOpacity, 1 });
}

FMDLFunctionGenerator::~FMDLFunctionGenerator()
{
    // everything's already down here, regarding FunctionFactory, so don't try to RemoveFromRoot()!
}

void FMDLFunctionGenerator::UpdateFunction(UMaterialFunction* Function, const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes)
{
    FunctionGenerationData* FGD = FunctionGenerators.Find(AssetName);
    if (FGD)
    {
        UMaterialFunction* OriginalFunction = (UMaterialFunction*)StaticDuplicateObject(Function, GetTransientPackage(), NAME_None, ~RF_Standalone, UMaterialFunction::StaticClass()); 

        UPackage* Package = Function->GetOutermost();
        FString FunctionName = AssetName;
        for (auto ArraySize : ArrayInputSizes)
        {
            if (0 < ArraySize)
            {
                FunctionName += TEXT("_") + FString::FromInt(ArraySize);
            }
        }
        Function->FunctionExpressions.Empty();
        FGD->Generator(Function, ArrayInputSizes);
        Function->Description += TEXT("\nVersion ") + FString::FromInt(FGD->Version);

        for (int32 ExpressionIndex = 0; ExpressionIndex < Function->FunctionExpressions.Num(); ExpressionIndex++)
        {
            UMaterialExpression* CurrentExpression = Function->FunctionExpressions[ExpressionIndex];
            UMaterialExpressionFunctionOutput* OutputExpression = Cast<UMaterialExpressionFunctionOutput>(CurrentExpression);
            UMaterialExpressionFunctionInput* InputExpression = Cast<UMaterialExpressionFunctionInput>(CurrentExpression);

            // Restore original Input and Output Id
            if (InputExpression)
            {
                for (int32 OriginalExpressionIndex = 0; OriginalExpressionIndex < OriginalFunction->FunctionExpressions.Num(); OriginalExpressionIndex++)
                {
                    UMaterialExpressionFunctionInput* OriginalInputExpression = Cast<UMaterialExpressionFunctionInput>(OriginalFunction->FunctionExpressions[OriginalExpressionIndex]);

                    if (OriginalInputExpression
                    && OriginalInputExpression->InputName.IsEqual(InputExpression->InputName, ENameCase::CaseSensitive)
                    && OriginalInputExpression->InputType == InputExpression->InputType)
                    {
                        InputExpression->Id = OriginalInputExpression->Id;
                    }
                }
            }
            else if (OutputExpression)
            {
                for (int32 OriginalExpressionIndex = 0; OriginalExpressionIndex < OriginalFunction->FunctionExpressions.Num(); OriginalExpressionIndex++)
                {
                    UMaterialExpressionFunctionOutput* OriginalOutputExpression = Cast<UMaterialExpressionFunctionOutput>(OriginalFunction->FunctionExpressions[OriginalExpressionIndex]);

                    if (OriginalOutputExpression && OriginalOutputExpression->OutputName.IsEqual(OutputExpression->OutputName, ENameCase::CaseSensitive))
                    {
                        OutputExpression->Id = OriginalOutputExpression->Id;
                    }
                }
            }
        }

        OriginalFunction = nullptr;

        for (int32 ExpressionIndex = 0; ExpressionIndex < Function->FunctionExpressions.Num(); ExpressionIndex++)
        {
            UMaterialExpression* CurrentExpression = Function->FunctionExpressions[ExpressionIndex];
            ensureMsgf(CurrentExpression, TEXT("Invalid expression at index [%i] whilst saving material function."), ExpressionIndex);

            // Link the expressions back to their function
            if (CurrentExpression)
            {
                CurrentExpression->Material = nullptr;
                CurrentExpression->Function = Function;
            }	
        }

        // Arrange editor nodes
        TArray<UMaterialExpression*> OutputNodes = Function->FunctionExpressions.FilterByPredicate(
            [](UMaterialExpression* CurrentExpression)
        {
            return CurrentExpression->IsA<UMaterialExpressionFunctionOutput>();
        });
        ArrangeNodes(Function, OutputNodes);

        UMaterialEditingLibrary::UpdateMaterialFunction(Function, nullptr);

        const UMDLSettings* Settings = GetDefault<UMDLSettings>();
        if (Settings && Settings->bAutoSaveMaterialFunctions)
        {
            FString FilePath;// Save to disk
            if (FPackageName::TryConvertLongPackageNameToFilename(AssetPath / FunctionName, FilePath, FPackageName::GetAssetPackageExtension()))
            {
                UPackage::SavePackage(Package, Function, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
            }
        }
    }
}

UMaterialFunction* FMDLFunctionGenerator::GenerateFunction(const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes)
{
    UMaterialFunction * Function = nullptr;
    FunctionGenerationData* FGD = FunctionGenerators.Find(AssetName);
    if (FGD)
    {
        FString FunctionName = AssetName;
        for (auto ArraySize : ArrayInputSizes)
        {
            if (0 < ArraySize)
            {
                FunctionName += TEXT("_") + FString::FromInt(ArraySize);
            }
        }

        UPackage* Package = CreatePackage(*(AssetPath / FunctionName));

        Function = dynamic_cast<UMaterialFunction*>(FunctionFactory->FactoryCreateNew(UMaterialFunction::StaticClass(), Package, *FunctionName, RF_Public | RF_Standalone, nullptr, GWarn));
        check(Function);
        Function->StateId = FGuid::NewGuid();

        FGD->Generator(Function, ArrayInputSizes);
        Function->Description += TEXT("\nVersion ") + FString::FromInt(FGD->Version);

        // Arrange editor nodes
        TArray<UMaterialExpression*> OutputNodes = Function->FunctionExpressions.FilterByPredicate(
            [](UMaterialExpression* CurrentExpression)
        {
            return CurrentExpression->IsA<UMaterialExpressionFunctionOutput>();
        });
        ArrangeNodes(Function, OutputNodes);

        Function->PostLoad();

        FAssetRegistryModule::AssetCreated(Function);		// make sure, this asset is listed in the Content Browser

        const UMDLSettings* Settings = GetDefault<UMDLSettings>();
        if (Settings && Settings->bAutoSaveMaterialFunctions)
        {
            FString FilePath;// Save to disk
            if (FPackageName::TryConvertLongPackageNameToFilename(AssetPath / FunctionName, FilePath, FPackageName::GetAssetPackageExtension()))
            {
                UPackage::SavePackage(Package, Function, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
            }
        }
    }
    return Function;
}

int32 FMDLFunctionGenerator::GetVersion(const FString& AssetName)
{
    FunctionGenerationData* FGD = FunctionGenerators.Find(AssetName);
    return FGD ? FGD->Version : -1;
}

static int32 GetVersion(UMaterialFunction* Function)
{
    int32 Version = -1;
    int32 VersionPosition = Function->Description.Find(TEXT("\nVersion "), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
    if (0 <= VersionPosition)
    {
        check(VersionPosition + 9 < Function->Description.Len());
        FString VersionString = Function->Description.RightChop(VersionPosition + 9);
        Version = FCString::Atoi(*VersionString);
    }
    return Version;
}

UMaterialFunction* LoadFunction(const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes)
{
    static TMap<FString, UMaterialFunction*>	LoadedFunctions;
    static FMDLFunctionGenerator FunctionGenerator;

    FString FunctionName = AssetName;
    for (auto ArraySize : ArrayInputSizes)
    {
        if (0 < ArraySize)
        {
            FunctionName += TEXT("_") + FString::FromInt(ArraySize);
        }
    }

    UMaterialFunction* Function = nullptr;
    UMaterialFunction** FunctionPtr = LoadedFunctions.Find(FunctionName);
    if (FunctionPtr && (*FunctionPtr)->IsValidLowLevel())
    {
        Function = *FunctionPtr;
    }
    else
    {
        Function = LoadObject<UMaterialFunction>(nullptr, *(AssetPath / FunctionName), nullptr, LOAD_EditorOnly | LOAD_NoWarn, nullptr);
    }

    if (Function && (GetVersion(Function) < FunctionGenerator.GetVersion(AssetName)))
    {
        FunctionGenerator.UpdateFunction(Function, AssetPath, AssetName, ArrayInputSizes);
    }
    
    if (!Function)
    {
        Function = FunctionGenerator.GenerateFunction(AssetPath, AssetName, ArrayInputSizes);
        if (Function)
        {
            LoadedFunctions.Add(AssetName, Function);
        }
    }

    return Function;
}

#endif