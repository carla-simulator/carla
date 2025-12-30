// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if WITH_MDL_SDK
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "MDLDependencies.h"
#endif

class MDL_API IMDLModule : public IModuleInterface
{
public:
    virtual void CommitAndCreateTransaction() = 0;
    virtual void AddModulePath(const FString& Path, bool IgnoreCheck = false) = 0;
    virtual void RemoveModulePath(const FString& Path) = 0;
    virtual void SetExternalFileReader(TSharedPtr<class IMDLExternalReader> FileReader) = 0;
    virtual void RegisterDynamicMipsLoader(class IDynamicMipsLoader* Loader) = 0;
    virtual TSharedPtr<class IDynamicMipsLoader> GetDynamicMipsLoader() = 0;
    virtual TSharedPtr<class IMDLMaterialImporter> CreateDefaultImporter() = 0;
    virtual int32 LoadMdlModule(const FString& ModuleName) = 0;
    virtual mi::base::Handle<const mi::neuraylib::IModule> GetLoadedMdlModule(const FString& ModuleName) = 0;
    virtual void RemoveMdlModule(const FString& ModuleName) = 0;

    virtual mi::base::Handle<mi::neuraylib::ITransaction> GetTransaction() = 0;
    virtual mi::base::Handle<mi::neuraylib::IMdl_distiller_api> GetDistiller() = 0;
    virtual mi::base::Handle<mi::neuraylib::IMdl_factory> GetFactory() = 0;
    virtual mi::base::Handle<mi::neuraylib::IMdl_configuration> GetConfiguration() = 0;
};

