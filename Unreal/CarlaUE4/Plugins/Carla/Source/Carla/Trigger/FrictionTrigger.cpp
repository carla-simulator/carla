// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "FrictionTrigger.h"
#include "Vehicle/CarlaWheeledVehicle.h"

void AFrictionTrigger::UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle != nullptr)
    Vehicle->SetWheelsFrictionScale(NewFriction);
}

void AFrictionTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{

  // Save original friction of the wheels
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if(Vehicle == nullptr)
    return;

  OldFrictionValues = Vehicle->GetWheelsFrictionScale();
  TArray<float> TriggerFriction = {Friction, Friction, Friction, Friction};
  UpdateWheelsFriction(OtherActor, TriggerFriction);
}

void AFrictionTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle == nullptr)
    return;

  // Set Back Default Friction Value
  UpdateWheelsFriction(OtherActor, OldFrictionValues);

  TArray<float> CurrFriction = Vehicle->GetWheelsFrictionScale();
}

void AFrictionTrigger::SetAttributes(const FActorDescription& Description)
{
  // Retrieve Friction
  float Friction = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("friction",
      Description.Variations,
      3.5f);
  SetFriction(Friction);
}
