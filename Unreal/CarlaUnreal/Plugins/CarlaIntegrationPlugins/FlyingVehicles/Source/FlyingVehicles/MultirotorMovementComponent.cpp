#include "MultirotorMovementComponent.h"

#include "MultirotorBlueprintHelpers.h"
#include "MultirotorPawn.h"

void UMultirotorMovementComponent::Initialize(AMultirotorPawn* Owner)
{

    check(RotorSetups.Num() > 0);

    Multirotor = Owner;

    auto RootComponent = Cast<UPrimitiveComponent>(Multirotor->GetRootComponent());
    check(RootComponent);

    for (size_t i = 0; i < RotorSetups.Num(); ++i) {
        UStaticMeshComponent* RotorMesh = UMultirotorBlueprintHelpers::GetActorComponent<UStaticMeshComponent>(Multirotor, TEXT("Prop") + FString::FromInt(i));
        if (RotorMesh) {
            auto ComponentName = TEXT("RotorComponent") + FString::FromInt(i);
            URotorPhysics *Rotor = NewObject<URotorPhysics>(this, *ComponentName);
            Rotors.Add(Rotor);
            auto Location = RotorMesh->GetRelativeLocation();
            Rotor->Initialize(RootComponent, RotorMesh, Location, &RotorSetups[i]);

        }
        else {
            UE_LOG(LogFlyingVehicles, Warning, TEXT("Error: Could not find Prop%d element"), i);
        }
    }
}

void UMultirotorMovementComponent::Shutdown()
{
    Multirotor = nullptr;
    Rotors.Empty();
}

void UMultirotorMovementComponent::SetRotorInput(const TArray<float>& Throttle)
{
    if (Throttle.Num() != Rotors.Num())
    {
        UE_LOG(LogFlyingVehicles, Warning, TEXT("Got rotor command with %d inputs (expected %d)"), Throttle.Num(), Rotors.Num());
        return;
    }

    for(size_t i = 0; i < Rotors.Num(); ++i)
    {
        Rotors[i]->SetThrottle(Throttle[i]);
    }
}

float UMultirotorMovementComponent::GetRotorSpeed(size_t Index) const
{
    if (Index >= Rotors.Num()) {
        return 0;
    }

    return Rotors[Index]->GetRotorSpeed();
}

void UMultirotorMovementComponent::UpdatePhysics()
{
    for (auto Rotor : Rotors) {
        if (IsValid(Rotor)) {
            Rotor->UpdatePhysics();
        }
        else {
            UE_LOG(LogFlyingVehicles, Warning, TEXT("Can't update rotor physics, rotor component is not valid"));
        }
    }
}
