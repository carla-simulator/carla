#pragma once

#include "RotorSetup.h"

#include "RotorPhysics.generated.h"

UCLASS()
class FLYINGVEHICLES_API URotorPhysics : public UObject
{
    GENERATED_BODY()
public:

    URotorPhysics(const FObjectInitializer &ObjectInitializer);

    /// Initialize the physics component.
    void Initialize(UPrimitiveComponent* MeshComponent, UStaticMeshComponent* RotorMeshComponent, FVector MeshLocation, FRotorSetup* RotorSetup);

    /// Set the throttle setpoint.
    void SetThrottle(float Throttle);

    /// Get the current actual throttle value
    float GetThrottle() const;

    /// Get rotor rotation speed in rad/sec
    float GetRotorSpeed() const;

    /// Update the rotor's physics
    /// Using the given setpoint, update the rotor's physics.
    /// Sets forces and torques on the static mesh component.
    void UpdatePhysics();

private:
    void CalculateMaxThrust();

    UPrimitiveComponent* Mesh = nullptr;
    FRotorSetup* Setup = nullptr;
    UStaticMeshComponent* RotorMesh = nullptr;
    FVector Location;

    float ThrottleSetpoint = 0.0F;
    float CurrentThrottle = 0.0F;

    float MaxThrust;
    float MaxTorque;
    float MaxSpeedSquare;

    float CurrentSpeed; // radians/sec
};
