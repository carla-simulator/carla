// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#if WITH_MDL_SDK

#include "MDLMaterialImporter.h"
#include "MDLOutputLogger.h"
#include "MDLMaterialFactory.h"
#include "MDLSettings.h"
#include "MDLKeywords.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Containers/ArrayBuilder.h"
#include "NodeArrangement.h"
#include "UObject/UObjectGlobals.h"
#include "MDLPrivateModule.h"
#include "MDLImporterUtility.h"
#include "MDLPathUtility.h"

using namespace MDLPathUtility;

#define CheckConnection(Connection)	\
{ \
    if (Connection.Num() == 0) \
    { \
        FMDLImporterUtility::ClearMaterial(CurrentUE4Material); \
        Temporaries.Empty(); \
        return false; \
    } \
}

FMDLMaterialImporter::FMDLMaterialImporter(UMDLMaterialFactory* Factory)
    : MDLModule(FModuleManager::GetModulePtr<FMDLModule>("MDL"))
    , ParentPackage(nullptr)
    , ParentFactory(Factory)
    , CurrentUE4Material(nullptr)
    , CurrentClearCoatNormal(nullptr)
    , MakeFloat2(::LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat2.MakeFloat2")))
    , MakeFloat3(::LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat3.MakeFloat3")))
    , MakeFloat4(::LoadFunction(TEXT("/Engine/Functions/Engine_MaterialFunctions02/Utility"), TEXT("MakeFloat4.MakeFloat4")))
    , InGeometryExpression(false)
    , LoadTextureCallback(nullptr)
{
    if (ParentFactory == nullptr)
    {
        ParentFactory = GetDefault<UMDLMaterialFactory>();
    }
}

mi::base::Handle<const mi::neuraylib::IExpression> GetField(mi::base::Handle<const mi::neuraylib::ICompiled_material> const& CompiledMaterial, std::string const& Name)
{
    mi::base::Handle<const mi::neuraylib::IExpression_direct_call> Body(CompiledMaterial->get_body());
    mi::base::Handle<const mi::neuraylib::IExpression_list> Arguments(Body->get_arguments());
    return mi::base::make_handle(Arguments->get_expression(Name.c_str()));
}

static bool CheckClearCodeCondition(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial,
                                    const mi::base::Handle<const mi::neuraylib::IExpression>& Expression, bool EncounteredClearCodeLayer);

static bool CheckClearCodeCondition(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial,
                                    const mi::base::Handle<const mi::neuraylib::IExpression_list>& Expression, bool EncounteredClearCodeLayer)
{
    for (mi::Size i = 0; i < Expression->get_size(); i++)
    {
        const char* Name = Expression->get_name(i);
        if (CheckClearCodeCondition(CompiledMaterial, mi::base::make_handle(Expression->get_expression(i)), EncounteredClearCodeLayer))
        {
            return true;
        }
    }
    return false;
}

static bool CheckClearCodeCondition(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial,
                                    const mi::base::Handle<const mi::neuraylib::IExpression_direct_call>& Expression, bool EncounteredClearCodeLayer)
{
    FString Definition(Expression->get_definition());
    if (Definition.StartsWith(TEXT("mdl::df::custom_curve_layer")) || Definition.StartsWith(TEXT("mdl::df::fresnel_layer")))
    {
        if (EncounteredClearCodeLayer)
        {
            return true;
        }
        else
        {
            EncounteredClearCodeLayer = true;
        }
    }
    return CheckClearCodeCondition(CompiledMaterial, mi::base::make_handle(Expression->get_arguments()), EncounteredClearCodeLayer);
}

static bool CheckClearCodeCondition(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial,
                                    const mi::base::Handle<const mi::neuraylib::IExpression_temporary>& Expression, bool EncounteredClearCodeLayer)
{
    return CheckClearCodeCondition(CompiledMaterial, mi::base::make_handle(CompiledMaterial->get_temporary(Expression->get_index())), EncounteredClearCodeLayer);
}

static bool CheckClearCodeCondition(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial,
                                    const mi::base::Handle<const mi::neuraylib::IExpression>& Expression, bool EncounteredClearCodeLayer)
{
    mi::neuraylib::IExpression::Kind Kind = Expression->get_kind();
    switch (Kind)
    {
        case mi::neuraylib::IExpression::EK_CONSTANT:
        case mi::neuraylib::IExpression::EK_PARAMETER:
            return false;		// Constants and Parameter will never fullfill a ClearCoat condition!
        case mi::neuraylib::IExpression::EK_DIRECT_CALL:
            return CheckClearCodeCondition(CompiledMaterial, Expression.get_interface<const mi::neuraylib::IExpression_direct_call>(), EncounteredClearCodeLayer);
        case mi::neuraylib::IExpression::EK_TEMPORARY:
            return CheckClearCodeCondition(CompiledMaterial, Expression.get_interface<const mi::neuraylib::IExpression_temporary>(), EncounteredClearCodeLayer);
    }
    check(false);
    return false;
}

static bool IsTransparentMode(FMaterialExpressionConnection const& Input)
{
    // Note: for ScalarParameter controlling transparency, we're lost. Whatever we'd do is wrong. So just take the default value as the indicator for transparency
    check(Input.ConnectionType == EConnectionType::Expression);
    check(!Input.ExpressionData.Expression || Input.ExpressionData.Expression->IsA<UMaterialExpressionConstant>() || Input.ExpressionData.Expression->IsA<UMaterialExpressionScalarParameter>());
    return Input.ExpressionData.Expression
        && ((Input.ExpressionData.Expression->IsA<UMaterialExpressionConstant>() && (Cast<UMaterialExpressionConstant>(Input.ExpressionData.Expression)->R != 0))
            || (Input.ExpressionData.Expression->IsA<UMaterialExpressionScalarParameter>() && (Cast<UMaterialExpressionScalarParameter>(Input.ExpressionData.Expression)->DefaultValue != 0)));
}

static void SetTransparency(UMaterial* Material)
{
    Material->BlendMode = BLEND_Translucent;
    Material->TranslucencyLightingMode = TLM_Surface;
    Material->bCastRayTracedShadows = false;
}

UMaterial* FMDLMaterialImporter::CreateMaterial(UObject* InParent, FName InName, EObjectFlags Flags, FFeedbackContext* Warn) const
{
    return (UMaterial*)ParentFactory->MaterialFactory->FactoryCreateNew(UMaterial::StaticClass(), InParent, InName, Flags, nullptr, Warn);
}

bool FMDLMaterialImporter::ImportMaterial(UMaterial* Material,
    const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& InCompiledMaterial, UMaterialExpressionClearCoatNormalCustomOutput*& OutClearCoatNormalCustomOutput, FLoadTextureCallback InCallback)
{
    LoadTextureCallback = InCallback;
    ParentPackage = Material->GetOuter();
    PackagePath = ParentPackage->GetName().Replace(TEXT("/Game/"), *FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()));
    ImportErrors.Empty();
    OutClearCoatNormalCustomOutput = nullptr;

    CurrentClearCoatNormal = nullptr;
    TranslucentOpacity = FMaterialExpressionConnection();
    EmissiveOpacity = FMaterialExpressionConnection();
    SubsurfaceColor = FMaterialExpressionConnection();
    SubsurfaceOpacity = FMaterialExpressionConnection();
    OpacityEnabled = FMaterialExpressionConnection();

    // Create Unreal Engine material and translate expressions
    CurrentUE4Material = Material;
    check(CurrentUE4Material != nullptr);

    // handle MaterialDefinition->get_annotations() !

    const UMDLSettings* Settings = GetDefault<UMDLSettings>();
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UseWorldAlignedTextureParameter = NewMaterialExpressionScalarParameter(CurrentUE4Material, TEXT("World-Aligned Textures"), Settings->bWorldAlignedTextures);
    Cast<UMaterialExpressionParameter>(UseWorldAlignedTextureParameter.ExpressionData.Expression)->Group = TEXT("Texture Projection");
# else
    UseWorldAlignedTextureParameter = NewMaterialExpressionStaticBoolParameter(CurrentUE4Material, "World-Aligned Textures", Settings->bWorldAlignedTextures, "Texture Projection");
# endif
#endif

    ImportParameters(InMaterialDefinition, InCompiledMaterial);
    // Temporaries are imported on demand, as they might change after having read the geometry part
    Temporaries.SetNum(InCompiledMaterial->get_temporary_count());

    // Build expression trees
    // For the geometry part, use the geometry_normal, instead of the normal, to prevent issues with shader domains!
    CurrentNormalExpression = NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_geometry_normal")), {});
    InGeometryExpression = true;
    TArray<FMaterialExpressionConnection> GeometryExpression = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "geometry"), "geometry");
    CheckConnection(GeometryExpression);
    InGeometryExpression = false;
    check(GeometryExpression.Num() == 3); // { displacement, cutout_opacity, normal }

    if (CurrentNormalExpression != GeometryExpression[2])
    {
        // if the normal has changed, clean-up the temporaries, they are rebuild on next usage, potentially different
        for (TArray<FMaterialExpressionConnection>& Temporary : Temporaries)
        {
            Temporary.Empty();
        }
        // from now on, the normal expression is what we've got out of the Geometry field!
        CurrentNormalExpression = GeometryExpression[2];
    }
    else
    {
        // Otherwise, use the standard normal from now on
        CurrentNormalExpression = NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal")), {});
    }

    const TArray<FMaterialExpressionConnection> ThinWalled = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "thin_walled"), "thin_walled");
    CheckConnection(ThinWalled);
    check(ThinWalled[0].ConnectionType == EConnectionType::Expression);
    // Hardcode TwoSided to true for now
    //CurrentUE4Material->TwoSided = EvaluateBool(ThinWalled[0]);
    CurrentUE4Material->TwoSided = true;

    const TArray<FMaterialExpressionConnection> SurfaceExpression = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "surface"), "surface");
    CheckConnection(SurfaceExpression);
    check(SurfaceExpression.Num() == 4); // { scattering, { emission, intensity, mode } }

    const TArray<FMaterialExpressionConnection> BackfaceExpression = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "backface"), "backface");
    CheckConnection(BackfaceExpression);
    check(BackfaceExpression.Num() == 4); // { scattering, { emission, intensity, mode } }

    const TArray<FMaterialExpressionConnection> IORExpression = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "ior"), "ior");
    CheckConnection(IORExpression);

    const TArray<FMaterialExpressionConnection> Volume = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "volume"), "volume");
    CheckConnection(Volume);
    check(Volume.Num() == 4); // { scattering, absorption coefficient, scattering coefficient, emission intensity }

    const TArray<FMaterialExpressionConnection> Hair = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "hair"), "hair");
    CheckConnection(Hair);

    if (CheckClearCodeCondition(InCompiledMaterial, GetField(InCompiledMaterial, "surface"), false))		// ClearCode depends on surface field only !
    {
        CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_ClearCoat);
    }
    else
    {
        CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);
        CurrentClearCoatNormal = nullptr;
    }

    // Connect expression trees to material node
    const FMaterialExpressionConnection& Scattering = SurfaceExpression[0];
    check(Scattering.ConnectionType == EConnectionType::Expression);
    if (Scattering.ExpressionData.IsDefault)
    {
        CurrentUE4Material->MaterialAttributes.Connect(0, NewMaterialExpressionMakeMaterialAttributes(CurrentUE4Material, { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f));
    }
    else
    {
        CurrentUE4Material->MaterialAttributes.Connect(Scattering.ExpressionData.Index, Scattering.ExpressionData.Expression);
    }

    if (!Hair[0].ExpressionData.IsDefault
    && Hair[0].ConnectionType == EConnectionType::Expression
    && Hair[0].ExpressionData.Expression && !Hair[0].ExpressionData.Expression->IsA<UMaterialExpressionMakeMaterialAttributes>())
    {
        CurrentUE4Material->MaterialAttributes.Connect(0, Hair[0].ExpressionData.Expression);
    }

    if (!SubsurfaceColor.ExpressionData.IsDefault || !SubsurfaceOpacity.ExpressionData.IsDefault)
    {
        CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_Subsurface);
        CurrentUE4Material->BlendMode = BLEND_Opaque;

        CurrentUE4Material->MaterialAttributes.Connect(0,
            NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_OverrideSubsurface")),
            {
                CurrentUE4Material->MaterialAttributes.Expression,
                SubsurfaceColor,
                SubsurfaceOpacity,
                {}
            }));
    }

    if (!TranslucentOpacity.ExpressionData.IsDefault)
    {
        check(TranslucentOpacity.ExpressionData.Expression);
        CurrentUE4Material->MaterialAttributes.Connect(0,
            NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_OverrideOpacity")),
            {
                CurrentUE4Material->MaterialAttributes.Expression,
                TranslucentOpacity,
                {}
            }));
    }

    const FMaterialExpressionConnection& Emission = SurfaceExpression[1];
    check(Emission.ConnectionType == EConnectionType::Expression);
    if (!Emission.ExpressionData.IsDefault)
    {
        const FMaterialExpressionConnection& Intensity = SurfaceExpression[2];
        check(Intensity.ExpressionData.Expression);
        const FMaterialExpressionConnection Emissive =
            NewMaterialExpressionMultiply(CurrentUE4Material,
                FMaterialExpressionConnection(NewMaterialExpressionBreakMaterialAttributes(CurrentUE4Material, Emission), MAI_EmissiveColor),
                NewMaterialExpressionDivide(CurrentUE4Material, Intensity, 2560.0f));
                //(ImporterSettings->FluxDivider == 1.0f) ? Intensity : NewMaterialExpressionDivide(CurrentUE4Material, Intensity, ImporterSettings->FluxDivider));

        CurrentUE4Material->MaterialAttributes.Connect(0,
            NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_Emissive")),
            {
                CurrentUE4Material->MaterialAttributes.Expression,
                Emissive
            }));
    }

    const FMaterialExpressionConnection& Displacement = GeometryExpression[0];
    check(Displacement.ConnectionType == EConnectionType::Expression);
    if (!Displacement.ExpressionData.IsDefault)
    {
        CurrentUE4Material->MaterialAttributes.Connect(0,
            NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_OverrideWorldPositionOffset")),
            {
                CurrentUE4Material->MaterialAttributes.Expression,
                {},
                Displacement
            }));
    }

    const FMaterialExpressionConnection& CutoutOpacity = GeometryExpression[1];
    check(CutoutOpacity.ConnectionType == EConnectionType::Expression);
    if (!CutoutOpacity.ExpressionData.IsDefault && TranslucentOpacity.ExpressionData.IsDefault)
    {
        if (CurrentUE4Material->BlendMode != BLEND_Translucent) 
        {
            CurrentUE4Material->BlendMode = BLEND_Masked;
            CurrentUE4Material->DitherOpacityMask = true;

            CurrentUE4Material->MaterialAttributes.Connect(0,
                NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_OverrideOpacityMask")),
                {
                    CurrentUE4Material->MaterialAttributes.Expression,
                    CutoutOpacity,{}
                }));
        }
        else
        {
            // The opacity_threshold is interpreted as follows:
            // 1. opacity_threshold > 0: binary opacity masking mode
            //   i.e.: if opacity_value > opacity_threshold we remap opacity_value to 1.0, otherwise to 0.0
            // 2. opacity_threshold == 0: fractional opacity mode. Leaves the opacity_value untouched,
            //   allowing the renderer to use fractional cutout_opacity values, if it supports this feature
            //
            // NOTE: when the renderer does not enable 'fractional cutout opacity' it interprets
            // cutout_opacity values of 1.0 as opaque and anything below as fully transparent
            if (!OpacityEnabled.ExpressionData.IsDefault)
            {
                // Opacity disabled, using default
                CurrentUE4Material->MaterialAttributes.Connect(0,
                    NewMaterialExpressionIfEqual(CurrentUE4Material, OpacityEnabled, 1.0f, NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_OverrideOpacity")),
                    {
                        CurrentUE4Material->MaterialAttributes.Expression,
                        CutoutOpacity,
                        {}
                    }),
                    CurrentUE4Material->MaterialAttributes.Expression, true
                    ));
            }
        }
    }

    if (CurrentUE4Material->BlendMode == BLEND_Translucent)
    {
        CurrentUE4Material->MaterialAttributes.Connect(0,
            NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(TEXT("/Engine/Functions/MaterialLayerFunctions"), TEXT("MatLayerBlend_OverrideMetalness")),
            {
                CurrentUE4Material->MaterialAttributes.Expression,
                {0.5f,0.5f,0.5f},
                {}
            }));
    }

    check(IORExpression[0].ConnectionType == EConnectionType::Expression);
    if (!IORExpression[0].ExpressionData.IsDefault &&
        !(IORExpression[0].ExpressionData.Expression->IsA<UMaterialExpressionConstant3Vector>() &&
            Cast<UMaterialExpressionConstant3Vector>(IORExpression[0].ExpressionData.Expression)->Constant == FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)))
    {
        CurrentUE4Material->MaterialAttributes.Connect(0,
            NewMaterialExpressionFunctionCall(CurrentUE4Material,
                ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_set_refraction")),
                {
                    CurrentUE4Material->MaterialAttributes.Expression,
                    NewMaterialExpressionComponentMask(CurrentUE4Material, IORExpression[0].ExpressionData.Expression, 1)
                }));
    }

    SetPropertiesFromAnnotation(InMaterialDefinition);

    // Clean up and finish
    Temporaries.Empty();

    OutClearCoatNormalCustomOutput = CurrentClearCoatNormal;
    return true;
}

// Small struct used to store the result of a texture baking process
// of a material sub expression
struct FMaterialParameter
{
    typedef void(FRemapFunc(mi::base::IInterface*));

    mi::base::Handle<mi::neuraylib::ICanvas> Texture;
    mi::base::Handle<const mi::neuraylib::IExpression> Expr;

    FString ValueType;
    FString BakePath;

    FRemapFunc* RemapFunc;

    FMaterialParameter() : RemapFunc(nullptr)
    {

    }

    FMaterialParameter(
        const FString& VT,
        FRemapFunc* Func = nullptr)
        : ValueType(VT)
        , RemapFunc(Func)
    {

    }
};

using FDistilledMaterialInfo = TMap<FString, FMaterialParameter>;


mi::base::Handle<const mi::neuraylib::IExpression_direct_call> ToDirectCall(
    const mi::neuraylib::IExpression* Expr,
    const mi::neuraylib::ICompiled_material* CM)
{
    if (!Expr)
    {
        return mi::base::make_handle<const mi::neuraylib::IExpression_direct_call>(nullptr);
    }
    switch (Expr->get_kind())
    {
    case mi::neuraylib::IExpression::EK_DIRECT_CALL:
        return mi::base::make_handle<const mi::neuraylib::IExpression_direct_call>(Expr->get_interface<mi::neuraylib::IExpression_direct_call>());
    case mi::neuraylib::IExpression::EK_TEMPORARY:
    {
        mi::base::Handle<const mi::neuraylib::IExpression_temporary> ExprTmp(Expr->get_interface<const mi::neuraylib::IExpression_temporary>());
        mi::base::Handle<const mi::neuraylib::IExpression_direct_call> RefExpr(
            CM->get_temporary<const mi::neuraylib::IExpression_direct_call>(ExprTmp->get_index()));
        return RefExpr;
    }
    default:
        break;
    }
    return mi::base::make_handle<const mi::neuraylib::IExpression_direct_call>(nullptr);
}

// Returns the semantic of the function definition which corresponds to
// the given call or DS::UNKNOWN in case the expression
// is NULL
mi::neuraylib::IFunction_definition::Semantics GetCallSemantic(mi::neuraylib::ITransaction* Transaction, const mi::neuraylib::IExpression_direct_call* Call)
{
    if (!(Call))
    {
        return mi::neuraylib::IFunction_definition::DS_UNKNOWN;
    }
    mi::base::Handle<const mi::neuraylib::IFunction_definition> MdlDef(
        Transaction->access<const mi::neuraylib::IFunction_definition>(Call->get_definition()));
    check(MdlDef.is_valid_interface());

    return MdlDef->get_semantic();
}

