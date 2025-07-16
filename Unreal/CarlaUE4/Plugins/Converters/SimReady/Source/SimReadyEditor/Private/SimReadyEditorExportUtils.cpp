// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyEditorExportUtils.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"

#include "SimReadyStageActor.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyUSDHelper.h"
#include "SimReadyPxr.h"
#include "SimReadyAssetExportHelper.h"
#include "SimReadySettings.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyPathHelper.h"

#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"


bool FSimReadyEditorExportUtils::AreObjectsFromTheSameClass(const TArray<class UObject*>& Objects)
{
    if (Objects.Num() < 2)
    {
        return true;
    }

    auto StaticClassName = Objects[0]->GetClass()->GetName();
    bool bSameTypeAssets = true;
    for (int32 ObjectIndex = 1; ObjectIndex < Objects.Num(); ObjectIndex++)
    {
        if (Objects[ObjectIndex]->GetClass()->GetName() != StaticClassName)
        {
            bSameTypeAssets = false;
        }
    }

    return bSameTypeAssets;
}
