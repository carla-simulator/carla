// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

// This file is included before any other file in every compile unit within the
// plugin.
#pragma once


#include "Util/NonCopyable.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCarla, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaServer, Log, All);

// Options to compile with extra debug log.
#if WITH_EDITOR
// #define CARLA_AI_VEHICLES_EXTRA_LOG
// #define CARLA_AI_WALKERS_EXTRA_LOG
// #define CARLA_ROAD_GENERATOR_EXTRA_LOG
// #define CARLA_SERVER_EXTRA_LOG
// #define CARLA_TAGGER_EXTRA_LOG
// #define CARLA_WEATHER_EXTRA_LOG
#endif // WITH_EDITOR

class FCarlaModule : public IModuleInterface
{
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleSettingsSaved();


public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
