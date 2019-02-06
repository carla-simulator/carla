// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CommandletPluginPrivate.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"


DEFINE_LOG_CATEGORY(LogCommandletPlugin);


/**
 * Implements the CommandletPlugin module.
 */
class FCommandletPluginModule
	: public IModuleInterface
{
public:

	//~ IModuleInterface interface

	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};


IMPLEMENT_MODULE(FCommandletPluginModule, CommandletPlugin);
