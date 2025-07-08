// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "Misc/SlowTask.h"

class SIMREADYRUNTIME_API FSimReadySlowTask
{
public:
    FSimReadySlowTask();

    virtual ~FSimReadySlowTask();

    void BeginProgress(float InAmountOfWork, const FText& InDefaultMessage, bool ShowProgressDialog = true, bool bShowCancelButton = true);
    void UpdateProgress(float ExpectedWorkThisFrame, const FText& Text);
    void EndProgress();
    bool IsProgressCancelled();

    static FSimReadySlowTask& Get();

protected:
    friend FSlowTask;

    TUniquePtr<FSlowTask> SlowTask;
};