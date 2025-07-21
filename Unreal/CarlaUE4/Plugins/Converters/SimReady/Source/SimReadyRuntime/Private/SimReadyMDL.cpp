// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMDL.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/UObjectHash.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"

#if WITH_EDITOR
#include "MDLSettings.h"
#include "NodeArrangement.h"
#include "MDLModule.h"
#include "MDLImporter.h"
#include "MDLImporterUtility.h"
#include "MDLExporterUtility.h"

#include "AssetToolsModule.h"
#include "IMaterialEditor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "MaterialEditorModule.h"
#include "EditorFramework/AssetImportData.h"
#endif

#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionTextureBase.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionClearCoatNormalCustomOutput.h"
#include "Engine/TextureCube.h"

#include "SimReadyRuntimePrivate.h"
#include "SimReadyPathHelper.h"
#include "SimReadyTexture.h"
#include "SimReadySettings.h"
#include "SimReadyRuntimeModule.h"
#include "SimReadyMDLHelper.h"
#include "SimReadyAssetExportHelper.h"
#include "SimReadyMDLReader.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyMessageLogContext.h"
#include "SimReadyAssetImportHelper.h"
#include "SimReadyMaterialReparentUtility.h"


TMap<FString, FMDLParametersList> USimReadyMDL::ImportedParametersSheet;

#if WITH_EDITOR
template <typename ReturnType, typename MDLType>
static ReturnType GetExpressionConstant(mi::base::Handle<mi::neuraylib::IExpression const> const& Expression)
{
    check(Expression->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT);

    mi::base::Handle<mi::neuraylib::IValue const> const& Value = mi::base::make_handle(Expression.get_interface<mi::neuraylib::IExpression_constant const>()->get_value());
    check(Value.get_interface<MDLType const>());

    return Value.get_interface<MDLType const>()->get_value();
}

template <typename ReturnType, typename MDLType>
static TArray<ReturnType> GetExpressionConstant(mi::base::Handle<mi::neuraylib::IExpression_list const> ExpressionList)
{
    TArray<ReturnType> ReturnValues;
    for (mi::Size ExpressionListIndex = 0, ExpressionListSize = ExpressionList->get_size(); ExpressionListIndex < ExpressionListSize; ExpressionListIndex++)
    {
        ReturnValues.Add(GetExpressionConstant<ReturnType, MDLType>(mi::base::make_handle(ExpressionList->get_expression(ExpressionListIndex))));
    }
    return ReturnValues;
}

void CloseAssetEditor(UObject* Asset)
{
    auto* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);
    if (AssetEditor)
    {
        GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Asset);
    }
}

void RefreshMaterialEditor(UMaterialInterface* Interface)
{
    auto* MaterialEditor = (IMaterialEditor*)GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Interface, false);
    if (MaterialEditor)
    {
        IMaterialEditorModule* MaterialEditorModule = &FModuleManager::LoadModuleChecked<IMaterialEditorModule>( "MaterialEditor" );
        if (MaterialEditorModule)
        {
            if(UMaterialInstance* Instance = Cast<UMaterialInstance>(Interface))
            {
                TSharedRef<IMaterialEditor> NewEditor = MaterialEditorModule->CreateMaterialInstanceEditor(EToolkitMode::Standalone, nullptr, Instance);
                GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(Interface, &NewEditor.Get());
            }
            else if(UMaterial* Material = Cast<UMaterial>(Interface))
            {
                TSharedRef<IMaterialEditor> NewEditor = MaterialEditorModule->CreateMaterialEditor(EToolkitMode::Standalone, nullptr, Material);
                GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(Interface, &NewEditor.Get());
            }
        }
    }
}
#endif

USimReadyMDL::USimReadyMDL()
{
    static bool bInitialized = false;
    if(!bInitialized)
    {
        bInitialized = true;
    }
}

