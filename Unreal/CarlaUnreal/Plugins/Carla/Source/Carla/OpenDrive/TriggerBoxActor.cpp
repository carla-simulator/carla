// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TriggerBoxActor.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Carla.h"

ATriggerBoxActor::ATriggerBoxActor()
{
    PrimaryActorTick.bCanEverTick = false;
    TrafficLightName = TEXT("Unknown");

    // Create wireframe mesh component for visibility
    WireframeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WireframeMesh"));
    WireframeMesh->SetupAttachment(GetCollisionComponent());

    // Configure collision component for triggers
    UBoxComponent* BoxComp = Cast<UBoxComponent>(GetCollisionComponent());
    if (BoxComp)
    {
        BoxComp->SetCollisionProfileName(TEXT("Trigger"));
        BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
        BoxComp->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
        BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        BoxComp->SetGenerateOverlapEvents(true);

        // Hide the collision component itself
        BoxComp->SetHiddenInGame(true);
        BoxComp->SetVisibility(false);
    }

    // Configure wireframe mesh for visibility
    if (WireframeMesh)
    {
        // Load wireframe cube mesh
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            WireframeMesh->SetStaticMesh(CubeMesh);

            // Make it wireframe only - no collision, just visual
            WireframeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            WireframeMesh->SetHiddenInGame(false);
            WireframeMesh->SetVisibility(true);

            // Set wireframe material
            UMaterialInterface* WireframeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EditorMaterials/WireframeMaterial.WireframeMaterial"));
            if (WireframeMaterial)
            {
                WireframeMesh->SetMaterial(0, WireframeMaterial);
            }
        }
    }
}

void ATriggerBoxActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    UBoxComponent* BoxComp = Cast<UBoxComponent>(GetCollisionComponent());
    if (BoxComp)
    {
        BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ATriggerBoxActor::OnOverlapBegin);
        BoxComp->OnComponentEndOverlap.AddDynamic(this, &ATriggerBoxActor::OnOverlapEnd);

        UE_LOG(LogCarla, Warning, TEXT("TriggerBoxActor: Overlap events bound for '%s'"), *TrafficLightName);
    }
}

void ATriggerBoxActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACarlaWheeledVehicle>())
    {
        UE_LOG(LogCarla, Warning, TEXT("🚗 %s has entered in %s triggerbox"), *OtherActor->GetName(), *TrafficLightName);
    }
    else if (OtherActor)
    {
        UE_LOG(LogCarla, Log, TEXT("👤 %s (%s) entered %s triggerbox"), *OtherActor->GetName(), *OtherActor->GetClass()->GetName(), *TrafficLightName);
    }
}

void ATriggerBoxActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACarlaWheeledVehicle>())
    {
        UE_LOG(LogCarla, Warning, TEXT("🚗 %s has exited from %s triggerbox"), *OtherActor->GetName(), *TrafficLightName);
    }
    else if (OtherActor)
    {
        UE_LOG(LogCarla, Log, TEXT("👤 %s (%s) exited %s triggerbox"), *OtherActor->GetName(), *OtherActor->GetClass()->GetName(), *TrafficLightName);
    }
}