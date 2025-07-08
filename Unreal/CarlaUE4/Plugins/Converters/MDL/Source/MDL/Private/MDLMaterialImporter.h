// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if WITH_MDL_SDK
#include "MDLImporter.h"
#include "MDLDependencies.h"
#include "MaterialExpressions.h"


template <typename ReturnType, typename MDLType>
static bool GetExpressionConstant(mi::base::Handle<mi::neuraylib::IExpression const> const& Expression, ReturnType& OutValue)
{
    check(Expression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT);

    mi::base::Handle<mi::neuraylib::IValue const> const& Value = mi::base::make_handle(Expression.get_interface<mi::neuraylib::IExpression_constant const>()->get_value());

    if (Value.is_valid_interface() && Value.get_interface<MDLType const>())
    {
        OutValue = Value.get_interface<MDLType const>()->get_value();
        return true;
    }

    return false;
}

template <typename ReturnType, typename MDLType>
static TArray<ReturnType> GetExpressionConstant(mi::base::Handle<mi::neuraylib::IExpression_list const> ExpressionList)
{
    TArray<ReturnType> ReturnValues;
    for (mi::Size ExpressionListIndex = 0, ExpressionListSize = ExpressionList->get_size(); ExpressionListIndex < ExpressionListSize; ExpressionListIndex++)
    {
        ReturnType OutValue;
        if (GetExpressionConstant<ReturnType, MDLType>(mi::base::make_handle(ExpressionList->get_expression(ExpressionListIndex)), OutValue))
        {
            ReturnValues.Add(OutValue);
        }
    }
    return ReturnValues;
}

class UMDLMaterialFactory;

class FMDLMaterialImporter : public IMDLMaterialImporter
{
public:
    FMDLMaterialImporter(UMDLMaterialFactory* Factory);
    virtual ~FMDLMaterialImporter() {}

    UMaterial* CreateMaterial(UObject* InParent, FName InName, EObjectFlags Flags, FFeedbackContext* Warn) const;
    virtual bool ImportMaterial(UMaterial* Material, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& InCompiledMaterial, UMaterialExpressionClearCoatNormalCustomOutput*& OutClearCoatNormalCustomOutput, FLoadTextureCallback InCallback = nullptr) override;
    virtual bool ImportDistilledMaterial(UMaterial* Material, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& InCompiledMaterial, UMaterialExpressionClearCoatNormalCustomOutput*& OutClearCoatNormalCustomOutput, FLoadTextureCallback InCallback = nullptr) override;

    virtual bool IsDistillOff(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition) override;
    virtual const TArray<FString>& GetLastImportErrors() override { return ImportErrors; }
private:
    UTexture* LoadResource(const char* FilePath, const char* OwnerModule, const float Gamma, TextureCompressionSettings InCompression = TC_Default);
    UTexture* LoadTexture(const FString& Filename, const FString& AssetDir, const FString& AssetName, bool srgb = true, TextureCompressionSettings InCompression = TC_Default);
    TArray<FMaterialExpressionConnection> MakeFunctionCall(const FString& CallPath, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& FunctionDefinition, const TArray<int32>& ArrayInputSizes, const FString& AssetNamePostfix, TArray<FMaterialExpressionConnection>& Inputs);

    TArray<FMaterialExpressionConnection> CreateExpression(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, const mi::base::Handle<const mi::neuraylib::IExpression>& MDLExpression, const FString& CallPath);
    TArray<FMaterialExpressionConnection> CreateExpressionConstant(const mi::base::Handle<const mi::neuraylib::IValue>& MDLConstant);
    TArray<FMaterialExpressionConnection> CreateExpressionFunctionCall(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, const mi::base::Handle<const mi::neuraylib::IExpression_direct_call>& MDLFunctionCall, const FString& CallPath);
    TArray<FMaterialExpressionConnection> CreateExpressionConstructorCall(const mi::base::Handle<const mi::neuraylib::IType>& MDLType, const TArray<FMaterialExpressionConnection>& Arguments);
    TArray<FMaterialExpressionConnection> CreateExpressionTemporary(const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, const mi::base::Handle<const mi::neuraylib::IExpression_temporary>& MDLExpression, const FString& CallPath);

    TArray<FMaterialExpressionConnection> GetExpressionParameter(const mi::base::Handle<const mi::neuraylib::IExpression_parameter>& MDLExpression);

    mi::neuraylib::IValue::Kind ImportParameter(TArray<FMaterialExpressionConnection>& Parameter, FString Name, const mi::base::Handle<mi::neuraylib::IValue const>& Value, TextureCompressionSettings InCompression = TC_Default);
    void ImportParameters(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& MaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial);

    void SetClearCoatNormal(const FMaterialExpressionConnection& Base, const UMaterialExpression* Normal);
    void SetPropertiesFromAnnotation(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition);
    int32 GetStructInputSize(const FString& StructName);
    bool IsNormalPostfix(class UMaterialExpression* Expression);
    void ReplaceWithNormalmap(class UMaterialExpression* Expression);

private:
    class FMDLModule* MDLModule;
    UObject* ParentPackage;
    const UMDLMaterialFactory* ParentFactory;
    UMaterial* CurrentUE4Material;
    UMaterialExpressionClearCoatNormalCustomOutput* CurrentClearCoatNormal;
    TArray<TArray<FMaterialExpressionConnection>> Parameters, Temporaries;
    FMaterialExpressionConnection CurrentNormalExpression;
    FMaterialExpressionConnection TranslucentOpacity;
    FMaterialExpressionConnection EmissiveOpacity;
    FMaterialExpressionConnection SubsurfaceColor;
    FMaterialExpressionConnection SubsurfaceOpacity;
    FMaterialExpressionConnection OpacityEnabled;

    UMaterialFunction* MakeFloat2;
    UMaterialFunction* MakeFloat3;
    UMaterialFunction* MakeFloat4;
    bool InGeometryExpression;
    FString PackagePath;
    FLoadTextureCallback LoadTextureCallback;
    TArray<FString> ImportErrors;

#if defined(USE_WORLD_ALIGNED_TEXTURES)
    FMaterialExpressionConnection UseWorldAlignedTextureParameter;
#endif
};

extern UMaterialFunction* LoadFunction(const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes = TArray<int32>());

#endif
