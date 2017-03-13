// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaServerController.h"

#include "DrawDebugHelpers.h"
#include "Engine.h"

void ACarlaServerController::Tick(float DeltaTime)
{
  Super::PlayerTick(DeltaTime);

  if (!IsPossessingAVehicle())
    return;

  auto Speed = GetVehicleForwardSpeed();
  auto Location = GetVehicleLocation();
  auto Orientation = GetVehicleOrientation();
  auto LineEndPoint = Location + std::min(1.0f, Speed) * 1000.0f * Orientation;
  auto Color = FColor(255u, 0u, 0u);
  DrawDebugPoint(GetWorld(), Location, 10.0f, Color);
  DrawDebugLine(GetWorld(), Location, LineEndPoint, Color);
}

 void ACarlaServerController::OnCollisionEvent(
     AActor* Actor,
     AActor* OtherActor,
     FVector NormalImpulse,
     const FHitResult& Hit)
 {
   if(GEngine)
     GEngine->AddOnScreenDebugMessage(
         -1,
         15.0f,
         FColor::Yellow,
         FString::Printf(TEXT("We hit a %s"), *OtherActor->GetName()));
 }
