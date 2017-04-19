// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

// This file is included before any other file in every compile unit within the
// plugin.
#pragma once

#include "ModuleManager.h"

#include "NonCopyable.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCarla, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaServer, Log, All);

// Options to compile with extra debug options.
#ifdef WITH_EDITOR
// #define CARLA_ROAD_GENERATOR_EXTRA_LOG /// @todo #1 Crashes in Linux.
// #define CARLA_SERVER_EXTRA_LOG
// #define CARLA_TAGGER_EXTRA_LOG
#endif // WITH_EDITOR

class FCarlaModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
