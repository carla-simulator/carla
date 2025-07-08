// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLOutputLogger.h"

DEFINE_LOG_CATEGORY(LogMDLOutput);

#if WITH_MDL_SDK

// never check this in set to 1
#define MY_LOGGING_CHECK	0

FMDLOutputLogger GMDLOutputLogger;

void FMDLOutputLogger::message(mi::base::Message_severity level, const char* module_category, const mi::base::Message_details& message_details, const char* message)
{
    const FString MessageString(message);
    const FString CategoryString(module_category);

    switch (level)
    {
        case mi::base::MESSAGE_SEVERITY_FATAL:
#if MY_LOGGING_CHECK
            check(false);
#endif
            UE_LOG(LogMDLOutput, Fatal, TEXT("[%s] %s"), *CategoryString, *MessageString);
            break;
        case mi::base::MESSAGE_SEVERITY_ERROR:
#if MY_LOGGING_CHECK
            check(MessageString.EndsWith(TEXT("is still referenced while transaction 1 is committed.")) || MessageString.StartsWith(TEXT("Unable to resolve file path")));
#endif
            if (!MessageString.Contains(TEXT("No image plugin found to handle"))
            && !MessageString.Contains(TEXT("Failed to open image file")))
            {
                UE_LOG(LogMDLOutput, Error, TEXT("[%s] %s"), *CategoryString, *MessageString);
            }
            break;
        case mi::base::MESSAGE_SEVERITY_WARNING:
#if MY_LOGGING_CHECK
            check( MessageString.EndsWith(TEXT("does not have power of two dimensions and therefore no mipmaps will be generated"))
                || MessageString.StartsWith(TEXT("Failed to resolve"))
                || MessageString.Contains(TEXT("Unable to resolve file path"))
                || MessageString.Contains(TEXT("unused parameter")));
#endif
            UE_LOG(LogMDLOutput, Warning, TEXT("[%s] %s"), *CategoryString, *MessageString);
            break;
        case mi::base::MESSAGE_SEVERITY_INFO:
#if MY_LOGGING_CHECK
            check( MessageString.StartsWith(TEXT("Distilling time:"))
                || MessageString.StartsWith(TEXT("Distilling to target"))
                || MessageString.StartsWith(TEXT("Loaded"))
                || MessageString.StartsWith(TEXT("Loading image"))
                || MessageString.StartsWith(TEXT("Loading module"))
                || MessageString.StartsWith(TEXT("MDL SDK")));
#endif
            UE_LOG(LogMDLOutput, Display, TEXT("[%s] %s"), *CategoryString, *MessageString);
            break;
        case mi::base::MESSAGE_SEVERITY_VERBOSE:
#if MY_LOGGING_CHECK
            check(false);
#endif
            UE_LOG(LogMDLOutput, Verbose, TEXT("[%s] %s"), *CategoryString, *MessageString);
            break;
        case mi::base::MESSAGE_SEVERITY_DEBUG:
#if MY_LOGGING_CHECK
            check( MessageString.StartsWith(TEXT("... and mapped to texture"))
                || MessageString.StartsWith(TEXT("Attempting to load library"))
                || MessageString.StartsWith(TEXT("open file"))
                || MessageString.StartsWith(TEXT("Resolved")));
#endif
            UE_LOG(LogMDLOutput, VeryVerbose, TEXT("[%s] %s"), *CategoryString, *MessageString);
            break;
    }
}

#endif
