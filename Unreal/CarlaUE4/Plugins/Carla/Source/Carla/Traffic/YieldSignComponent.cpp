// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "YieldSignComponent.h"
#include "TrafficLightState.h"
#include <queue>

#include <compiler/disable-ue4-macros.h>
#include <carla/road/element/RoadInfoSpeed.h>
#include <compiler/enable-ue4-macros.h>

void UYieldSignComponent::InitializeSign(const carla::road::Map &Map)
{

  const double epsilon = 0.00001;

  auto References = GetAllReferencesToThisSignal(Map);

  for (auto& Reference : References)
  {
    auto RoadId = Reference.first;
    const auto* SignalReference = Reference.second;
    TSet<carla::road::RoadId> SignalPredecessors;
    // Yield box
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
            0.9*Map.GetLaneWidth(signal_waypoint)/2.0);
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
        GenerateYieldBox(Map.ComputeTransform(signal_waypoint), UnrealBoxSize);

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
                0.9*Map.GetLaneWidth(NextWaypoint)/2.0);
            // Prevent a situation where the road width is 0,
            // this could happen in a lane that is just appearing
            BoxSize = std::max(0.01f, BoxSize);
            float UEBoxSize = 100*BoxSize;
            GenerateCheckBox(Map.ComputeTransform(NextWaypoint), UEBoxSize);
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

              GenerateCheckBox(Map.ComputeTransform(NextWaypoint), UEBoxSize);
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

void UYieldSignComponent::GenerateYieldBox(const FTransform BoxTransform,
    float BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UYieldSignComponent::OnOverlapBeginYieldEffectBox);
  BoxComponent->OnComponentEndOverlap.AddDynamic(this, &UYieldSignComponent::OnOverlapEndYieldEffectBox);
  AddEffectTriggerVolume(BoxComponent);
}

void UYieldSignComponent::GenerateCheckBox(const FTransform BoxTransform,
    float BoxSize)
{
  UBoxComponent* BoxComponent = GenerateTriggerBox(BoxTransform, BoxSize);
  BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &UYieldSignComponent::OnOverlapBeginYieldCheckBox);
  BoxComponent->OnComponentEndOverlap.AddDynamic(this, &UYieldSignComponent::OnOverlapEndYieldCheckBox);
}

void UYieldSignComponent::GiveWayIfPossible()
{
  if (VehiclesToCheck.Num() == 0)
  {
    for (auto Vehicle : VehiclesInYield)
    {
      AWheeledVehicleAIController* VehicleController =
        Cast<AWheeledVehicleAIController>(Vehicle->GetController());
      VehicleController->SetTrafficLightState(ETrafficLightState::Green);
    }
  }
  else
  {
    if(VehiclesInYield.Num())
    {
      for (auto Vehicle : VehiclesInYield)
      {
        AWheeledVehicleAIController* VehicleController =
          Cast<AWheeledVehicleAIController>(Vehicle->GetController());
        VehicleController->SetTrafficLightState(ETrafficLightState::Red);
      }
      // 0.5 second delay
      DelayedGiveWay(0.5f);
    }
  }
}

void UYieldSignComponent::DelayedGiveWay(float Delay)
{
  FTimerHandle TimerHandler;
  GetWorld()->GetTimerManager().
      SetTimer(TimerHandler, this, &UYieldSignComponent::GiveWayIfPossible, Delay);
}

void UYieldSignComponent::OnOverlapBeginYieldEffectBox(UPrimitiveComponent *OverlappedComp,
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
      VehiclesInYield.Add(Vehicle);
      RemoveSameVehicleInBothLists();
      GiveWayIfPossible();
    }
  }
}

void UYieldSignComponent::OnOverlapEndYieldEffectBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    VehiclesInYield.Remove(Vehicle);
  }
}

void UYieldSignComponent::OnOverlapBeginYieldCheckBox(UPrimitiveComponent *OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult)
{
  ACarlaWheeledVehicle * Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle)
  {
    if(!VehiclesInYield.Contains(Vehicle))
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

void UYieldSignComponent::OnOverlapEndYieldCheckBox(UPrimitiveComponent *OverlappedComp,
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
    // 0.5 second delay
    DelayedGiveWay(0.5);
  }
}
void UYieldSignComponent::RemoveSameVehicleInBothLists()
{
  for(auto* Vehicle : VehiclesInYield)
  {
    if(VehiclesToCheck.Contains(Vehicle))
    {
      VehiclesToCheck.Remove(Vehicle);
    }
  }
}
