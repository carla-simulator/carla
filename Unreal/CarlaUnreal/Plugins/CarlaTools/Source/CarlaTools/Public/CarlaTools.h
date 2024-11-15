// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "EngineMinimal.h"
#include <util/ue-header-guard-end.h>


DECLARE_LOG_CATEGORY_EXTERN(LogCarlaTools, Log, All);

class FCarlaToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
