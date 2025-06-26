#pragma once

#include "GameFramework/RotatingMovementComponent.h"

#include "MultirotorControl.h"
#include "MultirotorPhysicsControl.h"

#include <vector>

#include "MultirotorPawn.generated.h"


class UMultirotorMovementComponent;

UCLASS()
class FLYINGVEHICLES_API AMultirotorPawn : public APawn
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debugging")
    float RotatorFactor = 1.0f;

    AMultirotorPawn();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    FMultirotorControl GetMultirotorControl() const;

    FMultirotorPhysicsControl GetMultirotorPhysicsControl() const;

    void ApplyMultirotorControl(const FMultirotorControl& Control);

    void ApplyMultirotorPhysicsControl(const FMultirotorPhysicsControl& Control);

    void SetSimulatePhysics(bool Enabled);

private:
    void UpdateRotorSpeeds();

    UPROPERTY()
    TArray<URotatingMovementComponent*> rotating_movements_;

    UPROPERTY(Category="Multirotors", Instanced, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UMultirotorMovementComponent * MultirotorMovementComponent = nullptr;
};

