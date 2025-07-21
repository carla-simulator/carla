// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once 

#include "Widgets/SWindow.h"
#include "SimReadyImporterUI.h"
#include "SimReadyImportSettings.h"

class SSimReadyImporterDialog : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SSimReadyImporterDialog)
    {}

    SLATE_ARGUMENT(USimReadyImporterUI*, ImporterUI)

    SLATE_END_ARGS()

    //~SSimReadyImporterDialog();

    /** 
     * Constructs a new importer dialog. 
     *
     * @param InArgs Slate arguments. 
     */
    void Construct(const FArguments& InArgs);


    // Show the dialog
    static void ShowDialog(ESimReadyImporterType DialogType, const FString& FullPath);

    static TWeakPtr<class SSimReadyImporterDialog> DialogPtr;

private:
    FReply OkClicked();
    FReply CancelClicked();
    bool IsImporterReady() const;

private:
    void ConstructDialog();

    USimReadyImporterUI*	ImporterUI;
    TSharedPtr<class IDetailsView> DetailsView;
};