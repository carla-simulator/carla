// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "Logging/TokenizedMessage.h"

class SIMREADYRUNTIME_API FSimReadyMessageLogContext
{
public:
    static void AddMessage(EMessageSeverity::Type MessageSeverity, FText Message);
    static void AddMessage(const TSharedRef< class FTokenizedMessage >& Message);
    static void DisplayMessages();
    static void ClearMessages();
};