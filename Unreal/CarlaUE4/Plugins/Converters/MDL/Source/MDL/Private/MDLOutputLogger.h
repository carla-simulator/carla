// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMDLOutput, Log, All);

#if WITH_MDL_SDK

#include "MDLDependencies.h"
#include "Logging/LogMacros.h"

class FMDLOutputLogger : public mi::base::Interface_implement<mi::base::ILogger>
{
public:
    void message(mi::base::Message_severity level, const char* module_category, const mi::base::Message_details&, const char* message) override;
};

extern FMDLOutputLogger GMDLOutputLogger;

#endif
