// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/CarlaLidarSubsystem.h"
#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla/Sensor/SensorManager.h"

#include <util/ue-header-guard-begin.h>
#include "Async/ParallelFor.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include <util/ue-header-guard-end.h>

static TAutoConsoleVariable<int32> CVarLidarBatch(
    TEXT("carla.Lidar.Batch"), 1,
    TEXT("Simulate multiple LiDARs in parallel (1) or one after another (0)."));

void UCarlaLidarSubsystem::Deinitialize()
{
  if (LidarSimulationTask.IsValid())
  {
    FTaskGraphInterface::Get().WaitUntilTaskCompletes(LidarSimulationTask);
    LidarSimulationTask = nullptr;
  }
  LidarsToSimulate.Empty();
  ActiveLidars.Empty();
  Super::Deinitialize();
}

void UCarlaLidarSubsystem::RegisterLidar(ARayCastSemanticLidar *Lidar)
{
  if (Lidar != nullptr)
  {
    ActiveLidars.AddUnique(Lidar);

    // Remove from FSensorManager so this subsystem is the sole manager of LiDAR ticking
    UCarlaEpisode* CurrentEpisode = UCarlaStatics::GetCurrentEpisode(GetWorld());
    if (CurrentEpisode != nullptr)
    {
      CurrentEpisode->GetSensorManager().DeRegisterSensor(Lidar);
    }
  }
}

void UCarlaLidarSubsystem::UnregisterLidar(ARayCastSemanticLidar *Lidar)
{
  ActiveLidars.Remove(Lidar);
  ActiveLidars.RemoveAll([](const TWeakObjectPtr<ARayCastSemanticLidar>& Ptr) { return !Ptr.IsValid(); });
}

void UCarlaLidarSubsystem::StartLidarSimulations(UWorld *World, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCarlaLidarSubsystem::StartLidarSimulations);

  if (World == nullptr || World->GetPhysicsScene() == nullptr || !HasActiveLidars())
  {
    return;
  }

  // Snapshot on the game thread; the task must not read ActiveLidars while the game thread can modify it.
  LidarsToSimulate.Reset();
  for (const TWeakObjectPtr<ARayCastSemanticLidar> &WeakLidar : ActiveLidars)
  {
    ARayCastSemanticLidar *Lidar = WeakLidar.Get();
    if (Lidar != nullptr && Lidar->IsReadyToTick())
    {
      LidarsToSimulate.Add(Lidar);
    }
  }

  if (LidarsToSimulate.Num() == 0)
  {
    return;
  }

  const bool bBatch = (CVarLidarBatch.GetValueOnGameThread() != 0) && (LidarsToSimulate.Num() > 1);

  // Dispatch raycast simulation as a high-priority Task Graph job on worker threads
  LidarSimulationTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
    [this, World, DeltaSeconds, bBatch]()
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(TaskGraphLidarSimulations);
      auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(World->GetPhysicsScene());

      if (bBatch)
      {
        ParallelFor(LidarsToSimulate.Num(), [this, DeltaSeconds](int32 Index)
        {
          LidarsToSimulate[Index]->SimulateLidar(DeltaSeconds, /*bLockPhysics=*/false);
        });
      }
      else
      {
        for (ARayCastSemanticLidar *Lidar : LidarsToSimulate)
        {
          Lidar->SimulateLidar(DeltaSeconds, /*bLockPhysics=*/false);
        }
      }

      LockedPhysObject.Release();
    },
    TStatId(),
    nullptr,
    ENamedThreads::AnyBackgroundHiPriTask
  );
}

void UCarlaLidarSubsystem::FinishLidarSimulations(UWorld *World, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(UCarlaLidarSubsystem::FinishLidarSimulations);

  if (LidarSimulationTask.IsValid())
  {
    FTaskGraphInterface::Get().WaitUntilTaskCompletes(LidarSimulationTask);
    LidarSimulationTask = nullptr;
  }

  // Streaming and ROS2 publishing stay on the game thread.
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(LidarSendData);
    for (ARayCastSemanticLidar *Lidar : LidarsToSimulate)
    {
      if (IsValid(Lidar))
      {
        Lidar->SendData(DeltaSeconds);
        Lidar->ClearReadyToTick();
      }
    }
  }
  LidarsToSimulate.Reset();
}
