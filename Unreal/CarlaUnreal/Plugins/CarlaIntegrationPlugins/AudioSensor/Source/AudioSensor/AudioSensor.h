
#pragma once

#include "Engine.h"
#include "Logging/LogMacros.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAudioSensor, Log, All);

class FAudioSensor : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

