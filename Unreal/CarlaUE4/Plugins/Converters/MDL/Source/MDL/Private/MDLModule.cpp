// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLModule.h"
#include "Editor.h"
#include "MDLPrivateModule.h"
#include "MDLOutputLogger.h"
#include "Interfaces/IPluginManager.h"
#include "MDLEntityResolver.h"
#include "DynamicMipsLoader.h"
#include "MDLKeywords.h"
#include "MDLImporterUtility.h"
#include "ISettingsModule.h"
#include "MDLMaterialImporter.h"
#include "MDLPathUtility.h"

#define LOCTEXT_NAMESPACE "MDL Importer"
using namespace MDLPathUtility;

FMDLEntityResolver* GMDLEntityResolver = nullptr;

void FMDLModule::CommitAndCreateTransaction()
{
    if (Transaction)
    {
        Transaction->commit();
        Transaction.reset();
    }

    if (MDLScope)
    {
        Transaction = mi::base::make_handle(MDLScope->create_transaction());
    }
}

void FMDLModule::AddModulePath(const FString& Path, bool IgnoreCheck)
{
    GMDLEntityResolver->AddModulePath(Path, IgnoreCheck);
}

void FMDLModule::RemoveModulePath(const FString& Path)
{
    GMDLEntityResolver->RemoveModulePath(Path);
}

void FMDLModule::SetExternalFileReader(TSharedPtr<IMDLExternalReader> FileReader)
{
    GMDLEntityResolver->SetExternalFileReader(FileReader);
}

void FMDLModule::RegisterDynamicMipsLoader(IDynamicMipsLoader* Loader)
{
    DynamicMipsLoader = MakeShareable<IDynamicMipsLoader>(Loader);
}

TSharedPtr<IDynamicMipsLoader> FMDLModule::GetDynamicMipsLoader()
{
    return DynamicMipsLoader;
}

TSharedPtr<IMDLMaterialImporter> FMDLModule::CreateDefaultImporter()
{
    return MakeShareable<IMDLMaterialImporter>(new FMDLMaterialImporter(nullptr));
}

int32 FMDLModule::LoadMdlModule(const FString& ModuleName)
{
    // Only need mangling when loading the module.
    FString MangledModuleName = MangleMdlPath(ModuleName);
    mi::base::Handle<mi::neuraylib::IMdl_execution_context> Context(MDLFactory->create_execution_context());
    Context->set_option("mdl_next", true); // for mdl 1.8
    Context->set_option("resolve_resources", false); // for skip loading texture after 367100
    mi::Sint32 ErrorCode = MDLImpexpAPI->load_module(Transaction.get(), TCHAR_TO_UTF8(*MangledModuleName), Context.get());
    return ErrorCode;
}

mi::base::Handle<const mi::neuraylib::IModule> FMDLModule::GetLoadedMdlModule(const FString& ModuleName)
{
    FString MangledModuleName = MangleMdlPath(ModuleName);
    return mi::base::make_handle(Transaction->access<mi::neuraylib::IModule>(TCHAR_TO_UTF8(*(TEXT("mdl") + MangledModuleName))));
}

void FMDLModule::RemoveMdlModule(const FString& ModuleName)
{
    FString MangledModuleName = MangleMdlPath(ModuleName);
    Transaction->remove(TCHAR_TO_UTF8(*(TEXT("mdl") + MangledModuleName)));
}

void FMDLModule::StartupModule()
{
    DynamicMipsLoader = nullptr;
#if WITH_MDL_SDK
    FString MDLSDKPathArray = FPlatformMisc::GetEnvironmentVariable(TEXT("MDL_SDK_PATH"));

    FString MDLSDKPath = MDLSDKPathArray;
    const FString PluginBaseDir = IPluginManager::Get().FindPlugin("MDL")->GetBaseDir();
    if (MDLSDKPath.IsEmpty())
    {
#if PLATFORM_WINDOWS		
        MDLSDKPath = PluginBaseDir / TEXT("Source/ThirdParty/mdl_sdk/bin");
#elif PLATFORM_LINUX
        MDLSDKPath = PluginBaseDir / TEXT("Source/ThirdParty/mdl_sdk/lib");
#endif		
    }

    FString LibMDLSDK = TEXT("libmdl_sdk" MI_BASE_DLL_FILE_EXT);
    FString MDLDistiller = TEXT("mdl_distiller" MI_BASE_DLL_FILE_EXT);

    MDLSDKHandle = FPlatformProcess::GetDllHandle(*(MDLSDKPath / LibMDLSDK));
    check(MDLSDKHandle != nullptr);

    Neuray = mi::neuraylib::mi_factory<mi::neuraylib::INeuray>(FPlatformProcess::GetDllExport(MDLSDKHandle, TEXT("mi_factory")));
    check(Neuray.is_valid_interface());

    GMDLEntityResolver = new FMDLEntityResolver();

    // Set up paths
    FString UserPath = FPlatformMisc::GetEnvironmentVariable(TEXT("MDL_USER_PATH"));
    //FPlatformMisc::GetEnvironmentVariable(TEXT("MDL_SYSTEM_PATH"), SystemPath, ARRAY_COUNT(SystemPath));

    // Set up configuration
    MDLConfiguration = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::IMdl_configuration>());
    mi::base::Handle<mi::neuraylib::ILogging_configuration> LoggingConfiguration = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::ILogging_configuration>());
    LoggingConfiguration->set_receiving_logger(&GMDLOutputLogger);

    // Add local template paths
    AddModulePath(FPaths::ConvertRelativePathToFull(PluginBaseDir / TEXT("Library/mdl/mdl")));
    AddModulePath(FPaths::ConvertRelativePathToFull(PluginBaseDir / TEXT("Library/mdl/Base")));
    AddModulePath(FPaths::ConvertRelativePathToFull(PluginBaseDir / TEXT("Library/mdl/Ue4")));

