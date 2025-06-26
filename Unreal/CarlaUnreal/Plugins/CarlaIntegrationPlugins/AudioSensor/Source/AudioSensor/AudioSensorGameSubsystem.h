#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "AudioSensorGameSubsystem.generated.h"

UCLASS()
class UAudioSensorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    // Begin USubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // End USubsystem

private:
    // All my variables
};