//template<typename T>
//T * USimReadyMDL::FindMaterial(FName Name)
//{
//	// Find material by name. 
//	return StaticCast<T*>(FindObjectWithOuter(GetOuter(), T::StaticClass(), Name));
//}
//
//UMaterialInterface* USimReadyMDL::FindMaterial(FName Name)
//{
//	UMaterialInterface* Material = FindMaterial<UMaterialInstanceConstant>(Name);
//	if (Material == nullptr)
//	{
//		return FindMaterial<UMaterial>(Name);
//	}
//
//	return Material;
//}

void LoadTextureBySamplerType(class UTexture*& Texture, EMaterialSamplerType SamplerType)
{
    switch(SamplerType)
    {
    case SAMPLERTYPE_Grayscale:
        Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultGrayscale")), nullptr, LOAD_None, nullptr);
        break;
    case SAMPLERTYPE_Normal:
        Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultNormal")), nullptr, LOAD_None, nullptr);
        break;
    case SAMPLERTYPE_Masks:
        Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultMask")), nullptr, LOAD_None, nullptr);
        break;
    case SAMPLERTYPE_LinearColor:
        Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultLinearColor")), nullptr, LOAD_None, nullptr);
        break;
    case SAMPLERTYPE_LinearGrayscale:
        Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultLinearGrayscale")), nullptr, LOAD_None, nullptr);
        break;
    case SAMPLERTYPE_Color:
    default:
        Texture = LoadObject<UTexture2D>(nullptr, *FMDLImporterUtility::GetContentPath(TEXT("/MDL/Textures/DefaultColor")), nullptr, LOAD_None, nullptr);
    }
}

bool USimReadyMDL::IsMDLLibrary(const FString& Header)
{
    return (Header == TEXT("adobe")
        || Header == TEXT("alg")
        || Header == TEXT("iray_for_rhino")
        || Header == TEXT("nvidia"));
}

bool USimReadyMDL::IsLocalBaseMDL(const FString& InPath)
{
    return FMDLImporterUtility::IsBaseModule(InPath);
}

bool USimReadyMDL::IsLocalMaterialGraphMDL(const FString& InPath)
{
    return FMDLImporterUtility::IsMaterialGraphModule(InPath);
}

UMaterialInterface* USimReadyMDL::GetLocalBaseMaterial(UMaterialInterface* MaterialInterface)
{
    return FMDLImporterUtility::GetBaseMaterial(MaterialInterface);
}

UMaterialInterface* USimReadyMDL::FindLocalBaseMDL(const FString& FileName, const FString& MaterialName)
{
    return FMDLImporterUtility::FindBaseModule(FileName, MaterialName);
}

UMaterialInterface* USimReadyMDL::LoadLocalBaseMDL(const FString& FileName, const FString& MaterialName)
{
    return FMDLImporterUtility::LoadBaseModule(FileName, MaterialName);
}

bool USimReadyMDL::GetDisplayNameFromLocalBaseMDL(const FString& MaterialName, const FString& ParameterName, FString& DisplayName)
{
    return FMDLImporterUtility::GetDisplayNameFromBaseModule(MaterialName, ParameterName, DisplayName);
}

bool USimReadyMDL::GetMdlParameterTypeAndNameFromLocalBaseMDL(const FString& InMaterialName, const FString& InDisplayName, EMdlValueType& ValueType, FString& ParameterName)
{
    return FMDLImporterUtility::GetMdlParameterTypeAndNameFromBaseModule(InMaterialName, InDisplayName, ValueType, ParameterName);
}

bool USimReadyMDL::GetMDLModuleByMaterialName(const FString& InMaterialName, FString& ModuleName)
{
    return FMDLImporterUtility::GetBaseModuleByMaterialName(InMaterialName, ModuleName);
}

bool USimReadyMDL::GetMDLModuleByMaterial(const UMaterialInterface* Material, FString& ModuleName)
{
    return FMDLImporterUtility::GetBaseModuleByMaterial(Material, ModuleName);
}

void USimReadyMDL::LoadMaterialGraphDefinitions()
{
    FMDLImporterUtility::LoadMaterialGraphDefinitions();
}

void USimReadyMDL::UnloadMaterialGraphDefinitions()
{
    FMDLImporterUtility::UnloadMaterialGraphDefinitions();
}

