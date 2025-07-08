// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SIMREADYUSD_API ISimReadyUSDModule : public IModuleInterface
{
public:
    virtual ~ISimReadyUSDModule()
    {
    }

    virtual void RegisterUSDTranslator(class ISimReadyUSDTranslator* UsdTranslator) = 0;

    virtual bool HasUSDTranslator() = 0;

    virtual TArray<TSharedPtr<ISimReadyUSDTranslator>> GetUSDTranslators() const = 0;
};

