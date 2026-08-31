// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Carla.h"
#include "Settings/CarlaSettings.h"

#include <util/ue-header-guard-begin.h>
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/PackageName.h"
#include "HAL/FileManager.h"
#include "ShaderCore.h"
#include <util/ue-header-guard-end.h>

#define LOCTEXT_NAMESPACE "FCarlaModule"

DEFINE_LOG_CATEGORY(LogCarla);
DEFINE_LOG_CATEGORY(LogCarlaServer);

void FCarlaModule::StartupModule()
{
    MountExternalPackageRoots();
    MountEditorContentPacks();
	AddShaderSearchPaths();
	RegisterSettings();
	LoadChronoDll();
}

void FCarlaModule::MountExternalPackageRoots()
{
    // One-file-per-actor packages are named by the engine as
    //     /Game/__ExternalActors__/<path of the level under /Game>
    // i.e. the __ExternalActors__ tree belongs at the mount root (Content/).
    // CARLA content ships instead as a self-contained bundle that is cloned into
    // a subfolder of Content (Content/Carla by default, but any name), carrying
    // its own copy of the tree:
    //     Content/<Root>/__ExternalActors__/<path of the level under /Game>
    // Those files are therefore named /Game/<Root>/__ExternalActors__/... and do
    // not satisfy the references stored inside the levels, so every actor of a
    // World Partition map is unresolvable: empty World Partition, no cells.
    //
    // Rather than moving content around, map the names the levels actually ask
    // for onto the directories the bundles actually ship. Mount lookup picks the
    // most specific root (FPackageName uses a prefix tree), so these nested
    // mounts win over "/Game/" without disturbing anything else.
    const FString ContentDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());

    // Handles both external actors and external objects (actor-owned sub-objects
    // such as data layers use the same scheme with a different folder).
    const TCHAR* ExternalFolderNames[] = { TEXT("__ExternalActors__"), TEXT("__ExternalObjects__") };

    TArray<FString> ContentRoots;
    IFileManager::Get().FindFiles(ContentRoots, *(ContentDir / TEXT("*")), false, true);

    for (const FString& Root : ContentRoots)
    {
        for (const TCHAR* FolderName : ExternalFolderNames)
        {
            const FString BundleFolder = ContentDir / Root / FolderName;
            if (!FPaths::DirectoryExists(BundleFolder))
            {
                continue;
            }

            // The bundle mirrors the level's package path underneath, e.g.
            // Content/Carla/__ExternalActors__/Carla/Maps/Town12/Town12. Mount each
            // first-level entry separately so two bundles (Content/Carla and, say,
            // Content/TestingMap) can coexist under the same /Game/__External*__/
            // namespace without colliding.
            TArray<FString> Inner;
            IFileManager::Get().FindFiles(Inner, *(BundleFolder / TEXT("*")), false, true);
            for (const FString& InnerName : Inner)
            {
                const FString RootPath = FString::Printf(
                    TEXT("/Game/%s/%s/"), FolderName, *InnerName);
                const FString ContentPath = BundleFolder / InnerName / TEXT("");

                // Registering the same pair twice (editor hot-reload re-runs
                // StartupModule) would duplicate the mount, so drop any previous one.
                FPackageName::UnRegisterMountPoint(RootPath, ContentPath);
                FPackageName::RegisterMountPoint(RootPath, ContentPath);
                UE_LOG(LogCarla, Log, TEXT("Mounted external package root %s -> %s"),
                    *RootPath, *ContentPath);
            }
        }
    }
}

void FCarlaModule::MountEditorContentPacks()
{
#if WITH_EDITOR
    // Content packs are authored under Plugins/Packs/<Pack>/ with
    // "ExplicitlyLoaded": true, which the stock plugin manager leaves
    // unmounted. The interactive editor needs the pack content visible for
    // authoring, so mount them here. Commandlets (the cooker) are left alone:
    // the DLC cook enables the pack explicitly through the packer, and a base
    // cook must not see pack content at all. Game processes use
    // UCarlaContentPackManager instead.
    if (!GIsEditor || IsRunningCommandlet())
    {
        return;
    }
    const FString PacksDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir() / TEXT("Packs"));
    TArray<FString> PackDirs;
    IFileManager::Get().FindFiles(PackDirs, *(PacksDir / TEXT("*")), false, true);
    PackDirs.Sort();
    IPluginManager& PluginManager = IPluginManager::Get();
    for (const FString& PackName : PackDirs)
    {
        const FString PluginFile = PacksDir / PackName / (PackName + TEXT(".uplugin"));
        if (!FPaths::FileExists(PluginFile))
        {
            continue;
        }
        TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin(PackName);
        if (!Plugin.IsValid())
        {
            FText FailReason;
            if (!PluginManager.AddToPluginsList(PluginFile, &FailReason))
            {
                UE_LOG(LogCarla, Warning, TEXT("Content pack plugin '%s' not registered: %s"),
                    *PluginFile, *FailReason.ToString());
                continue;
            }
            Plugin = PluginManager.FindPlugin(PackName);
        }
        if (!Plugin.IsValid() || Plugin->IsMounted())
        {
            continue;
        }
        if (!Plugin->GetDescriptor().bExplicitlyLoaded)
        {
            // Regular plugin: the plugin manager enables it by its own rules.
            continue;
        }
        if (PluginManager.MountExplicitlyLoadedPlugin(PackName))
        {
            UE_LOG(LogCarla, Log, TEXT("Content pack plugin '%s' mounted for the editor/cooker (%s)"),
                *PackName, *PluginFile);
        }
        else
        {
            UE_LOG(LogCarla, Warning, TEXT("Content pack plugin '%s' could not be mounted (%s)"),
                *PackName, *PluginFile);
        }
    }
