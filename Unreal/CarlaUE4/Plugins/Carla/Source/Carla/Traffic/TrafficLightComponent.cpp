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
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "carla/road/element/RoadInfoSignal.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/road/element/RoadInfoSignal.h>
#include <compiler/enable-ue4-macros.h>

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

        carla::road::element::Waypoint signal_waypoint;
        boost::optional<carla::road::element::Waypoint> opt_signal_waypoint = Map.GetWaypoint(
            RoadId, lane, SignalReference->GetS());
        if(opt_signal_waypoint){
          signal_waypoint = opt_signal_waypoint.get();
        }else{
          UE_LOG(LogCarla, Error, TEXT("signal_waypoint is not valid") );
          continue;
        }
        // Prevent adding the bounding box inside the intersection
        if (Map.IsJunction(RoadId)) {
          auto predecessors = Map.GetPredecessors(signal_waypoint);
          if (predecessors.size() == 1) {
            auto predecessor = predecessors.front();
            if (!Map.IsJunction(predecessor.road_id)) {
              signal_waypoint = predecessor;
            }
          }
        }

        if(Map.GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        // Get 50% of the half size of the width of the lane
        float BoxWidth = static_cast<float>(
            0.5f*Map.GetLaneWidth(signal_waypoint)*0.5);
        float BoxLength = 1.5f;
        float BoxHeight = 1.0f;

        // Prevent a situation where the road width is 0,
        // this could happen in a lane that is just appearing
        BoxWidth = std::max(0.01f, BoxWidth);
        // Get min and max
        double LaneLength = Map.GetLane(signal_waypoint).GetLength();
        double LaneDistance = Map.GetLane(signal_waypoint).GetDistance();
        // Safe distance to avoid overlapping the bounding box with the intersection
        float AdditionalDistance = 1.5f;
        if(lane < 0)
        {
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s - (BoxLength + AdditionalDistance),
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        else
        {
          signal_waypoint.s = FMath::Clamp(signal_waypoint.s + (BoxLength + AdditionalDistance),
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        FTransform BoxTransform = Map.ComputeTransform(signal_waypoint);
        ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
        if (LargeMapManager)
        {
          BoxTransform = LargeMapManager->GlobalToLocalTransform(BoxTransform);
        }
        GenerateTrafficLightBox(BoxTransform, FVector(100*BoxLength, 100*BoxWidth, 100*BoxHeight));
      }
    }
  }
}

void UTrafficLightComponent::GenerateTrafficLightBox(const FTransform BoxTransform,
    const FVector BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UTrafficLightComponent::OnBeginOverlapTriggerBox);
  BoxComponent->OnComponentEndOverlap.AddDynamic(this, &UTrafficLightComponent::OnEndOverlapTriggerBox);
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
    }
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

void UTrafficLightComponent::OnBeginOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
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
      Vehicles.Add(VehicleController);
      VehicleController->SetTrafficLight(Cast<ATrafficLightBase>(GetOwner()));
    }
  }
}

void UTrafficLightComponent::OnEndOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    AWheeledVehicleAIController* VehicleController =
        Cast<AWheeledVehicleAIController>(Vehicle->GetController());
    if (VehicleController)
    {
      VehicleController->SetTrafficLightState(ETrafficLightState::Green);
      VehicleController->SetTrafficLight(nullptr);
      Vehicles.Remove(VehicleController);
    }
  }
}

void UTrafficLightComponent::SetController(UTrafficLightController* Controller)
{
  TrafficLightController = Controller;
}