// Returns the argument 'ArgumentName' of the given Call
mi::base::Handle<const mi::neuraylib::IExpression> GetArgument(const mi::neuraylib::ICompiled_material* CM, const mi::neuraylib::IExpression_direct_call* Call, const FString& ArgumentName)
{
    return mi::base::make_handle<const mi::neuraylib::IExpression>(Call ? mi::base::make_handle(Call->get_arguments())->get_expression(TCHAR_TO_ANSI(*ArgumentName)) : nullptr);
}

// Looks up the sub expression 'Path' within the compiled material
// starting at ParentCall. If ParentCall is NULL,the
// material will be traversed from the root
mi::base::Handle<const mi::neuraylib::IExpression_direct_call> LookupCall(
    const FString& Path,
    const mi::neuraylib::ICompiled_material* CM,
    const mi::neuraylib::IExpression_direct_call *ParentCall = nullptr
)
{
    mi::base::Handle<const mi::neuraylib::IExpression_direct_call> 	ResultCall;

    if (ParentCall == nullptr)
    {
        mi::base::Handle<const mi::neuraylib::IExpression> Expr(CM->lookup_sub_expression(TCHAR_TO_ANSI(*Path)));
        ResultCall = ToDirectCall(Expr.get(), CM);
    }
    else
    {
        ResultCall = mi::base::make_handle_dup(ParentCall);

        TArray<FString> SubExpr;
        Path.ParseIntoArray(SubExpr, TEXT("."));
        for (const auto& ExprStr : SubExpr)
        {
            mi::base::Handle<const mi::neuraylib::IExpression> Tmp(GetArgument(CM, ResultCall.get(), TCHAR_TO_ANSI(*ExprStr)));

            ResultCall = ToDirectCall(Tmp.get(), CM);
            if (!ResultCall)
            {
                mi::base::make_handle<const mi::neuraylib::IExpression_direct_call>(nullptr);
            }
        }
    }
    return ResultCall;
}

template <typename T>
mi::base::Handle<const mi::neuraylib::IExpression> CreateConstant(FMDLModule* MDLModule, const mi::neuraylib::IType* VT, const T& DefaultValue)
{
    mi::base::Handle<mi::neuraylib::IValue_factory> ValueFactory(
        MDLModule->MDLFactory->create_value_factory(MDLModule->Transaction.get()));
    mi::base::Handle<mi::neuraylib::IExpression_factory> ExpressionFactory(
        MDLModule->MDLFactory->create_expression_factory(MDLModule->Transaction.get()));

    mi::base::Handle<mi::neuraylib::IValue> V(ValueFactory->create(VT));
    mi::neuraylib::set_value(V.get(), DefaultValue);

    return mi::base::make_handle<const mi::neuraylib::IExpression>(ExpressionFactory->create_constant(V.get()));
}

// Setup material parameters according to target model and
// collect relevant bake paths
void SetupTargetMaterial(FMDLModule * MDLModule, const EDistillationTarget TargetModel, const mi::neuraylib::ICompiled_material* CM, FDistilledMaterialInfo& OutMaterial)
{
    mi::base::Handle<mi::neuraylib::IType_factory> TypeFactory(
        MDLModule->MDLFactory->create_type_factory(MDLModule->Transaction.get()));

    // Access surface.scattering function
    mi::base::Handle<const mi::neuraylib::IExpression_direct_call> ParentCall(LookupCall(TEXT("surface.scattering"), CM));
    // ... and get its semantic
    mi::neuraylib::IFunction_definition::Semantics Semantic(GetCallSemantic(MDLModule->Transaction.get(), ParentCall.get()));

    OutMaterial.Add(TEXT("base_color"), FMaterialParameter(TEXT("Rgb_fp")));
    OutMaterial.Add(TEXT("metallic"), FMaterialParameter(TEXT("Float32")));
    OutMaterial.Add(TEXT("specular"), FMaterialParameter(TEXT("Float32")));
    OutMaterial.Add(TEXT("roughness"), FMaterialParameter(TEXT("Float32")));
    OutMaterial.Add(TEXT("emissive_color"), FMaterialParameter(TEXT("Rgb_fp")));
    OutMaterial.Add(TEXT("under_clearcoat_normal"), FMaterialParameter(TEXT("Float32<3>")/*, remap_normal*/));

    OutMaterial.Add(TEXT("clearcoat_weight"), FMaterialParameter(TEXT("Float32")));
    OutMaterial.Add(TEXT("clearcoat_roughness"), FMaterialParameter(TEXT("Float32")));
    OutMaterial.Add(TEXT("normal"), FMaterialParameter(TEXT("Float32<3>") /*,remap_normal*/));

    OutMaterial.Add(TEXT("opacity"), FMaterialParameter(TEXT("Float32")));
    OutMaterial.Add(TEXT("displacement"), FMaterialParameter(TEXT("Float32<3>")));

    // Opacity path is always the same
    OutMaterial[TEXT("opacity")].BakePath = TEXT("geometry.cutout_opacity");
    OutMaterial[TEXT("opacity")].Expr = mi::base::make_handle(CM->lookup_sub_expression("geometry.cutout_opacity"));

    OutMaterial[TEXT("displacement")].BakePath = TEXT("geometry.displacement");
    OutMaterial[TEXT("displacement")].Expr = mi::base::make_handle(CM->lookup_sub_expression("geometry.displacement"));


    switch (TargetModel)
    {
        case EDistillationTarget::Diffuse:
        {
            // The target model is supposed to be a diffuse reflection bsdf
            check(Semantic ==
                mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF);

            // Setup diffuse material parameters

            // Specify bake paths
            OutMaterial[TEXT("base_color")].BakePath = TEXT("surface.scattering.tint");
            OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, ParentCall.get(), TEXT("tint"));

            ParentCall = LookupCall(TEXT("geometry"), CM);

            OutMaterial[TEXT("normal")].BakePath = TEXT("geometry.normal");
            OutMaterial[TEXT("normal")].Expr = GetArgument(CM, ParentCall.get(), TEXT("normal"));

            OutMaterial[TEXT("metallic")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
            OutMaterial[TEXT("specular")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
            OutMaterial[TEXT("roughness")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
            break;
        }
        case EDistillationTarget::DiffuseGlossy:
        {
            switch (Semantic)
            {
            case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF:

                OutMaterial[TEXT("base_color")].BakePath = TEXT("surface.scattering.tint");
                OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, ParentCall.get(), TEXT("tint"));

                OutMaterial[TEXT("metallic")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
                OutMaterial[TEXT("specular")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
                OutMaterial[TEXT("roughness")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
                break;

            case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SIMPLE_GLOSSY_BSDF:

                OutMaterial[TEXT("base_color")].BakePath = TEXT("surface.scattering.tint");
                OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, ParentCall.get(), TEXT("tint"));

                OutMaterial[TEXT("metallic")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 1.0f);
                OutMaterial[TEXT("specular")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);

                OutMaterial[TEXT("roughness")].BakePath = TEXT("surface.scattering.roughness_u");
                OutMaterial[TEXT("roughness")].Expr = GetArgument(CM, ParentCall.get(), TEXT("roughness_u"));

                break;

            case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_FRESNEL_LAYER:


                OutMaterial[TEXT("specular")].BakePath = TEXT("surface.scattering.weight");
                OutMaterial[TEXT("specular")].Expr = GetArgument(CM, ParentCall.get(), TEXT("weight"));

                OutMaterial[TEXT("metallic")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);

                mi::base::Handle<const mi::neuraylib::IExpression_direct_call> TmpCall(LookupCall(TEXT("base"), CM, ParentCall.get()));

                OutMaterial[TEXT("base_color")].BakePath = TEXT("surface.scattering.base.tint");
                OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, TmpCall.get(), TEXT("tint"));

                TmpCall = LookupCall(TEXT("layer"), CM, ParentCall.get());

                OutMaterial[TEXT("roughness")].BakePath = TEXT("surface.scattering.layer.roughness_u");
                OutMaterial[TEXT("roughness")].Expr = GetArgument(CM, TmpCall.get(), TEXT("roughness_u"));
                break;
            }

            ParentCall = LookupCall(TEXT("geometry"), CM);

            OutMaterial[TEXT("normal")].BakePath = TEXT("geometry.normal");
            OutMaterial[TEXT("normal")].Expr = GetArgument(CM, ParentCall.get(), TEXT("normal"));

            break;
        }
        case EDistillationTarget::UE4:
        {
            mi::base::Handle<const mi::neuraylib::IExpression> EmissionExpression = GetArgument(CM, LookupCall(TEXT("surface.emission"), CM).get(), TEXT("intensity"));
            if (EmissionExpression)
            {
                check(mi::base::make_handle(EmissionExpression->get_type())->get_kind() == mi::neuraylib::IType::TK_COLOR);
                bool isDefault = false;
                if (EmissionExpression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
                {
                    const mi::base::Handle<const mi::neuraylib::IValue_color> Value(EmissionExpression.get_interface<const mi::neuraylib::IExpression_constant>()->get_value<const mi::neuraylib::IValue_color>());
                    isDefault = (mi::base::make_handle<const mi::neuraylib::IValue_float>(Value->get_value(0))->get_value() == 0.0f)
                        && (mi::base::make_handle<const mi::neuraylib::IValue_float>(Value->get_value(1))->get_value() == 0.0f)
                        && (mi::base::make_handle<const mi::neuraylib::IValue_float>(Value->get_value(2))->get_value() == 0.0f);
                }
                if (!isDefault)
                {
                    OutMaterial[TEXT("emissive_color")].BakePath = TEXT("surface.emission.intensity");
                    OutMaterial[TEXT("emissive_color")].Expr = EmissionExpression;
                }
            }

            FString PathPrefix = TEXT("surface.scattering.");

            // Check for a clearcoat layer, first. If present, it is the outermost layer
            if (Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CUSTOM_CURVE_LAYER)
            {
                // Setup clearcoat bake paths
                OutMaterial[TEXT("clearcoat_weight")].BakePath = PathPrefix + TEXT("weight");
                OutMaterial[TEXT("clearcoat_weight")].Expr = GetArgument(CM, ParentCall.get(), TEXT("weight"));

                OutMaterial[TEXT("under_clearcoat_normal")].BakePath = PathPrefix + TEXT("normal");
                OutMaterial[TEXT("under_clearcoat_normal")].Expr = GetArgument(CM, ParentCall.get(), TEXT("normal"));

                mi::base::Handle<const mi::neuraylib::IExpression_direct_call> TmpCall(LookupCall(TEXT("layer"), CM, ParentCall.get()));

                OutMaterial[TEXT("clearcoat_roughness")].BakePath = PathPrefix + TEXT("layer.roughness_u");
                OutMaterial[TEXT("clearcoat_roughness")].Expr = GetArgument(CM, TmpCall.get(), TEXT("roughness_u"));

                // Get clear-coat base layer
                ParentCall = LookupCall(TEXT("base"), CM, ParentCall.get());
                // Get clear-coat base layer semantic
                Semantic = GetCallSemantic(MDLModule->Transaction.get(), ParentCall.get());
                // Extend path prefix
                PathPrefix += TEXT("base.");
            }

            // Check for a weighted layer. Sole purpose of this layer is the transportation of
            // the under-clearcoat-normal. It contains an empty base and a layer with the
            // actual material body
            if (Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_WEIGHTED_LAYER)
            {
                // Collect normal
                OutMaterial[TEXT("normal")].BakePath = PathPrefix + TEXT("normal");
                OutMaterial[TEXT("normal")].Expr = GetArgument(CM, ParentCall.get(), TEXT("normal"));

                // Chain further
                ParentCall = LookupCall(TEXT("layer"), CM, ParentCall.get());
                Semantic = GetCallSemantic(MDLModule->Transaction.get(), ParentCall.get());
                PathPrefix += TEXT("layer.");

                // Check for a normalized mix. This mix combines the metallic and dielectric parts
                // of the material
                if (Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_NORMALIZED_MIX)
                {
                    // The top-mix component is supposed to be a glossy bsdf
                    // Collect metallic weight

                    mi::base::Handle<const mi::neuraylib::IExpression_direct_call> TmpCall(
                        LookupCall(TEXT("components.value1"), CM, ParentCall.get()));
                    OutMaterial[TEXT("metallic")].BakePath = PathPrefix + TEXT("components.value1.weight");
                    OutMaterial[TEXT("metallic")].Expr = GetArgument(CM, TmpCall.get(), TEXT("weight"));

                    // And other metallic parameters
                    TmpCall = LookupCall(TEXT("components.value1.component"), CM, ParentCall.get());

                    OutMaterial[TEXT("roughness")].BakePath = PathPrefix + TEXT("components.value1.component.roughness_u");
                    OutMaterial[TEXT("roughness")].Expr = GetArgument(CM, TmpCall.get(), TEXT("roughness_u"));

                    // Base_color can be taken from any of the leaf-bsdfs. It is supposed to
                    // be the same.
                    OutMaterial[TEXT("base_color")].BakePath = PathPrefix + TEXT("components.value1.component.tint");
                    OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, TmpCall.get(), TEXT("tint"));

                    // Chain further
                    ParentCall = LookupCall(
                        TEXT("components.value0.component"), CM, ParentCall.get());
                    Semantic = GetCallSemantic(MDLModule->Transaction.get(), ParentCall.get());
                    PathPrefix += TEXT("components.value0.component.");
                }
                if (Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CUSTOM_CURVE_LAYER)
                {
                    // Collect specular parameters
                    OutMaterial[TEXT("specular")].BakePath = PathPrefix + TEXT("weight");
                    OutMaterial[TEXT("specular")].Expr = GetArgument(CM, ParentCall.get(), TEXT("weight"));

                    if (OutMaterial[TEXT("roughness")].BakePath.IsEmpty())
                    {
                        mi::base::Handle<const mi::neuraylib::IExpression_direct_call> TmpCall(LookupCall("layer", CM, ParentCall.get()));

                        OutMaterial[TEXT("roughness")].BakePath = PathPrefix + TEXT("layer.roughness_u");
                        OutMaterial[TEXT("roughness")].Expr = GetArgument(CM, TmpCall.get(), TEXT("roughness_u"));
                    }
                    // Chain further
                    ParentCall = LookupCall(TEXT("base"), CM, ParentCall.get());
                    Semantic = GetCallSemantic(MDLModule->Transaction.get(), ParentCall.get());
                    PathPrefix += "base.";
                }
                if (Semantic ==
                    mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_VCAVITIES_BSDF)
                {
                    if (OutMaterial[TEXT("metallic")].BakePath.IsEmpty())
                    {
                        OutMaterial[TEXT("metallic")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 1.0f);
                    }
                    if (OutMaterial[TEXT("roughness")].BakePath.IsEmpty())
                    {
                        OutMaterial[TEXT("roughness")].BakePath = PathPrefix + TEXT("roughness_u");
                        OutMaterial[TEXT("roughness")].Expr = GetArgument(CM, ParentCall.get(), TEXT("roughness_u"));
                    }
                    if (OutMaterial[TEXT("base_color")].BakePath.IsEmpty())
                    {
                        OutMaterial[TEXT("base_color")].BakePath = PathPrefix + TEXT("tint");
                        OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, ParentCall.get(), TEXT("tint"));
                    }
                }
                else if (Semantic ==
                    mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF)
                {
                    if (OutMaterial[TEXT("base_color")].BakePath.IsEmpty())
                    {
                        OutMaterial[TEXT("base_color")].BakePath = PathPrefix + TEXT("tint");
                        OutMaterial[TEXT("base_color")].Expr = GetArgument(CM, ParentCall.get(), TEXT("tint"));
                    }
                }
            }

            if (!OutMaterial[TEXT("roughness")].Expr && !OutMaterial[TEXT("specular")].Expr)
            {
                if (OutMaterial[TEXT("clearcoat_roughness")].Expr)
                {
                    // promote clearcoat_roughness to roughness
                    OutMaterial[TEXT("roughness")] = OutMaterial[TEXT("clearcoat_roughness")];
                    OutMaterial[TEXT("clearcoat_roughness")] = FMaterialParameter();
                }
                else
                {
                    OutMaterial[TEXT("roughness")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 1.0f);
                }

                if (OutMaterial[TEXT("clearcoat_weight")].Expr)
                {
                    // promote clearcoat_weight to specular
                    OutMaterial[TEXT("specular")] = OutMaterial[TEXT("clearcoat_weight")];
                    OutMaterial[TEXT("clearcoat_weight")] = FMaterialParameter();
                }
                else
                {
                    OutMaterial[TEXT("specular")].Expr = CreateConstant<mi::Float32>(MDLModule, TypeFactory->create_float(), 0.0f);
                }

                OutMaterial[TEXT("under_clearcoat_normal")] = FMaterialParameter();
            }
        }
    }
}

template <typename T>
void Connect(T& Target, const TArray<FMaterialExpressionConnection>& SrcExpr)
{
    check(SrcExpr.Num() == 1);
    Target.Connect(SrcExpr[0].ExpressionData.Index, SrcExpr[0].ExpressionData.Expression);
}

bool FMDLMaterialImporter::ImportDistilledMaterial(UMaterial* Material,
    const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& InCompiledMaterial, UMaterialExpressionClearCoatNormalCustomOutput*& OutClearCoatNormalCustomOutput, FLoadTextureCallback InCallback)
{
    LoadTextureCallback = InCallback;
    ParentPackage = Material->GetOuter();
    ImportErrors.Empty();
    OutClearCoatNormalCustomOutput = nullptr;

    // Create Unreal Engine material and translate expressions
    CurrentUE4Material = Material;
    check(CurrentUE4Material != nullptr);
    TranslucentOpacity = FMaterialExpressionConnection();
    EmissiveOpacity = FMaterialExpressionConnection();
    SubsurfaceColor = FMaterialExpressionConnection();
    SubsurfaceOpacity = FMaterialExpressionConnection();
    OpacityEnabled = FMaterialExpressionConnection();

    const UMDLSettings* Settings = GetDefault<UMDLSettings>();
#if defined(USE_WORLD_ALIGNED_TEXTURES)
# if defined(USE_WAT_AS_SCALAR)
    UseWorldAlignedTextureParameter = NewMaterialExpressionScalarParameter(CurrentUE4Material, TEXT("World-Aligned Textures"), Settings->bWorldAlignedTextures);
    Cast<UMaterialExpressionParameter>(UseWorldAlignedTextureParameter.ExpressionData.Expression)->Group = TEXT("Texture Projection");
# else
    UseWorldAlignedTextureParameter = NewMaterialExpressionStaticBoolParameter(CurrentUE4Material, "World-Aligned Textures", Settings->bWorldAlignedTextures, "Texture Projection");
# endif
#endif

    ImportParameters(InMaterialDefinition, InCompiledMaterial);
    // Temporaries are imported on demand, as they might change after having read the geometry part
    Temporaries.SetNum(InCompiledMaterial->get_temporary_count());

    FDistilledMaterialInfo MaterialInfo;
    SetupTargetMaterial(MDLModule, Settings->DistillationTarget, InCompiledMaterial.get(), MaterialInfo);

    // Build expression trees
    // For the geometry part, use the geometry_normal, instead of the normal, to prevent issues with shader domains!
    CurrentNormalExpression = NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_geometry_normal")), {});
    InGeometryExpression = true;
    UMaterialExpressionClearCoatNormalCustomOutput* UnderClearcoatNormal = nullptr;

    check(MaterialInfo[TEXT("normal")].Expr);
    TArray<FMaterialExpressionConnection> NormalExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("normal")].Expr, MaterialInfo[TEXT("normal")].BakePath);
    CheckConnection(NormalExpression);
    TArray<FMaterialExpressionConnection> UnderClearcoatNormalExpression;

    const TArray<FMaterialExpressionConnection> ThinWalled = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "thin_walled"), "thin_walled");
    CheckConnection(ThinWalled);
    check(ThinWalled[0].ConnectionType == EConnectionType::Expression);
    
    
    // Hardcode TwoSided to true for now
    //CurrentUE4Material->TwoSided = EvaluateBool(ThinWalled[0]);
    CurrentUE4Material->TwoSided = true;

    if (MaterialInfo[TEXT("under_clearcoat_normal")].Expr)
    {
        UnderClearcoatNormalExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("under_clearcoat_normal")].Expr, MaterialInfo[TEXT("under_clearcoat_normal")].BakePath);
        CheckConnection(UnderClearcoatNormalExpression);
        UnderClearcoatNormal = NewMaterialExpressionClearCoatNormalCustomOutput(CurrentUE4Material, UnderClearcoatNormalExpression[0]);
    }
    InGeometryExpression = false;
    FMaterialExpressionConnection BaseColor, Metallic, Specular, Roughness, EmissiveColor, OpacityMask, Normal, ClearCoat, ClearCoatRoughness, WorldPositionOffset, Refraction;
    if (CurrentNormalExpression != NormalExpression[0])
    {
        // if the normal has changed, clean-up the temporaries, they are rebuild on next usage, potentially different
        for (TArray<FMaterialExpressionConnection>& Temporary : Temporaries)
        {
            Temporary.Empty();
        }
        // from now on, the normal expression is what we've got out of the Geometry field!
        CurrentNormalExpression = NormalExpression[0];
    }
    else
    {
        // Otherwise, use the standard normal from now on
        CurrentNormalExpression = NewMaterialExpressionFunctionCall(CurrentUE4Material, ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdl_state_normal")), {});
    }
    Normal = CurrentNormalExpression;

    if (MaterialInfo[TEXT("base_color")].Expr)
    {
        auto BaseColorExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("base_color")].Expr, MaterialInfo[TEXT("base_color")].BakePath);
        CheckConnection(BaseColorExpression);
        BaseColor = BaseColorExpression[0];
    }
    if (MaterialInfo[TEXT("metallic")].Expr)
    {
        auto MetallicExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("metallic")].Expr, MaterialInfo[TEXT("metallic")].BakePath);
        CheckConnection(MetallicExpression);
        Metallic = MetallicExpression[0];
    }
    if (MaterialInfo[TEXT("specular")].Expr)
    {
        auto SpecularExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("specular")].Expr, MaterialInfo[TEXT("specular")].BakePath);
        CheckConnection(SpecularExpression);
        Specular = SpecularExpression[0];
    }
    if (MaterialInfo[TEXT("roughness")].Expr)
    {
        // Square roughness
        auto RoughnessExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("roughness")].Expr, MaterialInfo[TEXT("roughness")].BakePath);
        CheckConnection(RoughnessExpression);
        Roughness = NewMaterialExpressionSquareRoot(CurrentUE4Material, RoughnessExpression[0]);
    }
    if (MaterialInfo[TEXT("emissive_color")].Expr)
    {
        auto EmissiveColorExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("emissive_color")].Expr, MaterialInfo[TEXT("emissive_color")].BakePath);
        CheckConnection(EmissiveColorExpression);
        EmissiveColor = NewMaterialExpressionDivide(CurrentUE4Material, EmissiveColorExpression[0], 2560.0f);
    }
    if (MaterialInfo[TEXT("clearcoat_weight")].Expr)
    {
        CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_ClearCoat);
        auto ClearCoatExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("clearcoat_weight")].Expr, MaterialInfo[TEXT("clearcoat_weight")].BakePath);
        CheckConnection(ClearCoatExpression);
        ClearCoat = ClearCoatExpression[0];

        if (MaterialInfo[TEXT("clearcoat_roughness")].Expr)
        {
            // Square roughness
            auto ClearCoatRoughnessExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("clearcoat_roughness")].Expr, MaterialInfo[TEXT("clearcoat_roughness")].BakePath);
            CheckConnection(ClearCoatRoughnessExpression);
            ClearCoatRoughness = NewMaterialExpressionSquareRoot(CurrentUE4Material, ClearCoatRoughnessExpression[0]);
        }
    }
    else
    {
        bool bUnlit = false;
        if (MaterialInfo[TEXT("emissive_color")].Expr
            && !MaterialInfo[TEXT("metallic")].Expr
            && !MaterialInfo[TEXT("specular")].Expr
            && !MaterialInfo[TEXT("roughness")].Expr)
        {
            if (MaterialInfo[TEXT("base_color")].Expr && MaterialInfo[TEXT("normal")].Expr)
            {
                if (BaseColor.ExpressionData.Expression->IsA<UMaterialExpressionConstant3Vector>()
                && Normal.ExpressionData.Expression->IsA<UMaterialExpressionMaterialFunctionCall>())
                {
                    auto Color = Cast<UMaterialExpressionConstant3Vector>(BaseColor.ExpressionData.Expression);
                    auto Function = Cast<UMaterialExpressionMaterialFunctionCall>(Normal.ExpressionData.Expression);
                    if (Color->Constant.Equals(FLinearColor(0, 0, 0)) && Function->MaterialFunction && Function->MaterialFunction->GetName() == TEXT("mdl_state_normal"))
                    {
                        bUnlit = true;
                    }
                }
            }
        }

        if (bUnlit)
        {
            // Drop the distilled base color and normal, because they're not needed at unlit.
            BaseColor = FMaterialExpressionConnection();
            Normal = FMaterialExpressionConnection();
            CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_Unlit);
        }
        else
        {
            CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);
        }
    }

    if (MaterialInfo[TEXT("displacement")].Expr)
    {
        auto WorldPositionOffsetExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("displacement")].Expr, MaterialInfo[TEXT("displacement")].BakePath);
        CheckConnection(WorldPositionOffsetExpression);
        WorldPositionOffset = WorldPositionOffsetExpression[0];
    }

    if (EmissiveOpacity.ExpressionData.IsDefault)
    {
        auto OpacityMaskExpression = CreateExpression(InCompiledMaterial, MaterialInfo[TEXT("opacity")].Expr, MaterialInfo[TEXT("opacity")].BakePath);
        CheckConnection(OpacityMaskExpression);
        OpacityMask = OpacityMaskExpression[0];
        if (!OpacityMask.ExpressionData.IsDefault)
        {
            CurrentUE4Material->BlendMode = BLEND_Masked;
            CurrentUE4Material->DitherOpacityMask = true;
        }
    }
    else
    {
        SetTransparency(CurrentUE4Material);
        if (!TranslucentOpacity.ExpressionData.IsDefault)
        {
            BaseColor = FMaterialExpressionConnection();
            Normal = FMaterialExpressionConnection();
            Metallic = FMaterialExpressionConnection();
            Specular = FMaterialExpressionConnection();
            Roughness = FMaterialExpressionConnection();
            CurrentUE4Material->SetShadingModel(EMaterialShadingModel::MSM_Unlit);
        }
        else
        {
            TranslucentOpacity = EmissiveOpacity;
        }

        // NOTE: The refraction connecting to the material attribute isn't working before 4.26
        const TArray<FMaterialExpressionConnection> IOR = CreateExpression(InCompiledMaterial, GetField(InCompiledMaterial, "ior"), "ior");
        CheckConnection(IOR);
        check(IOR[0].ConnectionType == EConnectionType::Expression);
        Refraction = NewMaterialExpressionLinearInterpolate(CurrentUE4Material, NewMaterialExpressionConstant(CurrentUE4Material, 1.0f), IOR[0], NewMaterialExpressionFresnel(CurrentUE4Material));
    }

    CurrentUE4Material->MaterialAttributes.Connect(0,
        NewMaterialExpressionMakeMaterialAttributes(CurrentUE4Material, BaseColor, Metallic, Specular, Roughness, EmissiveColor, TranslucentOpacity, OpacityMask, Normal, WorldPositionOffset, {}, {}, {}, ClearCoat, ClearCoatRoughness, {}, Refraction));

    SetPropertiesFromAnnotation(InMaterialDefinition);

    // Clean up and finish
    Temporaries.Empty();

    OutClearCoatNormalCustomOutput = UnderClearcoatNormal;
    return true;
}

