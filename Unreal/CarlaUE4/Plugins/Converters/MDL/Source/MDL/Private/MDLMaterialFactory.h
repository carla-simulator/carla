// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "Factories/Factory.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"
#include "Misc/FeedbackContext.h"
#if WITH_MDL_SDK
#include "MDLPrivateModule.h"
#include "MDLMaterialImporter.h"
#endif
#include "MDLMaterialFactory.generated.h"


UCLASS(HideCategories=Object, CollapseCategories)
class UMDLMaterialFactory : public UFactory
{
    GENERATED_UCLASS_BODY()

public:
    //~ Begin UFactory Interface
    virtual bool FactoryCanImport(const FString& Filename) override;
    virtual UObject* ImportObject(UClass* InClass, UObject* InOuter, FName InName, EObjectFlags InFlags, const FString& Filename, const TCHAR* Parms, bool& OutCanceled);
    virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
    //~   End UFactory Interface

#if WITH_MDL_SDK
    friend class FMDLMaterialImporter;

private:
    FString														AnnotationText(const FString& Indentation, const mi::base::Handle<const mi::neuraylib::IAnnotation_block> AnnotationBlock);
    FString														ExpressionText(mi::base::Handle<const mi::neuraylib::IExpression> const& Expression, const FString& NameSpace = "");
    MaterialData												GetMaterialData(const FString& MaterialName, mi::Size StartIndex = 0);
    ModuleData													GetModuleData(const mi::base::Handle<const mi::neuraylib::IModule>& Module);
    bool														MaterialIsHidden(const FString& MaterialName);
    const mi::base::Handle<const mi::neuraylib::IModule>		PrepareModule(FString& ModulePath, const FString& ModuleName, FFeedbackContext* Warn);
    mi::base::Handle<const mi::neuraylib::IFunction_definition> WrapMaterial(const ModuleData& WrappeeModuleData, const FString& Material, FFeedbackContext* Warn);
    FString														ValueText(const mi::base::Handle<const mi::neuraylib::IValue>& Value, const FString& NameSpace = "");

private:
    UTextureFactory* TextureFactory;
    UMaterialFactoryNew* MaterialFactory;
    FMDLModule* MDLModule;
    TUniquePtr<FMDLMaterialImporter> MaterialImporter;
#endif
};
