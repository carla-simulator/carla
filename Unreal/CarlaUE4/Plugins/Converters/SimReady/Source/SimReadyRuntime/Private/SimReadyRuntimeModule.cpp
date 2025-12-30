// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyRuntimeModule.h"

#include "Async/Async.h"
#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
#include "Engine/EngineBaseTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/CoreDelegates.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#include "SimReadyPxr.h"
#include "SimReadyRuntimePrivate.h"
#include "SimReadyPathHelper.h"
#include "SimReadySettings.h"
#include "SimReadyNotificationHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogSimReady);

FSimReadyRuntimeModule * FSimReadyRuntimeModule::Singleton;

void FSimReadyRuntimeModule::StartupModule()
{
    Singleton = this;

#if PLATFORM_WINDOWS && PLATFORM_64BITS
    const FString BaseDir = IPluginManager::Get().FindPlugin("SimReady")->GetBaseDir() / "ThirdParty";
    nvCudartHandle = FPlatformProcess::GetDllHandle(*(BaseDir / "nvtt/win64_release/cudart64_110.dll"));
    nvTextureToolsHandle = FPlatformProcess::GetDllHandle(*(BaseDir / "nvtt/win64_release/nvtt30106.dll"));
#endif
}

void FSimReadyRuntimeModule::ShutdownModule()
{
#if PLATFORM_WINDOWS && PLATFORM_64BITS	
    FPlatformProcess::FreeDllHandle(nvTextureToolsHandle);
    FPlatformProcess::FreeDllHandle(nvCudartHandle);
    nvTextureToolsHandle = nullptr;
    nvCudartHandle = nullptr;
#endif

    Singleton = nullptr;
}

FString FSimReadyRuntimeModule::GetUsdVersion() const
{
    return FString::Printf(TEXT("%i.%i"), PXR_MINOR_VERSION, PXR_PATCH_VERSION);
}

IMPLEMENT_MODULE(FSimReadyRuntimeModule, SimReadyRuntime)
