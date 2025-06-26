#include "RotorPhysics.h"

#include "Math/UnrealMathUtility.h"

#define PI_CONSTANT 3.14159265358979

URotorPhysics::URotorPhysics(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{

}
void URotorPhysics::Initialize(UPrimitiveComponent* RootComponent, UStaticMeshComponent* RotorMeshComponent, FVector MeshLocation, FRotorSetup* RotorSetup)
{
    Mesh = RootComponent;
    RotorMesh = RotorMeshComponent;
    Setup = RotorSetup;
    Location = MeshLocation;

    CalculateMaxThrust();
}

void URotorPhysics::SetThrottle(float Throttle)
{
    ThrottleSetpoint = FMath::Clamp(Throttle, 0.0F, 1.0F);
}

float URotorPhysics::GetThrottle() const
{
    return CurrentThrottle;
}

float URotorPhysics::GetRotorSpeed() const
{
    return CurrentSpeed;
}

void URotorPhysics::UpdatePhysics()
{
    CurrentThrottle = ThrottleSetpoint;

    const float AirDensityRatio = 1.0F;
    const float TurningDirection = Setup->Clockwise ? 1.0F : -1.0F;

    // Compute Thrusts
    const float Thrust = CurrentThrottle * MaxThrust;
    const float TorqueScaler = CurrentThrottle * MaxTorque * TurningDirection;
    CurrentSpeed = sqrt(CurrentThrottle * MaxSpeedSquare) * TurningDirection;

    const float Force = Thrust * AirDensityRatio * 100.0F;
    const float Torque = TorqueScaler * AirDensityRatio * 100.0F * 100.0F;

    Mesh->AddForceAtLocationLocal(FVector(0.0F, 0.0F, Force), Location);
    RotorMesh->AddTorqueInRadians(FVector(0.0F, 0.0F, Torque));
}

void URotorPhysics::CalculateMaxThrust()
{
    const float MaxRevolutionsPerSecond = Setup->MaxRPM / 60.0F;
    const float MaxSpeed = MaxRevolutionsPerSecond * 2.0F * PI_CONSTANT; // radians / sec
    MaxSpeedSquare = pow(MaxSpeed, 2.0f);

    const float AirDensity = 1.225f; //  kg/m^3
    const float NSquared = MaxRevolutionsPerSecond * MaxRevolutionsPerSecond;
    MaxThrust = Setup->ThrustCoefficient * AirDensity * NSquared * static_cast<float>(pow(Setup->PropellerDiameter, 4.0F));
    MaxTorque = Setup->TorqueCoefficient * AirDensity * NSquared * static_cast<float>(pow(Setup->PropellerDiameter, 5.0F)) / (2.0F * PI_CONSTANT);
}
