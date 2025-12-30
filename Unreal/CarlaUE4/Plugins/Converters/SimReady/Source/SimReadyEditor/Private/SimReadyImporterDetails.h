// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"


class FSimReadyImporterDetails : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization interface */
    virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

private:
    FText GetInfoContentName() const;
    FText GetSelectContentName() const;
    FText GetInfoPathText() const;
    FText GetSelectPathText() const;
    FText GetSelectButtonTipText() const;

    void OnSelectPathTextCommitted(const FText& Text, ETextCommit::Type CommitType);
    FReply OnSelectPathClicked();

};