void FMDLMaterialImporter::SetPropertiesFromAnnotation(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition)
{
    const mi::base::Handle<const mi::neuraylib::IAnnotation_block> AnnotationBlock(InMaterialDefinition->get_annotations());
    if (AnnotationBlock)
    {
        const mi::IString* DecodedModuleName = MDLModule->MDLFactory->decode_name(InMaterialDefinition->get_mdl_module_name());
        FString ModuleName = UnmangleMdlPath(UTF8_TO_TCHAR(DecodedModuleName->get_c_str()));
        const FString DitherMaskedOffAnnotation = ModuleName + TEXT("::") + FString(UE4Annotations[UE4_DitherMaskedOff]);
        const FString WorldSpaceNormalAnnotation = ModuleName + TEXT("::") + FString(UE4Annotations[UE4_WorldSpaceNormal]);

        for (mi::Size i = 0; i < AnnotationBlock->get_size(); ++i)
        {
            const mi::base::Handle<const mi::neuraylib::IAnnotation> Annotation(AnnotationBlock->get_annotation(i));
            FString AnnotationName = UnmangleMdlPath(Annotation->get_name());

            if (AnnotationName == DitherMaskedOffAnnotation)
            {
                CurrentUE4Material->DitherOpacityMask = false;
            }
            else if (AnnotationName == WorldSpaceNormalAnnotation)
            {
                CurrentUE4Material->bTangentSpaceNormal = false;
            }
        }
    }

    CurrentUE4Material->bUsedWithStaticLighting = true;
    CurrentUE4Material->bUsedWithSkeletalMesh = true;
    CurrentUE4Material->bUsedWithInstancedStaticMeshes = true;
}

bool FMDLMaterialImporter::IsDistillOff(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition)
{
    const mi::base::Handle<const mi::neuraylib::IAnnotation_block> AnnotationBlock(InMaterialDefinition->get_annotations());
    if (AnnotationBlock)
    {
        const FString DistillOffAnnotation = TEXT("::") + FString(UE4Annotations[UE4_DistillOff]);

        for (mi::Size i = 0; i < AnnotationBlock->get_size(); ++i)
        {
            const mi::base::Handle<const mi::neuraylib::IAnnotation> Annotation(AnnotationBlock->get_annotation(i));
            FString AnnotationName = UnmangleMdlPath(Annotation->get_name());

            if (AnnotationName.Contains(DistillOffAnnotation))
            {
                return true;
            }
        }
    }

    return false;
}

UTexture* FMDLMaterialImporter::LoadResource(const char* FilePath, const char* OwnerModule, const float Gamma, TextureCompressionSettings InCompression)
{
    UTexture* Texture = nullptr;
    if (FilePath)
    {
        mi::base::Handle<mi::neuraylib::IMdl_entity_resolver> EntityResolver(MDLModule->MDLConfiguration->get_entity_resolver());

        // NOTE: MDL SDK 367100 is more restricted to the texture loading, can't use ITexture or IImage without loading texture.
        // This's the way to manually resolve textures. Also requires the option "resolve_resources" to false in context
        mi::base::Handle<mi::neuraylib::IMdl_resolved_resource> ResolvedResource;
        if (OwnerModule)
        {
            mi::base::Handle<const mi::IString> owner_db_name(MDLModule->MDLFactory->get_db_module_name(OwnerModule));

            if (owner_db_name.is_valid_interface())
            {
                mi::base::Handle<const mi::neuraylib::IModule> owner_module(
                    MDLModule->Transaction->access<mi::neuraylib::IModule>(owner_db_name->get_c_str()));

                if (owner_module.is_valid_interface())
                {
                    ResolvedResource = EntityResolver->resolve_resource(FilePath, owner_module->get_filename(), OwnerModule, 0, 0);
                }
            }
        }

        if (!ResolvedResource)
        {
            ResolvedResource = EntityResolver->resolve_resource(FilePath, nullptr, nullptr, 0, 0);
        }

        if (ResolvedResource)
        {
            FString Filename = UTF8_TO_TCHAR(ResolvedResource->get_filename_mask());
            FString AssetDir;
            if (Filename.StartsWith(MDLModule->MDLUserPath))
            {
                FString PackageName = FPaths::GetPath(Filename);
                verify(PackageName.RemoveFromStart(MDLModule->MDLUserPath));
                PackageName.RemoveFromStart("/nvidia/vMaterials");
                AssetDir = FMDLImporterUtility::GetProjectMdlRootPath() / PackageName;
            }
            else
            {
                AssetDir = FPaths::GetPath(ParentPackage->GetName());
            }

            if (LoadTextureCallback)
            {
                LoadTextureCallback(Texture, Filename, Gamma, InCompression);
            }

            if (Texture == nullptr)
            {
                FString AssetName = FPaths::GetBaseFilename(Filename);
                FString Filepath = FPaths::GetPath(Filename);

                FString Left, Right;
                if (Filepath.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
                {
                    AssetDir = AssetDir / Right;
                }
                else
                {
                    AssetDir = AssetDir / Filepath;
                }

                // If LoadTextureCallback failed with the a possible Omniverse Filename (URL), make sure we have a valid package path
                if (!FPackageName::IsValidLongPackageName(AssetDir / AssetName))
                {
                    return nullptr;
                }

                Texture = LoadTexture(Filename, AssetDir, AssetName, Gamma != 1.0f, InCompression);
            }
        }
    }

    return Texture;
}

UTexture* FMDLMaterialImporter::LoadTexture(const FString& Filename, const FString& AssetDir, const FString& AssetName, bool srgb, TextureCompressionSettings InCompression)
{
    // Check if texture already exists and do not re-import if that is the case
    UTexture* Texture = LoadObject<UTexture>(nullptr, *(AssetDir / AssetName), nullptr, LOAD_NoWarn);

    // NOTE: this only support loading local file
    if (Texture == nullptr && FPaths::FileExists(Filename))
    {
        // Using AssetTools to load textures so they are imported into separate packages
        IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
        TArray<UObject*> ImportedAssets = AssetTools.ImportAssets({ Filename }, AssetDir, ParentFactory->TextureFactory, false);

        if (ImportedAssets.Num() == 1)
        {
            Texture = Cast<UTexture>(ImportedAssets[0]);

            if (!Texture->IsNormalMap())
            {
                if (!FMDLImporterUtility::IsHDRTexture(Texture))
                {
                    Texture->SRGB = srgb;
                }
                Texture->UpdateResource();
            }

            if (Texture->CompressionSettings == TC_Default)
            {
                Texture->CompressionSettings = InCompression;
            }

            if (!FMath::IsPowerOfTwo((int)Texture->GetSurfaceWidth()) ||
                !FMath::IsPowerOfTwo((int)Texture->GetSurfaceHeight()))
            {
                GMDLOutputLogger.printf(mi::base::MESSAGE_SEVERITY_WARNING, "MDL", "Texture %s does not have power of two dimensions and therefore no mipmaps will be generated", TCHAR_TO_ANSI(*Texture->GetName()));
            }
        }
    }

    return Texture;
}

#define unimplemented_control_path(format, ...) checkf(0, format, ##__VA_ARGS__); return { }

static int32 CalcArraySize(const mi::base::Handle<const mi::neuraylib::IExpression>& Expression)
{
    const mi::base::Handle<const mi::neuraylib::IType_array> Type(Expression->get_type<mi::neuraylib::IType_array>());

    if (Type->get_kind() == mi::neuraylib::IType::TK_ARRAY)
    {
        return Expression->get_kind() != mi::neuraylib::IExpression::EK_CONSTANT ?
            (int32)Type->get_size() :
            (int32)mi::base::make_handle(Expression.get_interface<const mi::neuraylib::IExpression_constant>()->get_value<const mi::neuraylib::IValue_array>())->get_size();
    }

    return 1;
}

static void CreateDirectory(const FString& DirectoryPath)
{
    if (!FPaths::DirectoryExists(DirectoryPath))
    {
        CreateDirectory(FPaths::GetPath(DirectoryPath));
        verify(FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*DirectoryPath));
    }
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::CreateExpression(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, const mi::base::Handle<const mi::neuraylib::IExpression>& MDLExpression, const FString& CallPath)
{
    check(MDLExpression != nullptr);

    TArray<FMaterialExpressionConnection> Outputs;
    mi::neuraylib::IExpression::Kind kind = MDLExpression->get_kind();
    switch (kind)
    {
        case mi::neuraylib::IExpression::EK_CONSTANT:
            Outputs = CreateExpressionConstant(mi::base::make_handle(MDLExpression.get_interface<const mi::neuraylib::IExpression_constant>()->get_value()));
            break;
        case mi::neuraylib::IExpression::EK_DIRECT_CALL:
            Outputs = CreateExpressionFunctionCall(CompiledMaterial, MDLExpression.get_interface<const mi::neuraylib::IExpression_direct_call>(), CallPath);
            if (ImportErrors.Num() == 0 && Outputs.Num() == 0)
            {
                mi::base::Handle<const mi::neuraylib::IBaker> Baker(MDLModule->MDLDistillerAPI->create_baker(CompiledMaterial.get(), TCHAR_TO_UTF8(*CallPath)));
                if (Baker.is_valid_interface())
                {
                    FString PixelType = Baker->get_pixel_type();
                    check((PixelType == "Float32") || (PixelType == "Float32<3>") || (PixelType == "Rgb_fp"));
                    if (Baker->is_uniform())
                    {
                        if (PixelType == "Float32")
                        {
                            mi::base::Handle<mi::IFloat32> V(MDLModule->Transaction->create<mi::IFloat32>());
                            mi::base::Handle<mi::IData> Data(V->get_interface<mi::IData>());
                            verify(Baker->bake_constant(Data.get()) == 0);
                            Outputs = { NewMaterialExpressionConstant(CurrentUE4Material, V->get_value<mi::Float32>()) };
                        }
                        else if (PixelType == "Float32<3>")
                        {
                            mi::base::Handle<mi::IFloat32_3> V(MDLModule->Transaction->create<mi::IFloat32_3>());
                            mi::base::Handle<mi::IData> Data(V->get_interface<mi::IData>());
                            verify(Baker->bake_constant(Data.get()) == 0);
                            Outputs = { NewMaterialExpressionConstant(CurrentUE4Material, V->get_value().x, V->get_value().y, V->get_value().z) };
                        }
                        else
                        {
                            check(PixelType == "Rgb_fp");
                            mi::base::Handle<mi::IColor> V(MDLModule->Transaction->create<mi::IColor>());
                            mi::base::Handle<mi::IData> Data(V->get_interface<mi::IData>());
                            verify(Baker->bake_constant(Data.get()) == 0);
                            Outputs = { NewMaterialExpressionConstant(CurrentUE4Material, V->get_value().r, V->get_value().g, V->get_value().b, V->get_value().a) };
                        }
                    }
                    else
                    {
                        const UMDLSettings* Settings = GetDefault<UMDLSettings>();
                        mi::base::Handle<mi::neuraylib::ICanvas> Canvas(MDLModule->ImageAPI->create_canvas(TCHAR_TO_UTF8(*PixelType), Settings->BakedTextureHeight, Settings->BakedTextureWidth));
                        verify(Baker->bake_texture(Canvas.get(), Settings->BakedTextureSamples) == 0);

                        CreateDirectory(PackagePath);
                        FString TexturePath = PackagePath / CallPath + ".png";
                        verify(MDLModule->MDLImpexpAPI->export_canvas(TCHAR_TO_UTF8(*TexturePath), Canvas.get()) == 0);

                        Outputs = { NewMaterialExpressionTextureSample(CurrentUE4Material, LoadTexture(TexturePath, ParentPackage->GetName(), CallPath), {}) };
                    }
                }
            }
            break;
        case mi::neuraylib::IExpression::EK_PARAMETER:
            Outputs = GetExpressionParameter(MDLExpression.get_interface<const mi::neuraylib::IExpression_parameter>());
            break;
        case mi::neuraylib::IExpression::EK_TEMPORARY:
            Outputs = CreateExpressionTemporary(CompiledMaterial, MDLExpression.get_interface<const mi::neuraylib::IExpression_temporary>(), CallPath);
            break;
        default:
            checkf(0, TEXT("Unhandled MDL expression type %d"), (int32)kind);
            break;
    }

    return Outputs;
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::CreateExpressionConstant(const mi::base::Handle<const mi::neuraylib::IValue>& MDLConstant)
{
    mi::neuraylib::IValue::Kind Kind = MDLConstant->get_kind();
    switch (Kind)
    {
        case mi::neuraylib::IValue::VK_BOOL:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_bool> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_bool>());

            return { NewMaterialExpressionStaticBool(CurrentUE4Material, Value->get_value()) };
        }
        case mi::neuraylib::IValue::VK_INT:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_int> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_int>());

            return { NewMaterialExpressionConstant(CurrentUE4Material, (float)Value->get_value()) };
        }
        case mi::neuraylib::IValue::VK_ENUM:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_enum> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_enum>());

            return { NewMaterialExpressionConstant(CurrentUE4Material, (float)Value->get_value()) };
        }
        case mi::neuraylib::IValue::VK_FLOAT:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_float> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_float>());

            return { NewMaterialExpressionConstant(CurrentUE4Material, Value->get_value()) };
        }
        case mi::neuraylib::IValue::VK_DOUBLE:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_double> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_double>());

            return { NewMaterialExpressionConstant(CurrentUE4Material, (float)Value->get_value()) };
        }
        case mi::neuraylib::IValue::VK_STRING:
        {
            return { };
        }
        case mi::neuraylib::IValue::VK_VECTOR:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_vector> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_vector>());

            switch (Value->get_size())
            {
                case 2:
                    {
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecX(mi::base::make_handle(Value->get_value(0)).get_interface<const mi::neuraylib::IValue_float>());
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecY(mi::base::make_handle(Value->get_value(1)).get_interface<const mi::neuraylib::IValue_float>());
                        return { NewMaterialExpressionConstant(CurrentUE4Material,
                            VecX ? VecX->get_value() : 0.0f,
                            VecY ? VecY->get_value() : 0.0f) };
                    }
                case 3:
                    {
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecX(mi::base::make_handle(Value->get_value(0)).get_interface<const mi::neuraylib::IValue_float>());
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecY(mi::base::make_handle(Value->get_value(1)).get_interface<const mi::neuraylib::IValue_float>());
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecZ(mi::base::make_handle(Value->get_value(2)).get_interface<const mi::neuraylib::IValue_float>());
                        return { NewMaterialExpressionConstant(CurrentUE4Material,
                            VecX ? VecX->get_value() : 0.0f,
                            VecY ? VecY->get_value() : 0.0f,
                            VecZ ? VecZ->get_value() : 0.0f) };
                    }
                case 4:
                    {
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecX(mi::base::make_handle(Value->get_value(0)).get_interface<const mi::neuraylib::IValue_float>());
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecY(mi::base::make_handle(Value->get_value(1)).get_interface<const mi::neuraylib::IValue_float>());
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecZ(mi::base::make_handle(Value->get_value(2)).get_interface<const mi::neuraylib::IValue_float>());
                        const mi::base::Handle<const mi::neuraylib::IValue_float> VecW(mi::base::make_handle(Value->get_value(3)).get_interface<const mi::neuraylib::IValue_float>());
                        return { NewMaterialExpressionConstant(CurrentUE4Material,
                            VecX ? VecX->get_value() : 0.0f,
                            VecY ? VecY->get_value() : 0.0f,
                            VecZ ? VecZ->get_value() : 0.0f,
                            VecW ? VecW->get_value() : 0.0f) };
                    }
            }
            break;
        }
        case mi::neuraylib::IValue::VK_MATRIX:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_matrix> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_matrix>());

            TArray<FMaterialExpressionConnection> Result;
            Result.Reserve(Value->get_size());
            for (mi::Size i = 0; i < Value->get_size(); i++)
            {
                Result.Append(CreateExpressionConstant(mi::base::make_handle(Value->get_value(i))));
            }

            return Result;
        }
        case mi::neuraylib::IValue::VK_COLOR:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_color> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_color>());

            return { NewMaterialExpressionConstant(CurrentUE4Material,
                mi::base::make_handle<const mi::neuraylib::IValue_float>(Value->get_value(0))->get_value(),
                mi::base::make_handle<const mi::neuraylib::IValue_float>(Value->get_value(1))->get_value(),
                mi::base::make_handle<const mi::neuraylib::IValue_float>(Value->get_value(2))->get_value()) };
        }
        case mi::neuraylib::IValue::VK_ARRAY:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_array> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_array>());

            TArray<FMaterialExpressionConnection> Result;
            for (mi::Size i = 0; i < Value->get_size(); i++)
            {
                Result.Append(CreateExpressionConstant(mi::base::make_handle(Value->get_value(i))));
            }

            return Result;
        }
        case mi::neuraylib::IValue::VK_STRUCT:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_struct> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_struct>());

            TArray<FMaterialExpressionConnection> Result;
            Result.Reserve(Value->get_size());
            for (mi::Size i = 0; i < Value->get_size(); i++)
            {
                Result.Append(CreateExpressionConstant(mi::base::make_handle(Value->get_value(i))));
            }

            return Result;
        }
        case mi::neuraylib::IValue::VK_INVALID_DF:
        {
            return{ NewMaterialExpressionMakeMaterialAttributes(CurrentUE4Material, {0.0f, 0.0f, 0.0f}, 0.0f, 0.0f, 0.0f) };
        }
        case mi::neuraylib::IValue::VK_TEXTURE:
        {
            const mi::base::Handle<const mi::neuraylib::IValue_texture> Value(MDLConstant.get_interface<const mi::neuraylib::IValue_texture>());
            return { NewMaterialExpressionTextureObject(CurrentUE4Material, LoadResource(Value->get_file_path(), Value->get_owner_module(), Value->get_gamma())) };
        }
        case mi::neuraylib::IValue::VK_BSDF_MEASUREMENT:
        {
            // a BSDF_MEASUREMENT is something like a texture, which special accessing operations!
            // as that's not (yet) implemented in UE4, provide some default color, for now!
#if 0
            const mi::base::Handle<const mi::neuraylib::IValue_bsdf_measurement> BSDFMeasurement(MDLConstant.get_interface<const mi::neuraylib::IValue_bsdf_measurement>());
            const char* value = BSDFMeasurement->get_value();
#else
            return{ NewMaterialExpressionConstant(CurrentUE4Material, 0.0f, 0.0f, 0.0f) };
#endif
        }
    }

    unimplemented_control_path(TEXT("Unhandled MDL constant expression type %d"), (int32)MDLConstant->get_kind());
}

