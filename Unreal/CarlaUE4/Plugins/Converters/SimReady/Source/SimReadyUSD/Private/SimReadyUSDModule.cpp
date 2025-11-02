// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUSDModule.h"
#include "SimReadyUSDLog.h"
#include "EditorModeRegistry.h"
#include "SimReadyUSDTranslator.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogSimReadyUsd);

class FSimReadyUSDModule : public ISimReadyUSDModule
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual void RegisterUSDTranslator(ISimReadyUSDTranslator* UsdTranslator) override;
    virtual bool HasUSDTranslator() override;
    virtual TArray<TSharedPtr<ISimReadyUSDTranslator>> GetUSDTranslators() const override;
    void BumpUSDLog();

private:
    TArray<TSharedPtr<ISimReadyUSDTranslator>>	ExternalUsdTranslators;
};

// Do not replace new and delete. It conflicts with USD.
#undef REPLACEMENT_OPERATOR_NEW_AND_DELETE
#define REPLACEMENT_OPERATOR_NEW_AND_DELETE

IMPLEMENT_MODULE(FSimReadyUSDModule, SimReadyUSD)

void FSimReadyUSDModule::StartupModule()
{

    FString BasePluginPath = FPaths::ConvertRelativePathToFull(FPaths::EnginePluginsDir() + FString(TEXT("Importers/USDImporter")));
#if PLATFORM_WINDOWS
    BasePluginPath /= TEXT("Resources/UsdResources/Windows/plugins");
#elif PLATFORM_LINUX
    BasePluginPath /= ("Resources/UsdResources/Linux/plugins");
#endif

    {
        std::vector< std::string > UsdPluginDirectories;

        UsdPluginDirectories.push_back(TCHAR_TO_UTF8(*BasePluginPath));

        pxr::PlugRegistry::GetInstance().RegisterPlugins(UsdPluginDirectories);
    }

#ifdef _WIN64
#ifdef _DEBUG
    const FString Config = "win64_debug";
#else
    const FString Config = "win64_release";
#endif
#endif

    // USD log
    FCoreDelegates::OnBeginFrame.AddLambda(
        [this]()
        {
            BumpUSDLog();
        }
    );
}

void FSimReadyUSDModule::ShutdownModule()
{
    ExternalUsdTranslators.Reset(0);
}

void FSimReadyUSDModule::RegisterUSDTranslator(ISimReadyUSDTranslator* UsdTranslator)
{
    ExternalUsdTranslators.Add(MakeShareable<ISimReadyUSDTranslator>(UsdTranslator));
}

bool FSimReadyUSDModule::HasUSDTranslator()
{
    return ExternalUsdTranslators.Num() > 0;
}

TArray<TSharedPtr<ISimReadyUSDTranslator>> FSimReadyUSDModule::GetUSDTranslators() const
{
    return ExternalUsdTranslators;
}

void FSimReadyUSDModule::BumpUSDLog()
{
    static pxr::TfErrorMark ErrorMark;

    for (auto Error : ErrorMark)
    {
        FString SourceFileName = Error.GetSourceFileName().c_str();
        FString SourceFunction = Error.GetSourceFunction().c_str();
        UE_LOG(LogSimReadyUsd, Warning, TEXT("USD Error in %s at line %d in file %s : %s"), *SourceFunction, Error.GetSourceLineNumber(), *SourceFileName, ANSI_TO_TCHAR(Error.GetCommentary().c_str()));
    }

    ErrorMark.Clear();
    ErrorMark.SetMark();
}