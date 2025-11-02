// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyNotificationHelper.h"
#include "Async/Async.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "SimReadyRuntimePrivate.h"

TSharedPtr<SNotificationItem> FSimReadyNotificationHelper::ShowInstantNotification(const FString & Text, SNotificationItem::ECompletionState CompletionState,
    float ExpireDuration, float FadeOutDuration, bool bDismissButton, FSimpleDelegate OnDismiss)
{
    TSharedPtr<SNotificationItem> NotificationItem = nullptr;

    FNotificationInfo Info(FText::GetEmpty());
    if (bDismissButton)
    {
        Info.bFireAndForget = false;
        Info.ExpireDuration = ExpireDuration;
        Info.FadeOutDuration = FadeOutDuration;
        Info.bUseSuccessFailIcons = false;
        Info.bUseLargeFont = false;
        Info.ButtonDetails.Add(FNotificationButtonInfo(
            FText::FromString("Dismiss"),
            FText(),
            OnDismiss,
            CompletionState
        ));

        NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
        if(NotificationItem.IsValid())
        {
            NotificationItem->SetCompletionState(CompletionState);
            NotificationItem->SetText(FText::FromString(Text));
        }
    }
    else
    {
        Info.bFireAndForget = true;
        Info.ExpireDuration = ExpireDuration;
        Info.FadeOutDuration = FadeOutDuration;
        Info.bUseLargeFont = false;

        NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
        if(NotificationItem.IsValid())
        {
            NotificationItem->SetCompletionState(CompletionState);
            NotificationItem->SetText(FText::FromString(Text));
        }
    }

    return NotificationItem;
}

void FSimReadyNotificationHelper::NotifyError(const FString& Error)
{
#if WITH_EDITOR
    const FText NotificationErrorText = FText::FromString(Error);
    FNotificationInfo Info(NotificationErrorText);
    Info.ExpireDuration = 10.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
#endif
    UE_LOG(LogSimReady, Error, TEXT("%s"), *Error);
}

TWeakPtr<SNotificationItem> FSimReadyNotificationHelper::NotifySelection(const FString& Message, const FString& OKMessage, const FString& OKTip, FSimpleDelegate OKDelegate, const FString& CancelMessage, const FString& CancelTip, FSimpleDelegate CancelDelegate)
{
    FNotificationInfo Notification(FText::FromString(Message));
    Notification.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString(OKMessage), FText::FromString(OKTip), OKDelegate));
    Notification.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString(CancelMessage), FText::FromString(CancelTip), CancelDelegate));
    Notification.bFireAndForget = false;
    Notification.bUseLargeFont = false;
    Notification.bUseSuccessFailIcons = false;
    Notification.bUseThrobber = false;
    Notification.FadeOutDuration = 0.0f;
    Notification.ExpireDuration = 0.0f;

    TWeakPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Notification);
    NotificationItem.Pin()->SetCompletionState(SNotificationItem::CS_Pending);

    return NotificationItem;
}