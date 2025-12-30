// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyAsset.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/SecureHash.h"
#include "Misc/Guid.h"
#include "AssetRegistryModule.h"
#include "Interfaces/Interface_AssetUserData.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UObjectHash.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#if WITH_EDITOR
#include "FileHelpers.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Editor/EditorEngine.h"
#include "Settings/EditorLoadingSavingSettings.h"
#endif
#include "SimReadyRuntimeModule.h"
#include "SimReadyRuntimePrivate.h"
#include "SimReadyPathHelper.h"
#include "SimReadyNotificationHelper.h"

bool SIMREADYRUNTIME_API GListingAssetsForEditor = false;

bool USimReadyAsset::bSavingCache = false;

USimReadyAsset::USimReadyAsset()
{
}

USimReadyAsset::~USimReadyAsset()
{
}

void USimReadyAsset::HandleMaxPathHyperlinkNavigate()
{
    FPlatformProcess::LaunchURL(TEXT("https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd#enable-long-paths-in-windows-10-version-1607-and-later"), nullptr, nullptr);
}

void USimReadyAsset::ShowMaxPathNotification(const FString& InText)
{
    const FText NotificationText = FText::FromString(InText);
    FNotificationInfo Notification(NotificationText);
    Notification.bFireAndForget = true;
    Notification.bUseLargeFont = false;
    Notification.bUseSuccessFailIcons = false;
    Notification.bUseThrobber = false;
    Notification.ExpireDuration = 10.0f;
    Notification.Hyperlink = FSimpleDelegate::CreateStatic(&USimReadyAsset::HandleMaxPathHyperlinkNavigate);
    Notification.HyperlinkText = FText::FromString(TEXT("Show Solution"));
    FSlateNotificationManager::Get().AddNotification(Notification);
}

USimReadyAsset * USimReadyAsset::GetSimReadyAsset(UObject& Object)
{
    // This class is mostly stripped of its usefulness, TODO remove
    return nullptr;

    auto SimReadyAsset = Cast<USimReadyAsset>(&Object);
    if(SimReadyAsset)
    {
        return SimReadyAsset;
    }

    SimReadyAsset = Cast<USimReadyAsset>(Object.GetOuter());
    if(SimReadyAsset)
    {
        return SimReadyAsset;
    }

    return StaticCast<USimReadyAsset*>(FindObjectWithOuter(Object.GetOuter(), StaticClass()));
}