static UMaterialExpression* CompareStaticBool(UMaterial* Parent, UMaterialExpressionStaticBool* StaticBool, UMaterialExpression* RHS, bool EqualCheck)
{
    if (StaticBool->Value == EqualCheck)
    {
        // either check for equal to true, or not equal to false -> just keep the right hand side
        return RHS;
    }
    else if (RHS->IsA<UMaterialExpressionStaticBoolParameter>() || RHS->IsA<UMaterialExpressionStaticSwitch>())
    {
        // determine !RHS by an inverting StaticSwitch
        return NewMaterialExpressionStaticSwitch(Parent, RHS, NewMaterialExpressionStaticBool(Parent, false), NewMaterialExpressionStaticBool(Parent, true));
    }
    else
    {
        // determine !RHS by an IfEqual comparing to 0.0f
        return NewMaterialExpressionIfEqual(Parent, RHS, 0.0f, 1.0f, 0.0f);
    }
}

static bool IsScatterModeReflect(UMaterialExpression* Expression)
{
    if (Expression)
    {
        check(Expression->IsA<UMaterialExpressionConstant>());
        UMaterialExpressionConstant* Constant = Cast<UMaterialExpressionConstant>(Expression);
        return (Constant->R == 0.0f);		// == scatter_reflect
    }
    return true;
}

static bool IsOpaqueBSDF(UMaterialExpression* Expression)
{
    if (Expression)
    {
        if (Expression->IsA<UMaterialExpressionMakeMaterialAttributes>())
        {
            UMaterialExpressionMakeMaterialAttributes* MakeMaterialAttributes = Cast<UMaterialExpressionMakeMaterialAttributes>(Expression);
            return !MakeMaterialAttributes->Opacity.Expression && !MakeMaterialAttributes->OpacityMask.Expression;
        }
        else
        {
            check(Expression->IsA<UMaterialExpressionMaterialFunctionCall>());
            UMaterialExpressionMaterialFunctionCall* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression);
            check(FunctionCall->MaterialFunction);
            FString Name = FunctionCall->MaterialFunction->GetName();
            if ((Name == "mdl_df_backscattering_glossy_reflection_bsdf") ||
                (Name == "mdl_df_diffuse_reflection_bsdf"))
            {
                return true;
            }
            else if (Name == "mdl_df_diffuse_transmission_bsdf")
            {
                return false;
            }
            else if (Name == "mdl_df_fresnel_layer")
            {
                return IsOpaqueBSDF(FunctionCall->FunctionInputs[2].Input.Expression) && IsOpaqueBSDF(FunctionCall->FunctionInputs[3].Input.Expression);
            }
            else if (Name == "mdl_df_measured_curve_factor_5")
            {
                return IsOpaqueBSDF(FunctionCall->FunctionInputs[5].Input.Expression);
            }
            else if (Name == "mdl_df_simple_glossy_bsdf")
            {
                return IsScatterModeReflect(FunctionCall->FunctionInputs[4].Input.Expression);
            }
            else if (Name == "mdl_df_specular_bsdf")
            {
                return IsScatterModeReflect(FunctionCall->FunctionInputs[1].Input.Expression);
            }
            else if (Name == "mdl_df_tint")
            {
                return IsOpaqueBSDF(FunctionCall->FunctionInputs[1].Input.Expression);
            }
            else if (Name == "mdl_df_weighted_layer")
            {
                return IsOpaqueBSDF(FunctionCall->FunctionInputs[1].Input.Expression) && IsOpaqueBSDF(FunctionCall->FunctionInputs[2].Input.Expression);
            }
            else
            {
                check(false);
            }
        }
    }
    return true;
}

static void RerouteNormal(const FMaterialExpressionConnection& From, UMaterialExpression* ToExpression)
{
    check((From.ConnectionType == EConnectionType::Expression) && From.ExpressionData.Expression && ToExpression);
    if (ToExpression->IsA<UMaterialExpressionMakeMaterialAttributes>())
    {
        Cast<UMaterialExpressionMakeMaterialAttributes>(ToExpression)->Normal.Connect(From.ExpressionData.Index, From.ExpressionData.Expression);
    }
    else if (ToExpression->IsA<UMaterialExpressionMaterialFunctionCall>())
    {
        check(Cast<UMaterialExpressionMaterialFunctionCall>(ToExpression)->FunctionInputs.Last().Input.InputName == TEXT("normal"));
        Cast<UMaterialExpressionMaterialFunctionCall>(ToExpression)->FunctionInputs.Last().Input.Connect(From.ExpressionData.Index, From.ExpressionData.Expression);
    }
    else
    {
        check(ToExpression->IsA<UMaterialExpressionStaticSwitch>());
        UMaterialExpressionStaticSwitch* StaticSwitch = Cast<UMaterialExpressionStaticSwitch>(ToExpression);
        RerouteNormal(From, StaticSwitch->A.Expression);
        RerouteNormal(From, StaticSwitch->B.Expression);
    }
}

