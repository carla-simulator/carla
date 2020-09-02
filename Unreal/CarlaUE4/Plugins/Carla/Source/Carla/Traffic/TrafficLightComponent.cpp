// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "TrafficLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TrafficLightController.h"
#include "TrafficLightGroup.h"
#include "TrafficLightInterface.h"
#include "TrafficLightManager.h"

UTrafficLightComponent::UTrafficLightComponent()
  : Super()
{
}

void UTrafficLightComponent::InitializeSign(const carla::road::Map &Map)
{
  const double epsilon = 0.00001;

  auto References = GetAllReferencesToThisSignal(Map);

  for (auto& Reference : References)
  {
    auto RoadId = Reference.first;
    const auto* SignalReference = Reference.second;
    for(auto &validity : SignalReference->GetValidities())
    {
      for(auto lane : carla::geom::Math::GenerateRange(validity._from_lane, validity._to_lane))
      {
        if(lane == 0)
          continue;

        auto signal_waypoint = Map.GetWaypoint(
            RoadId, lane, SignalReference->GetS()).get();

        if(Map.GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        // Get 90% of the half size of the width of the lane
        float BoxSize = static_cast<float>(
            0.9f*Map.GetLaneWidth(signal_waypoint)*0.5);
        // Prevent a situation where the road width is 0,
        // this could happen in a lane that is just appearing
        BoxSize = std::max(0.01f, BoxSize);
        // Get min and max
        double LaneLength = Map.GetLane(signal_waypoint).GetLength();
        double LaneDistance = Map.GetLane(signal_waypoint).GetDistance();
        if(lane < 0)
        {
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s - BoxSize,
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        else
        {
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s + BoxSize,
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        float UnrealBoxSize = 100*BoxSize;
        GenerateTrafficLightBox(Map.ComputeTransform(signal_waypoint), UnrealBoxSize);
      }
    }
  }
}

void UTrafficLightComponent::GenerateTrafficLightBox(const FTransform BoxTransform,
    float BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UTrafficLightComponent::OnOverlapTriggerBox);
  AddEffectTriggerVolume(BoxComponent);
}

void UTrafficLightComponent::SetLightState(ETrafficLightState NewState)
{
  LightState = NewState;
  LightChangeDispatcher.Broadcast();
  if (GetOwner()->Implements<UTrafficLightInterface>())
  {
    ITrafficLightInterface::Execute_LightChanged(GetOwner(), LightState);
  }
  ATrafficLightBase* OldTrafficLight = Cast<ATrafficLightBase>(GetOwner());
  if (OldTrafficLight)
  {
    OldTrafficLight->LightChangedCompatibility(NewState);
  }

  for (auto Controller : Vehicles)
  {
    if (Controller != nullptr)
    {
      Controller->SetTrafficLightState(LightState);
      if (LightState == ETrafficLightState::Green)
      {
        Controller->SetTrafficLight(nullptr);
      }
    }
  }
  if (LightState == ETrafficLightState::Green)
  {
    Vehicles.Empty();
  }
}

ETrafficLightState UTrafficLightComponent::GetLightState() const
{
  return LightState;
}

void UTrafficLightComponent::SetFrozenGroup(bool InFreeze)
{
  if (GetGroup())
  {
    GetGroup()->SetFrozenGroup(InFreeze);
  }
}

ATrafficLightGroup* UTrafficLightComponent::GetGroup()
{
  return TrafficLightController->GetGroup();
}

const ATrafficLightGroup* UTrafficLightComponent::GetGroup() const
{
  return TrafficLightController->GetGroup();
}

UTrafficLightController* UTrafficLightComponent::GetController()
{
  return TrafficLightController;
}

const UTrafficLightController* UTrafficLightComponent::GetController() const
{
  return TrafficLightController;
}

void UTrafficLightComponent::OnOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    AWheeledVehicleAIController* VehicleController =
        Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (VehicleController)
    {
      VehicleController->SetTrafficLightState(LightState);
      if (LightState != ETrafficLightState::Green)
      {
        Vehicles.Add(VehicleController);
        VehicleController->SetTrafficLight(
            Cast<ATrafficLightBase>(GetOwner()));
      }
    }
  }
}

void UTrafficLightComponent::SetController(UTrafficLightController* Controller)
{
  TrafficLightController = Controller;
}
