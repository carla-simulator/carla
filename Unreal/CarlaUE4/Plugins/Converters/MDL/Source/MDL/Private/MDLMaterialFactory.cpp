// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLMaterialFactory.h"
#include "MDLImporterUtility.h"
#include "MDLSettings.h"
#include "MDLOutputLogger.h"
#include "NodeArrangement.h"
#include "AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Materials/Material.h"
#include "Misc/Paths.h"
#include "Editor.h"
#include "HAL/FileManager.h"

UMDLMaterialFactory::UMDLMaterialFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
#if WITH_MDL_SDK
    , MDLModule(FModuleManager::GetModulePtr<FMDLModule>("MDL"))
    , MaterialImporter(new FMDLMaterialImporter(this))
#endif
{
#if WITH_MDL_SDK
    bCreateNew = bText = false;
    bEditorImport = true;

    SupportedClass = UMaterial::StaticClass();

    Formats.Add(TEXT("MDL;MDL Material file"));

    TextureFactory = CreateDefaultSubobject<UTextureFactory>(TEXT("TextureFactory"));
    MaterialFactory = CreateDefaultSubobject<UMaterialFactoryNew>(TEXT("MaterialFactory"));
#endif
}

bool UMDLMaterialFactory::FactoryCanImport(const FString& Filename)
{
#if WITH_MDL_SDK
    return MDLModule && MDLModule->Neuray && Super::FactoryCanImport(Filename);
#else
    return false;
#endif
}

UObject* UMDLMaterialFactory::ImportObject(UClass* InClass, UObject* InOuter, FName InName, EObjectFlags InFlags, const FString& Filename, const TCHAR* Parms, bool& OutCanceled)
{
    UObject* Object = Super::ImportObject(InClass, InOuter, InName, InFlags, Filename, Parms, OutCanceled);
#if WITH_MDL_SDK
    if (!Object && !IFileManager::Get().FileExists(*Filename))
    {
        // Assume the Filename is relative module path, reconstruct the absolute filename by
        // trying to prepend each of the module paths.
        FString FilenameFixed;
        for (mi::Size index = 0; index < MDLModule->MDLConfiguration->get_mdl_paths_length(); index++)
        {
            const mi::IString* modulePath = MDLModule->MDLConfiguration->get_mdl_path(index);
            FilenameFixed = FString(modulePath->get_c_str()) + Filename;
            if (IFileManager::Get().FileExists(*FilenameFixed))
            {
                Object = Super::ImportObject(InClass, InOuter, InName, InFlags, FilenameFixed, Parms, OutCanceled);
                break;
            }
        }
    }
#endif
    return Object;
}

