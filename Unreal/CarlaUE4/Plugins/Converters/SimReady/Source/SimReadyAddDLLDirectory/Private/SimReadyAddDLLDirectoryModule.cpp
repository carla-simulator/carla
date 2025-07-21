// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

class SIMREADYADDDLLDIRECTORY_API FSimReadyAddDLLDirectoryModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
#ifdef PLATFORM_WINDOWS
        const FString UsdDllDir = IPluginManager::Get().FindPlugin("USDImporter")->GetBaseDir() / "Binaries" / "Win64";
        FPlatformProcess::AddDllDirectory(*(UsdDllDir));
#elif PLATFORM_LINUX
        // Linux doesn't support DLL Directories, so we'll just load these shared libraries here so they can be picked up by all of the plugins
        // The mechanisms in SimReadyRuntime.Build.cs don't appear to be working to provide absolute rpaths
        const FString UsdLibDir = IPluginManager::Get().FindPlugin("USDImporter")->GetBaseDir() / "Binaries" / "Linux" / "x86_64-unknown-linux-gnu";
        const TArray<FString> UsdLibs = 
        {
            "libboost_python37.so",
            "libar.so",
            "libarch.so",
            "libgf.so",
            "libjs.so",
            "libkind.so",
            "libndr.so",
            "libpcp.so",
            "libplug.so",
            "libsdf.so",
            "libsdr.so",
            "libtf.so",
            "libtrace.so",
            "libusd.so",
            "libusdGeom.so",
            //"libusdHydra.so",
            "libusdLux.so",
            "libusdMedia.so",
            //"libusdRender.so",
            //"libusdRi.so",
            "libusdShade.so",
            "libusdSkel.so",
            "libusdUI.so",
            "libusdUtils.so",
            "libusdVol.so",
            "libvt.so",
            "libwork.so"
        };

        for (const FString& UsdLib : UsdLibs)
        {
            FPlatformProcess::GetDllHandle(*(UsdLibDir / UsdLib));
        }
#endif
    }

    virtual void ShutdownModule() override
    {
    }
};

IMPLEMENT_MODULE(FSimReadyAddDLLDirectoryModule, SimReadyAddDLLDirectory)
