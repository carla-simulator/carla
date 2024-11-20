// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "StopSignComponent.h"
#include "TrafficLightState.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <util/disable-ue4-macros.h>
#include <carla/road/element/RoadInfoSpeed.h>
#include <carla/road/element/RoadInfoSignal.h>
#include <util/enable-ue4-macros.h>

#include <queue>


void UStopSignComponent::InitializeSign(const carla::road::Map &Map)
{

  const double epsilon = 0.00001;

  auto References = GetAllReferencesToThisSignal(Map);

  for (auto& Reference : References)
  {
    auto RoadId = Reference.first;
    const auto* SignalReference = Reference.second;
    TSet<carla::road::RoadId> SignalPredecessors;
    // Stop box
    for(auto &validity : SignalReference->GetValidities())
    {
      for(auto lane : carla::geom::Math::GenerateRange(validity._from_lane, validity._to_lane))
      {
        if(lane == 0)
          continue;

        auto signal_waypoint = Map.GetWaypoint(
            RoadId, lane, SignalReference->GetS()).value();

        if(Map.GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        auto box_waypoint = signal_waypoint;
        // Prevent adding the bounding box inside the intersection
        if (Map.IsJunction(RoadId)) {
          auto predecessors = Map.GetPredecessors(box_waypoint);
          if (predecessors.size() == 1) {
            auto predecessor = predecessors.front();
            if (!Map.IsJunction(predecessor.road_id)) {
              box_waypoint = predecessor;
            }
          }
        }

        // Get 50% of the half size of the width of the lane
        float BoxWidth = static_cast<float>(
            0.5f*Map.GetLaneWidth(box_waypoint)*0.5);
        float BoxLength = 1.5f;
        float BoxHeight = 1.0f;
        // Prevent a situation where the road width is 0,
        // this could happen in a lane that is just appearing
        BoxWidth = std::max(0.01f, BoxWidth);
        // Get min and max
        double LaneLength = Map.GetLane(box_waypoint).GetLength();
        double LaneDistance = Map.GetLane(box_waypoint).GetDistance();
        // Safe distance to avoid overlapping the bounding box with the intersection
        float AdditionalDistance = 1.5f;
        if(lane < 0)
        {
          box_waypoint.s = FMath::Clamp(box_waypoint.s - (BoxLength + AdditionalDistance),
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        else
        {
          box_waypoint.s = FMath::Clamp(box_waypoint.s + (BoxLength + AdditionalDistance),
              LaneDistance + epsilon, LaneDistance + LaneLength - epsilon);
        }
        FTransform BoxTransform = Map.ComputeTransform(box_waypoint);
        ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
        if (LargeMapManager)
        {
          BoxTransform = LargeMapManager->GlobalToLocalTransform(BoxTransform);
        }
        GenerateStopBox(BoxTransform, FVector(100*BoxLength, 100*BoxWidth, 100*BoxHeight));

        auto Predecessors = Map.GetPredecessors(signal_waypoint);
        for(auto &Prev : Predecessors)
        {
          if(!SignalPredecessors.Contains(Prev.road_id))
          {
            SignalPredecessors.Add(Prev.road_id);
          }
        }
      }
    }

    //Check boxes
    if(Map.IsJunction(RoadId))
    {
      auto JuncId = Map.GetJunctionId(RoadId);
      const auto * Junction = Map.GetJunction(JuncId);
      if(Junction->RoadHasConflicts(RoadId))
      {
        const auto &ConflictingRoads = Junction->GetConflictsOfRoad(RoadId);
        for(const auto &Conflict : ConflictingRoads)
        {
          auto Waypoints = Map.GenerateWaypointsInRoad(Conflict);
          for(auto& Waypoint : Waypoints)
          {
            // Skip roads that share the same previous road
            bool bHasSamePredecessor = false;
            auto Predecessors = Map.GetPredecessors(Waypoint);
            for(auto &Prev : Predecessors)
            {
              if(SignalPredecessors.Contains(Prev.road_id))
              {
                bHasSamePredecessor = true;
              }
            }
            if(bHasSamePredecessor)
            {
              continue;
            }
            if(Map.GetLane(Waypoint).GetType() != cr::Lane::LaneType::Driving)
              continue;

            // Cover the road within the junction
            auto CurrentWaypoint = Waypoint;
            auto NextWaypoint = CurrentWaypoint;
            float BoxSize = static_cast<float>(
                0.9*Map.GetLaneWidth(NextWaypoint)*0.5);
            // Prevent a situation where the road width is 0
            // This could happen in a lane that is just appearing
            BoxSize = std::max(0.01f, BoxSize);
            float UEBoxSize = 100*BoxSize;

            FTransform BoxTransform = Map.ComputeTransform(NextWaypoint);
            ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(GetWorld());
            if (LargeMapManager)
            {
              BoxTransform = LargeMapManager->GlobalToLocalTransform(BoxTransform);
            }
            GenerateCheckBox(BoxTransform, UEBoxSize);
            while (true)
            {
              auto Next = Map.GetNext(NextWaypoint, 2*BoxSize);
              if (Next.size() != 1)
              {
                break;
              }
              NextWaypoint = Next.front();
              if(NextWaypoint.road_id != Waypoint.road_id)
              {
                break;
              }
              BoxTransform = Map.ComputeTransform(NextWaypoint);
              if (LargeMapManager)
              {
                BoxTransform = LargeMapManager->GlobalToLocalTransform(BoxTransform);
              }
              GenerateCheckBox(BoxTransform, UEBoxSize);
            }
            // Cover the road before the junction
            // Hard coded anticipation time (boxes placed prior to the junction)
            double AnticipationTime = 0.1;
            auto Previous = Map.GetPrevious(Waypoint, 2*BoxSize);
            std::queue<std::pair<float, carla::road::element::Waypoint>>
                WaypointQueue;
            for (auto & Prev : Previous)
            {
              WaypointQueue.push({AnticipationTime, Prev});
            }
            while (!WaypointQueue.empty())
            {
              auto CurrentElement = WaypointQueue.front();
              WaypointQueue.pop();
              GenerateCheckBox(Map.ComputeTransform(CurrentElement.second), UEBoxSize);

              float Speed = 40;
              auto* InfoSpeed = Map.GetLane(CurrentElement.second).GetRoad()->GetInfo<carla::road::element::RoadInfoSpeed>(CurrentElement.second.s);
              if(InfoSpeed)
              {
                Speed = InfoSpeed->GetSpeed();
              }
              float RemainingTime = CurrentElement.first - BoxSize/Speed;
              if(RemainingTime > 0)
              {
                Previous = Map.GetPrevious(CurrentElement.second, 2*BoxSize);
                for (auto & Prev : Previous)
                {
                  WaypointQueue.push({RemainingTime, Prev});
                }
              }
            }
          }
        }
      }
    }
  }
}

void UStopSignComponent::GenerateStopBox(const FTransform BoxTransform,
    const FVector BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UStopSignComponent::OnOverlapBeginStopEffectBox);
  BoxComponent->OnComponentEndOverlap.AddDynamic(this, &UStopSignComponent::OnOverlapEndStopEffectBox);
  AddEffectTriggerVolume(BoxComponent);
}

void UStopSignComponent::GenerateCheckBox(const FTransform BoxTransform,
    float BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UStopSignComponent::OnOverlapBeginStopCheckBox);
  BoxComponent->OnComponentEndOverlap.AddDynamic(this, &UStopSignComponent::OnOverlapEndStopCheckBox);
}

void UStopSignComponent::GiveWayIfPossible()
{
  if (VehiclesToCheck.Num() == 0)
  {
    for (auto Vehicle : VehiclesInStop)
    {
      AWheeledVehicleAIController* VehicleController =
        Cast<AWheeledVehicleAIController>(Vehicle->GetController());
      VehicleController->SetTrafficLightState(ETrafficLightState::Green);
    }
  }
  else
  {
    if(VehiclesInStop.Num())
    {
      for (auto Vehicle : VehiclesInStop)
      {
        AWheeledVehicleAIController* VehicleController =
          Cast<AWheeledVehicleAIController>(Vehicle->GetController());
        VehicleController->SetTrafficLightState(ETrafficLightState::Red);
      }
      // 1 second delay
      DelayedGiveWay(1.0f);
    }
  }
}

void UStopSignComponent::DelayedGiveWay(float Delay)
{
  FTimerHandle TimerHandler;
  GetWorld()->GetTimerManager().
      SetTimer(TimerHandler, this, &UStopSignComponent::GiveWayIfPossible, Delay);
}

void UStopSignComponent::OnOverlapBeginStopEffectBox(UPrimitiveComponent *OverlappedComp,
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
      VehicleController->SetTrafficLightState(ETrafficLightState::Red);
      VehiclesInStop.Add(Vehicle);

      // 2 second delay for stop
      DelayedGiveWay(2.0f);
    }
  }
  RemoveSameVehicleInBothLists();
}

void UStopSignComponent::OnOverlapEndStopEffectBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    VehiclesInStop.Remove(Vehicle);
  }
}

void UStopSignComponent::OnOverlapBeginStopCheckBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    if(!VehiclesInStop.Contains(Vehicle))
    {
      if (!VehiclesToCheck.Contains(Vehicle))
      {
        VehiclesToCheck.Add(Vehicle, 0);
      }
      VehiclesToCheck[Vehicle]++;
    }
    GiveWayIfPossible();
  }
}

void UStopSignComponent::OnOverlapEndStopCheckBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    if(VehiclesToCheck.Contains(Vehicle))
    {
      VehiclesToCheck[Vehicle]--;
      if(VehiclesToCheck[Vehicle] <= 0)
      {
        VehiclesToCheck.Remove(Vehicle);
      }
    }
    // 0.5s delay
    DelayedGiveWay(0.5f);
  }
}
void UStopSignComponent::RemoveSameVehicleInBothLists()
{
  for(auto* Vehicle : VehiclesInStop)
  {
    if(VehiclesToCheck.Contains(Vehicle))
    {
      VehiclesToCheck.Remove(Vehicle);
    }
  }
}
