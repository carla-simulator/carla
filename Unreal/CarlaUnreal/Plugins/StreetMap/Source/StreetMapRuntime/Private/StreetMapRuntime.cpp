// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMapRuntime.h"
#include "Modules/ModuleManager.h"


class FStreetMapRuntimeModule : public IModuleInterface
{

public:

	// IModuleInterface interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};


IMPLEMENT_MODULE( FStreetMapRuntimeModule, StreetMapRuntime )



void FStreetMapRuntimeModule::StartupModule()
{
}


void FStreetMapRuntimeModule::ShutdownModule()
{
}