#endif // WITH_EDITOR
}

void FCarlaModule::AddShaderSearchPaths()
{
	// The shader virtual path is process-global. Re-running StartupModule
	// (editor Live Coding / hot-reload) must not register it twice, as
	// AddShaderSourceDirectoryMapping asserts the path is not already mapped.
	if (AllShaderSourceDirectoryMappings().Contains(TEXT("/Plugin/Carla")))
		return;

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Carla"));
	if (!Plugin.IsValid())
	{
		UE_LOG(LogCarla, Error,
			TEXT("AddShaderSearchPaths: Carla plugin not found; shaders will be unavailable."));
		return;
	}

	const FString ShadersDirectoryPath = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders")));
	if (!FPaths::DirectoryExists(ShadersDirectoryPath))
	{
		UE_LOG(LogCarla, Error,
			TEXT("AddShaderSearchPaths: shader directory '%s' does not exist; shaders will be unavailable."),
			*ShadersDirectoryPath);
		return;
	}

	UE_LOG(LogCarla, Log, TEXT("Carla shader directory: %s"), *ShadersDirectoryPath);
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Carla"), ShadersDirectoryPath);
}

void FCarlaModule::LoadChronoDll()
{
	#if defined(WITH_CHRONO) && PLATFORM_WINDOWS
	const FString BaseDir = FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("Carla"));
	const FString DllDir = FPaths::Combine(*BaseDir, TEXT("CarlaDependencies"), TEXT("dll"));
	FString ChronoEngineDll = FPaths::Combine(*DllDir, TEXT("ChronoEngine.dll"));
	FString ChronoVehicleDll = FPaths::Combine(*DllDir, TEXT("ChronoEngine_vehicle.dll"));
	FString ChronoModelsDll = FPaths::Combine(*DllDir, TEXT("ChronoModels_vehicle.dll"));
	FString ChronoRobotDll = FPaths::Combine(*DllDir, TEXT("ChronoModels_robot.dll"));
	UE_LOG(LogCarla, Log, TEXT("Loading Dlls from: %s"), *DllDir);
	auto ChronoEngineHandle = FPlatformProcess::GetDllHandle(*ChronoEngineDll);
	if (!ChronoEngineHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoEngine.dll could not be loaded"));
	}
	auto ChronoVehicleHandle = FPlatformProcess::GetDllHandle(*ChronoVehicleDll);
	if (!ChronoVehicleHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoEngine_vehicle.dll could not be loaded"));
	}
	auto ChronoModelsHandle = FPlatformProcess::GetDllHandle(*ChronoModelsDll);
	if (!ChronoModelsHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoModels_vehicle.dll could not be loaded"));
	}
	auto ChronoRobotHandle = FPlatformProcess::GetDllHandle(*ChronoRobotDll);
	if (!ChronoRobotHandle)
	{
		UE_LOG(LogCarla, Warning, TEXT("Error: ChronoModels_robot.dll could not be loaded"));
	}
	#endif
}

void FCarlaModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FCarlaModule::RegisterSettings()
{
	// Registering some settings is just a matter of exposing the default UObject of
	// your desired class, add here all those settings you want to expose
	// to your LDs or artists.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Create the new category
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("CARLASettings",
			LOCTEXT("RuntimeWDCategoryName", "CARLA Settings"),
			LOCTEXT("RuntimeWDCategoryDescription", "CARLA plugin settings"));

		// Register the settings
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "CARLASettings", "General",
			LOCTEXT("RuntimeGeneralSettingsName", "General"),
			LOCTEXT("RuntimeGeneralSettingsDescription", "General configuration for the CARLA plugin"),
			GetMutableDefault<UCarlaSettings>()
		);

		// Register the save handler to your settings, you might want to use it to
		// validate those or just act to settings changes.
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FCarlaModule::HandleSettingsSaved);
		}
	}
}

void FCarlaModule::UnregisterSettings()
{
	// Ensure to unregister all of your registered settings here, hot-reload would
	// otherwise yield unexpected results.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "CustomSettings", "General");
	}
}

bool FCarlaModule::HandleSettingsSaved()
{
	UCarlaSettings* Settings = GetMutableDefault<UCarlaSettings>();
	bool ResaveSettings = false;

	// Put any validation code in here and resave the settings in case an invalid
	// value has been entered

	if (ResaveSettings)
	{
		Settings->SaveConfig();
	}

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCarlaModule, Carla)

// =============================================================================
// -- Implement carla throw_exception ------------------------------------------
// =============================================================================

#ifdef LIBCARLA_NO_EXCEPTIONS
#include <util/disable-ue4-macros.h>
#include <carla/Exception.h>
#include <util/enable-ue4-macros.h>

#include <exception>
namespace carla {

  void throw_exception(const std::exception &e) {
    UE_LOG(LogCarla, Fatal, TEXT("Exception thrown: %s"), UTF8_TO_TCHAR(e.what()));
    // It should never reach this part.
    std::terminate();
  }

} // namespace carla
#endif
