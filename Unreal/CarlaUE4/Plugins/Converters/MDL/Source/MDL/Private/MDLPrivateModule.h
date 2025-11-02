// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if WITH_MDL_SDK
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "MDLModule.h"
#include "MDLSettings.h"
#endif

class FMDLModule : public IMDLModule
{
public:
#if WITH_MDL_SDK
    void*													MDLSDKHandle = nullptr;
    mi::base::Handle<mi::neuraylib::INeuray>				Neuray;

    mi::base::Handle<mi::neuraylib::IImage_api>				ImageAPI;
    mi::base::Handle<mi::neuraylib::IMdl_impexp_api>		MDLImpexpAPI;
    mi::base::Handle<mi::neuraylib::IMdl_distiller_api>		MDLDistillerAPI;
    mi::base::Handle<mi::neuraylib::IMdl_factory>			MDLFactory;
    mi::base::Handle<mi::neuraylib::IScope>					MDLScope;
    mi::base::Handle<mi::neuraylib::ITransaction>			Transaction;
    mi::base::Handle<mi::neuraylib::IMdl_configuration>		MDLConfiguration;

    FString													MDLUserPath;
    FString													WrapperMaterialPath;
    MaterialData											WrapperMaterialData;
    ModuleData												WrapperModuleData;
    TSharedPtr<class IDynamicMipsLoader>					DynamicMipsLoader;

    virtual void CommitAndCreateTransaction() override;
    virtual void AddModulePath(const FString& Path, bool IgnoreCheck = false) override;
    virtual void RemoveModulePath(const FString& Path) override;
    virtual void SetExternalFileReader(TSharedPtr<class IMDLExternalReader> FileReader) override;
    virtual void RegisterDynamicMipsLoader(class IDynamicMipsLoader* Loader) override;
    virtual TSharedPtr<class IDynamicMipsLoader> GetDynamicMipsLoader() override;
    virtual TSharedPtr<class IMDLMaterialImporter> CreateDefaultImporter() override;
    virtual int32 LoadMdlModule(const FString& ModuleName) override;
    virtual mi::base::Handle<const mi::neuraylib::IModule> GetLoadedMdlModule(const FString& ModuleName) override;
    virtual void RemoveMdlModule(const FString& ModuleName) override;

    virtual mi::base::Handle<mi::neuraylib::ITransaction> GetTransaction() override { return Transaction; }
    virtual mi::base::Handle<mi::neuraylib::IMdl_distiller_api> GetDistiller() override { return MDLDistillerAPI; }
    virtual mi::base::Handle<mi::neuraylib::IMdl_factory> GetFactory() override { return MDLFactory; }
    virtual mi::base::Handle<mi::neuraylib::IMdl_configuration> GetConfiguration() override { return MDLConfiguration; }


#endif // WITH_MDL_SDK

private:
    void StartupModule() override;
    void ShutdownModule() override;
};

