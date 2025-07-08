// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMessageLogContext.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Modules/ModuleManager.h"

void FSimReadyMessageLogContext::AddMessage(EMessageSeverity::Type MessageSeverity, FText Message)
{
    const TCHAR* LogTitle = TEXT("SimReady");
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
    TSharedPtr<class IMessageLogListing> LogListing = MessageLogModule.GetLogListing(LogTitle);

    LogListing->AddMessage(FTokenizedMessage::Create(MessageSeverity, Message));
}

void FSimReadyMessageLogContext::AddMessage(const TSharedRef< class FTokenizedMessage >& Message)
{
    const TCHAR* LogTitle = TEXT("SimReady");
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
    TSharedPtr<class IMessageLogListing> LogListing = MessageLogModule.GetLogListing(LogTitle);

    LogListing->AddMessage(Message);
}

void FSimReadyMessageLogContext::ClearMessages()
{
    const TCHAR* LogTitle = TEXT("SimReady");
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
    TSharedPtr<class IMessageLogListing> LogListing = MessageLogModule.GetLogListing(LogTitle);
    LogListing->ClearMessages();
}

void FSimReadyMessageLogContext::DisplayMessages()
{
    //Always clear the old message after an import or re-import
    const TCHAR* LogTitle = TEXT("SimReady");
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
    TSharedPtr<class IMessageLogListing> LogListing = MessageLogModule.GetLogListing(LogTitle);
    LogListing->SetLabel(FText::FromString(LogTitle));
    if (LogListing->NumMessages(EMessageSeverity::Info) > 0)
    {
        MessageLogModule.OpenMessageLog(LogTitle);
    }
}