static bool IsValidUE4PathChar(TCHAR c)
{
    return (c != '\'' && c != ' ' && c != ',' && c != '.' && c != '&' && c != '!' && c != '~' && c != '@' && c != '#');
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::MakeFunctionCall(const FString& CallPath, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& FunctionDefinition, const TArray<int32>& ArrayInputSizes,
    const FString& AssetNamePostfix, TArray<FMaterialExpressionConnection>& Inputs)
{
    check(FunctionDefinition != nullptr);

    FString AssetPath = FMDLImporterUtility::GetProjectMdlFunctionPath();
    const mi::IString* DecodedFunctionName = MDLModule->MDLFactory->decode_name(FunctionDefinition->get_mdl_name());
    FString FunctionName(UTF8_TO_TCHAR(DecodedFunctionName->get_c_str()));
    const mi::IString* DecodedModuleName = MDLModule->MDLFactory->decode_name(FunctionDefinition->get_module());
    FString FunctionModuleName(UTF8_TO_TCHAR(DecodedModuleName->get_c_str()));
    FunctionModuleName.RemoveFromStart(TEXT("mdl"));

    FString AssetName;
    if (FunctionName.StartsWith(FunctionModuleName))
    {
        FunctionName.RemoveAt(0, FunctionModuleName.Len());
        FString FunctionAssetPath = UnmangleMdlPath(FunctionModuleName);
        AssetName = MangleAssetPath(FunctionAssetPath) + FunctionName;
    }
    else
    {
        AssetName = FunctionName;
    }


    int Index = INDEX_NONE;
    TArray<FName> MDLTemplates;
    MDLTemplates.Append(UE4Templates);
    MDLTemplates.Append(BaseTemplates);
    MDLTemplates.Append(ApertureTemplates);
    MDLTemplates.Append(ReallusionTemplates);
    MDLTemplates.Append(MinecraftTemplates);

    for (auto TemplateName : MDLTemplates)
    {
        FString ModuleName = TemplateName.ToString();
        ModuleName.ReplaceInline(TEXT("/"), TEXT("::"));
        FString Module = TEXT("::") + ModuleName + TEXT("::");
        Index = AssetName.Find(Module);
        int32 ArgumentIndex = AssetName.Find(TEXT("("));
        // NOTE: ModuleName could be existed for the parameters, we should avoid the case
        if (Index != INDEX_NONE && ArgumentIndex != INDEX_NONE && Index < ArgumentIndex)
        {
            AssetName = AssetName.RightChop(Index);
            break;
        }
    }

    if (InGeometryExpression && (AssetName == TEXT("::base::coordinate_source(::base::texture_coordinate_system,int)")))
    {
        // in the geometry expression path, we don't support the full functionality of base::coordinate_source -> fall back to just the base::texture_coordinate_info constructor
        check(Inputs.Num() == 2);
        AssetName = TEXT("::base::texture_coordinate_info()");
        Inputs[0] = FMaterialExpressionConnection();
        Inputs[1] = FMaterialExpressionConnection();
        Inputs.Push(FMaterialExpressionConnection());
    }

    if (AssetName == TEXT("::OmniUe4Translucent::get_translucent_tint(color,float)"))
    {
        TranslucentOpacity = Inputs[1];
    }
    else if (AssetName == TEXT("::OmniUe4Translucent::get_emissive_intensity(color,float)"))
    {
        EmissiveOpacity = Inputs[1];
    }
    else if (AssetName == TEXT("::OmniUe4Subsurface::get_subsurface_color(color)"))
    {
        SubsurfaceColor = Inputs[0];
    }
    else if (AssetName == TEXT("::OmniUe4Subsurface::get_subsurface_opacity(float)"))
    {
        SubsurfaceOpacity = Inputs[0];
    }

    if (AssetName == TEXT("operator*(float4x4,float4x4)"))
    {
        AssetName = TEXT("_math_multiply_float4x4_float4x4");
    }
    else if (AssetName == TEXT("operator*(float4x4,float4)"))
    {
        AssetName = TEXT("_math_multiply_float4x4_float4");
    }
    else if (AssetName == TEXT("operator*(float4,float4x4)"))
    {
        AssetName = TEXT("_math_multiply_float4_float4x4");
    }
    else if (AssetName == TEXT("operator*(float3x3,float3x3)"))
    {
        AssetName = TEXT("_math_multiply_float3x3_float3x3");
    }
    else if (AssetName == TEXT("operator*(float3x3,float3)"))
    {
        AssetName = TEXT("_math_multiply_float3x3_float3");
    }
    else if (AssetName == TEXT("operator*(float3,float3x3)"))
    {
        AssetName = TEXT("_math_multiply_float3_float3x3");
    }
    else if (AssetName == TEXT("operator*(float2x2,float2x2)"))
    {
        AssetName = TEXT("_math_multiply_float2x2_float2x2");
    }
    else if (AssetName == TEXT("operator*(float2x2,float2)"))
    {
        AssetName = TEXT("_math_multiply_float2x2_float2");
    }
    else if (AssetName == TEXT("operator*(float2,float2x2)"))
    {
        AssetName = TEXT("_math_multiply_float2_float2x2");
    }
    else if (AssetName.Contains(TEXT("::distilling_support")))
    {
        AssetName = AssetName.Replace(TEXT("("), TEXT("_")).Replace(TEXT(")"), TEXT("")).Replace(TEXT(","), TEXT("_")).Replace(TEXT("::"), TEXT("_"));
    }
    else
    {
        AssetName = AssetName.Left(AssetName.Find(TEXT("("))).Replace(TEXT("::"), TEXT("_"));
    }
    AssetName = TEXT("mdl") + AssetName + AssetNamePostfix;

    int32 NormalmapIndex = INDEX_NONE;
    if (AssetName == TEXT("mdl_nvidia_DCC_support_ad_3dsmax_bitmap_normalmap"))
    {
        NormalmapIndex = 13;
    }
    else if (AssetName == TEXT("mdl_base_tangent_space_normal_texture"))
    {
        NormalmapIndex = 0;
    }

    if (NormalmapIndex > INDEX_NONE && NormalmapIndex < Inputs.Num())
    {
        ReplaceWithNormalmap(Inputs[NormalmapIndex].ExpressionData.Expression);
    }

    TArray<FMaterialExpressionConnection> Outputs;
    UMaterialFunction* Function = ::LoadFunction(AssetPath, AssetName, ArrayInputSizes);

    if (!Function)
    {
        // Try removing module
        FunctionName = FunctionName.Left(FunctionName.Find(TEXT("("))).Replace(TEXT("::"), TEXT("_"));
        FunctionName = TEXT("mdl") + FunctionName;
        Function = ::LoadFunction(AssetPath, FunctionName, ArrayInputSizes);
    }

    if (Function)
    {
        int32 TextureSelectionIndex = Inputs.FindLastByPredicate([](FMaterialExpressionConnection const& MEC) { return MEC.ConnectionType == EConnectionType::TextureSelection; });
        if (TextureSelectionIndex != INDEX_NONE)
        {
            FExpressionData Value = Inputs[TextureSelectionIndex].TextureSelectionData[0];
            FExpressionData True = Inputs[TextureSelectionIndex].TextureSelectionData[1];
            FExpressionData False = Inputs[TextureSelectionIndex].TextureSelectionData[2];
            Inputs[TextureSelectionIndex] = { True.Expression, True.Index, True.IsDefault };
            check(Inputs.FindLastByPredicate([](FMaterialExpressionConnection const& MEC) { return MEC.ConnectionType == EConnectionType::TextureSelection; }) == INDEX_NONE);
            UMaterialExpressionMaterialFunctionCall* TrueCall = NewMaterialExpressionFunctionCall(CurrentUE4Material, Function, Inputs);
            Inputs[TextureSelectionIndex] = { False.Expression, False.Index, False.IsDefault };
            UMaterialExpressionMaterialFunctionCall* FalseCall = NewMaterialExpressionFunctionCall(CurrentUE4Material, Function, Inputs);
            check(TrueCall->Outputs.Num() == FalseCall->Outputs.Num());

            FMaterialExpressionConnection ValueConnection(Value.Expression, Value.Index, Value.IsDefault);
            Outputs.Reserve(TrueCall->Outputs.Num());
            for (int32 i = 0; i < TrueCall->Outputs.Num(); i++)
            {
                if (IsStatic(ValueConnection))
                {
                    Outputs.Add(NewMaterialExpressionStaticSwitch(CurrentUE4Material, ValueConnection, { TrueCall, i }, { FalseCall, i }));
                }
                else
                {
                    Outputs.Add(NewMaterialExpressionIfEqual(CurrentUE4Material, ValueConnection, 0.0f, { FalseCall, i }, { TrueCall, i }));
                }
            }
        }
        else
        {
            UMaterialExpressionMaterialFunctionCall* FunctionCall = NewMaterialExpressionFunctionCall(CurrentUE4Material, Function, Inputs);
            Outputs.Reserve(FunctionCall->Outputs.Num());
            for (int32 i = 0; i < FunctionCall->Outputs.Num(); i++)
            {
                Outputs.Add(FMaterialExpressionConnection(FunctionCall, i));
            }
        }
    }
    else
    {
        FString Error = FString::Printf(TEXT("Failed to distill function %s"), *AssetName);
        ImportErrors.Add(Error);
    }

    return Outputs;
}

int32 FMDLMaterialImporter::GetStructInputSize(const FString& StructName)
{
    int32 InputCount = 0;
    mi::base::Handle<mi::neuraylib::IType_factory> TypeFactory(MDLModule->MDLFactory->create_type_factory(MDLModule->Transaction.get()));
    mi::base::Handle<const mi::neuraylib::IType_struct> TypeStruct(TypeFactory->create_struct(TCHAR_TO_UTF8(*StructName)));
    if (TypeStruct)
    {
        int32 FieldSize = TypeStruct->get_size();

        for (int32 FieldIndex = 0; FieldIndex < FieldSize; ++FieldIndex)
        {
            mi::base::Handle<const mi::neuraylib::IType> ParamType(TypeStruct->get_field_type(FieldIndex));
            if (ParamType)
            {
                switch (ParamType->get_kind())
                {
                case mi::neuraylib::IType::TK_STRUCT:
                {
                    const mi::base::Handle<const mi::neuraylib::IType_struct> StructType(ParamType.get_interface<const mi::neuraylib::IType_struct>());
                    FString StructSymbol = StructType->get_symbol();
                    if (StructSymbol == "::base::texture_coordinate_info")
                    {
                        InputCount += 3;
#if defined(USE_WORLD_ALIGNED_TEXTURES)
                        InputCount += 3;
#endif
                    }
                    else if (StructSymbol == "::base::anisotropy_return")
                    {
                        InputCount += 3;
                    }
                    else if (StructSymbol == "::base::texture_return")
                    {
                        InputCount += 2;
                    }
                    else if (StructSymbol == "::base::color_layer")
                    {
                        InputCount += 3;
                    }
                    else
                    {
                        InputCount += GetStructInputSize(StructSymbol);
                    }
                    break;
                }

                default:
                    ++InputCount;
                    break;
                }
            }
        }
    }

    return InputCount;
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::CreateExpressionFunctionCall(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, const mi::base::Handle<const mi::neuraylib::IExpression_direct_call>& MDLFunctionCall, const FString& CallPath)
{
    TArray<FMaterialExpressionConnection> EmptyError;
    if (ImportErrors.Num() > 0)
    {
        return EmptyError;
    }

    mi::base::Handle<mi::neuraylib::IExpression_factory> ExpressionFactory(
        MDLModule->MDLFactory->create_expression_factory(MDLModule->Transaction.get()));

    const char* Name = MDLFunctionCall->get_definition();
    const mi::base::Handle<const mi::neuraylib::IFunction_definition> FunctionDefinition(MDLModule->Transaction->access<mi::neuraylib::IFunction_definition>(Name));
    const mi::base::Handle<const mi::neuraylib::IExpression_list> Arguments(MDLFunctionCall->get_arguments());
    const mi::base::Handle<const mi::neuraylib::IExpression_list> DefaultArguments(FunctionDefinition->get_defaults());
    const mi::base::Handle<const mi::neuraylib::IType> ReturnType(FunctionDefinition->get_return_type());
    const mi::base::Handle<const mi::neuraylib::IType_list> ParameterTypes(FunctionDefinition->get_parameter_types());
    mi::neuraylib::IFunction_definition::Semantics Semantic = FunctionDefinition->get_semantic();
    FString FunctionMDLName = FunctionDefinition->get_mdl_name();
    const mi::IString* DecodedFunctionName = MDLModule->MDLFactory->decode_name(FunctionDefinition->get_mdl_name());
    FString FunctionName(UTF8_TO_TCHAR(DecodedFunctionName->get_c_str()));

    FString AssetNamePostfix;
    TArray<FMaterialExpressionConnection> Inputs;
    TArray<FName> InputsNames;
    TArray<int32> ArrayInputSizes;
    int32 NumArguments = Arguments->get_size();
    for (mi::Size i = 0; i < NumArguments; i++)
    {
        const mi::base::Handle<const mi::neuraylib::IExpression> Argument(Arguments->get_expression(i));
        mi::base::Handle<const mi::neuraylib::IType> ParameterType(ParameterTypes->get_type(i));

        // Use argument type instead of parameter type in case of arrays so to have immediate size information
        if (ParameterType == nullptr || ParameterType->get_kind() == mi::neuraylib::IType::TK_ARRAY)
        {
            ParameterType = mi::base::make_handle(Argument->get_type());
        }

        InputsNames.Add(Arguments->get_name(i));
        ArrayInputSizes.AddZeroed();
        
        mi::neuraylib::IType::Kind ParameterKind = mi::base::make_handle(ParameterType->skip_all_type_aliases())->get_kind();
        if (ParameterKind != mi::neuraylib::IType::TK_STRING)
        {
            // Apply additional processing if parameter type is an array
            if (ParameterKind == mi::neuraylib::IType::TK_ARRAY)
            {
                int32 ArraySize = CalcArraySize(Argument);
                check(0 < ArraySize);
                ArrayInputSizes.Last() = ArraySize;
            }

            TArray<FMaterialExpressionConnection> ArgumentExpressions = CreateExpression(CompiledMaterial, Argument, CallPath + TEXT(".") + Arguments->get_name(i));
            if (ArgumentExpressions.Num() == 0)
            {
                return TArray<FMaterialExpressionConnection>();
            }

            // mark default arguments as default
            const mi::base::Handle<const mi::neuraylib::IExpression> DefaultArgument(DefaultArguments->get_expression(FunctionDefinition->get_parameter_name(i)));
            if (ExpressionFactory->compare(Argument.get(), DefaultArgument.get()) == 0)
            {
                for (int32 k = 0; k < ArgumentExpressions.Num(); k++)
                {
                    check(ArgumentExpressions[k].ConnectionType == EConnectionType::Expression);
                    ArgumentExpressions[k].ExpressionData.IsDefault = true;
                }
            }

            Inputs.Append(ArgumentExpressions);
        }
        // NOTE: BSDF function need a string input as name, but mdl load functions ignored them all.
        // We just input the string for those functions we want to.
        // We only support constant and parameter for now.
        else if (Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_FLOAT
            || Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_FLOAT3
            || Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_FLOAT4
            || Semantic == mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_COLOR
            || Semantic == mi::neuraylib::IFunction_definition::DS_NOT_EQUAL
            || Semantic == mi::neuraylib::IFunction_definition::DS_EQUAL)
        {
            mi::neuraylib::IExpression::Kind Kind = Argument->get_kind();

            if (Kind == mi::neuraylib::IExpression::EK_CONSTANT)
            {
                mi::base::Handle<const mi::neuraylib::IValue> Constant = mi::base::make_handle(Argument.get_interface<const mi::neuraylib::IExpression_constant>()->get_value());
                if (Constant->get_kind() == mi::neuraylib::IValue::VK_STRING)
                {
                    // convert string to enum
                    const mi::base::Handle<const mi::neuraylib::IValue_string> Value(Constant.get_interface<const mi::neuraylib::IValue_string>());

                    FString String = Value->get_value();
                    int32 Enum = FMDLImporterUtility::SceneDataStringToEnum(String);
                    Inputs.Append({ NewMaterialExpressionConstant(CurrentUE4Material, Enum) });
                }
            }
            else if (Kind == mi::neuraylib::IExpression::EK_PARAMETER)
            {
                Inputs.Append(GetExpressionParameter(Argument.get_interface<const mi::neuraylib::IExpression_parameter>()));
            }
            else if (Kind == mi::neuraylib::IExpression::EK_TEMPORARY)
            {
                Inputs.Append(CreateExpressionTemporary(CompiledMaterial, Argument.get_interface<const mi::neuraylib::IExpression_temporary>(), CallPath));
            }
        }
        else if (Semantic == mi::neuraylib::IFunction_definition::DS_UNKNOWN)
        {
            // TODO: String input
            Inputs.Append({ NewMaterialExpressionConstant(CurrentUE4Material, 0) });
        }
    }

    // set Postfix if needed
    switch (Semantic)
    {
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_COS:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LOG:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LOG2:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LOG10:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SIN:
            check(Inputs.Num() == 1);
            {
                mi::base::Handle<const mi::neuraylib::IType> Type(ParameterTypes->get_type(mi::Size(0)));
                mi::neuraylib::IType::Kind kind = Type->get_kind();
                switch (kind)
                {
                    case mi::neuraylib::IType::TK_COLOR:
                        AssetNamePostfix = "_float3";
                        break;
                    case mi::neuraylib::IType::TK_FLOAT:
                        AssetNamePostfix = "_float";
                        break;
                    case mi::neuraylib::IType::TK_VECTOR:
                        {
                            const mi::base::Handle<const mi::neuraylib::IType_vector> VectorType(Type->get_interface<const mi::neuraylib::IType_vector>());
                            check(mi::base::make_handle(VectorType->get_element_type())->get_kind() == mi::neuraylib::IType::TK_FLOAT);
                            AssetNamePostfix = "_float" + FString::FromInt(VectorType->get_size());
                        }
                        break;
                    default:
                        check(false);
                }
            }
            break;

        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT2:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT3:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT4:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_COLOR:
            {
                mi::base::Handle<const mi::neuraylib::IType> Type(ParameterTypes->get_type(mi::Size(0)));
                mi::base::Handle<const mi::neuraylib::IType> NonAliasType(Type->skip_all_type_aliases());
                mi::neuraylib::IType::Kind ParameterKind = NonAliasType->get_kind();		
                if (ParameterKind == mi::neuraylib::IType::TK_TEXTURE)
                {
                    const mi::base::Handle<const mi::neuraylib::IType_texture> TextureType(NonAliasType->get_interface<const mi::neuraylib::IType_texture>());
                    switch(TextureType->get_shape())
                    {
                    case mi::neuraylib::IType_texture::Shape::TS_2D:
                        if (IsNormalPostfix(Inputs[0].ExpressionData.Expression))
                        {
                            AssetNamePostfix = "_normal";
                        }
                        else
                        {
                            AssetNamePostfix = "_2d";
                        }
                        break;
                    case mi::neuraylib::IType_texture::Shape::TS_3D:
                        check(false); // TODO??
                        break;
                    case mi::neuraylib::IType_texture::Shape::TS_CUBE:
                        AssetNamePostfix = "_cube";
                        break;
                    }
                }
            }
            break;
    }

    // Special distribution function handling, step one: add a normal to the bsdfs
    switch (Semantic)
    {
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_TRANSMISSION_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SPECULAR_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SIMPLE_GLOSSY_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_BACKSCATTERING_GLOSSY_REFLECTION_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_NORMALIZED_MIX:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_THIN_FILM:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_TINT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIRECTIONAL_FACTOR:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_CURVE_FACTOR:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_BECKMANN_SMITH_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_SMITH_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_BECKMANN_VCAVITIES_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_VCAVITIES_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_WARD_GEISLER_MORODER_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CHIANG_HAIR_BSDF:
            Inputs.Push(CurrentNormalExpression);
            break;

        // These DFs need some special handling to determine if the CurrentNormalExpression is to be pushed!
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CLAMPED_MIX:	// only if mixing BSDFs
            check(false);
            break;
    }

    // Special distribution function handling, step two: special material settings
    switch (Semantic)
    {
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_TRANSMISSION_BSDF:
            SetTransparency(CurrentUE4Material);
            break;
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SPECULAR_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SIMPLE_GLOSSY_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_BECKMANN_SMITH_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_SMITH_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_BECKMANN_VCAVITIES_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_VCAVITIES_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_BSDF:
            {
                int32 NameIndex = InputsNames.FindLastByPredicate([](FName const& Name) { return Name.Compare("mode") == 0; });
                if (NameIndex != INDEX_NONE && IsTransparentMode(Inputs[NameIndex]))
                {
                    SetTransparency(CurrentUE4Material);
                }
            }
            break;
    }

    // weighted_layer, fresnel_layer, custom_curve_layer, and measured_curve_layer reroute their normal to their layer argument
    switch(Semantic)
    {
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_WEIGHTED_LAYER:
            check(Inputs[1].ConnectionType == EConnectionType::Expression);
            check(Inputs[1].ExpressionData.Expression && Inputs[3].ExpressionData.Expression);
            if (Inputs[1].ExpressionData.Expression->IsA<UMaterialExpressionStaticSwitch>())
            {
                UMaterialExpressionStaticSwitch* StaticSwitch = Cast<UMaterialExpressionStaticSwitch>(Inputs[1].ExpressionData.Expression);
                check(StaticSwitch->A.Expression && StaticSwitch->B.Expression && StaticSwitch->Value.Expression);
                RerouteNormal(Inputs[3], StaticSwitch->A.Expression);
                RerouteNormal(Inputs[3], StaticSwitch->B.Expression);
            }
            else
            {
                RerouteNormal(Inputs[3], Inputs[1].ExpressionData.Expression);
            }
            break;
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_FRESNEL_LAYER:
            check((Inputs[2].ConnectionType == EConnectionType::Expression) && Inputs[2].ExpressionData.Expression);
            check((Inputs[3].ConnectionType == EConnectionType::Expression) && Inputs[3].ExpressionData.Expression);
            check((Inputs[4].ConnectionType == EConnectionType::Expression) && Inputs[4].ExpressionData.Expression);
            RerouteNormal(Inputs[4], Inputs[2].ExpressionData.Expression);
            break;
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CUSTOM_CURVE_LAYER:
            check(Inputs[4].ConnectionType == EConnectionType::Expression);
            check((Inputs[5].ConnectionType == EConnectionType::Expression) && Inputs[5].ExpressionData.Expression);
            check((Inputs[6].ConnectionType == EConnectionType::Expression) && Inputs[6].ExpressionData.Expression);
            check(Inputs[4].ExpressionData.Expression || (Inputs[6].ExpressionData.Expression->IsA<UMaterialExpressionMaterialFunctionCall>() && Cast<UMaterialExpressionMaterialFunctionCall>(Inputs[6].ExpressionData.Expression)->MaterialFunction->GetName() == TEXT("mdl_state_normal")));
            if (Inputs[4].ExpressionData.Expression)
            {
                RerouteNormal(Inputs[6], Inputs[4].ExpressionData.Expression);
            }
            break;
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_CURVE_LAYER:
            check((Inputs[2].ConnectionType == EConnectionType::Expression) && Inputs[2].ExpressionData.Expression);
            check((Inputs[3].ConnectionType == EConnectionType::Expression) && Inputs[3].ExpressionData.Expression);
            check((Inputs[4].ConnectionType == EConnectionType::Expression) && Inputs[4].ExpressionData.Expression);
            RerouteNormal(Inputs[4], Inputs[2].ExpressionData.Expression);
            break;
    }

    // fresnel_layer and custom_curve_layer use the clear coat shading model
    switch (Semantic)
    {
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_FRESNEL_LAYER:
            check(Inputs[4].ConnectionType == EConnectionType::Expression);
            SetClearCoatNormal(Inputs[3], Inputs[4].ExpressionData.Expression);
            break;
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CUSTOM_CURVE_LAYER:
            check(Inputs[6].ConnectionType == EConnectionType::Expression);
            SetClearCoatNormal(Inputs[5], Inputs[6].ExpressionData.Expression);
            break;
    }

    if (FunctionName.StartsWith("::base::file_texture"))
    {
        mi::base::Handle<const mi::neuraylib::IType> Type(ParameterTypes->get_type(mi::Size(0)));
        mi::base::Handle<const mi::neuraylib::IType> NonAliasType(Type->skip_all_type_aliases());
        mi::neuraylib::IType::Kind ParameterKind = NonAliasType->get_kind();		
        if (ParameterKind == mi::neuraylib::IType::TK_TEXTURE)
        {
            const mi::base::Handle<const mi::neuraylib::IType_texture> TextureType(NonAliasType->get_interface<const mi::neuraylib::IType_texture>());
            switch(TextureType->get_shape())
            {
            case mi::neuraylib::IType_texture::Shape::TS_2D:
                if (IsNormalPostfix(Inputs[0].ExpressionData.Expression))
                {
                    AssetNamePostfix = "_normal";
                }
                break;
            }
        }
    }

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    if (FunctionName.StartsWith("::base::file_bump_texture") ||
        FunctionName.StartsWith("::base::file_texture") ||
        FunctionName.StartsWith("::base::tangent_space_normal_texture") ||
        FunctionName.StartsWith("::nvidia::DCC_support::ad_3dsmax_bitmap") ||
        FunctionName.StartsWith("::nvidia::DCC_support::ad_3dsmax_dent") ||
        FunctionName.StartsWith("::nvidia::DCC_support::ad_3dsmax_noise"))
    {
        Inputs.Push(UseWorldAlignedTextureParameter);
    }

    if (FunctionName == "::base::texture_coordinate_info(float3,float3,float3)")
    {
        // very special handling for this function, which in fact is a DS_ELEM_CONSTRUCTOR
        // -> needs to add the world-aligned coordinate stuff, so handle it as if it was a general function call
        check(Semantic == mi::neuraylib::IFunction_definition::DS_ELEM_CONSTRUCTOR);
        Semantic = mi::neuraylib::IFunction_definition::DS_UNKNOWN;
    }
#endif

    // General material function and operator handling
    switch (Semantic)
    {
        case mi::neuraylib::IFunction_definition::DS_UNKNOWN:		// handle all the functions
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_ANISOTROPIC_VDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_BACKSCATTERING_GLOSSY_REFLECTION_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_COLOR_FRESNEL_LAYER:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CUSTOM_CURVE_LAYER:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_EDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIFFUSE_TRANSMISSION_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_DIRECTIONAL_FACTOR:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_FRESNEL_LAYER:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_LIGHT_PROFILE_MAXIMUM:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_LIGHT_PROFILE_POWER:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_CURVE_FACTOR:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MEASURED_EDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_BECKMANN_SMITH_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_BECKMANN_VCAVITIES_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_SMITH_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_MICROFACET_GGX_VCAVITIES_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_NORMALIZED_MIX:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SPECULAR_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SIMPLE_GLOSSY_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_CHIANG_HAIR_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_SPOT_EDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_THIN_FILM:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_TINT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_WARD_GEISLER_MORODER_BSDF:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DF_WEIGHTED_LAYER:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_AVERAGE:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_COS:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LOG:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LOG2:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LOG10:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LUMINANCE:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_MAX_VALUE:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_MIN_VALUE:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SIN:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_ANIMATION_TIME:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_DIRECTION:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_GEOMETRY_NORMAL:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_GEOMETRY_TANGENT_U:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_GEOMETRY_TANGENT_V:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_METERS_PER_SCENE_UNIT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_OBJECT_ID:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_POSITION:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_SCENE_UNITS_PER_METER:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TANGENT_SPACE:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TEXTURE_COORDINATE:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TEXTURE_SPACE_MAX:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TEXTURE_TANGENT_U:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TEXTURE_TANGENT_V:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TRANSFORM_POINT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TRANSFORM_VECTOR:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_TRANSFORM_NORMAL:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_COLOR:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT2:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT3:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_LOOKUP_FLOAT4:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_TEXTURE_ISVALID:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_FLOAT:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_FLOAT3:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_FLOAT4:
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_SCENE_DATA_LOOKUP_COLOR:
            return MakeFunctionCall(CallPath, FunctionDefinition, ArrayInputSizes, AssetNamePostfix, Inputs);

        case mi::neuraylib::IFunction_definition::DS_CONV_CONSTRUCTOR:
        case mi::neuraylib::IFunction_definition::DS_ELEM_CONSTRUCTOR:
        case mi::neuraylib::IFunction_definition::DS_MATRIX_ELEM_CONSTRUCTOR:
        case mi::neuraylib::IFunction_definition::DS_MATRIX_DIAG_CONSTRUCTOR:
            return CreateExpressionConstructorCall(ReturnType, Inputs);

#pragma region Unary Operators
        case mi::neuraylib::IFunction_definition::DS_CONV_OPERATOR:
            check(Inputs.Num() == 1);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);
            check(Inputs[0].ExpressionData.Expression->IsA<UMaterialExpressionScalarParameter>() && (ReturnType->get_kind() == mi::neuraylib::IType::TK_INT));
            return Inputs;

        case mi::neuraylib::IFunction_definition::DS_LOGICAL_NOT:
        {
            check(Inputs.Num() == 1);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);

            if (IsStatic(Inputs[0]))
            {
                return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0], NewMaterialExpressionStaticBool(CurrentUE4Material, false), NewMaterialExpressionStaticBool(CurrentUE4Material, true)) };
            }
            else
            {
                return{ NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], 0.0f, 1.0f, 0.0f) };
            }
        }
        case mi::neuraylib::IFunction_definition::DS_POSITIVE:
        {
            check(Inputs.Num() == 1);

            return Inputs;
        }
        case mi::neuraylib::IFunction_definition::DS_NEGATIVE:
        {
            check(Inputs.Num() == 1);

            return { NewMaterialExpressionNegate(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_PRE_INCREMENT:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionAdd(CurrentUE4Material, Inputs[0], 1.0f) };
        }
        case mi::neuraylib::IFunction_definition::DS_PRE_DECREMENT:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionSubtract(CurrentUE4Material, Inputs[0], 1.0f) };
        }
