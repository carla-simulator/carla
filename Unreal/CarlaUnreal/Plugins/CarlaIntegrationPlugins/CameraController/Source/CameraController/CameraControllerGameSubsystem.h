#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "CameraControllerGameSubsystem.generated.h"

UCLASS()
class UCameraControllerSubsystem : public UGameInstanceSubsystem
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