UObject* UMDLMaterialFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
#if WITH_MDL_SDK
    const UMDLSettings* Settings = GetDefault<UMDLSettings>();
    if (!Settings->WrapperMaterial.FilePath.IsEmpty() && (Settings->WrapperMaterial.FilePath != MDLModule->WrapperMaterialPath))
    {
        FString WrapperPath = FPaths::ConvertRelativePathToFull(FPaths::GetPath(Settings->WrapperMaterial.FilePath));
        const mi::base::Handle<const mi::neuraylib::IModule> WrapperModule = PrepareModule(WrapperPath, TEXT("::") + FPaths::GetBaseFilename(Settings->WrapperMaterial.FilePath), Warn);
        if (WrapperModule.is_valid_interface())
        {
            MDLModule->WrapperModuleData = GetModuleData(WrapperModule);
            MDLModule->WrapperMaterialData = GetMaterialData(WrapperModule->get_material(0), 1);
            MDLModule->WrapperMaterialPath = Settings->WrapperMaterial.FilePath;
        }
        else
        {
            MDLModule->WrapperMaterialPath.Empty();
        }
    }

    // add the module path for this file
    FString ModuleName = FPaths::ConvertRelativePathToFull(Filename);
    FString ModulePath = FPaths::GetPath(ModuleName);
    ModuleName = ModulePath / FPaths::GetBaseFilename(ModuleName);
    int32 Colon = ModuleName.Find(TEXT(":"));
    if (Colon != INDEX_NONE)
    {
        ModuleName = ModuleName.RightChop(Colon + 1);
    }
    ModuleName.ReplaceInline(TEXT("/"), TEXT("::"));

    UObject* Package = InParent;
    if (ModulePath.StartsWith(MDLModule->MDLUserPath + TEXT("/nvidia/vMaterials")))
    {
        FString PackageName = ModulePath;
        verify(PackageName.RemoveFromStart(MDLModule->MDLUserPath + TEXT("/nvidia/vMaterials")));
        ModulePath = MDLModule->MDLUserPath + TEXT("/nvidia/vMaterials");
        ModuleName = PackageName.Replace(TEXT("/"), TEXT("::")) + ModuleName;
        PackageName = FMDLImporterUtility::GetProjectMdlRootPath() / PackageName / InName.ToString();
        Package = CreatePackage(*PackageName);
    }

    GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, Package, InName, TEXT("MDL"));

    TArray<UObject*> ResultAssets;
    {
        const mi::base::Handle<const mi::neuraylib::IModule> FileModule = PrepareModule(ModulePath, ModuleName, Warn);
        if (!FileModule.is_valid_interface())
        {
            GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
            return nullptr;
        }

        mi::Size ResultCount = FileModule->get_material_count();

        if (Settings->bImportOnlyFirstMaterialPerFile)
        {
            ResultCount = std::min<mi::Size>(ResultCount, 1);
        }

        // Set up progress dialog
        FScopedSlowTask Progress((float)ResultCount, FText::FromString(TEXT("...")), true, *Warn);

        ModuleData WrappeeModuleData = GetModuleData(FileModule);

        for (mi::Size i = 0; i < ResultCount; i++)
        {
            if (MaterialIsHidden(FileModule->get_material(i)))
            {
                continue;
            }

            mi::base::Handle<const mi::neuraylib::IFunction_definition> MaterialDefinition = MDLModule->WrapperMaterialPath.IsEmpty()
                ? mi::base::make_handle(MDLModule->Transaction->access<mi::neuraylib::IFunction_definition>(FileModule->get_material(i)))
                : WrapMaterial(WrappeeModuleData, FileModule->get_material(i), Warn);

            FString MaterialName = FString(MaterialDefinition->get_mdl_name());
            MaterialName = MaterialName.Left(MaterialName.Find(TEXT("(")));
            int32 ColonIndex;
            verify(MaterialName.FindLastChar(':', ColonIndex));
            MaterialName = MaterialName.RightChop(ColonIndex + 1);

            // Update progress
            Progress.EnterProgressFrame(1.0f, FText::Format(NSLOCTEXT("AssetTools", "Import_ImportingFile", "Importing \"{0}\"..."), FText::FromString(MaterialName)));

            // Get a MaterialInstance from the MaterialDefinition
            mi::Sint32 ErrorCode;
            mi::neuraylib::Definition_wrapper DefinitionWrapper(
                MDLModule->GetTransaction().get(), FileModule->get_material(i), MDLModule->GetFactory().get());

            mi::base::Handle<mi::neuraylib::IScene_element> MaterialInstanceSE(DefinitionWrapper.create_instance(nullptr, &ErrorCode));

            if (ErrorCode < 0)
            {
                Warn->Logf(ELogVerbosity::Error, TEXT("Failed to create MDL material '%s', error code is %d"), *MaterialName, ErrorCode);
                continue;
            }

            mi::base::Handle<const mi::neuraylib::ICompiled_material> CompiledMaterial = FMDLImporterUtility::CreateCompiledMaterial(MaterialInstanceSE);
            if (!CompiledMaterial.is_valid_interface())
            {
                Warn->Logf(ELogVerbosity::Error, TEXT("Failed to compile MDL material '%s'"), *MaterialName);
                continue;
            }

            UMaterial* Material = (UMaterial*)MaterialImporter->CreateMaterial(CreatePackage(*(Package->GetName() / MaterialName)), *MaterialName, Flags, Warn);
            check(Material != nullptr);
            Material->bTangentSpaceNormal = true;
            Material->bUseMaterialAttributes = true;

            // Create material asset from MDL material
            UMaterialExpressionClearCoatNormalCustomOutput* ClearcoatNormal = nullptr;
            bool bImportSuccess = false;
            if (Settings->bDistillation)
            {
                // get the distilling target from the settings
                FString Target = FMDLImporterUtility::GetDistillerTargetName();

                // and distill the Material
                mi::base::Handle<const mi::neuraylib::ICompiled_material> DistilledMaterial(MDLModule->MDLDistillerAPI->distill_material(CompiledMaterial.get(), TCHAR_TO_UTF8(*Target), nullptr, &ErrorCode));
                if (ErrorCode < 0)
                {
                    Warn->Logf(ELogVerbosity::Error, TEXT("Failed to distill MDL material '%s' to target '%s', error code is %d"), *MaterialName, *Target, ErrorCode);
                    continue;
                }
                check(DistilledMaterial);
                bImportSuccess = MaterialImporter->ImportDistilledMaterial(Material, MaterialDefinition, DistilledMaterial, ClearcoatNormal);
            }
            else
            {
                bImportSuccess = MaterialImporter->ImportMaterial(Material, MaterialDefinition, CompiledMaterial, ClearcoatNormal);
            }

            if (!bImportSuccess)
            {
                auto InvalidFunctions = MaterialImporter->GetLastImportErrors();
                if (InvalidFunctions.Num() > 0)
                {
                    FString FunctionName;
                    for (auto Name : InvalidFunctions)
                    {
                        FunctionName += Name + TEXT(" ");
                    }
                    Warn->Logf(ELogVerbosity::Error, TEXT("Failed to compile functions %sin MDL material '%s'"), *FunctionName, *MaterialName);
                }
                else
                {
                    Warn->Logf(ELogVerbosity::Error, TEXT("Failed to compile MDL material '%s'"), *MaterialName);
                }
                continue;
            }

    #if defined(ADD_CLIP_MASK)
            Material->MaterialAttributes.Connect(0,
                NewMaterialExpressionFunctionCall(Material,
                    ::LoadFunction(FMDLImporterUtility::GetProjectMdlFunctionPath(), TEXT("mdlimporter_set_clip_mask")), { Material->MaterialAttributes.Expression }));
            if (Material->BlendMode == BLEND_Opaque)
            {
                Material->BlendMode = BLEND_Masked;
                Material->DitherOpacityMask = true;
            }
    #endif

            // Arrange expression nodes in material editor
            TArray<UMaterialExpression*> OutputExpressions;
            OutputExpressions.Add(Material->MaterialAttributes.Expression);
            if (ClearcoatNormal)
            {
                OutputExpressions.Add(ClearcoatNormal);
            }
            ArrangeNodes(Material, OutputExpressions);

            GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, Material);
            FAssetRegistryModule::AssetCreated(Material);		// make sure, this asset is listed in the Content Browser

            Material->MarkPackageDirty();
            Material->PostEditChange();

            ResultAssets.Add(Material);
        }
    }

    MDLModule->RemoveMdlModule(ModuleName);
    MDLModule->CommitAndCreateTransaction();

    if (ResultAssets.Num() == 0)
    {
        GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
        return nullptr;
    }
    else if (ResultAssets.Num() == 1)
    {
        return ResultAssets[0];
    }
    else
    {
        return ResultAssets[0]->GetOutermost();
    }
