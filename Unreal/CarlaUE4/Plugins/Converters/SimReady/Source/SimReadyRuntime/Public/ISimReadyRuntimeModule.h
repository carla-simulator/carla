// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Stats/Stats.h"
#include "Containers/UnrealString.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class SIMREADYRUNTIME_API ISimReadyRuntimeModule : public IModuleInterface
{
public:
    /**
     * Singleton-like access to this module's interface.  This is just for convenience!
     * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
     *
     * @return Returns singleton instance.
     */
    static inline ISimReadyRuntimeModule& Get()
    {
        return FModuleManager::GetModuleChecked< ISimReadyRuntimeModule >("SimReadyRuntime");
    }

    /**
     * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
     *
     * @return True if the module is loaded and ready to use
     */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("SimReadyRuntime");
    }

    virtual FString GetUsdVersion() const = 0;
};

DECLARE_STATS_GROUP(TEXT("SimReady"), STATGROUP_SimReady, STATCAT_Advanced);
