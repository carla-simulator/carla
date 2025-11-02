// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "ISimReadyRuntimeModule.h"

class FSimReadyRuntimeModule : public ISimReadyRuntimeModule
{
public: // IModuleInterface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

public: // ISimReadyRuntimeModule
    virtual FString GetUsdVersion() const override;

public: // FSimReadyRuntimeModule
    static FSimReadyRuntimeModule & Get() { return *Singleton; }

private:
    static FSimReadyRuntimeModule * Singleton;

    void* nvCudartHandle;
    void* nvTextureToolsHandle;
};