#else
    UE_LOG(LogMDLOutput, Error, TEXT("Plugin was not compiled with MDL SDK"));
    return nullptr;
#endif
}

#if WITH_MDL_SDK
static FString StripArguments(const FString& Name);

static bool CheckStandardType(mi::base::Handle<const mi::neuraylib::IType> Type)
{
    mi::neuraylib::IType::Kind Kind = Type->get_kind();
    switch (Kind)
    {
    case mi::neuraylib::IType::Kind::TK_ALIAS:
        return CheckStandardType(mi::base::make_handle(Type.get_interface<const mi::neuraylib::IType_alias>()->get_aliased_type()));
    case mi::neuraylib::IType::Kind::TK_BOOL:
    case mi::neuraylib::IType::Kind::TK_ENUM:
    case mi::neuraylib::IType::Kind::TK_FLOAT:
    case mi::neuraylib::IType::Kind::TK_INT:
        return true;
    case mi::neuraylib::IType::Kind::TK_COLOR:
    case mi::neuraylib::IType::Kind::TK_STRUCT:
    case mi::neuraylib::IType::Kind::TK_TEXTURE:
    case mi::neuraylib::IType::Kind::TK_VECTOR:
        return false;
    default:
        check(false);
        return false;
    }
}

FString GammaText(mi::Float32 gamma)
{
    if (gamma == 0.0f)
    {
        return "::tex::gamma_default";
    }
    else if (gamma == 1.0f)
    {
        return "::tex::gamma_linear";
    }
    check(gamma == 2.2f);
    return "::tex::gamma_srgb";
}

