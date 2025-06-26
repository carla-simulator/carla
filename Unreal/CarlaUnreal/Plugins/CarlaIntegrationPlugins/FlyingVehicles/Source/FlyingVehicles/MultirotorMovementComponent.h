#pragma once

#include "RotorPhysics.h"
#include "RotorSetup.h"

#include "UObject/Object.h"

#include "MultirotorMovementComponent.generated.h"

class AMultirotorPawn;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class FLYINGVEHICLES_API UMultirotorMovementComponent : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Category="Rotor Setup", EditAnywhere, BlueprintReadWrite)
    TArray<FRotorSetup> RotorSetups;

    void Initialize(AMultirotorPawn* Owner);

    void Shutdown();

    void UpdatePhysics();

    void SetRotorInput(const TArray<float>& Throttle);

    float GetRotorSpeed(size_t Index) const;

private:

    UPROPERTY()
    AMultirotorPawn* Multirotor;

    UPROPERTY()
    TArray<URotorPhysics*> Rotors;
};
