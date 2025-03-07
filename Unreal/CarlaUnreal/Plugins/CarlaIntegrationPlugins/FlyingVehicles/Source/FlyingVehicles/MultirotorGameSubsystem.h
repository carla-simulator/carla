#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "MultirotorGameSubsystem.generated.h"

UCLASS()
class UMultirotorSubsystem : public UGameInstanceSubsystem
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