#pragma endregion

#pragma region Binary Operators
        case mi::neuraylib::IFunction_definition::DS_MULTIPLY:
        {
            // TODO: Matrix * Vector and Matrix * Matrix for all matrix sizes
            // float4 x float4x4
            // float4x4 x float4
            // float4x4 x float4x4
            // float3 x float3x3
            // float3x3 x float3
            // float3x3 x float3x3
            // float2 x float2x2
            // float2x2 x float2
            // float2x2 x float2x2
            if (Inputs.Num() == 3 || Inputs.Num() == 4 || Inputs.Num() == 5 || Inputs.Num() == 6 || Inputs.Num() == 8)
            {
                return MakeFunctionCall(CallPath, FunctionDefinition, ArrayInputSizes, AssetNamePostfix, Inputs);
            }
            else
            {
                check(Inputs.Num() == 2);

                return { NewMaterialExpressionMultiply(CurrentUE4Material, Inputs[0], Inputs[1]) };
            }
        }
        case mi::neuraylib::IFunction_definition::DS_DIVIDE:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionDivide(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_MODULO:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionFmod(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_PLUS:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionAdd(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_MINUS:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionSubtract(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_LESS:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionIfLess(CurrentUE4Material, Inputs[0], Inputs[1], 1.0f, 0.0f) };
        }
        case mi::neuraylib::IFunction_definition::DS_LESS_OR_EQUAL:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionIfGreater(CurrentUE4Material, Inputs[1], Inputs[0], 1.0f, 0.0f) };
        }
        case mi::neuraylib::IFunction_definition::DS_GREATER_OR_EQUAL:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionIfLess(CurrentUE4Material, Inputs[1], Inputs[0], 1.0f, 0.0f) };
        }
        case mi::neuraylib::IFunction_definition::DS_GREATER:
        {
            check(Inputs.Num() == 2);

            return { NewMaterialExpressionIfGreater(CurrentUE4Material, Inputs[0], Inputs[1], 1.0f, 0.0f) };
        }
        case mi::neuraylib::IFunction_definition::DS_EQUAL:
        {
            check(Inputs.Num() == 2);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);
            check(Inputs[1].ConnectionType == EConnectionType::Expression);

            if (Inputs[0].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>())
            {
                check(!Inputs[1].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>());
                return{ CompareStaticBool(CurrentUE4Material, Cast<UMaterialExpressionStaticBool>(Inputs[0].ExpressionData.Expression), Inputs[1].ExpressionData.Expression, true) };
            }
            else if (Inputs[1].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>())
            {
                return{ CompareStaticBool(CurrentUE4Material, Cast<UMaterialExpressionStaticBool>(Inputs[1].ExpressionData.Expression), Inputs[0].ExpressionData.Expression, true) };
            }
            else
            {
                return{ NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], Inputs[1], 1.0f, 0.0f) };
            }
        }
        case mi::neuraylib::IFunction_definition::DS_NOT_EQUAL:
        {
            check(Inputs.Num() == 2);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);
            check(Inputs[1].ConnectionType == EConnectionType::Expression);

            if (Inputs[0].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>())
            {
                check(!Inputs[1].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>());
                return{ CompareStaticBool(CurrentUE4Material, Cast<UMaterialExpressionStaticBool>(Inputs[0].ExpressionData.Expression), Inputs[1].ExpressionData.Expression, false) };
            }
            else if (Inputs[1].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>())
            {
                return{ CompareStaticBool(CurrentUE4Material, Cast<UMaterialExpressionStaticBool>(Inputs[1].ExpressionData.Expression), Inputs[0].ExpressionData.Expression, false) };
            }
            else
            {
                return{ NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], Inputs[1], 0.0f, 1.0f) };
            }
        }
        case mi::neuraylib::IFunction_definition::DS_LOGICAL_AND:
        {
            check(Inputs.Num() == 2);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);
            check(Inputs[1].ConnectionType == EConnectionType::Expression);

            if (IsStatic(Inputs[0]))
            {
                if (IsStatic(Inputs[1]))
                {
                    return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0],
                        NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[1],
                            NewMaterialExpressionStaticBool(CurrentUE4Material, true),
                            NewMaterialExpressionStaticBool(CurrentUE4Material, false)),
                        NewMaterialExpressionStaticBool(CurrentUE4Material, false)) };
                }
                else
                {
                    return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0],
                        NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[1], 0.0f, 0.0f, 1.0f),
                        0.0f) };
                }
            }
            else if (IsStatic(Inputs[1]))
            {
                return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[1],
                    NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], 0.0f, 0.0f, 1.0f),
                    0.0f) };
            }
            else
            {
                return{ NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], 0.0f, 0.0f, NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[1], 0.0f, 0.0f, 1.0f)) };
            }
        }
        case mi::neuraylib::IFunction_definition::DS_LOGICAL_OR:
        {
            check(Inputs.Num() == 2);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);
            check(Inputs[1].ConnectionType == EConnectionType::Expression);

            if (IsStatic(Inputs[0]))
            {
                if (IsStatic(Inputs[1]))
                {
                    return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0],
                        NewMaterialExpressionStaticBool(CurrentUE4Material, true),
                        NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[1],
                            NewMaterialExpressionStaticBool(CurrentUE4Material, true),
                            NewMaterialExpressionStaticBool(CurrentUE4Material, false))) };
                }
                else
                {
                    return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0],
                        1.0f,
                        NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[1], 0.0f, 0.0f, 1.0f)) };
                }
            }
            else if (IsStatic(Inputs[1]))
            {
                return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[1],
                    1.0f,
                    NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], 0.0f, 0.0f, 1.0f)) };
            }
            else
            {
                return{ NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], 0.0f, NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[1], 0.0f, 0.0f, 1.0f), 1.0f) };
            }
        }
#pragma endregion

#pragma region Ternary Operators
        case mi::neuraylib::IFunction_definition::DS_TERNARY:
        {
            check((Inputs.Num() - 1) % 2 == 0 && Inputs.Num() >= 3);
            check(Inputs[0].ConnectionType == EConnectionType::Expression);

            const int32 NumOutputs = (Inputs.Num() - 1) / 2;
            TArray<FMaterialExpressionConnection> Outputs;
            Outputs.Reserve(NumOutputs);

            for (int32 i = 1; i <= NumOutputs; i++)
            {
                if (IsTexture(Inputs[i]))
                {
                    // StaticSwitch and If on Texture would fail -> store everything for evaluating later on
                    check((Inputs.Num() == 3)
                        && (Inputs[0].ConnectionType == EConnectionType::Expression)
                        && (Inputs[1].ConnectionType == EConnectionType::Expression)
                        && (Inputs[2].ConnectionType == EConnectionType::Expression));
                    Outputs.Add(FMaterialExpressionConnection(Inputs[0].ExpressionData, Inputs[1].ExpressionData, Inputs[2].ExpressionData));
                }
                else if (IsStatic(Inputs[0]))
                {
                    Outputs.Add(NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0], Inputs[i], Inputs[i + NumOutputs]));
                }
                else if (IsMaterialAttribute(Inputs[i]))
                {
                    // If on MaterialAttribute would fail -> use a helper function instead
                    // Note: static switch on a MaterialAttribute works! Therefore, check for MaterialAttribute after check for Static !
                    check(IsMaterialAttribute(Inputs[i + NumOutputs]));
                    Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material,
                        ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_select_bsdf")), { Inputs[0], Inputs[i], Inputs[i + NumOutputs], CurrentNormalExpression }));
                }
                else
                {
                    Outputs.Add(NewMaterialExpressionIfEqual(CurrentUE4Material, Inputs[0], 0.0f, Inputs[i + NumOutputs], Inputs[i]));
                }
            }

            return Outputs;
        }
#pragma endregion