bool USimReadyMDL::CreateMdlInstance(const FString& ModuleName, const FString& FunctionName, const FString& InstanceName)
{
    return FMDLImporterUtility::CreateMdlInstance(ModuleName, FunctionName, InstanceName);
}

bool USimReadyMDL::SetCall(const FString& InstanceTarget, const FString& ParameterName, const FString& InstanceCall)
{
    return FMDLImporterUtility::SetCall(InstanceTarget, ParameterName, InstanceCall);
}

bool USimReadyMDL::DistillMaterialInstance(UMaterial* Material, const FString& MaterialInstanceName, bool bUseDisplayName)
{
    return FMDLImporterUtility::DistillMaterialInstance(Material, MaterialInstanceName, bUseDisplayName);
}

FString USimReadyMDL::GetContentPath(const FString& Path)
{
    return FMDLImporterUtility::GetContentPath(Path);
}

UMaterialInterface* USimReadyMDL::ImportMDL(UObject* InPackage, const FString& InModuleName, const FString& InMaterialName, const TArray<FString>& ModulePaths, 
    TSharedPtr<IMDLExternalReader> ExternalReader, FName Name, EObjectFlags Flags, const FString& SourceFile, TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> Callback)
{
    UMaterialInterface* OutMaterial = nullptr;
    auto MDLPluginModule = FModuleManager::GetModulePtr<IMDLModule>("MDL");
    {
        for (auto Module : ModulePaths)
        {
            MDLPluginModule->AddModulePath(Module, true);
        }

        if (ExternalReader == nullptr)
        {
            ExternalReader = MakeShareable<FSimReadyMDLReader>(new FSimReadyMDLReader());
        }
#ifdef WITH_CARLA
        FMDLImporterUtility::SetProjectMdlRootPath(TEXT("/Game/Carla/Omniverse"));
#endif
        MDLPluginModule->SetExternalFileReader(ExternalReader);		
        int32 Result = MDLPluginModule->LoadMdlModule(InModuleName);
        MDLPluginModule->SetExternalFileReader(nullptr);
        if (Result >= 0)
        {		
            mi::base::Handle<const mi::neuraylib::IModule> MDLModule = MDLPluginModule->GetLoadedMdlModule(InModuleName);

            if (MDLModule.is_valid_interface())
            {
                // Convert MDL to UE4 material
                for (auto MaterialIdx = 0; MaterialIdx < MDLModule->get_material_count(); ++MaterialIdx)
                {
                    FString MaterialName = MDLModule->get_material(MaterialIdx);
                    FString Prototype = FMDLImporterUtility::GetPrototype(MaterialName);
                    MaterialName = MaterialName.Left(MaterialName.Find(TEXT("(")));
                    FString(MoveTemp(MaterialName)).Split("::", nullptr, &MaterialName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

                    if (MaterialName != InMaterialName)
                    {
                        continue;
                    }

                    // Compile
                    mi::base::Handle<const mi::neuraylib::IFunction_definition> MaterialDefinition = mi::base::make_handle(MDLPluginModule->GetTransaction()->access<mi::neuraylib::IFunction_definition>(MDLModule->get_material(MaterialIdx)));

                    FString MDLHeader, SubMDL;
                    FString(Prototype).Split("::", &MDLHeader, &SubMDL, ESearchCase::CaseSensitive, ESearchDir::FromStart);
                    if (!Prototype.IsEmpty() && !IsMDLLibrary(MDLHeader))
                    {
                        // Convert mdl path to Omni path
                        FString BaseMaterialFile, BaseMaterialName;
                        FString(Prototype).Split("::", &BaseMaterialFile, &BaseMaterialName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
                        BaseMaterialFile.RemoveFromStart(TEXT("mdl"));
                        BaseMaterialFile.ReplaceInline(TEXT("::"), TEXT("/"));

                        // Load Base MDL
                        BaseMaterialFile = BaseMaterialFile + TEXT(".mdl");
                        UMaterialInterface* ParentMaterial = nullptr;
                        bool IsLocalBaseMDL = USimReadyMDL::IsLocalBaseMDL(BaseMaterialFile);
                        if (IsLocalBaseMDL)
                        {
                            int32 ReparentMaterialIndex = FSimReadyMaterialReparentUtility::FindUnrealMaterial(BaseMaterialName, ParentMaterial, ESimReadyReparentUsage::Import);
                            // Only reparent material is imported as the material instance, other mdls including mdl presets are imported as material
                            if (ParentMaterial)
                            {
                                auto MaterialInstance = NewObject<UMaterialInstanceConstant>(InPackage, Name, Flags);

                                MaterialInstance->ClearParameterValuesEditorOnly();
                                MaterialInstance->SetParentEditorOnly(ParentMaterial);
                                MaterialInstance->SetScalarParameterValueEditorOnly(TEXT("World-Aligned Textures"), 0);

                                static FLoadInstanceTextureCallback LoadInstanceTexture;
                                LoadInstanceTexture = [&](const FString& InTextureFileName, const FString& DisplayName, float Gamma)
                                {
                                    IFileHandle* TextureHandle = ExternalReader->OpenRead(InTextureFileName);
                                    if (TextureHandle)
                                    {
                                        int64 Size = TextureHandle->Size();
                                        uint8* Buffer = new uint8[Size];
                                        TextureHandle->Read(Buffer, Size);

                                        UTexture* OutTexture = nullptr;
                                        if (Callback)
                                        {
                                            Callback(Buffer, Size, InTextureFileName, OutTexture);
                                        }

                                        if (OutTexture)
                                        {
                                            bool bTextureChanged = false;
                                            if (Gamma > 0.0f)
                                            {
                                                bool SRGB = (Gamma != 1.0f);
                                                if (OutTexture->SRGB != SRGB)
                                                {
                                                    OutTexture->SRGB = SRGB;
                                                    // check current CompressionSettings accepts srgb
                                                    if (OutTexture->SRGB && OutTexture->CompressionSettings == TC_Normalmap)
                                                    {
                                                        OutTexture->CompressionSettings = TC_Default;
                                                    }
                                                    bTextureChanged = true;
                                                }
                                            }

                                            if (bTextureChanged)
                                            {
                                                OutTexture->PostEditChange();
                                            }

                                            OutTexture->MarkPackageDirty();

                                            MaterialInstance->SetTextureParameterValueEditorOnly(*DisplayName, OutTexture);
                                        }

                                        delete[] Buffer;
                                    }
                                };

                                static FGetDisplayNameCallback GetDisplayName;
                                GetDisplayName = [&](const FString& InMaterialName, const FString& ParameterName, FString& DisplayName) -> bool
                                {
                                    bool bRet = USimReadyMDL::GetDisplayNameFromLocalBaseMDL(InMaterialName, ParameterName, DisplayName);
                                    if (ReparentMaterialIndex != INDEX_NONE)
                                    {
                                        FSimReadyMaterialReparentUtility::FindUnrealParameter(ReparentMaterialIndex, MoveTemp(DisplayName), DisplayName);
                                    }
                                    return bRet;
                                };

                                FMDLImporterUtility::CreateInstanceFromBaseMDL(MaterialInstance, BaseMaterialName, MaterialDefinition, GetDisplayName, LoadInstanceTexture);
                                MaterialInstance->PostEditChange();
                                MaterialInstance->MarkPackageDirty();
                                FAssetRegistryModule::AssetCreated(MaterialInstance);
                                OutMaterial = MaterialInstance;
                                break;
                            }
                        }
                    }

                    // assuming the material has parameters without defaults
                    mi::neuraylib::Definition_wrapper DefinitionWrapper(
                        MDLPluginModule->GetTransaction().get(), MDLModule->get_material(MaterialIdx), MDLPluginModule->GetFactory().get());

                    mi::base::Handle<mi::neuraylib::IScene_element> MaterialInstanceSE(DefinitionWrapper.create_instance(nullptr));

                    mi::base::Handle<const mi::neuraylib::ICompiled_material> CompiledMaterial = FMDLImporterUtility::CreateCompiledMaterial(MaterialInstanceSE);
                    if (!CompiledMaterial.is_valid_interface())
                    {
                        break;
                    }

                    FMDLParametersList List;
                    FMDLImporterUtility::UpdateParametersSheet(MaterialDefinition, CompiledMaterial, List);
                    ImportedParametersSheet.Add(Name.ToString(), List);

                    auto Material = NewObject<UMaterial>(InPackage, Name, Flags);
                        
                    static FLoadTextureCallback LoadTexture;
                    LoadTexture = [&](UTexture*& OutTexture, const FString& TexturePath, float Gamma, TextureCompressionSettings Compression)
                    {
                        if (Callback)
                        {
                            Callback(nullptr, 0, TexturePath, OutTexture);
                        }

                        if (OutTexture == nullptr)
                        {

                            IFileHandle* TextureHandle = ExternalReader->OpenRead(TexturePath);
                            if (TextureHandle)
                            {
                                int64 Size = TextureHandle->Size();
                                uint8* Buffer = new uint8[Size];
                                TextureHandle->Read(Buffer, Size);

                                if (Callback)
                                {
                                    Callback(Buffer, Size, TexturePath, OutTexture);
                                }

                                if (OutTexture)
                                {
                                    bool bTextureChanged = false;
                                    if (OutTexture->CompressionSettings == TC_Default && OutTexture->CompressionSettings != Compression)
                                    {
                                        OutTexture->CompressionSettings = Compression;
                                        bTextureChanged = true;
                                    }

                                    if (Gamma > 0.0f)
                                    {
                                        bool SRGB = (Gamma != 1.0f);
                                        if (OutTexture->SRGB != SRGB)
                                        {
                                            OutTexture->SRGB = SRGB;
                                            // check current CompressionSettings accepts srgb
                                            if (OutTexture->SRGB && OutTexture->CompressionSettings == TC_Normalmap)
                                            {
                                                OutTexture->CompressionSettings = TC_Default;
                                            }
                                            bTextureChanged = true;
                                        }
                                    }

                                    if (bTextureChanged)
                                    {
                                        OutTexture->PostEditChange();
                                    }

                                    OutTexture->MarkPackageDirty();
                                }

                                delete[] Buffer;
                            }
                        }
                    };
                        
                    if (FMDLImporterUtility::DistillCompiledMaterial(Material, MaterialDefinition, CompiledMaterial, true, nullptr, LoadTexture))
                    {
                        OutMaterial = Material;
                        Material->PostEditChange();
                        Material->MarkPackageDirty();
                        FAssetRegistryModule::AssetCreated(Material);
                    }
                    else
                    {
                        Material->ClearFlags(RF_Standalone);
                        Material->Rename(NULL, GetTransientPackage(), REN_DontCreateRedirectors);
                    }
                    break;
                }
            }
        }
    }

    MDLPluginModule->RemoveMdlModule(InModuleName);
    MDLPluginModule->CommitAndCreateTransaction();
    for (auto Module : ModulePaths)
    {
        MDLPluginModule->RemoveModulePath(Module);
    }
#ifdef WITH_CARLA
    FMDLImporterUtility::ResetProjectMdlRootPath();
#endif

    return OutMaterial;
}

void USimReadyMDL::ImportMdlParameters(const FString& InModuleName, const FString& InMaterialName, const TArray<FString>& ModulePaths, 
    TSharedPtr<IMDLExternalReader> ExternalReader, const FString& InMaterialAssetName)
{
    UMaterialInterface* OutMaterial = nullptr;
    auto MDLPluginModule = FModuleManager::GetModulePtr<IMDLModule>("MDL");
    {
        for (auto Module : ModulePaths)
        {
            MDLPluginModule->AddModulePath(Module, true);
        }

        if (ExternalReader == nullptr)
        {
            ExternalReader = MakeShareable<FSimReadyMDLReader>(new FSimReadyMDLReader());
        }
        MDLPluginModule->SetExternalFileReader(ExternalReader);
        
        int32 Result = MDLPluginModule->LoadMdlModule(InModuleName);
        MDLPluginModule->SetExternalFileReader(nullptr);
        if (Result >= 0)
        {		
            mi::base::Handle<const mi::neuraylib::IModule> MDLModule = MDLPluginModule->GetLoadedMdlModule(InModuleName);

            if (MDLModule.is_valid_interface())
            {
                // Convert MDL to UE4 material
                for (auto MaterialIdx = 0; MaterialIdx < MDLModule->get_material_count(); ++MaterialIdx)
                {
                    FString MaterialName = MDLModule->get_material(MaterialIdx);
                    MaterialName = MaterialName.Left(MaterialName.Find(TEXT("(")));
                    FString(MoveTemp(MaterialName)).Split("::", nullptr, &MaterialName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

                    if (MaterialName != InMaterialName)
                    {
                        continue;
                    }

                    // Compile
                    mi::base::Handle<const mi::neuraylib::IFunction_definition> MaterialDefinition = mi::base::make_handle(MDLPluginModule->GetTransaction()->access<mi::neuraylib::IFunction_definition>(MDLModule->get_material(MaterialIdx)));
                    // assuming the material has parameters without defaults
                    mi::neuraylib::Definition_wrapper DefinitionWrapper(
                        MDLPluginModule->GetTransaction().get(), MDLModule->get_material(MaterialIdx), MDLPluginModule->GetFactory().get());

                    mi::base::Handle<mi::neuraylib::IScene_element> MaterialInstanceSE(DefinitionWrapper.create_instance(nullptr));

                    mi::base::Handle<const mi::neuraylib::ICompiled_material> CompiledMaterial = FMDLImporterUtility::CreateCompiledMaterial(MaterialInstanceSE);
                    if (!CompiledMaterial.is_valid_interface())
                    {
                        break;
                    }

                    FMDLParametersList List;
                    FMDLImporterUtility::UpdateParametersSheet(MaterialDefinition, CompiledMaterial, List);
                    ImportedParametersSheet.Add(InMaterialAssetName, List);
            
                    break;
                }
            }
        }
    }

    MDLPluginModule->RemoveMdlModule(InModuleName);
    MDLPluginModule->CommitAndCreateTransaction();
    for (auto Module : ModulePaths)
    {
        MDLPluginModule->RemoveModulePath(Module);
    }
}

void USimReadyMDL::ResetImportParametersSheet()
{
    ImportedParametersSheet.Reset();
}

bool USimReadyMDL::GetImportDisplayName(const FString& InMaterialName, const FString& InParameterName, FString& DisplayName)
{
    auto ParametersList = ImportedParametersSheet.Find(InMaterialName);
    if (ParametersList)
    {
        for (auto Parameter : ParametersList->ParametersList)
        {
            if (Parameter.ParameterName == InParameterName)
            {
                DisplayName = Parameter.DisplayName;
                return true;
            }
        }
    }

    return false;
}

void USimReadyMDL::GetLocalBaseMaterialNames(TArray<FString>& Names)
{
    FMDLImporterUtility::GetAllBaseMaterials(Names);
}

void USimReadyMDL::GetAllDisplayNamesFromLocalBaseMDL(const FString& InMaterialName, TMap<FString, EMaterialParameterType>& Parameters)
{
    FMDLImporterUtility::GetAllDisplayNamesFromBaseModule(InMaterialName, Parameters);
}

FString USimReadyMDL::MaterialParameterTypeToString(EMaterialParameterType Type)
{
    switch (Type)
    {
    case EMaterialParameterType::Scalar: return TEXT("Scalar");
    case EMaterialParameterType::Vector: return TEXT("Vector");
    case EMaterialParameterType::Texture: return TEXT("Texture");
    case EMaterialParameterType::Font: return TEXT("Font");
    case EMaterialParameterType::RuntimeVirtualTexture: return TEXT("RuntimeVirtualTexture");
    case EMaterialParameterType::StaticSwitch: return TEXT("StaticSwitch");
    default: return TEXT("Unknown");
    }
}