static FString ModifierText(mi::Uint32 modifier)
{
    switch (modifier)
    {
    case mi::neuraylib::IType::Modifier::MK_NONE:
        return "";
    case mi::neuraylib::IType::Modifier::MK_UNIFORM:
        return "uniform ";
    case mi::neuraylib::IType::Modifier::MK_VARYING:
        return "varying ";
    default:
        check(false);
        return "";
    }
}

static FString StripArguments(const FString& Name)
{
    return Name.Left(Name.Find("("));
}

static FString TypeText(mi::base::Handle<const mi::neuraylib::IType> Type)
{
    mi::neuraylib::IType::Kind Kind = Type->get_kind();
    switch (Kind)
    {
    case mi::neuraylib::IType::Kind::TK_ALIAS:
        return TypeText(mi::base::make_handle(Type.get_interface<const mi::neuraylib::IType_alias>()->get_aliased_type()));
    case mi::neuraylib::IType::Kind::TK_BOOL:
        return "bool";
    case mi::neuraylib::IType::Kind::TK_COLOR:
        return "color";
    case mi::neuraylib::IType::Kind::TK_ENUM:
        return Type.get_interface<const mi::neuraylib::IType_enum>()->get_symbol();
    case mi::neuraylib::IType::Kind::TK_FLOAT:
        return "float";
    case mi::neuraylib::IType::Kind::TK_INT:
        return "int";
    case mi::neuraylib::IType::Kind::TK_STRUCT:
        return Type.get_interface<const mi::neuraylib::IType_struct>()->get_symbol();
    case mi::neuraylib::IType::Kind::TK_TEXTURE:
        return "texture_2d";
    case mi::neuraylib::IType::Kind::TK_VECTOR:
    {
        const mi::base::Handle<const mi::neuraylib::IType_vector> VectorType(Type->get_interface<const mi::neuraylib::IType_vector>());
        return TypeText(mi::base::make_handle(VectorType->get_element_type())) + FString::FromInt(VectorType->get_size());
    }
    default:
        check(false);
        return "";
    }
}

FString UMDLMaterialFactory::AnnotationText(const FString& Indentation, const mi::base::Handle<const mi::neuraylib::IAnnotation_block> AnnotationBlock)
{
    FString Text;
    Text += Indentation + "[[\n";
    for (mi::Size annoIdx = 0; annoIdx < AnnotationBlock->get_size(); annoIdx++)
    {
        if (annoIdx != 0)
        {
            Text += ",\n";
        }
        mi::base::Handle<const mi::neuraylib::IAnnotation> Annotation(AnnotationBlock->get_annotation(annoIdx));
        Text += Indentation + "  " + StripArguments(Annotation->get_name()) + "(";
        mi::base::Handle<const mi::neuraylib::IExpression_list> Arguments(Annotation->get_arguments());
        for (mi::Size argIdx = 0; argIdx < Arguments->get_size(); argIdx++)
        {
            if (argIdx != 0)
            {
                Text += ",";
            }
            Text += ExpressionText(mi::base::make_handle(Arguments->get_expression(argIdx)));
        }
        Text += ")";
    }
    Text += "\n" + Indentation + "]]";
    return Text;
}

FString UMDLMaterialFactory::ExpressionText(mi::base::Handle<const mi::neuraylib::IExpression> const& Expression, const FString& NameSpace)
{
    mi::neuraylib::IExpression::Kind Kind = Expression->get_kind();
    switch (Kind)
    {
        case mi::neuraylib::IExpression::Kind::EK_CALL:
        {
            mi::base::Handle<const mi::neuraylib::IExpression_call> Call = Expression.get_interface<const mi::neuraylib::IExpression_call>();
            FString CallString(Call->get_call());
            check(CallString.StartsWith("mdl::") && CallString.Contains("()"));
            return CallString.RightChop(5).Left(CallString.Find("()") - 3);
        }
        case mi::neuraylib::IExpression::Kind::EK_CONSTANT:
            return ValueText(mi::base::make_handle(Expression.get_interface<const mi::neuraylib::IExpression_constant>()->get_value()), NameSpace);
        default:
            check(false);
            return "";
    }
}