#if PLATFORM_WINDOWS
    FString Root = TEXT("A:");
    for(int32 Index = 0; Index < 26; ++Index)
    {
        Root[0] = TCHAR('A') + Index;
        if (Root[0] != TCHAR('O'))
        {
            AddModulePath(Root);
        }
    }
#else
    AddModulePath(TEXT("/"));
#endif

    FString MDLModulePath = FPaths::ConvertRelativePathToFull(PluginBaseDir / TEXT("MDL"));
    MDLConfiguration->add_mdl_path(TCHAR_TO_UTF8(*MDLModulePath));
    AddModulePath(MDLModulePath);

    if (!UserPath.IsEmpty())
    {
        MDLConfiguration->add_mdl_path(TCHAR_TO_UTF8(*UserPath));
        AddModulePath(UserPath);
    }
    //MDLConfiguration->add_mdl_path(TCHAR_TO_UTF8(SystemPath));

    // load image plugin
    auto PluginConfiguration = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::IPlugin_configuration>());
    verify(PluginConfiguration->load_plugin_library(TCHAR_TO_UTF8(*(MDLSDKPath / MDLDistiller))) == 0);

    // start neuray AFTER loading the plugin !!
    check(Neuray->start() == 0);
    UE_LOG(LogMDLOutput, Log, TEXT("Neuray Library Version: %s"), *FString(Neuray->get_version()));

    // set external entity resolver
    MDLConfiguration->set_entity_resolver(GMDLEntityResolver);

    // Set up compiler
    MDLImpexpAPI = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::IMdl_impexp_api>());


    const mi::base::Handle<mi::neuraylib::IDatabase> Database(Neuray->get_api_component<mi::neuraylib::IDatabase>());
    MDLScope = mi::base::make_handle(Database->get_global_scope());
    Transaction = mi::base::make_handle(MDLScope->create_transaction());
    MDLFactory = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::IMdl_factory>());

    // Set up distiller and ImageAPI
    MDLDistillerAPI = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::IMdl_distiller_api>());
    ImageAPI = mi::base::make_handle(Neuray->get_api_component<mi::neuraylib::IImage_api>());

    // Register the MDL settings
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings("Project", "Plugins", "MDLSettings", LOCTEXT("RuntimeSettingsName", "MDL"), LOCTEXT("RuntimeSettingsDescription",
            "Configure MDL settings"), GetMutableDefault<UMDLSettings>());
    }

    MDLUserPath = FString(UserPath);
    FPaths::NormalizeDirectoryName(MDLUserPath);
#endif
    for (auto BaseName : BaseTemplates)
    {
        if (FMDLImporterUtility::LoadBaseModule(BaseName.ToString() + TEXT(".mdl")))
        {
            FMDLImporterUtility::UpdateBaseModuleParameters(BaseName.ToString());
        }
    }
}

void FMDLModule::ShutdownModule()
{
#if WITH_MDL_SDK
    // Unregister the MDL settings
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "MDLSettings");
    }

    Transaction->commit();
    Transaction.reset();
    MDLScope.reset();
    ImageAPI.reset();
    MDLDistillerAPI.reset();
    MDLFactory.reset();
    MDLImpexpAPI.reset();
    MDLConfiguration.reset();

    Neuray->shutdown();
    Neuray.reset();

    delete GMDLEntityResolver;
    DynamicMipsLoader = nullptr;
    FPlatformProcess::FreeDllHandle(MDLSDKHandle);
#endif
}

IMPLEMENT_MODULE(FMDLModule, MDL);

#undef LOCTEXT_NAMESPACE
