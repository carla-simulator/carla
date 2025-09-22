// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TriggerBoxActor.generated.h"

UCLASS()
class CARLA_API ATriggerBoxActor : public ATriggerBox
{
    GENERATED_BODY()

public:
    ATriggerBoxActor();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString TrafficLightName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* WireframeMesh;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};