MaterialData UMDLMaterialFactory::GetMaterialData(const FString& MaterialName, mi::Size StartIndex)
{
    MaterialData Data;

    check(MaterialName.Left(5) == "mdl::");
    Data.MaterialName = MaterialName.RightChop(MaterialName.Find(":", ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);

    // Access material
    const mi::base::Handle<const mi::neuraylib::IFunction_definition> MaterialDefinition(MDLModule->Transaction->access<mi::neuraylib::IFunction_definition>(TCHAR_TO_ANSI(*MaterialName)));
    mi::Size ParameterCount = MaterialDefinition->get_parameter_count();

    // Get a MaterialInstance from the MaterialDefinition
    mi::Sint32 ErrorCode;
    mi::base::Handle<mi::neuraylib::IFunction_call> MaterialInstance(MaterialDefinition->create_function_call(nullptr, &ErrorCode));	verify(ErrorCode == 0);

    const mi::base::Handle<const mi::neuraylib::IType_list> ParameterTypes(MaterialInstance->get_parameter_types());
    check(ParameterCount == ParameterTypes->get_size());

    if (StartIndex != 0)
    {
        mi::base::Handle<const mi::neuraylib::IType> ParameterType(ParameterTypes->get_type(mi::Size(0)));
        check(ParameterType->get_kind() == mi::neuraylib::IType::TK_STRUCT);
        const mi::base::Handle<const mi::neuraylib::IType_struct> StructType(ParameterType.get_interface<const mi::neuraylib::IType_struct>());
        check(strcmp(StructType->get_symbol(), "::material") == 0);
    }

    const mi::base::Handle<const mi::neuraylib::IExpression_list> Defaults(MaterialDefinition->get_defaults());
    const mi::base::Handle<const mi::neuraylib::IAnnotation_list> Annotations(MaterialDefinition->get_parameter_annotations());

    for (mi::Size i = 0; i < StartIndex; i++)
    {
        Data.ParameterNames.Add(MaterialDefinition->get_parameter_name(i));
    }

    for (mi::Size i = StartIndex; i < ParameterCount; ++i)
    {
        if (i != StartIndex)
        {
            Data.Parameters += ",\n";
        }

        mi::base::Handle<const mi::neuraylib::IType> ParameterType(ParameterTypes->get_type(i));
        bool IsStandardType = CheckStandardType(ParameterType);
        char const* Name = MaterialDefinition->get_parameter_name(i);
        FString ParameterTypeText = TypeText(ParameterType);
        FString ParameterTypeNameSpace = ParameterTypeText.Left(ParameterTypeText.Find(":", ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);
        Data.Parameters += "  " + ModifierText(ParameterType->get_all_type_modifiers()) + ParameterTypeText + " " + Name;
        Data.ParameterNames.Add(Name);

        mi::base::Handle<const mi::neuraylib::IExpression> DefaultExpression(Defaults->get_expression(Name));
        if (DefaultExpression.is_valid_interface())
        {
            Data.Parameters += " = ";
            if (!IsStandardType && (DefaultExpression->get_kind() != mi::neuraylib::IExpression::Kind::EK_CALL))
            {
                Data.Parameters += TypeText(ParameterType) + "(";
            }
            Data.Parameters += ExpressionText(DefaultExpression, ParameterTypeNameSpace);
            if (!IsStandardType && (DefaultExpression->get_kind() != mi::neuraylib::IExpression::Kind::EK_CALL))
            {
                Data.Parameters += ")";
            }
        }

        const mi::base::Handle<const mi::neuraylib::IAnnotation_block> ParameterAnnotationBlock(Annotations->get_annotation_block(Name));
        if (ParameterAnnotationBlock)
        {
            Data.Parameters += "\n";
            Data.Parameters += AnnotationText("  ", ParameterAnnotationBlock);
        }
    }

    const mi::base::Handle<const mi::neuraylib::IAnnotation_block> MaterialAnnotationBlock(MaterialDefinition->get_annotations());
    if (MaterialAnnotationBlock)
    {
        Data.Annotations = AnnotationText("", MaterialAnnotationBlock);
    }

    return Data;
}

ModuleData UMDLMaterialFactory::GetModuleData(const mi::base::Handle<const mi::neuraylib::IModule> & Module)
{
    ModuleData Data;

    Data.Name = Module->get_mdl_name();
    mi::Size n = Module->get_import_count();
    Data.Imports.Reserve(n);
    for (mi::Size i = 0; i < n; i++)
    {
        FString Import = Module->get_import(i);
        if (Import != TEXT("mdl::<builtins>"))
        {
            verify(Import.RemoveFromStart("mdl"));
            Data.Imports.Add(Import);
        }
    }

    return Data;
}
#endif

#if WITH_MDL_SDK
bool UMDLMaterialFactory::MaterialIsHidden(const FString& MaterialName)
{
    mi::base::Handle<const mi::neuraylib::IFunction_definition> MaterialDefinition = mi::base::make_handle(MDLModule->Transaction->access<mi::neuraylib::IFunction_definition>(TCHAR_TO_ANSI(*MaterialName)));

    const mi::base::Handle<const mi::neuraylib::IAnnotation_block> AnnotationBlock(MaterialDefinition->get_annotations());
    if (AnnotationBlock)
    {
        for (mi::Size annoIdx = 0; annoIdx < AnnotationBlock->get_size(); annoIdx++)
        {
            mi::base::Handle<const mi::neuraylib::IAnnotation> Annotation(AnnotationBlock->get_annotation(annoIdx));
            const char* anno = Annotation->get_name();
            if (strcmp(Annotation->get_name(), "::anno::hidden()") == 0)
            {
                return true;
            }
        }
    }
    return false;
}

const mi::base::Handle<const mi::neuraylib::IModule> UMDLMaterialFactory::PrepareModule(FString& ModulePath, const FString& ModuleName, FFeedbackContext* Warn)
{
    mi::base::Handle<const mi::neuraylib::IModule> Module;

    // Load and initialize MDL module
    int32 ErrorCode = MDLModule->LoadMdlModule(ModuleName);
    switch (ErrorCode)
    {
        case 1:
            Warn->Logf(ELogVerbosity::Warning, TEXT("Module %s already exists, loading from file was skipped."), *ModuleName);
            // fall-through !!
        case 0:
            Module = MDLModule->GetLoadedMdlModule(ModuleName);
            check(Module.is_valid_interface());
            break;
        case -1:
            Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load MDL module '%s': the module name is invalid"), *ModuleName, ErrorCode);
            break;
        case -2:
            Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load MDL module '%s': failed to find or compile the module"), *ModuleName, ErrorCode);
            break;
        case -3:
            Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load MDL module '%s': the DB name for an imported module is already in use but is not an MDL module, or the DB name for a definition in this module is already in use"), *ModuleName, ErrorCode);
            break;
        case -4:
            Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load MDL module '%s': initialization of an imported module failed"), *ModuleName, ErrorCode);
            break;
        default:
            Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load MDL module '%s': error code is %d"), *ModuleName, ErrorCode);
    }

    return Module;
}
#endif

#if WITH_MDL_SDK
mi::base::Handle<const mi::neuraylib::IFunction_definition> UMDLMaterialFactory::WrapMaterial(const ModuleData& WrappeeModuleData, const FString& Material, FFeedbackContext* Warn)
{
    MaterialData WrappeeMaterialData = GetMaterialData(Material);

    FString MaterialText = "mdl 1.3;\n";
    MaterialText += "\n";

    TSet<FString> Imports = WrappeeModuleData.Imports.Union(MDLModule->WrapperModuleData.Imports);
    for (const FString& Import : Imports)
    {
        MaterialText += "import " + Import + "::*;\n";
    }
    MaterialText += "import " + MDLModule->WrapperModuleData.Name + "::*;\n";
    MaterialText += "import " + WrappeeModuleData.Name + "::*;\n";
    MaterialText += "\n";
    MaterialText += FString("export material ") + WrappeeMaterialData.MaterialName + "\n";
    MaterialText += "(\n";
    MaterialText += WrappeeMaterialData.Parameters + ",\n";
    MaterialText += MDLModule->WrapperMaterialData.Parameters + "\n";
    MaterialText += ")\n";
    if (!WrappeeMaterialData.Annotations.IsEmpty())
    {
        MaterialText += WrappeeMaterialData.Annotations + "\n";
    }
    MaterialText += " = " + MDLModule->WrapperModuleData.Name + "::" + MDLModule->WrapperMaterialData.MaterialName + "\n";
    MaterialText += "(\n";
    MaterialText += "  " + MDLModule->WrapperMaterialData.ParameterNames[0] + " : " + WrappeeModuleData.Name + "::" + WrappeeMaterialData.MaterialName + "\n";
    MaterialText += "  (\n";
    for (mi::Size j = 0; j < WrappeeMaterialData.ParameterNames.Num(); j++)
    {
        if (j != 0)
        {
            MaterialText += ",\n";
        }
        MaterialText += "    " + WrappeeMaterialData.ParameterNames[j] + " : " + WrappeeMaterialData.ParameterNames[j];
    }
    MaterialText += "\n";
    MaterialText += "  )";
    for (mi::Size j = 1; j < MDLModule->WrapperMaterialData.ParameterNames.Num(); j++)
    {
        if (j != 0)
        {
            MaterialText += ",\n";
        }
        MaterialText += "  " + MDLModule->WrapperMaterialData.ParameterNames[j] + " : " + MDLModule->WrapperMaterialData.ParameterNames[j];
    }
    MaterialText += "\n";
    MaterialText += ");\n";

    mi::base::Handle<const mi::neuraylib::IFunction_definition> MaterialDefinition;
    FString WrappedMaterialModule = "::wrapped" + WrappeeModuleData.Name + "::" + WrappeeMaterialData.MaterialName;
    mi::Sint32 ErrorCode = MDLModule->MDLImpexpAPI->load_module_from_string(MDLModule->Transaction.get(), TCHAR_TO_ANSI(*WrappedMaterialModule), TCHAR_TO_ANSI(*MaterialText));
    switch (ErrorCode)
    {
    case 1:
        Warn->Logf(ELogVerbosity::Warning, TEXT("Module %s already exists, loading from string was skipped."), *WrappedMaterialModule);
        // fall-through !!
    case 0:
        // Access material
        WrappedMaterialModule = "mdl" + WrappedMaterialModule + "::" + WrappeeMaterialData.MaterialName;
        MaterialDefinition = mi::base::make_handle(MDLModule->Transaction->access<mi::neuraylib::IFunction_definition>(TCHAR_TO_ANSI(*WrappedMaterialModule)));
        break;
    default:
        Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load MDL module '%s', error code is %d"), *WrappedMaterialModule, ErrorCode);
        GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
    }
    return MaterialDefinition;
}

FString UMDLMaterialFactory::ValueText(const mi::base::Handle<const mi::neuraylib::IValue>& Value, const FString& NameSpace)
{
    mi::neuraylib::IValue::Kind Kind = Value->get_kind();
    switch (Kind)
    {
        case mi::neuraylib::IValue::Kind::VK_ARRAY:
        case mi::neuraylib::IValue::Kind::VK_COLOR:
        case mi::neuraylib::IValue::Kind::VK_VECTOR:
        {
            FString Result;
            mi::base::Handle<const mi::neuraylib::IValue_compound> Compound(Value.get_interface<const mi::neuraylib::IValue_compound>());
            for (mi::Size i = 0; i < Compound->get_size(); i++)
            {
                if (i != 0)
                {
                    Result += ", ";
                }
                Result += ValueText(mi::base::make_handle(Compound->get_value(i)));
            }
            return Result;
        }
        case mi::neuraylib::IValue::Kind::VK_BOOL:
            return Value.get_interface<const mi::neuraylib::IValue_bool>()->get_value() ? "true" : "false";
        case mi::neuraylib::IValue::Kind::VK_ENUM:
            return NameSpace + Value.get_interface<const mi::neuraylib::IValue_enum>()->get_name();
        case mi::neuraylib::IValue::Kind::VK_FLOAT:
            return FString::SanitizeFloat(Value.get_interface<const mi::neuraylib::IValue_float>()->get_value()) + "f";
        case mi::neuraylib::IValue::Kind::VK_INT:
            return FString::FromInt(Value.get_interface<const mi::neuraylib::IValue_int>()->get_value());
        case mi::neuraylib::IValue::Kind::VK_STRING:
            return FString("\"") + FString(Value.get_interface<const mi::neuraylib::IValue_string>()->get_value()).ReplaceCharWithEscapedChar() + "\"";
        case mi::neuraylib::IValue::Kind::VK_TEXTURE:
        {
            const mi::base::Handle<const mi::neuraylib::ITexture> Texture(MDLModule->Transaction->access<mi::neuraylib::ITexture>(Value.get_interface<const mi::neuraylib::IValue_texture>()->get_value()));
            if (Texture.is_valid_interface())
            {
                FString FileName = Texture->get_image();
                check(FileName.StartsWith("MI_default_image_"));
                return FString("\"") + FileName.RightChop(17).Replace(TEXT("\\"), TEXT("/")) + "\", " + GammaText(Texture->get_gamma());
            }
            else
            {
                return "\"\", ::tex::gamma_default";
            }
        }
        default:
            check(false);
            return "";
    }
}
#endif
