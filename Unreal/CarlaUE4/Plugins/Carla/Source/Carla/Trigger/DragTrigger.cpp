// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DragTrigger.h"

void ADragTrigger::OnTriggerBeginOverlap(
  UPrimitiveComponent *OverlappedComp,
  AActor *OtherActor,
  UPrimitiveComponent *OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult &SweepResult)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if(Vehicle == nullptr)
    return;

  UPrimitiveComponent* MeshComponent = Vehicle->GetMesh();
  OldLinearDrag = MeshComponent->GetLinearDamping();
  MeshComponent->SetLinearDamping(LinearDrag);
}

void ADragTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if(Vehicle == nullptr)
    return;

  UPrimitiveComponent* MeshComponent = Vehicle->GetMesh();
  MeshComponent->SetLinearDamping(OldLinearDrag);
}

void ADragTrigger::SetAttributes(const FActorDescription& Description)
{
  // Retrieve Drag
  float Drag = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("drag",
      Description.Variations,
      0.01f);
  LinearDrag = Drag;
}
