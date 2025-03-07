#include "MultirotorPawn.h"

#include "Components/StaticMeshComponent.h"
#include "MultirotorMovementComponent.h"

#include "MultirotorBlueprintHelpers.h"

#define PI_CONSTANT 3.14159265358979

AMultirotorPawn::AMultirotorPawn()
{
    MultirotorMovementComponent = CreateDefaultSubobject<UMultirotorMovementComponent>(TEXT("MultirotorMovementComponent"));
}

void AMultirotorPawn::BeginPlay()
{
    Super::BeginPlay();

    MultirotorMovementComponent->Initialize(this);

    for (auto i = 0; i < 4; ++i) {
        rotating_movements_.Add(UMultirotorBlueprintHelpers::GetActorComponent<URotatingMovementComponent>(this, TEXT("Rotation") + FString::FromInt(i)));
    }
}

void AMultirotorPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    check(IsValid(MultirotorMovementComponent));

    MultirotorMovementComponent->UpdatePhysics();

    UpdateRotorSpeeds();

}

void AMultirotorPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    MultirotorMovementComponent->Shutdown();
    rotating_movements_.Empty();
}

FMultirotorControl AMultirotorPawn::GetMultirotorControl() const
{
    FMultirotorControl Control;
    TArray<float> Throttle;
    for(size_t i = 0; i < rotating_movements_.Num(); ++i)
    {
        Throttle.Add(MultirotorMovementComponent->GetRotorSpeed(i));
    }
    Control.Throttle = Throttle;
    return Control;
}

FMultirotorPhysicsControl AMultirotorPawn::GetMultirotorPhysicsControl() const
{
    FMultirotorPhysicsControl Control;
    TArray<FRotorSetup> Rotors = MultirotorMovementComponent->RotorSetups;
    Control.Rotors = Rotors;
    return Control;
}


void AMultirotorPawn::ApplyMultirotorControl(const FMultirotorControl& Control)
{
    MultirotorMovementComponent->SetRotorInput(Control.Throttle);
}

void AMultirotorPawn::ApplyMultirotorPhysicsControl(const FMultirotorPhysicsControl& Control)
{
    (void)Control;
}

void AMultirotorPawn::UpdateRotorSpeeds()
{
    for (auto rotor_index = 0; rotor_index < 4; ++rotor_index) {
        auto comp = rotating_movements_[rotor_index];
        if (comp != nullptr) {
            float rotor_speed = MultirotorMovementComponent->GetRotorSpeed(rotor_index);
            comp->RotationRate.Yaw = rotor_speed * 180.0F / PI_CONSTANT * RotatorFactor;
        }
    }
}

void AMultirotorPawn::SetSimulatePhysics(bool Enabled)
{
    (void)Enabled;
    return;
}

