// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

class SIMREADYRUNTIME_API FSimReadyNotificationHelper
{
public:
    static TSharedPtr<class SNotificationItem> ShowInstantNotification(const FString& Text, SNotificationItem::ECompletionState CompletionState = SNotificationItem::CS_Fail,
        float ExpireDuration = 2.f, float FadeOutDuration = 2.f, bool bDismissButton = false, FSimpleDelegate OnDismiss = nullptr);

    static void NotifyError(const FString& Error);

    static TWeakPtr<class SNotificationItem> NotifySelection(const FString& Message, const FString& OKMessage, const FString& OKTip, FSimpleDelegate OKDelegate, const FString& CancelMessage, const FString& CancelTip, FSimpleDelegate CancelDelegate);

};