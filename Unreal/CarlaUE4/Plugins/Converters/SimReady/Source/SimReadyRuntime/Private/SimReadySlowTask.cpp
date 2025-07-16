// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadySlowTask.h"
#include "Misc/FeedbackContext.h"
#include "CoreGlobals.h"

FSimReadySlowTask& FSimReadySlowTask::Get()
{
    static FSimReadySlowTask Singleton;
    return Singleton;
}

FSimReadySlowTask::FSimReadySlowTask()
{
}

FSimReadySlowTask::~FSimReadySlowTask()
{
}

void FSimReadySlowTask::BeginProgress(float InAmountOfWork, const FText& InDefaultMessage, bool ShowProgressDialog, bool bShowCancelButton)
{
    TUniquePtr<FSlowTask> TempTask(new FSlowTask(InAmountOfWork, InDefaultMessage));
    SlowTask = MoveTemp(TempTask);
    SlowTask->Initialize();

    if (ShowProgressDialog)
    {
        SlowTask->Visibility = ESlowTaskVisibility::ForceVisible;
        SlowTask->MakeDialog(bShowCancelButton, true);
    }
}

bool FSimReadySlowTask::IsProgressCancelled()
{
    if (SlowTask.IsValid())
    {
        return SlowTask->ShouldCancel();
    }

    return GWarn->ReceivedUserCancel();
}

void FSimReadySlowTask::UpdateProgress(float ExpectedWorkThisFrame, const FText& Text)
{
    if (SlowTask.IsValid())
    {
        SlowTask->EnterProgressFrame(ExpectedWorkThisFrame, Text);
    }
}

void FSimReadySlowTask::EndProgress()
{
    if (SlowTask.IsValid())
    {
        SlowTask->Destroy();
        SlowTask.Reset();
    }
}