#pragma region Math Intrinsics
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SMOOTHSTEP:
        {
            check(Inputs.Num() == 3);

            return { NewMaterialExpressionSmoothStep(CurrentUE4Material, Inputs[0], Inputs[1], Inputs[2]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_ABS:
        {
            check(Inputs.Num() == 1);

            return { NewMaterialExpressionAbs(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_ACOS:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionArccosine(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_ASIN:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionArcsine(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_ATAN:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionArctangent(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_ATAN2:
        {
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionArctangent2(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_CEIL:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionCeil(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_CLAMP:
        {
            check(Inputs.Num() == 3);

            return{ NewMaterialExpressionClamp(CurrentUE4Material, Inputs[0], Inputs[1], Inputs[2]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_CROSS:
        {
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionCrossProduct(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_DEGREES:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionMultiply(CurrentUE4Material, Inputs[0], 180.0f / PI) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_DISTANCE:
        {
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionDistance(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_DOT:
        {
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionDotProduct(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_EXP:
        {
            check(Inputs.Num() == 1);

            const float e = 2.71828f;
            return{ NewMaterialExpressionPower(CurrentUE4Material, e, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_EXP2:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionPower(CurrentUE4Material, 2.0f, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_FLOOR:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionFloor(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_FMOD:
        {
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionFmod(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_FRAC:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionFrac(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LENGTH:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionSquareRoot(CurrentUE4Material, NewMaterialExpressionDotProduct(CurrentUE4Material, Inputs[0], Inputs[0])) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_LERP:
        {
            check(Inputs.Num() == 3);

            return{ NewMaterialExpressionLinearInterpolate(CurrentUE4Material, Inputs[0], Inputs[1], Inputs[2]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_MAX:
        {
            check(Inputs.Num() == 2);

            uint32 ACount = ComponentCount(Inputs[0]);
            uint32 BCount = ComponentCount(Inputs[1]);
            if (ACount == 1 && BCount != 1)
            {
                switch (BCount)
                {
                case 2:
                    return{ NewMaterialExpressionMax(CurrentUE4Material, NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { Inputs[0], Inputs[0] }), Inputs[1]) };
                case 3:
                    return{ NewMaterialExpressionMax(CurrentUE4Material, NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3, { Inputs[0], Inputs[0], Inputs[0] }), Inputs[1]) };
                case 4:
                    return{ NewMaterialExpressionMax(CurrentUE4Material, NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4, { Inputs[0], Inputs[0], Inputs[0], Inputs[0] }), Inputs[1]) };
                }
            }
            else if (ACount != 1 && BCount == 1)
            {
                switch (ACount)
                {
                case 2:
                    return{ NewMaterialExpressionMax(CurrentUE4Material, Inputs[0], NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { Inputs[1], Inputs[1] }) )};
                case 3:
                    return{ NewMaterialExpressionMax(CurrentUE4Material, Inputs[0], NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3, { Inputs[1], Inputs[1], Inputs[1] }) )};
                case 4:
                    return{ NewMaterialExpressionMax(CurrentUE4Material, Inputs[0], NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4, { Inputs[1], Inputs[1], Inputs[1], Inputs[1] }) )};
                }
            }

            return{ NewMaterialExpressionMax(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_MIN:
        {
            check(Inputs.Num() == 2);

            uint32 ACount = ComponentCount(Inputs[0]);
            uint32 BCount = ComponentCount(Inputs[1]);
            if (ACount == 1 && BCount != 1)
            {
                switch (BCount)
                {
                case 2:
                    return{ NewMaterialExpressionMin(CurrentUE4Material, NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { Inputs[0], Inputs[0] }), Inputs[1]) };
                case 3:
                    return{ NewMaterialExpressionMin(CurrentUE4Material, NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3, { Inputs[0], Inputs[0], Inputs[0] }), Inputs[1]) };
                case 4:
                    return{ NewMaterialExpressionMin(CurrentUE4Material, NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4, { Inputs[0], Inputs[0], Inputs[0], Inputs[0] }), Inputs[1]) };
                }
            }
            else if (ACount != 1 && BCount == 1)
            {
                switch (ACount)
                {
                case 2:
                    return{ NewMaterialExpressionMin(CurrentUE4Material, Inputs[0], NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { Inputs[1], Inputs[1] })) };
                case 3:
                    return{ NewMaterialExpressionMin(CurrentUE4Material, Inputs[0], NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3, { Inputs[1], Inputs[1], Inputs[1] })) };
                case 4:
                    return{ NewMaterialExpressionMin(CurrentUE4Material, Inputs[0], NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4, { Inputs[1], Inputs[1], Inputs[1], Inputs[1] })) };
                }
            }

            return{ NewMaterialExpressionMin(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_MODF:
        {
            check(Inputs.Num() == 1);

            UMaterialExpressionFrac* FractionalPart = NewMaterialExpressionFrac(CurrentUE4Material, Inputs[0]);
            UMaterialExpressionSubtract* IntegralPart = NewMaterialExpressionSubtract(CurrentUE4Material, Inputs[0], FractionalPart);

            return{ IntegralPart, FractionalPart };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_NORMALIZE:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionNormalize(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_POW:
        {
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionPower(CurrentUE4Material, Inputs[0], Inputs[1]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_RADIANS:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionMultiply(CurrentUE4Material, Inputs[0], PI / 180.0f) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_ROUND:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionFloor(CurrentUE4Material, NewMaterialExpressionAdd(CurrentUE4Material, Inputs[0], 0.5f)) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_RSQRT:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionDivide(CurrentUE4Material, 1.0f, NewMaterialExpressionSquareRoot(CurrentUE4Material, Inputs[0])) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SATURATE:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionSaturate(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SIGN:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionSign(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SINCOS:
        {
            check(Inputs.Num() == 1);

            return{
                NewMaterialExpressionSine(CurrentUE4Material, Inputs[0]),
                NewMaterialExpressionCosine(CurrentUE4Material, Inputs[0])
            };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_SQRT:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionSquareRoot(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_TAN:
        {
            check(Inputs.Num() == 1);

            return{ NewMaterialExpressionTangent(CurrentUE4Material, Inputs[0]) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_MATH_BLACKBODY:
        {
            check(Inputs.Num() == 1);

            return { NewMaterialExpressionBlackBody(CurrentUE4Material, Inputs[0]) };
        }
#pragma endregion

#pragma region State Intrinsics
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_NORMAL:
        {
            check(Inputs.Num() == 0);

            return { CurrentNormalExpression };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_STATE_ROUNDED_CORNER_NORMAL:		// just do something for rounded corner normal... we can't do that!
        {
            check(Inputs.Num() == 3);
            return{ CurrentNormalExpression };
        }
#pragma endregion

#pragma region Tex Intrinsics
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_WIDTH:
        {
            // MDL 1.7 add frame
            check(Inputs.Num() == 1 || Inputs.Num() == 2 || Inputs.Num() == 3);

            return { NewMaterialExpressionComponentMask(CurrentUE4Material, NewMaterialExpressionTextureProperty(CurrentUE4Material, Inputs[0], TMTM_TextureSize), 1) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_HEIGHT:
        {
            // MDL 1.7 add frame
            check(Inputs.Num() >= 1 || Inputs.Num() == 2 || Inputs.Num() == 3);

            return { NewMaterialExpressionComponentMask(CurrentUE4Material, NewMaterialExpressionTextureProperty(CurrentUE4Material, Inputs[0], TMTM_TextureSize), 2) };
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_TEX_DEPTH:
        {
            // MDL 1.7 add frame
            // UE4 doesn't know about 3D textures ?? Does that mean, the depth is always 1 ?
            check(Inputs.Num() == 2);

            return{ NewMaterialExpressionConstant(CurrentUE4Material, 1.0f) };
        }
#pragma endregion

#pragma region DAG Backend Intrinsics
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DAG_FIELD_ACCESS:
        {
            if (FunctionName == "::base::anisotropy_return.roughness_u(::base::anisotropy_return)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 3));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[0] });
            }
            else if (FunctionName == "::base::anisotropy_return.roughness_v(::base::anisotropy_return)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 3));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[1] });
            }
            else if (FunctionName == "::base::anisotropy_return.tangent_u(::base::anisotropy_return)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 3));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[2] });
            }
            else if (FunctionName == "::base::texture_return.tint(::base::texture_return)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 2));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[0] });
            }
            else if (FunctionName == "::base::color_layer.layer_color(::base::color_layer)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 3));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[0] });
            }
            else if (FunctionName == "::base::color_layer.weight(::base::color_layer)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 3));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[1] });
            }
            else if (FunctionName == "::base::color_layer.mode(::base::color_layer)")
            {
                check((Inputs.Num() == 0) || (Inputs.Num() == 3));
                return (Inputs.Num() == 0) ? Inputs : TArray<FMaterialExpressionConnection>({ Inputs[2] });
            }
            else
            {
                unsigned int crc32 = FCrc::StrCrc32(*FunctionName);
                switch (crc32)
                {
                case   84791706: // material_surface.scattering(material_surface)
                case  146468741: // material.thin_walled(material)
                case 1818454712: // ::base::texture_coordinate_info.position(::base::texture_coordinate_info)
                case 4090769780: // ::base::texture_coordinate_info.position
                    check(1 <= Inputs.Num());
                    return{ Inputs[0] };
                case  820916979: // ::base::texture_return.mono(::base::texture_return)
                case 3402110799: // ::base::texture_coordinate_info.tangent_u(::base::texture_coordinate_info)
                case 3501048615: // ::base::texture_coordinate_info.tangent_u
                    check(2 <= Inputs.Num());
                    return{ Inputs[1] };
                case 3256380617: // ::base::texture_coordinate_info.tangent_v
                case 2332782529: // ::base::texture_coordinate_info.tangent_v(::base::texture_coordinate_info)
                    check(3 <= Inputs.Num());
                    return{ Inputs[2] };
                case 2291824070: // material_surface.emission(material_surface)
                    check(4 <= Inputs.Num());
                    return{ Inputs[1], Inputs[2], Inputs[3] };
                case 2487874440: // material.surface(material)
                    check(5 <= Inputs.Num());
                    return{ Inputs[1], Inputs[2], Inputs[3], Inputs[4] };
                case 1789602102: // material.ior(material)
                    check(10 <= Inputs.Num());
                    return{ Inputs[9] };
                case 2121329778: // material.volume(material)
                    check(14 <= Inputs.Num());
                    return{ Inputs[10], Inputs[11], Inputs[12], Inputs[13] };
                }

                TArray<FMaterialExpressionConnection> AccessReturn;
                int32 End = FunctionMDLName.Find(TEXT("("));
                int32 Start = FunctionMDLName.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromEnd, End);
                if (End != INDEX_NONE && Start != INDEX_NONE && Start < End)
                {
                    FString StructName = FunctionMDLName.Mid(0, Start);
                    FString FieldName = FunctionMDLName.Mid(Start + 1, End - (Start + 1));
                    mi::base::Handle<mi::neuraylib::IType_factory> TypeFactory(MDLModule->MDLFactory->create_type_factory(MDLModule->Transaction.get()));
                    // NOTE: need encoded name to create structure, such as @ will be %40
                    mi::base::Handle<const mi::neuraylib::IType_struct> TypeStruct(TypeFactory->create_struct(TCHAR_TO_UTF8(*StructName)));
                    if (TypeStruct)
                    {
                        int32 Field = TypeStruct->find_field(TCHAR_TO_UTF8(*FieldName));
                        int32 StartInput = 0;
                        int32 InputCount = 0;
                        for (int32 FieldIndex = 0; FieldIndex < Field + 1; ++FieldIndex)
                        {
                            int32& CurrentInput = FieldIndex < Field ? StartInput : InputCount;

                            mi::base::Handle<const mi::neuraylib::IType> ParamType(TypeStruct->get_field_type(FieldIndex));
                            if (ParamType)
                            {
                                switch (ParamType->get_kind())
                                {
                                case mi::neuraylib::IType::TK_STRUCT:
                                {
                                    const mi::base::Handle<const mi::neuraylib::IType_struct> StructType(ParamType.get_interface<const mi::neuraylib::IType_struct>());
                                    FString StructSymbol = StructType->get_symbol();
                                    if (StructSymbol == "::base::texture_coordinate_info")
                                    {
                                        CurrentInput += 3;
#if defined(USE_WORLD_ALIGNED_TEXTURES)
                                        CurrentInput += 3;
#endif
                                    }
                                    else if (StructSymbol == "::base::anisotropy_return")
                                    {
                                        CurrentInput += 3;
                                    }
                                    else if (StructSymbol == "::base::texture_return")
                                    {
                                        CurrentInput += 2;
                                    }
                                    else if (StructSymbol == "::base::color_layer")
                                    {
                                        CurrentInput += 3;
                                    }
                                    else
                                    {
                                        CurrentInput += GetStructInputSize(StructSymbol);
                                    }
                                    break;
                                }

                                default:
                                    ++CurrentInput;
                                    break;
                                }
                            }
                        }

                        for (int32 ReturnIndex = 0; ReturnIndex < InputCount; ++ReturnIndex)
                        {
                            if (StartInput + ReturnIndex < Inputs.Num())
                            {
                                AccessReturn.Add(Inputs[StartInput + ReturnIndex]);
                            }
                        }
                    }
                    if (AccessReturn.Num() == 0)
                    {
                        ImportErrors.Add(FString::Printf(TEXT("Struct %s can't access field %s"), *StructName, *FieldName));
                    }
                }
                else
                {
                    ImportErrors.Add(FString::Printf(TEXT("Can't get struct and field from %s"), *FunctionMDLName));
                }

                return AccessReturn;
            }
        }
        case mi::neuraylib::IFunction_definition::DS_INTRINSIC_DAG_ARRAY_CONSTRUCTOR:
        {
            return Inputs;
        }
        case mi::neuraylib::IFunction_definition::DS_ARRAY_INDEX:
        {
            int32 ArraySize = ArrayInputSizes[0];
            if (ArraySize > 0)
            {
                // Fetch index from array
                check(Inputs.Num() == (ArraySize + 1));
                check(Inputs[ArraySize].ExpressionData.Expression->IsA<UMaterialExpressionConstant>());
                const int32 Index = (int32)Cast<UMaterialExpressionConstant>(Inputs[ArraySize].ExpressionData.Expression)->R;
                CurrentUE4Material->Expressions.Remove(Inputs[ArraySize].ExpressionData.Expression);
                return { Inputs[Index] };
            }
            else
            {
                check(Inputs.Num() > 1);
                int32 IndexInput = Inputs.Num() - 1;
                check(Inputs[IndexInput].ConnectionType == EConnectionType::Expression);
                check(Inputs[IndexInput].ExpressionData.Expression->IsA<UMaterialExpressionConstant>());
                check(FunctionName == "operator[](<0>[],int)");

                const int32 Index = (int32)Cast<UMaterialExpressionConstant>(Inputs[IndexInput].ExpressionData.Expression)->R;

                CurrentUE4Material->Expressions.Remove(Inputs[IndexInput].ExpressionData.Expression);
                if (Inputs.Num() == 2)
                {
                    // work as mask
                    check(Inputs[0].ConnectionType == EConnectionType::Expression);

                    // .x, .y, .z
                    if (Index < 3)
                    {
                        if(Inputs[0].ExpressionData.Expression->IsA<UMaterialExpressionMaterialFunctionCall>())
                        {
                            auto FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Inputs[0].ExpressionData.Expression);
                            if (FunctionCall->MaterialFunction)
                            {
                                auto MaterialFunctionName = FunctionCall->MaterialFunction->GetName();
                                if (MaterialFunctionName == TEXT("mdl_state_geometry_normal")
                                || MaterialFunctionName == TEXT("mdl_state_texture_tangent_u")
                                || MaterialFunctionName == TEXT("mdl_state_texture_tangent_v")
                                || MaterialFunctionName == TEXT("mdl_state_geometry_tangent_u")
                                || MaterialFunctionName == TEXT("mdl_state_geometry_tangent_v"))
                                {
                                    UMaterialFunction* ReplaceFunction = LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), MaterialFunctionName + TEXT("_") + ((Index == 0) ? "x" : ((Index == 1) ? "y" : "z")));
                                    if (MaterialFunctionName == TEXT("mdl_state_geometry_normal"))
                                    {
                                        return {NewMaterialExpressionFunctionCall(CurrentUE4Material, ReplaceFunction, {})};
                                    }
                                    else
                                    {
                                        return {NewMaterialExpressionFunctionCall(CurrentUE4Material, ReplaceFunction, {0})};
                                    }
                                }
                            }
                        }
                    }

                    return { NewMaterialExpressionComponentMask(CurrentUE4Material, Inputs[0], 1 << Index) };
                }
                else
                {
                    // work as array
                    check(Inputs[Index].ConnectionType == EConnectionType::Expression);
                    return { Inputs[Index] };
                }

            }
        }
#pragma endregion

        default:
            return TArray<FMaterialExpressionConnection>();
    }
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::CreateExpressionConstructorCall(const mi::base::Handle<const mi::neuraylib::IType>& MDLType, const TArray<FMaterialExpressionConnection>& Inputs)
{
    switch (MDLType->get_kind())
    {
        case mi::neuraylib::IType::TK_FLOAT:
        case mi::neuraylib::IType::TK_INT:
        {
            check(Inputs[0].ConnectionType == EConnectionType::Expression);
            if (Inputs.Num() == 1)
            {
                if ((Inputs[0].ExpressionData.Expression->IsA<UMaterialExpressionStaticBool>()
                    || Inputs[0].ExpressionData.Expression->IsA<UMaterialExpressionStaticBoolParameter>()))
                {
                    return{ NewMaterialExpressionStaticSwitch(CurrentUE4Material, Inputs[0], 1.0f, 0.0f) };
                }
                else
                {
                    check(IsScalar(Inputs[0]));
                    return Inputs;
                }
            }
            break;
        }
        case mi::neuraylib::IType::TK_VECTOR:
        {
            const mi::base::Handle<const mi::neuraylib::IType_vector> Type(MDLType->get_interface<const mi::neuraylib::IType_vector>());

            if (Inputs.Num() == 1)
            {
                if (Type->get_size() == ComponentCount(Inputs[0]))
                {
                    return Inputs;
                }
                else
                {
                    check(IsScalar(Inputs[0]));
                    switch (Type->get_size())
                    {
                        case 2:
                            return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { Inputs[0], Inputs[0]}) };
                        case 3:
                            return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3,{ Inputs[0], Inputs[0], Inputs[0] }) };
                        case 4:
                            return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4,{ Inputs[0], Inputs[0], Inputs[0], Inputs[0] }) };
                        default:
                            check(false);
                    }
                }
            }
            else
            {
                check(Inputs.Num() == Type->get_size());
                switch (Type->get_size())
                {
                    case 2:
                        check(IsScalar(Inputs[0]) && IsScalar(Inputs[1]));
                        return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2,{ Inputs[0], Inputs[1] }) };
                    case 3:
                        check(IsScalar(Inputs[0]) && IsScalar(Inputs[1]) && IsScalar(Inputs[2]));
                        return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3,{ Inputs[0], Inputs[1], Inputs[2] }) };
                    case 4:
                        check(IsScalar(Inputs[0]) && IsScalar(Inputs[1]) && IsScalar(Inputs[2]) && IsScalar(Inputs[3]));
                        return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4,{ Inputs[0], Inputs[1], Inputs[2], Inputs[3] }) };
                    default:
                        check(false);
                }
            }
            break;
        }
        case mi::neuraylib::IType::TK_MATRIX:
        {
            const mi::base::Handle<const mi::neuraylib::IType_matrix> MatrixType(MDLType->get_interface<const mi::neuraylib::IType_matrix>());
            const mi::base::Handle<const mi::neuraylib::IType_vector> ElementType(MatrixType->get_element_type());
            const int32 NumRows = (int32)ElementType->get_size(), NumColumns = (int32)MatrixType->get_size();

            if (Inputs.Num() == 1) // DS_MATRIX_DIAG_CONSTRUCTOR
            {
                UMaterialExpressionConstant* Zero = NewMaterialExpressionConstant(CurrentUE4Material, 0.0f);

                TArray<FMaterialExpressionConnection> Outputs;
                Outputs.Reserve(NumColumns);

                for (int32 i = 0; i < NumColumns; i++)
                {
                    switch (NumRows)
                    {
                        case 2:
                            Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { i == 0 ? Inputs[0] : Zero, i == 1 ? Inputs[0] : Zero }));
                            break;
                        case 3:
                            Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3, { i == 0 ? Inputs[0] : Zero, i == 1 ? Inputs[0] : Zero, i == 2 ? Inputs[0] : Zero }));
                            break;
                        case 4:
                            Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4, { i == 0 ? Inputs[0] : Zero, i == 1 ? Inputs[0] : Zero, i == 2 ? Inputs[0] : Zero, i == 3 ? Inputs[0] : Zero }));
                            break;
                    }
                }

                return Outputs;
            }
            else if (Inputs.Num() == NumColumns)
            {
                return Inputs;
            }
            else if (Inputs.Num() == NumColumns * NumRows) // DS_MATRIX_ELEM_CONSTRUCTOR
            {
                TArray<FMaterialExpressionConnection> Outputs;
                Outputs.Reserve(NumColumns);

                for (int32 i = 0; i < NumColumns; i++)
                {
                    switch (NumRows)
                    {
                        case 2:
                            check(IsScalar(Inputs[i*NumColumns]) && IsScalar(Inputs[i*NumColumns + 1]));
                            Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat2, { Inputs[i * NumColumns], Inputs[i * NumColumns + 1] }));
                            break;
                        case 3:
                            check(IsScalar(Inputs[i*NumColumns]) && IsScalar(Inputs[i*NumColumns + 1]) && IsScalar(Inputs[i*NumColumns + 2]));
                            Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3, { Inputs[i * NumColumns], Inputs[i * NumColumns + 1], Inputs[i * NumColumns + 2] }));
                            break;
                        case 4:
                            check(IsScalar(Inputs[i*NumColumns]) && IsScalar(Inputs[i*NumColumns + 1]) && IsScalar(Inputs[i*NumColumns + 2]) && IsScalar(Inputs[i*NumColumns + 3]));
                            Outputs.Add(NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat4, { Inputs[i * NumColumns], Inputs[i * NumColumns + 1], Inputs[i * NumColumns + 2], Inputs[i * NumColumns + 3] }));
                            break;
                    }
                }

                return Outputs;
            }
            break;
        }
        case mi::neuraylib::IType::TK_COLOR:
        {
            switch (Inputs.Num())
            {
                case 1:
                    if (IsVector3(Inputs[0]))
                    {
                        return{ Inputs[0] };
                    }
                    else
                    {
                        check(IsScalar(Inputs[0]));
                        return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3,{ Inputs[0], Inputs[0], Inputs[0] }) };
                    }
                case 3 :
                    check(IsScalar(Inputs[0]) && IsScalar(Inputs[1]) && IsScalar(Inputs[2]));
                    return{ NewMaterialExpressionFunctionCall(CurrentUE4Material, MakeFloat3,{ Inputs[0], Inputs[1], Inputs[2] }) };
            }
            break;
        }
        case mi::neuraylib::IType::TK_STRUCT:
        {
            return Inputs;
        }
    }

    unimplemented_control_path(TEXT("Unhandled MDL constructor expression type %d"), (int32)MDLType->get_kind());
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::CreateExpressionTemporary(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, const mi::base::Handle<const mi::neuraylib::IExpression_temporary>& MDLExpression, const FString& CallPath)
{
    const mi::Size Index = MDLExpression->get_index();
    if (Temporaries[Index].Num() == 0)
    {
        Temporaries[Index] = CreateExpression(CompiledMaterial, mi::base::make_handle(CompiledMaterial->get_temporary(Index)), CallPath);
    }
    return Temporaries[Index];
}

TArray<FMaterialExpressionConnection> FMDLMaterialImporter::GetExpressionParameter(const mi::base::Handle<const mi::neuraylib::IExpression_parameter>& MDLExpression)
{
    const mi::Size Index = MDLExpression->get_index();
    check(Index < Parameters.Num());
    return Parameters[Index];
}

static UMaterialExpression* ImportColorParameter(UObject* Parent, const FString& Name, const mi::base::Handle<const mi::neuraylib::IValue_color> Color)
{
    return NewMaterialExpressionVectorParameter(Parent, Name,
        FLinearColor(mi::base::make_handle(Color->get_value(0))->get_value(),
            mi::base::make_handle(Color->get_value(1))->get_value(),
            mi::base::make_handle(Color->get_value(2))->get_value()));
}

static UMaterialExpression* ImportVectorParameter(UObject* Parent, const FString& Name, const mi::base::Handle<const mi::neuraylib::IValue_vector> Vector)
{
    FLinearColor DefaultValue(EForceInit::ForceInitToZero);
    for (mi::Size I = 0; I < Vector->get_size(); I++)
    {
        mi::base::Handle<const mi::neuraylib::IValue> ElementValue(Vector->get_value(I));
        if(ElementValue->get_kind() == mi::neuraylib::IValue::VK_FLOAT)
        {
            DefaultValue.Component(I) = ElementValue.get_interface<const mi::neuraylib::IValue_float>()->get_value();
        }
        else if(ElementValue->get_kind() == mi::neuraylib::IValue::VK_INT)
        {
            DefaultValue.Component(I) = ElementValue.get_interface<const mi::neuraylib::IValue_int>()->get_value();
        }
    }
    UMaterialExpressionVectorParameter* Parameter = NewMaterialExpressionVectorParameter(Parent, Name, DefaultValue);
    switch (Vector->get_size())
    {
    case 2:
        return NewMaterialExpressionComponentMask(Parent, Parameter, 3);
    case 3:
        return Parameter;
    case 4:
        return NewMaterialExpressionAppendVector(Parent, { Parameter, 0 }, { Parameter, 4 });
    default:
        check(false);
        return nullptr;
    }
}

mi::neuraylib::IValue::Kind FMDLMaterialImporter::ImportParameter(TArray<FMaterialExpressionConnection>& Parameter, FString Name, const mi::base::Handle<mi::neuraylib::IValue const>& Value, TextureCompressionSettings InCompression)
{
    mi::base::Handle<mi::neuraylib::IType const> Type = mi::base::make_handle(Value->get_type());
    mi::neuraylib::IValue::Kind Kind = Value->get_kind();
    switch (Kind)
    {
        case mi::neuraylib::IValue::VK_BOOL:
            if (Name.Equals(TEXT("thin_walled")))
            {
                Parameter.Add(NewMaterialExpressionStaticBool(CurrentUE4Material, true));
            }
            else
            {
                Parameter.Add(NewMaterialExpressionStaticBoolParameter(CurrentUE4Material, Name, Value.get_interface<mi::neuraylib::IValue_bool const>()->get_value()));
            }
            break;
        case mi::neuraylib::IValue::VK_INT:
            Parameter.Add(NewMaterialExpressionScalarParameter(CurrentUE4Material, Name, Value.get_interface<mi::neuraylib::IValue_int const>()->get_value()));
            break;
        case mi::neuraylib::IValue::VK_ENUM:
            Parameter.Add(NewMaterialExpressionScalarParameter(CurrentUE4Material, Name, Value.get_interface<mi::neuraylib::IValue_enum const>()->get_value()));
            break;
        case mi::neuraylib::IValue::VK_STRING:
            {
                const mi::base::Handle<const mi::neuraylib::IValue_string> StringValue(Value.get_interface<const mi::neuraylib::IValue_string>());
                FString UEString = StringValue->get_value();
                int32 Enum = FMDLImporterUtility::SceneDataStringToEnum(UEString);
                Parameter.Add(NewMaterialExpressionScalarParameter(CurrentUE4Material, Name, Enum));
            }
            break;
        case mi::neuraylib::IValue::VK_FLOAT:
            Parameter.Add(NewMaterialExpressionScalarParameter(CurrentUE4Material, Name, Value.get_interface<mi::neuraylib::IValue_float const>()->get_value()));
            break;
        case mi::neuraylib::IValue::VK_DOUBLE:
            Parameter.Add(NewMaterialExpressionScalarParameter(CurrentUE4Material, Name, Value.get_interface<mi::neuraylib::IValue_double const>()->get_value()));
            break;
        case mi::neuraylib::IValue::VK_VECTOR:
            Parameter.Add(ImportVectorParameter(CurrentUE4Material, Name, Value.get_interface<const mi::neuraylib::IValue_vector>()));
            break;
        case mi::neuraylib::IValue::VK_MATRIX:
            {
                const mi::base::Handle<const mi::neuraylib::IValue_matrix> MatrixValue(Value.get_interface<const mi::neuraylib::IValue_matrix>());
                for (mi::Size i = 0; i < MatrixValue->get_size(); i++)
                {
                    ImportParameter(Parameter, Name + TEXT("_") + FString::FromInt(i), mi::base::make_handle(MatrixValue->get_value(i)));
                }
            }
            break;
        case mi::neuraylib::IValue::VK_COLOR:
            Parameter.Add(ImportColorParameter(CurrentUE4Material, Name, Value.get_interface<mi::neuraylib::IValue_color const>()));
            break;
        case mi::neuraylib::IValue::VK_ARRAY:
            {
                const mi::base::Handle<const mi::neuraylib::IValue_array> ArrayValue(Value.get_interface<const mi::neuraylib::IValue_array>());
                for (mi::Size i = 0; i < ArrayValue->get_size(); i++)
                {
                    ImportParameter(Parameter, Name + TEXT("_") + FString::FromInt(i), mi::base::make_handle(ArrayValue->get_value(i)));
                }
            }
            break;
        case mi::neuraylib::IValue::VK_STRUCT:
            {
                const mi::base::Handle<const mi::neuraylib::IValue_struct> StructValue(Value.get_interface<const mi::neuraylib::IValue_struct>());
                for (mi::Size i = 0; i < StructValue->get_size(); i++)
                {
                    ImportParameter(Parameter, Name + TEXT("_") + mi::base::make_handle(StructValue->get_type())->get_field_name(i), mi::base::make_handle(StructValue->get_value(i)));
                }
            }
            break;
        case mi::neuraylib::IValue::VK_INVALID_DF:
            //Parameter.Add(NewMaterialExpressionMakeMaterialAttributes(CurrentUE4Material));
            break;
        case mi::neuraylib::IValue::VK_TEXTURE:
            {
                const mi::base::Handle<const mi::neuraylib::IValue_texture> TextureValue(Value.get_interface<const mi::neuraylib::IValue_texture>());
                auto Texture = LoadResource(TextureValue->get_file_path(), TextureValue->get_owner_module(), TextureValue->get_gamma(), InCompression);
                Parameter.Add(NewMaterialExpressionTextureObjectParameter(CurrentUE4Material, Name, Texture));
            }
            break;
        case mi::neuraylib::IValue::VK_LIGHT_PROFILE:
            {
                check(Value.get_interface<const mi::neuraylib::IValue_light_profile>()->get_value() == nullptr);		// never encountered some real light profile !
                const mi::base::Handle<const mi::neuraylib::IValue_light_profile> LightValue(Value.get_interface<const mi::neuraylib::IValue_light_profile>());
                Parameter.Add(NewMaterialExpressionTextureObjectParameter(CurrentUE4Material, Name,
                    LoadResource(LightValue->get_file_path(), LightValue->get_owner_module(), 1.0f, InCompression)));
            }
            break;

        case mi::neuraylib::IValue::VK_BSDF_MEASUREMENT:
        default:
            check(false);
    }
    return Kind;
}

static UMaterialExpression* GetParameterExpression(UMaterialExpression* Expression)
{
    if (Expression->IsA<UMaterialExpressionAppendVector>())
    {
        UMaterialExpressionAppendVector* AppendVector = Cast<UMaterialExpressionAppendVector>(Expression);
        check(AppendVector->A.Expression == AppendVector->B.Expression);
        return AppendVector->A.Expression;
    }
    else if (Expression->IsA<UMaterialExpressionComponentMask>())
    {
        return GetParameterExpression(Cast<UMaterialExpressionComponentMask>(Expression)->Input.Expression);
    }
    else
    {
        return Expression;
    }
}

void FMDLMaterialImporter::ImportParameters(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& MaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial)
{
    mi::base::Handle<mi::neuraylib::IAnnotation_list const> AnnotationList = mi::base::make_handle(MaterialDefinition->get_parameter_annotations());

    mi::Size ParameterCount = CompiledMaterial->get_parameter_count();
    Parameters.SetNum(ParameterCount);
    TMap<FString, TArray<UMaterialExpression*>> NamedParameterExpressions;

    const mi::IString* DecodedModuleName = MDLModule->MDLFactory->decode_name(MaterialDefinition->get_mdl_module_name());
    FString ModuleName = UnmangleMdlPath(UTF8_TO_TCHAR(DecodedModuleName->get_c_str()));
    for (mi::Size ParameterIndex = 0; ParameterIndex < ParameterCount; ParameterIndex++)
    {
        Parameters[ParameterIndex].SetNum(0);

        // Pre-check custom sampler annotation
        TextureCompressionSettings Compression = TC_Default;
        {
            mi::base::Handle<mi::neuraylib::IAnnotation_block const> AnnotationBlock = mi::base::make_handle(AnnotationList->get_annotation_block(CompiledMaterial->get_parameter_name(ParameterIndex)));
            if (AnnotationBlock)
            {
                for (mi::Size AnnotationBlockIndex = 0, AnnotationBlockSize = AnnotationBlock->get_size(); AnnotationBlockIndex < AnnotationBlock->get_size(); AnnotationBlockIndex++)
                {
                    mi::base::Handle<mi::neuraylib::IAnnotation const> Annotation = mi::base::make_handle(AnnotationBlock->get_annotation(AnnotationBlockIndex));
                    FString AnnotationName = UnmangleMdlPath(Annotation->get_name());

                    for(int32 AnnoIndex = 0; AnnoIndex < MDLSamplerType::ST_MAX; ++AnnoIndex )
                    {
                        FString CustomAnnotation = ModuleName + TEXT("::") + FString(SamplerAnnotations[AnnoIndex]);
                        if (AnnotationName == CustomAnnotation)
                        {
                            switch(MDLSamplerType(AnnoIndex))
                            {
                            case ST_Normalmap:
                                Compression = TC_Normalmap;
                                break;
                            case ST_Grayscale:
                                Compression = TC_Grayscale;
                                break;
                            case ST_Alpha:
                                Compression = TC_Alpha;
                                break;
                            case ST_Masks:
                                Compression = TC_Masks;
                                break;
                            case ST_DistanceField:
                                Compression = TC_DistanceFieldFont;
                                break;
                            case ST_Color:
                            default: Compression = TC_Default;
                            }
                            break;
                        }
                    }
                }
            }
        }

        mi::neuraylib::IValue::Kind Kind = ImportParameter(Parameters[ParameterIndex], CompiledMaterial->get_parameter_name(ParameterIndex), mi::base::make_handle(CompiledMaterial->get_argument(ParameterIndex)), Compression);

        if (Parameters[ParameterIndex].Num() == 1)
        {
            // only for single-valued parameters, we can meaningfully set any annotations
            check(Parameters[ParameterIndex][0].ConnectionType == EConnectionType::Expression);
            UMaterialExpression* ParameterExpression = GetParameterExpression(Parameters[ParameterIndex][0].ExpressionData.Expression);

            if (ParameterExpression->IsA<UMaterialExpressionScalarParameter>() && (ModuleName == TEXT("::OmniGlass") || ModuleName == TEXT("::OmniGlass_Opacity")))
            {
                auto ScalarParameter = Cast<UMaterialExpressionScalarParameter>(ParameterExpression);
                if (ScalarParameter->ParameterName.IsEqual(TEXT("enable_opacity"), ENameCase::CaseSensitive))
                {
                    OpacityEnabled = Parameters[ParameterIndex][0];
                }
            }

            mi::base::Handle<mi::neuraylib::IAnnotation_block const> AnnotationBlock = mi::base::make_handle(AnnotationList->get_annotation_block(CompiledMaterial->get_parameter_name(ParameterIndex)));
            if (AnnotationBlock)
            {
                for (mi::Size AnnotationBlockIndex = 0, AnnotationBlockSize = AnnotationBlock->get_size(); AnnotationBlockIndex < AnnotationBlock->get_size(); AnnotationBlockIndex++)
                {
                    mi::base::Handle<mi::neuraylib::IAnnotation const> Annotation = mi::base::make_handle(AnnotationBlock->get_annotation(AnnotationBlockIndex));
                    FString AnnotationName = Annotation->get_name();
                    if (AnnotationName == TEXT("::anno::description(string)"))
                    {
                        TArray<FString> Descriptions = GetExpressionConstant<FString, mi::neuraylib::IValue_string>(mi::base::make_handle(Annotation->get_arguments()));
                        check(Descriptions.Num() == 1);
                        ParameterExpression->Desc = Descriptions[0];
                    }
                    else if (AnnotationName == TEXT("::anno::display_name(string)"))
                    {
                        const UMDLSettings* Settings = GetDefault<UMDLSettings>();
                        if (Settings->bUseDisplayNameForParameter)
                        {
                            TArray<FString> DisplayNames = GetExpressionConstant<FString, mi::neuraylib::IValue_string>(mi::base::make_handle(Annotation->get_arguments()));
                            check(DisplayNames.Num() == 1);
                            ParameterExpression->SetParameterName(*DisplayNames[0]);
                            NamedParameterExpressions.FindOrAdd(*DisplayNames[0]).Add(ParameterExpression);
                        }
                    }
                    else if ((AnnotationName == TEXT("::anno::hard_range(double,double)")) || (AnnotationName == TEXT("::anno::soft_range(double,double)")))
                    {
                        check(Kind == mi::neuraylib::IValue::VK_DOUBLE);
                        check(ParameterExpression->IsA<UMaterialExpressionScalarParameter>());
                        TArray<double> Range = GetExpressionConstant<double, mi::neuraylib::IValue_double>(mi::base::make_handle(Annotation->get_arguments()));
                        check(Range.Num() == 2);
                        UMaterialExpressionScalarParameter* ScalarParameter = Cast<UMaterialExpressionScalarParameter>(ParameterExpression);
                        ScalarParameter->SliderMin = Range[0];
                        ScalarParameter->SliderMax = Range[1];
                    }
                    else if ((AnnotationName == TEXT("::anno::hard_range(float,float)")) || (AnnotationName == TEXT("::anno::soft_range(float,float)")))
                    {
                        check(Kind == mi::neuraylib::IValue::VK_FLOAT);
                        check(ParameterExpression->IsA<UMaterialExpressionScalarParameter>());
                        TArray<float> Range = GetExpressionConstant<float, mi::neuraylib::IValue_float>(mi::base::make_handle(Annotation->get_arguments()));
                        check(Range.Num() == 2);
                        UMaterialExpressionScalarParameter* ScalarParameter = Cast<UMaterialExpressionScalarParameter>(ParameterExpression);
                        ScalarParameter->SliderMin = Range[0];
                        ScalarParameter->SliderMax = Range[1];
                    }
                    else if ((AnnotationName == TEXT("::anno::hard_range(int,int)")) || (AnnotationName == TEXT("::anno::soft_range(int,int)")))
                    {
                        check(ParameterExpression->IsA<UMaterialExpressionScalarParameter>());
                        TArray<float> Range;
                        if (Kind == mi::neuraylib::IValue::VK_FLOAT)
                        {
                            Range = GetExpressionConstant<float, mi::neuraylib::IValue_float>(mi::base::make_handle(Annotation->get_arguments()));
                        }
                        else
                        {
                            check(Kind == mi::neuraylib::IValue::VK_INT);
                            TArray<int> IntRange = GetExpressionConstant<int, mi::neuraylib::IValue_int>(mi::base::make_handle(Annotation->get_arguments()));
                            for (int32 I = 0; I < IntRange.Num(); I++)
                            {
                                Range.Add(IntRange[I]);
                            }
                        }
                        check(Range.Num() == 2);
                        UMaterialExpressionScalarParameter* ScalarParameter = Cast<UMaterialExpressionScalarParameter>(ParameterExpression);
                        ScalarParameter->SliderMin = Range[0];
                        ScalarParameter->SliderMax = Range[1];
                    }
                    else if (AnnotationName.StartsWith(TEXT("::anno::in_group")))
                    {
                        auto SetTextureObjectGroup = [](UMaterialExpression* ParameterExpression, const FString& Group)
                        {
                            UMaterialExpressionTextureObjectParameter* TextureObjectParameterExpression = Cast<UMaterialExpressionTextureObjectParameter>(ParameterExpression);
                            TextureObjectParameterExpression->Group = *Group;

                            if (TextureObjectParameterExpression->Texture
                                && TextureObjectParameterExpression->Texture->GetOuter()
                                && (TextureObjectParameterExpression->Texture->GetOuter()->GetName() == TEXT("/MDL/Textures/DefaultColor") || TextureObjectParameterExpression->Texture->GetOuter()->GetName() == TEXT("/MDL/Textures/DefaultColorVT")))
                            {
                                if (TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Normal")))
                                {
                                    if (!TextureObjectParameterExpression->Texture->IsNormalMap())
                                    {
                                        TextureObjectParameterExpression->Texture = LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/DefaultNormal"), nullptr, LOAD_None, nullptr);
                                        TextureObjectParameterExpression->SamplerType = SAMPLERTYPE_Normal;
                                    }
                                }
                                else if (TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Occlusion"))
                                    || TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Roughness"))
                                    || TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Metallic"))
                                    || TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Metalness"))
                                    || TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("ORM"), ESearchCase::CaseSensitive)
                                    || TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Mask"))
                                    || TextureObjectParameterExpression->ParameterName.ToString().Contains(TEXT("Specular")))
                                {
                                    if (TextureObjectParameterExpression->Texture->SRGB)
                                    {
                                        TextureObjectParameterExpression->Texture = LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/DefaultLinearColor"), nullptr, LOAD_None, nullptr);
                                        TextureObjectParameterExpression->SamplerType = SAMPLERTYPE_LinearColor;
                                    }
                                }
                            }
                        };

                        TArray<FString> InGroups = GetExpressionConstant<FString, mi::neuraylib::IValue_string>(mi::base::make_handle(Annotation->get_arguments()));
                        check(InGroups.Num() > 0);
                        if (ParameterExpression->IsA<UMaterialExpressionParameter>())
                        {
                            Cast<UMaterialExpressionParameter>(ParameterExpression)->Group = *InGroups[0];
                        }
                        else if (ParameterExpression->IsA<UMaterialExpressionTextureObjectParameter>())
                        {
                            SetTextureObjectGroup(ParameterExpression, InGroups[0]);
                        }
                    }
                    else
                    {
                        /*
                        check((AnnotationName == TEXT("::alg::base::annotations::gamma_type(::tex::gamma_mode)"))
                            || (AnnotationName == TEXT("::alg::base::annotations::sampler_usage(string,string)"))
                            || (AnnotationName == TEXT("::alg::base::annotations::visible_by_default(bool)"))
                            || (AnnotationName == TEXT("::anno::author(string)"))
                            || (AnnotationName == TEXT("::anno::contributor(string)"))
                            || (AnnotationName == TEXT("::anno::copyright_notice(string)"))
                            || (AnnotationName == TEXT("::anno::create(int,int,int,string)"))
                            || (AnnotationName == TEXT("::anno::dependency(string,int,int,int,string)"))
                            || (AnnotationName == TEXT("::anno::deprecated()")) || (AnnotationName == TEXT("::anno::deprecated(string)"))
                            || (AnnotationName == TEXT("::anno::hard_range(double2,double2)")) || (AnnotationName == TEXT("::anno::hard_range(double3,double3)")) || (AnnotationName == TEXT("::anno::hard_range(double4,double4)"))
                            || (AnnotationName == TEXT("::anno::hard_range(float2,float2)")) || (AnnotationName == TEXT("::anno::hard_range(float3,float3)")) || (AnnotationName == TEXT("::anno::hard_range(float4,float4)"))
                            || (AnnotationName == TEXT("::anno::hard_range(int2,int2)")) || (AnnotationName == TEXT("::anno::hard_range(int3,int3)")) || (AnnotationName == TEXT("::anno::hard_range(int4,int4)"))
                            || (AnnotationName == TEXT("::anno::hidden()"))
                            || (AnnotationName == TEXT("::anno::key_words(string[])"))
                            || (AnnotationName == TEXT("::anno::modified(int,int,int,string)"))
                            || (AnnotationName == TEXT("::anno::in_group(string,string)")) || (AnnotationName == TEXT("::anno:in_group(string,string,string)"))
                            || (AnnotationName == TEXT("::anno::soft_range(double2,double2)")) || (AnnotationName == TEXT("::anno::soft_range(double3,double3)")) || (AnnotationName == TEXT("::anno::soft_range(double4,double4)"))
                            || (AnnotationName == TEXT("::anno::soft_range(float2,float2)")) || (AnnotationName == TEXT("::anno::soft_range(float3,float3)")) || (AnnotationName == TEXT("::anno::soft_range(float4,float4)"))
                            || (AnnotationName == TEXT("::anno::soft_range(int2,int2)")) || (AnnotationName == TEXT("::anno::soft_range(int3,int3)")) || (AnnotationName == TEXT("::anno::soft_range(int4,int4)"))
                            || (AnnotationName == TEXT("::anno::unused()")) || (AnnotationName == TEXT("::anno::unused(string)"))
                            || (AnnotationName == TEXT("::anno::version(int,int,int,string)"))
                            || (AnnotationName == TEXT("::core_definitions::ui_position(int)"))
                            || (AnnotationName == TEXT("::ifm::enableIf(string,string)"))
                            || (AnnotationName == TEXT("::ifm::mayaName(string)"))
                            || (AnnotationName == TEXT("::ifm::state(string)"))
                            || (AnnotationName == TEXT("::ifm::uiProperty(string,string,string)"))
                            || (AnnotationName == TEXT("::ifm::visibleIf(string,string)"))
                            || (AnnotationName == TEXT("::mdl::Lw::ColourShader()"))
                            || (AnnotationName == TEXT("::mdl::Lw::DisabledIfFalse(string[N])"))
                            || (AnnotationName == TEXT("::mdl::Lw::DisabledIfTrue(string[N])"))
                            || (AnnotationName == TEXT("::mdl::Lw::FloatShader()"))
                            || (AnnotationName == TEXT("::mdl::Lw::MaxValue(float)"))
                            || (AnnotationName == TEXT("::mdl::Lw::MinValue(float)"))
                            || (AnnotationName == TEXT("::mdl::Lw::UserLevel(int)")));
                            */
                    }
                }
            }
        }
    }

    for (auto& Elem : NamedParameterExpressions)
    {
        if (1 < Elem.Value.Num())
        {
            for (int32 i = 0; i < Elem.Value.Num(); i++)
            {
                Elem.Value[i]->SetParameterName(*(Elem.Value[i]->GetParameterName().ToString() + TEXT(" ") + FString::FromInt(i + 1)));
            }
        }
    }
}

void FMDLMaterialImporter::SetClearCoatNormal(const FMaterialExpressionConnection& Base, const UMaterialExpression* Normal)
{
    check(Base.ConnectionType == EConnectionType::Expression);
    UMaterialExpression* BaseNormal = nullptr;
    if (Base.ExpressionData.Expression->IsA<UMaterialExpressionIf>())
    {
        UMaterialExpressionIf* If = Cast<UMaterialExpressionIf>(Base.ExpressionData.Expression);
        check(If->ALessThanB.Expression->IsA<UMaterialExpressionMaterialFunctionCall>() &&
            If->AEqualsB.Expression->IsA<UMaterialExpressionMaterialFunctionCall>() &&
            If->AGreaterThanB.Expression->IsA<UMaterialExpressionMaterialFunctionCall>());
        UMaterialExpression* BaseNormalALessThanB = Cast<UMaterialExpressionMaterialFunctionCall>(If->ALessThanB.Expression)->FunctionInputs.Last().Input.Expression;
        UMaterialExpression* BaseNormalAEqualsB = Cast<UMaterialExpressionMaterialFunctionCall>(If->AEqualsB.Expression)->FunctionInputs.Last().Input.Expression;
        UMaterialExpression* BaseNormalAGreaterThanB = Cast<UMaterialExpressionMaterialFunctionCall>(If->AGreaterThanB.Expression)->FunctionInputs.Last().Input.Expression;
        if ((BaseNormalALessThanB && (BaseNormalALessThanB != Normal)) ||
            (BaseNormalAEqualsB && (BaseNormalAEqualsB != Normal)) ||
            (BaseNormalAGreaterThanB && (BaseNormalAGreaterThanB != Normal)))
        {
            BaseNormal = NewMaterialExpressionIf(CurrentUE4Material, { If->A.Expression, If->A.OutputIndex }, { If->B.Expression, If->B.OutputIndex }, BaseNormalALessThanB, BaseNormalAEqualsB, BaseNormalAGreaterThanB);
        }
    }
    else if (Base.ExpressionData.Expression->IsA<UMaterialExpressionStaticSwitch>())
    {
        UMaterialExpressionStaticSwitch* StaticSwitch = Cast<UMaterialExpressionStaticSwitch>(Base.ExpressionData.Expression);
        check(StaticSwitch->A.Expression->IsA<UMaterialExpressionMaterialFunctionCall>() && StaticSwitch->B.Expression->IsA<UMaterialExpressionMaterialFunctionCall>());
        UMaterialExpression* BaseNormalA = Cast<UMaterialExpressionMaterialFunctionCall>(StaticSwitch->A.Expression)->FunctionInputs.Last().Input.Expression;
        UMaterialExpression* BaseNormalB = Cast<UMaterialExpressionMaterialFunctionCall>(StaticSwitch->B.Expression)->FunctionInputs.Last().Input.Expression;
        if ((BaseNormalA && (BaseNormalA != Normal)) || (BaseNormalB && (BaseNormalB != Normal)))
        {
            BaseNormal = NewMaterialExpressionStaticSwitch(CurrentUE4Material, { StaticSwitch->Value.Expression, StaticSwitch->Value.OutputIndex }, BaseNormalA, BaseNormalB);
        }
    }
    else
    {
        check(Base.ExpressionData.Expression->IsA<UMaterialExpressionMakeMaterialAttributes>() || Base.ExpressionData.Expression->IsA<UMaterialExpressionMaterialFunctionCall>());
        BaseNormal = Base.ExpressionData.Expression->IsA<UMaterialExpressionMakeMaterialAttributes>()
            ? Cast<UMaterialExpressionMakeMaterialAttributes>(Base.ExpressionData.Expression)->Normal.Expression
            : Cast<UMaterialExpressionMaterialFunctionCall>(Base.ExpressionData.Expression)->FunctionInputs.Last().Input.Expression;
    }
    if (BaseNormal && (BaseNormal != Normal))
    {
        CurrentClearCoatNormal = NewMaterialExpressionClearCoatNormalCustomOutput(CurrentUE4Material, BaseNormal);
    }
}

void FMDLMaterialImporter::ReplaceWithNormalmap(UMaterialExpression* Expression)
{
    if (Expression == nullptr)
    {
        return;
    }

    if (Expression->IsA<UMaterialExpressionTextureBase>())
    {
        auto TextureObject = Cast<UMaterialExpressionTextureBase>(Expression);
        if (TextureObject->Texture && !TextureObject->Texture->IsNormalMap())
        {
            // Replace with normal texture;
            TextureObject->Texture = LoadObject<UTexture2D>(nullptr, TEXT("/MDL/Textures/DefaultNormal"), nullptr, LOAD_None, nullptr);
            TextureObject->SamplerType = SAMPLERTYPE_Normal;
        }
    }
}

bool FMDLMaterialImporter::IsNormalPostfix(UMaterialExpression* Expression)
{
    if (Expression == nullptr)
    {
        return false;
    }

    if (Expression->IsA<UMaterialExpressionTextureBase>())
    {
        auto TextureObject = Cast<UMaterialExpressionTextureBase>(Expression);
        if (TextureObject->Texture && TextureObject->Texture->IsNormalMap())
        {
            return true;
        }
    }

    return false;
}
#endif
