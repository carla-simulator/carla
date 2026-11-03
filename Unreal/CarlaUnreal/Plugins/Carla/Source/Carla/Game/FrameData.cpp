// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FrameData.h"

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Actor/CarlaActor.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Traffic/TrafficLightController.h"
#include "Carla/Traffic/TrafficLightGroup.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Sensor/Sensor.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Weather/Weather.h"

#include <util/disable-ue4-macros.h>
#include "carla/multigpu/mirroredActors.h"
#include "carla/rpc/VehicleLightState.h"
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <util/ue-header-guard-end.h>

void FFrameData::GetFrameData(UCarlaEpisode *ThisEpisode, bool bAdditionalData, bool bIncludeActorsAgain)
{
  Episode = ThisEpisode;
  // PlatformTime.UpdateTime();
  const FActorRegistry &Registry = Episode->GetActorRegistry();

  if (bIncludeActorsAgain)
  {
    AddExistingActors();
  }

  // through all actors in registry
  for (auto It = Registry.begin(); It != Registry.end(); ++It)
  {
    FCarlaActor* View = It->Value.Get();

    if (bAdditionalData)
    {
      AddActorAppearance(View, bIncludeActorsAgain);
    }

    switch (View->GetActorType())
    {
      // save the transform for props
      case FCarlaActor::ActorType::Other:
        AddActorPosition(View);
        // a prop that simulates physics moves: mirrors need its velocities
        if (bAdditionalData && !View->IsDormant() && (View->GetActor() != nullptr))
        {
          const auto *Root = Cast<UPrimitiveComponent>(View->GetActor()->GetRootComponent());
          if ((Root != nullptr) && Root->IsSimulatingPhysics())
          {
            AddActorKinematics(View);
          }
        }
        break;

      case FCarlaActor::ActorType::Sensor:
        AddActorPosition(View);
        if (bAdditionalData)
        {
          AddSensorTick(View);
        }
        break;

      // save the transform of all vehicles
      case FCarlaActor::ActorType::Vehicle:
        AddActorPosition(View);
        AddVehicleAnimation(View);
        AddVehicleLight(View);
        AddVehicleParts(View);
        if (bAdditionalData)
        {
          AddActorKinematics(View);
        }
        break;

      // save the transform of all walkers
      case FCarlaActor::ActorType::Walker:
        AddActorPosition(View);
        AddWalkerAnimation(View);
        if (bAdditionalData)
        {
          AddActorKinematics(View);
        }
        break;

      // save the state of each traffic light
      case FCarlaActor::ActorType::TrafficLight:
        AddTrafficLightState(View);
        if (bAdditionalData)
        {
          // its timings too, cheap and self-healing for a secondary
          if (const auto *TrafficLight = Cast<ATrafficLightBase>(View->GetActor()))
          {
            TrafficLightTimes.Add(CarlaRecorderTrafficLightTime{
                View->GetActorId(),
                TrafficLight->GetGreenTime(),
                TrafficLight->GetYellowTime(),
                TrafficLight->GetRedTime()});
          }
        }
        break;
    }
  }
  if (bAdditionalData)
  {
    // Forget the appearance of the actors gone.
    auto &Appearances = Episode->GetMultiGpuWorldState().Appearances;
    if (Appearances.size() > static_cast<size_t>(Registry.Num()))
    {
      for (auto It = Appearances.begin(); It != Appearances.end();)
      {
        It = Registry.Contains(It->first) ? std::next(It) : Appearances.erase(It);
      }
    }
    AddWorldState(bIncludeActorsAgain);
  }
  GetFrameCounter();
}

namespace
{
  CarlaRecorderWeather FFrameData_ToRecord(const FWeatherParameters &Weather)
  {
    CarlaRecorderWeather Record;
    Record.Cloudiness              = Weather.Cloudiness;
    Record.Precipitation           = Weather.Precipitation;
    Record.PrecipitationDeposits   = Weather.PrecipitationDeposits;
    Record.WindIntensity           = Weather.WindIntensity;
    Record.SunAzimuthAngle         = Weather.SunAzimuthAngle;
    Record.SunAltitudeAngle        = Weather.SunAltitudeAngle;
    Record.FogDensity              = Weather.FogDensity;
    Record.FogDistance             = Weather.FogDistance;
    Record.FogFalloff              = Weather.FogFalloff;
    Record.Wetness                 = Weather.Wetness;
    Record.ScatteringIntensity     = Weather.ScatteringIntensity;
    Record.MieScatteringScale      = Weather.MieScatteringScale;
    Record.RayleighScatteringScale = Weather.RayleighScatteringScale;
    Record.DustStorm               = Weather.DustStorm;
    return Record;
  }

  std::vector<uint64_t> FFrameData_SortedIds(const TSet<uint64> &Ids)
  {
    std::vector<uint64_t> Sorted;
    Sorted.reserve(Ids.Num());
    for (const uint64 Id : Ids)
    {
      Sorted.push_back(Id);
    }
    std::sort(Sorted.begin(), Sorted.end());
    return Sorted;
  }

  TSet<uint64> FFrameData_IdSet(const std::vector<uint64_t> &Ids)
  {
    TSet<uint64> Set;
    Set.Reserve(static_cast<int32>(Ids.size()));
    for (const uint64_t Id : Ids)
    {
      Set.Add(Id);
    }
    return Set;
  }

  FWeatherParameters FFrameData_FromRecord(const CarlaRecorderWeather &Record)
  {
    FWeatherParameters Weather;
    Weather.Cloudiness              = Record.Cloudiness;
    Weather.Precipitation           = Record.Precipitation;
    Weather.PrecipitationDeposits   = Record.PrecipitationDeposits;
    Weather.WindIntensity           = Record.WindIntensity;
    Weather.SunAzimuthAngle         = Record.SunAzimuthAngle;
    Weather.SunAltitudeAngle        = Record.SunAltitudeAngle;
    Weather.FogDensity              = Record.FogDensity;
    Weather.FogDistance             = Record.FogDistance;
    Weather.FogFalloff              = Record.FogFalloff;
    Weather.Wetness                 = Record.Wetness;
    Weather.ScatteringIntensity     = Record.ScatteringIntensity;
    Weather.MieScatteringScale      = Record.MieScatteringScale;
    Weather.RayleighScatteringScale = Record.RayleighScatteringScale;
    Weather.DustStorm               = Record.DustStorm;
    return Weather;
  }
} // namespace

void FFrameData::AddActorAppearance(FCarlaActor *CarlaActor, bool bFullResync)
{
  const AActor *Actor = CarlaActor->IsDormant() ? nullptr : CarlaActor->GetActor();
  if (Actor == nullptr)
  {
    return;
  }
  FMirroredAppearance Appearance;
  Appearance.bCollisions = Actor->GetActorEnableCollision();
  Appearance.bHidden = Actor->IsHidden();
  Appearance.Scale = Actor->GetActorScale3D();
  auto &Sent = Episode->GetMultiGpuWorldState().Appearances;
  const auto It = Sent.find(CarlaActor->GetActorId());
  if (!bFullResync && (It != Sent.end()) && (It->second == Appearance))
  {
    return;
  }
  Sent[CarlaActor->GetActorId()] = Appearance;
  carla::multigpu::ActorAppearance Record;
  Record.actor_id = CarlaActor->GetActorId();
  Record.collisions = Appearance.bCollisions;
  Record.hidden = Appearance.bHidden;
  Record.scale = std::array<double, 3>{Appearance.Scale.X, Appearance.Scale.Y, Appearance.Scale.Z};
  MirrorAppearances.actors.push_back(std::move(Record));
}

void FFrameData::AddWorldState(bool bFullResync)
{
  FPrimaryWorldState &Sent = Episode->GetMultiGpuWorldState();
  UWorld *World = Episode->GetWorld();
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(World);
  UCarlaLightSubsystem *LightSubsystem = World ? World->GetSubsystem<UCarlaLightSubsystem>() : nullptr;

  // Applied again with the same values is still an apply to mirror.
  if (AWeather *Weather = Episode->GetWeather())
  {
    if (bFullResync || (Weather->GetApplySerial() != Sent.WeatherSerial))
    {
      Sent.WeatherSerial = Weather->GetApplySerial();
      Weathers.Add(FFrameData_ToRecord(Weather->GetCurrentWeather()));
    }
  }

  carla::multigpu::WorldGlobals Globals;
  if (GameMode != nullptr)
  {
    Globals.map_layers = static_cast<uint16_t>(GameMode->GetLoadedMapLayers());
    Globals.imu_gravity = GameMode->IMUSensorGravity;
  }
  if (LightSubsystem != nullptr)
  {
    Globals.day_night_cycle = LightSubsystem->GetDayNightCycle();
    Globals.has_day_time_state = LightSubsystem->GetDayTimeState(Globals.last_is_day);
  }
#if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2 && ROS2->IsEnabled())
  {
    Globals.publish_tf = ROS2->GetPublishTF();
  }
#endif
  MirrorWorldGlobals = Globals;

  if ((GameMode != nullptr) &&
      (bFullResync || (GameMode->GetEnvironmentObjectsSerial() != Sent.EnvironmentObjectsSerial)))
  {
    Sent.EnvironmentObjectsSerial = GameMode->GetEnvironmentObjectsSerial();
    MirrorWorldObjects.disabled_environment_objects =
        FFrameData_SortedIds(GameMode->GetDisabledEnvironmentObjects());
  }

  // Each light's state at the end of this frame, after any weather change.
  if (LightSubsystem != nullptr)
  {
    if (bFullResync)
    {
      for (const auto &Light : LightSubsystem->GetLights())
      {
        if (IsValid(Light.Value) && IsValid(Light.Value->GetOwner()))
        {
          AddEventLightSceneChanged(Light.Value);
        }
      }
    }
    else
    {
      for (const int32 LightId : Sent.ChangedLights)
      {
        UCarlaLight *Light = LightSubsystem->GetLight(LightId);
        if (IsValid(Light) && IsValid(Light->GetOwner()))
        {
          AddEventLightSceneChanged(Light);
        }
      }
    }
  }
  Sent.ChangedLights.Reset();
}

namespace
{
  /// The local actor a primary id was replayed onto, if any.
  template <typename MapT>
  std::optional<uint32_t> FindMirror(const MapT &MappedId, uint32_t PrimaryId)
  {
    const auto It = MappedId.find(PrimaryId);
    return (It != MappedId.end()) ? std::optional<uint32_t>(It->second) : std::nullopt;
  }
} // namespace

void FFrameData::PlayFrameData(
    UCarlaEpisode *ThisEpisode,
    std::unordered_map<uint32_t, uint32_t>& MappedId,
    FMirrorState &MirrorState,
    TFunctionRef<void(uint32_t, uint32_t, bool)> OnActorAdded,
    TFunctionRef<void(uint32_t)> OnActorRemoved)
{
  Episode = ThisEpisode;
  const bool bFullResync = MultiGpuFrameTime && MultiGpuFrameTime->full_resync;
  std::unordered_set<uint32_t> AddedIds;

  // First: what the rest of the frame sets can live in a map layer.
  PlayMapLayers();

  for(const CarlaRecorderEventAdd &EventAdd : EventsAdd.GetEvents())
  {
    uint32_t OldId = EventAdd.DatabaseId;
    auto Result = ProcessReplayerEventAdd(
        EventAdd.Location,
        EventAdd.Rotation,
        EventAdd.Description,
        EventAdd.DatabaseId,
        false,
        true,
        MappedId);
    switch (Result.first)
    {
      // actor not created
      case 0:
        UE_LOG(LogCarla, Log, TEXT("actor could not be created"));
        break;

      // actor created but with different id
      case 1:
        // mapping id (recorded Id is a new Id in replayer)
        MappedId[OldId] = Result.second;
        MirrorState.CreatedIds.insert(Result.second);
        PrepareMirroredActor(Episode->FindCarlaActor(Result.second), OldId);
        UE_LOG(LogCarla, Log, TEXT("actor created"));
        OnActorAdded(OldId, Result.second, true);
        break;

      // actor reused from existing
      case 2:
        // mapping id (say desired Id is mapped to what)
        MappedId[OldId] = Result.second;
        PrepareMirroredActor(Episode->FindCarlaActor(Result.second), OldId);
        UE_LOG(LogCarla, Log, TEXT("actor reused"));
        OnActorAdded(OldId, Result.second, false);
        break;
    }
    AddedIds.insert(OldId);
  }

  // Attachments, retrying those whose actors were not mirrored yet.
  {
    std::vector<std::pair<uint32_t, uint32_t>> Parents = std::move(MirrorState.PendingParents);
    MirrorState.PendingParents.clear();
    for (const CarlaRecorderEventParent &EventParent : EventsParent.GetEvents())
    {
      Parents.emplace_back(EventParent.DatabaseId, EventParent.DatabaseIdParent);
    }
    for (const auto &Parent : Parents)
    {
      if (!ProcessReplayerEventParent(Parent.first, Parent.second, MappedId))
      {
        MirrorState.PendingParents.push_back(Parent);
      }
    }
  }

  std::unordered_set<uint32_t> RemovedIds;
  for (const CarlaRecorderEventDel &EventDel : EventsDel.GetEvents())
  {
    if (const std::optional<uint32_t> Local = FindMirror(MappedId, EventDel.DatabaseId))
    {
      ProcessReplayerEventDel(*Local);
      MirrorState.CreatedIds.erase(*Local);
      MappedId.erase(EventDel.DatabaseId);
    }
    RemovedIds.insert(EventDel.DatabaseId);
    OnActorRemoved(EventDel.DatabaseId);
  }

  if (bFullResync)
  {
    // A full resync announces every live actor of the primary: a mirror this
    // secondary spawned for any other one outlived its actor (its removal was
    // missed) and goes away.
    for (auto It = MappedId.begin(); It != MappedId.end();)
    {
      const uint32_t PrimaryId = It->first;
      const uint32_t LocalId = It->second;
      if ((AddedIds.count(PrimaryId) == 0u) && (MirrorState.CreatedIds.count(LocalId) != 0u))
      {
        ProcessReplayerEventDel(LocalId);
        MirrorState.CreatedIds.erase(LocalId);
        It = MappedId.erase(It);
        RemovedIds.insert(PrimaryId);
        OnActorRemoved(PrimaryId);
      }
      else
      {
        ++It;
      }
    }
  }
  if (!RemovedIds.empty() || bFullResync)
  {
    // An attachment can no longer be replayed once either actor is gone.
    auto &Pending = MirrorState.PendingParents;
    Pending.erase(std::remove_if(Pending.begin(), Pending.end(), [&](const auto &Parent)
        {
          const auto Gone = [&](uint32_t Id)
          {
            return (RemovedIds.count(Id) != 0u) || (bFullResync && (AddedIds.count(Id) == 0u));
          };
          return Gone(Parent.first) || Gone(Parent.second);
        }),
        Pending.end());
  }

  PlayAppearances(MappedId);
  PlayPositions(MappedId, MirrorState);
  PlayKinematics(MappedId);

  for (const CarlaRecorderTrafficLightTime &Time : TrafficLightTimes.GetTrafficLightTimes())
  {
    const std::optional<uint32_t> Local = FindMirror(MappedId, Time.DatabaseId);
    FCarlaActor *CarlaActor = Local ? Episode->FindCarlaActor(*Local) : nullptr;
    auto *TrafficLight = CarlaActor ? Cast<ATrafficLightBase>(CarlaActor->GetActor()) : nullptr;
    if (TrafficLight == nullptr)
    {
      continue;
    }
    if (TrafficLight->GetGreenTime() != Time.GreenTime)
    {
      TrafficLight->SetGreenTime(Time.GreenTime);
    }
    if (TrafficLight->GetYellowTime() != Time.YellowTime)
    {
      TrafficLight->SetYellowTime(Time.YellowTime);
    }
    if (TrafficLight->GetRedTime() != Time.RedTime)
    {
      TrafficLight->SetRedTime(Time.RedTime);
    }
  }

  for (const CarlaRecorderStateTrafficLight &State : States.GetStates())
  {
    if (const std::optional<uint32_t> Local = FindMirror(MappedId, State.DatabaseId))
    {
      CarlaRecorderStateTrafficLight StateTrafficLight = State;
      StateTrafficLight.DatabaseId = *Local;
      ProcessReplayerStateTrafficLight(StateTrafficLight);
    }
  }

  // Before packet 8: the control replayed there takes the manual gear shift
  // from the drivetrain set here.
  PlayVehicleParts(MappedId);

  for (const CarlaRecorderAnimVehicle &AnimVehicle : Vehicles.GetVehicles())
  {
    if (const std::optional<uint32_t> Local = FindMirror(MappedId, AnimVehicle.DatabaseId))
    {
      CarlaRecorderAnimVehicle Vehicle = AnimVehicle;
      Vehicle.DatabaseId = *Local;
      ProcessReplayerAnimVehicle(Vehicle);
    }
  }

  for (const CarlaRecorderAnimWalker &AnimWalker : Walkers.GetWalkers())
  {
    if (const std::optional<uint32_t> Local = FindMirror(MappedId, AnimWalker.DatabaseId))
    {
      CarlaRecorderAnimWalker Walker = AnimWalker;
      Walker.DatabaseId = *Local;
      ProcessReplayerAnimWalker(Walker);
    }
  }

  for (const CarlaRecorderAnimBiker &AnimBiker : Bikers.GetBikers())
  {
    if (const std::optional<uint32_t> Local = FindMirror(MappedId, AnimBiker.DatabaseId))
    {
      CarlaRecorderAnimBiker Biker = AnimBiker;
      Biker.DatabaseId = *Local;
      ProcessReplayerAnimBiker(Biker);
    }
  }

  for (const CarlaRecorderLightVehicle &LightVehicle : LightVehicles.GetLightVehicles())
  {
    if (const std::optional<uint32_t> Local = FindMirror(MappedId, LightVehicle.DatabaseId))
    {
      CarlaRecorderLightVehicle Light = LightVehicle;
      Light.DatabaseId = *Local;
      ProcessReplayerLightVehicle(Light);
    }
  }

  // Before the scene lights: a day/night change switches lights, and the
  // lights then get the state the primary's had at the end of the frame.
  PlayWorldState(MirrorState);

  for (const CarlaRecorderLightScene &Light : LightScenes.GetLights())
  {
    ProcessReplayerLightScene(Light);
  }

  // Sensors last, once their mirrors exist: the state each primary sensor
  // starts this frame's tick from, then which of them tick, with what delta.
  const auto FindSensor = [&](uint32_t PrimaryId) -> ASensor *
  {
    const std::optional<uint32_t> Local = FindMirror(MappedId, PrimaryId);
    FCarlaActor *CarlaActor = Local ? Episode->FindCarlaActor(*Local) : nullptr;
    return CarlaActor ? Cast<ASensor>(CarlaActor->GetActor()) : nullptr;
  };
  for (const carla::multigpu::SensorState &State : MirrorSensorStates.states)
  {
    if (ASensor *Sensor = FindSensor(State.actor_id))
    {
      Sensor->SetMirrorState(State);
    }
  }
  for (const carla::multigpu::SensorTick &Tick : MirrorSensorTicks.ticks)
  {
    if (ASensor *Sensor = FindSensor(Tick.actor_id))
    {
      Sensor->SetPrimaryTick(Tick.delta_seconds);
    }
  }

  SetFrameCounter();
}

void FFrameData::PlayMapLayers()
{
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(Episode->GetWorld());
  if (!MirrorWorldGlobals || (GameMode == nullptr))
  {
    return;
  }
  // Both flush level streaming: the layers are there for the rest of the frame.
  const std::pair<uint16_t, uint16_t> Diff = carla::multigpu::ComputeMapLayerDiff(
      static_cast<uint16_t>(GameMode->GetLoadedMapLayers()), MirrorWorldGlobals->map_layers);
  if (Diff.first != 0u)
  {
    GameMode->LoadMapLayer(Diff.first);
  }
  if (Diff.second != 0u)
  {
    GameMode->UnLoadMapLayer(Diff.second);
  }
}

void FFrameData::PlayAppearances(const std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  for (const carla::multigpu::ActorAppearance &Appearance : MirrorAppearances.actors)
  {
    const std::optional<uint32_t> Local = FindMirror(MappedId, Appearance.actor_id);
    FCarlaActor *CarlaActor = Local ? Episode->FindCarlaActor(*Local) : nullptr;
    AActor *Actor = ((CarlaActor != nullptr) && !CarlaActor->IsDormant()) ? CarlaActor->GetActor() : nullptr;
    if (Actor == nullptr)
    {
      continue;
    }
    if (Appearance.collisions && (Actor->GetActorEnableCollision() != *Appearance.collisions))
    {
      Actor->SetActorEnableCollision(*Appearance.collisions);
    }
    if (Appearance.hidden && (Actor->IsHidden() != *Appearance.hidden))
    {
      Actor->SetActorHiddenInGame(*Appearance.hidden);
    }
    if (Appearance.scale)
    {
      const FVector Scale((*Appearance.scale)[0], (*Appearance.scale)[1], (*Appearance.scale)[2]);
      if (!Actor->GetActorScale3D().Equals(Scale, 0.0))
      {
        USceneComponent *Root = Actor->GetRootComponent();
        if ((Root != nullptr) && (Root->GetMobility() != EComponentMobility::Movable))
        {
          Root->SetMobility(EComponentMobility::Movable);
        }
        Actor->SetActorScale3D(Scale);
      }
    }
  }
}

void FFrameData::PlayWorldState(FMirrorState &MirrorState)
{
  UWorld *World = Episode->GetWorld();
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(World);
  UCarlaLightSubsystem *LightSubsystem = World ? World->GetSubsystem<UCarlaLightSubsystem>() : nullptr;

  if (MirrorWorldGlobals)
  {
    const carla::multigpu::WorldGlobals &Globals = *MirrorWorldGlobals;
    if (GameMode != nullptr)
    {
      GameMode->IMUSensorGravity = Globals.imu_gravity;
    }
    // Before the weather, which switches lights only with the cycle on.
    if ((LightSubsystem != nullptr) && (LightSubsystem->GetDayNightCycle() != Globals.day_night_cycle))
    {
      LightSubsystem->SetDayNightCycle(Globals.day_night_cycle);
    }
#if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2 && ROS2->IsEnabled() && (ROS2->GetPublishTF() != Globals.publish_tf))
    {
      ROS2->SetPublishTF(Globals.publish_tf);
    }
#endif
  }

  if (AWeather *Weather = Episode->GetWeather())
  {
    for (const CarlaRecorderWeather &Record : Weathers.GetWeathers())
    {
      Weather->ApplyWeather(FFrameData_FromRecord(Record));
    }
  }

  // The day/night state the primary's lights follow, even where this
  // server's own threshold decided otherwise.
  if (MirrorWorldGlobals && (LightSubsystem != nullptr) &&
      MirrorWorldGlobals->day_night_cycle && MirrorWorldGlobals->has_day_time_state)
  {
    bool bIsDay = true;
    if (!LightSubsystem->GetDayTimeState(bIsDay) || (bIsDay != MirrorWorldGlobals->last_is_day))
    {
      LightSubsystem->NotifyDayTimeChange(MirrorWorldGlobals->last_is_day);
    }
  }

  // Environment objects: again whenever this server registers its objects
  // anew, for those of a layer that was still streaming in.
  bool bApplyObjects = false;
  if (MirrorWorldObjects.disabled_environment_objects)
  {
    MirrorState.DisabledObjects = *MirrorWorldObjects.disabled_environment_objects;
    bApplyObjects = true;
  }
  if ((GameMode != nullptr) && MirrorState.DisabledObjects &&
      (bApplyObjects || (GameMode->GetEnvironmentObjectsRegistrations() != MirrorState.DisabledObjectsRegistrations)))
  {
    MirrorState.DisabledObjectsRegistrations = GameMode->GetEnvironmentObjectsRegistrations();
    const auto Diff = carla::multigpu::DiffSortedIds(
        FFrameData_SortedIds(GameMode->GetDisabledEnvironmentObjects()), *MirrorState.DisabledObjects);
    if (!Diff.second.empty())
    {
      GameMode->EnableEnvironmentObjects(FFrameData_IdSet(Diff.second), true);
    }
    if (!Diff.first.empty())
    {
      GameMode->EnableEnvironmentObjects(FFrameData_IdSet(Diff.first), false);
    }
  }
}

void FFrameData::PlayPositions(
    const std::unordered_map<uint32_t, uint32_t>& MappedId,
    FMirrorState &MirrorState)
{
  MirrorState.PlayedPoses.clear();
  if (MirrorPositions.positions.empty())
  {
    // A frame from a primary without packet 34.
    for (const CarlaRecorderPosition &Position : Positions.GetPositions())
    {
      if (const std::optional<uint32_t> Local = FindMirror(MappedId, Position.DatabaseId))
      {
        CarlaRecorderPosition Pos = Position;
        Pos.DatabaseId = *Local;
        ProcessReplayerPosition(Pos, Pos, 0.0, 0.0);
      }
    }
    return;
  }

  std::vector<uint32_t> LocalIds;
  std::unordered_map<uint32_t, FTransform> Poses;
  std::unordered_map<uint32_t, uint32_t> Parents;
  LocalIds.reserve(MirrorPositions.positions.size());
  for (const carla::multigpu::MirroredPosition &Position : MirrorPositions.positions)
  {
    const std::optional<uint32_t> Local = FindMirror(MappedId, Position.id);
    FCarlaActor *CarlaActor = Local ? Episode->FindCarlaActor(*Local) : nullptr;
    if (CarlaActor == nullptr)
    {
      continue;
    }
    LocalIds.push_back(*Local);
    Poses.emplace(*Local, FTransform(
        FQuat(Position.rotation[0], Position.rotation[1], Position.rotation[2], Position.rotation[3]),
        FVector(Position.location[0], Position.location[1], Position.location[2])));
    if (CarlaActor->GetParent() != 0u)
    {
      Parents.emplace(*Local, CarlaActor->GetParent());
    }
  }
  // Parents first: moving a parent moves its attached children, which then
  // get their own exact pose.
  for (const uint32_t LocalId : carla::multigpu::OrderParentsFirst(LocalIds, Parents))
  {
    FCarlaActor *CarlaActor = Episode->FindCarlaActor(LocalId);
    const FTransform &Pose = Poses[LocalId];
    PlaceMirror(*CarlaActor, Pose);
    MirrorState.PlayedPoses.emplace_back(LocalId, Pose);
  }
}

/// What the engine reads when no CARLA code asks the episode: the mirror's
/// movement component, vehicle motion and root velocity.
static void FFrameData_SetMirroredMotion(FCarlaActor &CarlaActor, const FVector &Velocity)
{
  if (CarlaActor.IsDormant() || (CarlaActor.GetActor() == nullptr))
  {
    return;
  }
  AActor *Actor = CarlaActor.GetActor();
  if (auto *Vehicle = Cast<ACarlaWheeledVehicle>(Actor))
  {
    Vehicle->SetMirroredMotion(Velocity, FVector::DotProduct(Velocity, Vehicle->GetActorForwardVector()));
  }
  else if (auto *Character = Cast<ACharacter>(Actor))
  {
    if (UCharacterMovementComponent *Movement = Character->GetCharacterMovement())
    {
      Movement->Velocity = Velocity;
    }
  }
  if (USceneComponent *Root = Actor->GetRootComponent())
  {
    Root->ComponentVelocity = Velocity;
  }
}

void FFrameData::PlayKinematics(const std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  const uint64_t ThisFrame = FrameCounter.FrameCounter;
  for (const CarlaRecorderKinematics &Kinematic : Kinematics.GetKinematics())
  {
    const std::optional<uint32_t> Local = FindMirror(MappedId, Kinematic.DatabaseId);
    FCarlaActor *CarlaActor = Local ? Episode->FindCarlaActor(*Local) : nullptr;
    if (CarlaActor != nullptr)
    {
      CarlaActor->SetMirroredKinematics(Kinematic.LinearVelocity, Kinematic.AngularVelocity, ThisFrame);
      FFrameData_SetMirroredMotion(*CarlaActor, Kinematic.LinearVelocity);
    }
  }
  // A mirror this frame carries no velocities for (a prop that came to
  // rest) stands still, rather than keep the last ones it got.
  for (const auto &Mapped : MappedId)
  {
    FCarlaActor *CarlaActor = Episode->FindCarlaActor(Mapped.second);
    if ((CarlaActor != nullptr) && CarlaActor->HasMirroredKinematics() &&
        (CarlaActor->GetMirroredKinematicsFrame() != ThisFrame))
    {
      CarlaActor->ClearMirroredKinematics();
      FFrameData_SetMirroredMotion(*CarlaActor, FVector::ZeroVector);
    }
  }
}

void FFrameData::PlaceMirror(FCarlaActor &CarlaActor, FTransform Transform)
{
  if (CarlaActor.IsDormant())
  {
    if (const FActorData *Data = CarlaActor.GetActorData())
    {
      Transform.SetScale3D(Data->Scale);
    }
    CarlaActor.SetActorGlobalTransform(Transform, ETeleportType::None);
    return;
  }
  AActor *Actor = CarlaActor.GetActor();
  if (Actor == nullptr)
  {
    return;
  }
  // The primary's pose, the mirror's own scale (a spawn attribute).
  Transform.SetScale3D(Actor->GetActorScale3D());
  USceneComponent *Root = Actor->GetRootComponent();
  if ((Root != nullptr) && (Root->GetMobility() != EComponentMobility::Movable))
  {
    // A static actor that moved on the primary: such a root refuses to move.
    if (CarlaActor.GetActorGlobalTransform().Equals(Transform, 1e-3))
    {
      return;
    }
    Root->SetMobility(EComponentMobility::Movable);
  }
  // No teleport: the renderer keeps the previous pose for motion vectors.
  CarlaActor.SetActorGlobalTransform(Transform, ETeleportType::None);
}

void FFrameData::PrepareMirroredActor(FCarlaActor *CarlaActor, uint32_t PrimaryActorId)
{
  if (CarlaActor == nullptr)
  {
    return;
  }
  CarlaActor->SetPrimaryActorId(PrimaryActorId);
  switch (CarlaActor->GetActorType())
  {
    case FCarlaActor::ActorType::Vehicle:
      // ProcessReplayerEventAdd disabled its physics already.
      break;

    case FCarlaActor::ActorType::Walker:
      // No local movement: MOVE_None, so it cannot drift between frames.
      CarlaActor->SetActorSimulatePhysics(false);
      break;

    case FCarlaActor::ActorType::Other:
      if (!CarlaActor->IsDormant() && (CarlaActor->GetActor() != nullptr))
      {
        // Not FCarlaActor::SetActorSimulatePhysics, which also changes the
        // collision settings the primary's prop has.
        if (auto *Root = Cast<UPrimitiveComponent>(CarlaActor->GetActor()->GetRootComponent()))
        {
          if (Root->IsSimulatingPhysics())
          {
            Root->SetSimulatePhysics(false);
          }
        }
      }
      break;

    default:
      break;
  }
}

void FFrameData::Clear()
{
  EventsAdd.Clear();
  EventsDel.Clear();
  EventsParent.Clear();
  Collisions.Clear();
  Positions.Clear();
  States.Clear();
  Vehicles.Clear();
  Walkers.Clear();
  Bikers.Clear();
  LightVehicles.Clear();
  LightScenes.Clear();
  Kinematics.Clear();
  BoundingBoxes.Clear();
  TriggerVolumes.Clear();
  PhysicsControls.Clear();
  TrafficLightTimes.Clear();
  Weathers.Clear();
  FrameCounter.FrameCounter = 0;
  MultiGpuFrameTime.reset();
  MirrorPositions.positions.clear();
  MirrorVehicleParts.vehicles.clear();
  MirrorSensorTicks.ticks.clear();
  MirrorSensorStates.states.clear();
  bHasSensorTicks = false;
  MirrorWorldGlobals.reset();
  MirrorWorldObjects.disabled_environment_objects.reset();
  MirrorAppearances.actors.clear();
}

static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuFrameTime) == carla::multigpu::FrameTime::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuPositions) == carla::multigpu::MirroredPositions::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuSensorTicks) == carla::multigpu::SensorTicks::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuSensorState) == carla::multigpu::SensorStates::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuVehicleParts) == carla::multigpu::MirroredVehicleParts::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuWorldGlobals) == carla::multigpu::WorldGlobals::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuWorldObjects) == carla::multigpu::WorldObjects::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");
static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::MultiGpuActorAppearance) == carla::multigpu::ActorAppearances::kPacketId,
    "packet ids are fixed: see LibCarla/source/carla/multigpu");

void FFrameData::Write(std::ostream& OutStream)
{
  // First, so that a secondary knows the frame's time before anything else.
  if (MultiGpuFrameTime)
  {
    MultiGpuFrameTime->Write(OutStream);
  }
  EventsAdd.Write(OutStream);
  EventsDel.Write(OutStream);
  EventsParent.Write(OutStream);
  MirrorPositions.Write(OutStream);
  Kinematics.Write(OutStream);
  States.Write(OutStream);
  Vehicles.Write(OutStream);
  MirrorVehicleParts.Write(OutStream);
  Walkers.Write(OutStream);
  Bikers.Write(OutStream);
  LightVehicles.Write(OutStream);
  LightScenes.Write(OutStream);
  TrafficLightTimes.Write(OutStream);
  Weathers.Write(OutStream);
  if (MirrorWorldGlobals)
  {
    MirrorWorldGlobals->Write(OutStream);
  }
  if (!MirrorWorldObjects.empty())
  {
    MirrorWorldObjects.Write(OutStream);
  }
  if (!MirrorAppearances.actors.empty())
  {
    MirrorAppearances.Write(OutStream);
  }
  if (!MirrorSensorStates.states.empty())
  {
    MirrorSensorStates.Write(OutStream);
  }
  // Always, even when empty: a secondary's mirrors tick only on what it lists.
  MirrorSensorTicks.Write(OutStream);
  FrameCounter.Write(OutStream);
}

static_assert(
    static_cast<uint8_t>(CarlaRecorderPacketId::FrameCounter) == carla::multigpu::kFrameCounterPacketId,
    "the frame data codec reads the frame counter itself");

carla::multigpu::FrameDataReadResult FFrameData::Read(const uint8_t *Data, size_t Size)
{
  Clear();
  const carla::multigpu::FrameDataReadResult Result = carla::multigpu::ReadFrameData(Data, Size,
      [this](uint8_t Id, std::istream &InStream, uint32_t PacketSize)
      {
        switch (static_cast<CarlaRecorderPacketId>(Id))
        {
          case CarlaRecorderPacketId::EventAdd:
            EventsAdd.Read(InStream);
            return true;
          case CarlaRecorderPacketId::EventDel:
            EventsDel.Read(InStream);
            return true;
          case CarlaRecorderPacketId::EventParent:
            EventsParent.Read(InStream);
            return true;
          case CarlaRecorderPacketId::Position:
            Positions.Read(InStream);
            return true;
          case CarlaRecorderPacketId::State:
            States.Read(InStream);
            return true;
          case CarlaRecorderPacketId::AnimVehicle:
            Vehicles.Read(InStream);
            return true;
          case CarlaRecorderPacketId::AnimWalker:
            Walkers.Read(InStream);
            return true;
          case CarlaRecorderPacketId::MultiGpuVehicleParts:
            MirrorVehicleParts.Read(InStream, PacketSize);
            return true;
          case CarlaRecorderPacketId::AnimBiker:
            Bikers.Read(InStream);
            return true;
          case CarlaRecorderPacketId::VehicleLight:
            LightVehicles.Read(InStream);
            return true;
          case CarlaRecorderPacketId::SceneLight:
            LightScenes.Read(InStream);
            return true;
          case CarlaRecorderPacketId::Kinematics:
            Kinematics.Read(InStream);
            return true;
          case CarlaRecorderPacketId::TrafficLightTime:
            TrafficLightTimes.Read(InStream);
            return true;
          case CarlaRecorderPacketId::Weather:
            Weathers.Read(InStream);
            return true;
          case CarlaRecorderPacketId::MultiGpuWorldGlobals:
          {
            carla::multigpu::WorldGlobals Globals;
            // A short packet fails InStream, so the codec stops there.
            if (Globals.Read(InStream, PacketSize))
            {
              MirrorWorldGlobals = Globals;
            }
            return true;
          }
          case CarlaRecorderPacketId::MultiGpuWorldObjects:
            MirrorWorldObjects.Read(InStream, PacketSize);
            return true;
          case CarlaRecorderPacketId::MultiGpuActorAppearance:
            MirrorAppearances.Read(InStream, PacketSize);
            return true;
          case CarlaRecorderPacketId::MultiGpuSensorTicks:
            bHasSensorTicks = MirrorSensorTicks.Read(InStream, PacketSize);
            return true;
          case CarlaRecorderPacketId::MultiGpuSensorState:
            MirrorSensorStates.Read(InStream, PacketSize);
            return true;
          case CarlaRecorderPacketId::MultiGpuPositions:
            // A short packet fails InStream, so the codec stops there.
            MirrorPositions.Read(InStream, PacketSize);
            return true;
          case CarlaRecorderPacketId::MultiGpuFrameTime:
          {
            carla::multigpu::FrameTime Time;
            // A short packet fails InStream, so the codec stops there.
            if (Time.Read(InStream, PacketSize))
            {
              MultiGpuFrameTime = Time;
            }
            return true;
          }
          default:
            // Unknown here (written by another build): the codec skips it.
            return false;
        }
      });
  FrameCounter.FrameCounter = Result.frame.value_or(0u);
  carla::multigpu::FrameDataReadResult Checked = Result;
  if (Checked.IsComplete() && MultiGpuFrameTime && (MultiGpuFrameTime->frame != FrameCounter.FrameCounter))
  {
    Checked.status = carla::multigpu::FrameDataStatus::Inconsistent;
    Checked.packet_id = carla::multigpu::FrameTime::kPacketId;
  }
  return Checked;
}

void FrameTimeFromSettings(const FEpisodeSettings &Settings, carla::multigpu::FrameTime &Time)
{
  Time.synchronous_mode = Settings.bSynchronousMode;
  Time.no_rendering_mode = Settings.bNoRenderingMode;
  Time.fixed_delta_seconds.reset();
  if (Settings.FixedDeltaSeconds.IsSet())
  {
    Time.fixed_delta_seconds = Settings.FixedDeltaSeconds.GetValue();
  }
  Time.substepping = Settings.bSubstepping;
  Time.max_substep_delta_time = Settings.MaxSubstepDeltaTime;
  Time.max_substeps = Settings.MaxSubsteps;
  Time.max_culling_distance = Settings.MaxCullingDistance;
  Time.deterministic_ragdolls = Settings.bDeterministicRagdolls;
  Time.tile_streaming_distance = Settings.TileStreamingDistance;
  Time.actor_active_distance = Settings.ActorActiveDistance;
  Time.spectator_as_ego = Settings.SpectatorAsEgo;
  Time.adjust_signs_height_to_ground = Settings.bAdjustSignsHeightToGround;
}

void SettingsFromFrameTime(const carla::multigpu::FrameTime &Time, FEpisodeSettings &Settings)
{
  Settings.bSynchronousMode = Time.synchronous_mode;
  Settings.FixedDeltaSeconds.Reset();
  if (Time.fixed_delta_seconds)
  {
    Settings.FixedDeltaSeconds = *Time.fixed_delta_seconds;
  }
  Settings.bSubstepping = Time.substepping;
  Settings.MaxSubstepDeltaTime = Time.max_substep_delta_time;
  Settings.MaxSubsteps = Time.max_substeps;
  Settings.MaxCullingDistance = Time.max_culling_distance;
  Settings.bDeterministicRagdolls = Time.deterministic_ragdolls;
  Settings.TileStreamingDistance = Time.tile_streaming_distance;
  Settings.ActorActiveDistance = Time.actor_active_distance;
  Settings.SpectatorAsEgo = Time.spectator_as_ego;
  Settings.bAdjustSignsHeightToGround = Time.adjust_signs_height_to_ground;
}

void FFrameData::CreateRecorderEventAdd(
    uint32_t DatabaseId,
    uint8_t Type,
    const FTransform &Transform,
    FActorDescription ActorDescription,
    bool bAddOtherRelatedInfo)
{
  CarlaRecorderActorDescription Description;
  Description.UId = ActorDescription.UId;
  Description.Id = ActorDescription.Id;

  // attributes
  Description.Attributes.reserve(ActorDescription.Variations.Num());
  for (const auto &item : ActorDescription.Variations)
  {
    CarlaRecorderActorAttribute Attr;
    Attr.Type = static_cast<uint8_t>(item.Value.Type);
    Attr.Id = item.Value.Id;
    Attr.Value = item.Value.Value;
    // check for empty attributes
    if (!Attr.Id.IsEmpty())
    {
      Description.Attributes.emplace_back(std::move(Attr));
    }
  }

  // recorder event
  CarlaRecorderEventAdd RecEvent
  {
    DatabaseId,
    Type,
    Transform.GetTranslation(),
    Transform.GetRotation().Euler(),
    std::move(Description)
  };
  AddEvent(std::move(RecEvent));

  if (!bAddOtherRelatedInfo)
  {
    return;
  }

  // Other events related to spawning actors
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(DatabaseId);
  if (!CarlaActor)
  {
    return;
  }
  
  // check if it is a vehicle to get initial physics control
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  if (Vehicle)
  {
    AddPhysicsControl(*Vehicle);
  }

  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(CarlaActor->GetActor());
  if (TrafficLight)
  {
    AddTrafficLightTime(*TrafficLight);
  }

  ATrafficSignBase* TrafficSign = Cast<ATrafficSignBase>(CarlaActor->GetActor());
  if (TrafficSign)
  {
    // Trigger volume in global coordinates
    AddTriggerVolume(*TrafficSign);
  }
  else
  {
    // Bounding box in local coordinates
    AddActorBoundingBox(CarlaActor);
  }
}


void FFrameData::AddActorPosition(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  // As a quaternion: an Euler round trip snaps pitch near +-90 degrees.
  const FTransform Transform = CarlaActor->GetActorGlobalTransform();
  const FVector Location = Transform.GetLocation();
  const FQuat Rotation = Transform.GetRotation();
  carla::multigpu::MirroredPosition Position;
  Position.id = CarlaActor->GetActorId();
  Position.location[0] = Location.X;
  Position.location[1] = Location.Y;
  Position.location[2] = Location.Z;
  Position.rotation[0] = Rotation.X;
  Position.rotation[1] = Rotation.Y;
  Position.rotation[2] = Rotation.Z;
  Position.rotation[3] = Rotation.W;
  MirrorPositions.positions.push_back(Position);
}

void FFrameData::AddVehicleAnimation(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  if (CarlaActor->IsPendingKill())
  {
    return;
  }

  FVehicleControl Control;
  CarlaActor->GetVehicleControl(Control);

  // save
  CarlaRecorderAnimVehicle Record;
  Record.DatabaseId = CarlaActor->GetActorId();
  Record.Steering = Control.Steer;
  Record.Throttle = Control.Throttle;
  Record.Brake = Control.Brake;
  Record.bHandbrake = Control.bHandBrake;
  Record.Gear = Control.Gear;
  AddAnimVehicle(Record);
}

void FFrameData::AddVehicleParts(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr)
  if (CarlaActor->IsPendingKill() || CarlaActor->IsDormant())
    return;
  if (CarlaActor->GetActorType() != FCarlaActor::ActorType::Vehicle)
    return;

  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  if (CarlaVehicle == nullptr)
    return;

  const UChaosWheeledVehicleMovementComponent* Movement = CarlaVehicle->GetChaosWheeledVehicleMovementComponent();
  if (Movement == nullptr)
    return;

  carla::multigpu::VehicleParts Parts;
  Parts.actor_id = CarlaActor->GetActorId();
  Parts.wheels.reserve(Movement->WheelSetups.Num());

  // The getters return the pose shown, whether Chaos, the animation instance
  // or the wheel animation override set it.
  for (uint8 i = 0; CarlaVehicle->HasWheel(i); ++i)
  {
    const EVehicleWheelLocation Location = static_cast<EVehicleWheelLocation>(i);
    Parts.wheels.push_back(carla::multigpu::WheelPose{
        CarlaVehicle->GetWheelSteerAngle(Location),
        CarlaVehicle->GetWheelPitchAngle(Location),
        CarlaVehicle->GetWheelSuspensionOffset(Location)});
  }
  Parts.current_gear = CarlaVehicle->GetVehicleCurrentGear();
  Parts.manual_gear_shift = CarlaVehicle->GetVehicleControl().bManualGearShift;
  const auto *Root = Cast<UPrimitiveComponent>(CarlaVehicle->GetRootComponent());
  Parts.simulating_physics = (Root != nullptr) && Root->IsSimulatingPhysics();
  for (const TPair<int32, FTransform> &Door : CarlaVehicle->GetOpenDoorPoses())
  {
    carla::multigpu::DoorPose Pose;
    Pose.door = static_cast<uint8_t>(Door.Key);
    const FVector Location = Door.Value.GetLocation();
    const FQuat Rotation = Door.Value.GetRotation();
    Pose.location = {Location.X, Location.Y, Location.Z};
    Pose.rotation = {Rotation.X, Rotation.Y, Rotation.Z, Rotation.W};
    Parts.open_doors.push_back(Pose);
  }
  MirrorVehicleParts.vehicles.push_back(std::move(Parts));

  if (CarlaVehicle->IsTwoWheeledVehicle())
  {
    AddAnimBiker(CarlaRecorderAnimBiker
    {
      CarlaActor->GetActorId(),
      Movement->GetForwardSpeed(),
      Movement->GetEngineRotationSpeed() / Movement->GetEngineMaxRotationSpeed()
    });
  }
}

void FFrameData::AddWalkerAnimation(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  if (!CarlaActor->IsPendingKill())
  {
    FWalkerControl Control;
    CarlaActor->GetWalkerControl(Control);
    AddAnimWalker(CarlaRecorderAnimWalker
    {
      CarlaActor->GetActorId(),
      Control.Speed
    });
  }
}

void FFrameData::AddTrafficLightState(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  ETrafficLightState LightState = CarlaActor->GetTrafficLightState();
  UTrafficLightController* Controller = CarlaActor->GetTrafficLightController();
  if (Controller)
  {
    ATrafficLightGroup* Group = Controller->GetGroup();
    if (Group)
    {
      AddState(CarlaRecorderStateTrafficLight
      {
        CarlaActor->GetActorId(),
        Group->IsFrozen(),
        Controller->GetElapsedTime(),
        static_cast<char>(LightState)
      });
    }
  }
}

void FFrameData::AddVehicleLight(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  FVehicleLightState LightState;
  CarlaActor->GetVehicleLightState(LightState);
  CarlaRecorderLightVehicle LightVehicle;
  LightVehicle.DatabaseId = CarlaActor->GetActorId();
  LightVehicle.State = carla::rpc::VehicleLightState(LightState).light_state;
  AddLightVehicle(LightVehicle);
}

void FFrameData::AddActorKinematics(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  // World frame, in UE units: cm/s and rad/s (for Multi-GPU secondaries; the
  // .log recorder writes its own kinematics).
  CarlaRecorderKinematics Kinematic =
  {
    CarlaActor->GetActorId(),
    CarlaActor->GetActorVelocity(),
    FMath::DegreesToRadians(CarlaActor->GetActorAngularVelocity())
  };
  AddKinematics(Kinematic);
}

void FFrameData::AddSensorTick(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);
  // Still set here: the frame is built before the sensors capture, which
  // clears it. Its state is therefore the one this tick starts from.
  const ASensor *Sensor = Cast<ASensor>(CarlaActor->GetActor());
  if ((Sensor == nullptr) || !Sensor->IsReadyToTick())
  {
    return;
  }
  MirrorSensorTicks.ticks.push_back(
      carla::multigpu::SensorTick{CarlaActor->GetActorId(), Sensor->GetReadyToTickDeltaSeconds()});
  carla::multigpu::SensorState State;
  State.actor_id = CarlaActor->GetActorId();
  Sensor->GetMirrorState(State);
  if (!State.empty())
  {
    MirrorSensorStates.states.push_back(std::move(State));
  }
}

void FFrameData::AddActorBoundingBox(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  const auto &Box = CarlaActor->GetActorInfo()->BoundingBox;
  CarlaRecorderActorBoundingBox BoundingBox =
  {
    CarlaActor->GetActorId(),
    {Box.Origin, Box.Extent}
  };

  AddBoundingBox(BoundingBox);
}

void FFrameData::AddTriggerVolume(const ATrafficSignBase &TrafficSign)
{
  TArray<UBoxComponent*> Triggers = TrafficSign.GetTriggerVolumes();
  if(!Triggers.Num())
  {
    return;
  }
  auto CarlaActor = Episode->GetActorRegistry().FindCarlaActor(&TrafficSign);
  if (!CarlaActor)
  {
    return;
  }
  UBoxComponent* Trigger = Triggers.Top();
  auto VolumeOrigin = Trigger->GetComponentLocation();
  auto VolumeExtent = Trigger->GetScaledBoxExtent();
  CarlaRecorderActorBoundingBox TriggerVolume =
  {
    CarlaActor->GetActorId(),
    {VolumeOrigin, VolumeExtent}
  };
  TriggerVolumes.Add(TriggerVolume);
}

void FFrameData::AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle)
{
  if (auto CarlaActor = Episode->GetActorRegistry().FindCarlaActor(&Vehicle))
  {
    CarlaRecorderPhysicsControl Control;
    Control.DatabaseId = CarlaActor->GetActorId();
    Control.VehiclePhysicsControl = Vehicle.GetVehiclePhysicsControl();
    PhysicsControls.Add(Control);
  }
}

void FFrameData::AddTrafficLightTime(const ATrafficLightBase& TrafficLight)
{
  if (auto CarlaActor = Episode->GetActorRegistry().FindCarlaActor(&TrafficLight))
  {
    auto DatabaseId = CarlaActor->GetActorId();
    CarlaRecorderTrafficLightTime TrafficLightTime{
      DatabaseId,
      TrafficLight.GetGreenTime(),
      TrafficLight.GetYellowTime(),
      TrafficLight.GetRedTime()
    };
    TrafficLightTimes.Add(TrafficLightTime);
  }
}

void FFrameData::AddPosition(const CarlaRecorderPosition &Position)
{
  Positions.Add(Position);
}

void FFrameData::AddEvent(const CarlaRecorderEventAdd &Event)
{
  EventsAdd.Add(std::move(Event));
}

void FFrameData::AddEvent(const CarlaRecorderEventDel &Event)
{
  // An actor destroyed through the episode is reported again when the engine
  // destroys it; a secondary needs it once.
  for (const CarlaRecorderEventDel &Existing : EventsDel.GetEvents())
  {
    if (Existing.DatabaseId == Event.DatabaseId)
    {
      return;
    }
  }
  EventsDel.Add(std::move(Event));
}

void FFrameData::AddEvent(const CarlaRecorderEventParent &Event)
{
  EventsParent.Add(std::move(Event));
}

void FFrameData::AddCollision(AActor *Actor1, AActor *Actor2)
{
  CarlaRecorderCollision Collision;

  // // some inits
  // Collision.Id = NextCollisionId++;
  // Collision.IsActor1Hero = false;
  // Collision.IsActor2Hero = false;

  // // check actor 1
  // FCarlaActor *FoundActor1 = Episode->GetActorRegistry().FindCarlaActor(Actor1);
  // if (FoundActor1 != nullptr) {
  //   if (FoundActor1->GetActorInfo() != nullptr)
  //   {
  //     auto Role = FoundActor1->GetActorInfo()->Description.Variations.Find("role_name");
  //     if (Role != nullptr)
  //       Collision.IsActor1Hero = (Role->Value == "hero");
  //   }
  //   Collision.DatabaseId1 = FoundActor1->GetActorId();
  // }
  // else {
  //   Collision.DatabaseId1 = uint32_t(-1); // actor1 is not a registered Carla actor
  // }

  // // check actor 2
  // FCarlaActor *FoundActor2 = Episode->GetActorRegistry().FindCarlaActor(Actor2);
  // if (FoundActor2 != nullptr) {
  //   if (FoundActor2->GetActorInfo() != nullptr)
  //   {
  //     auto Role = FoundActor2->GetActorInfo()->Description.Variations.Find("role_name");
  //     if (Role != nullptr)
  //       Collision.IsActor2Hero = (Role->Value == "hero");
  //   }
  //   Collision.DatabaseId2 = FoundActor2->GetActorId();
  // }
  // else {
  //   Collision.DatabaseId2 = uint32_t(-1); // actor2 is not a registered Carla actor
  // }

  Collisions.Add(std::move(Collision));
}

void FFrameData::AddState(const CarlaRecorderStateTrafficLight &State)
{
  States.Add(State);
}

void FFrameData::AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle)
{
  Vehicles.Add(Vehicle);
}

void FFrameData::AddAnimBiker(const CarlaRecorderAnimBiker &Biker)
{
  Bikers.Add(Biker);
}

void FFrameData::AddAnimWalker(const CarlaRecorderAnimWalker &Walker)
{
  Walkers.Add(Walker);
}

void FFrameData::AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle)
{
  LightVehicles.Add(LightVehicle);
}

void FFrameData::AddEventLightSceneChanged(const UCarlaLight* Light)
{
  CarlaRecorderLightScene LightScene =
  {
    Light->GetId(),
    Light->GetLightIntensity(),
    Light->GetLightColor(),
    Light->GetLightOn(),
    static_cast<uint8>(Light->GetLightType())
  };

  LightScenes.Add(LightScene);
}

void FFrameData::AddKinematics(const CarlaRecorderKinematics &ActorKinematics)
{
  Kinematics.Add(ActorKinematics);
}

void FFrameData::AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox)
{
  BoundingBoxes.Add(ActorBoundingBox);
}

void FFrameData::GetFrameCounter()
{
  FrameCounter.FrameCounter = FCarlaEngine::GetFrameCounter();
}

namespace
{
  // Local actors as seen by carla::multigpu::PlanReplayedActor.
  class FReplayedActorLookup
  {
  public:
    FReplayedActorLookup(UCarlaEpisode &InEpisode, const FActorDescription &InDescription, const FVector &InLocation)
      : Episode(InEpisode), Description(InDescription), Location(InLocation) {}

    bool IsSameKind(uint32_t LocalId) const
    {
      const FActorInfo *Info = FindInfo(LocalId);
      return (Info != nullptr) && (Info->Description.Id == Description.Id);
    }

    std::optional<uint32_t> FindMapActor() const
    {
      if (Description.Id == TEXT("spectator"))
      {
        APawn *Spectator = Episode.GetSpectatorPawn();
        const FCarlaActor *CarlaActor = (Spectator != nullptr) ? Episode.FindCarlaActor(Spectator) : nullptr;
        return (CarlaActor != nullptr) ? std::optional<uint32_t>(CarlaActor->GetActorId()) : std::nullopt;
      }
      if (Description.Id != TEXT("static.prop.mesh"))
      {
        return std::nullopt;
      }
      // Same truncated location as FindTrafficSignAt.
      const FIntVector Cell(static_cast<int32>(Location.X), static_cast<int32>(Location.Y), static_cast<int32>(Location.Z));
      for (auto It = Episode.GetActorRegistry().begin(); It != Episode.GetActorRegistry().end(); ++It)
      {
        const FCarlaActor *CarlaActor = It->Value.Get();
        if ((CarlaActor == nullptr) || !IsMapActorLike(CarlaActor->GetActorId()))
        {
          continue;
        }
        const FVector Other = CarlaActor->GetActorGlobalLocation();
        if (Cell == FIntVector(static_cast<int32>(Other.X), static_cast<int32>(Other.Y), static_cast<int32>(Other.Z)))
        {
          return CarlaActor->GetActorId();
        }
      }
      return std::nullopt;
    }

    // The map registers the spectator and movable static meshes at BeginPlay
    // (UCarlaEpisode::InitializeAtBeginPlay); the primary resends them on a
    // full resync.
    bool IsMapActorLike(uint32_t LocalId) const
    {
      const FActorInfo *Info = FindInfo(LocalId);
      if ((Info == nullptr) || (Info->Description.Id != Description.Id))
      {
        return false;
      }
      if (Description.Id == TEXT("spectator"))
      {
        return true;
      }
      if (Description.Id != TEXT("static.prop.mesh"))
      {
        return false;
      }
      const FActorAttribute *Mesh = Description.Variations.Find(TEXT("mesh_path"));
      const FActorAttribute *OtherMesh = Info->Description.Variations.Find(TEXT("mesh_path"));
      return (Mesh != nullptr) && (OtherMesh != nullptr) && (Mesh->Value == OtherMesh->Value);
    }

    bool Contains(uint32_t LocalId) const
    {
      return Episode.GetActorRegistry().Contains(LocalId);
    }

  private:
    const FActorInfo *FindInfo(uint32_t LocalId) const
    {
      const FCarlaActor *CarlaActor = Episode.FindCarlaActor(LocalId);
      return (CarlaActor != nullptr) ? CarlaActor->GetActorInfo() : nullptr;
    }

    UCarlaEpisode &Episode;
    const FActorDescription &Description;
    const FVector &Location;
  };
}

// create or reuse an actor for replaying
std::pair<int, FCarlaActor*> FFrameData::CreateOrReuseActor(
    FVector &Location,
    FVector &Rotation,
    FActorDescription &ActorDesc,
    uint32_t DesiredId,
    bool SpawnSensors,
    std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  check(Episode != nullptr);

  // check type of actor we need
  if (ActorDesc.Id.StartsWith("traffic."))
  {
    FCarlaActor* CarlaActor = FindTrafficSignAt(Location);
    if (CarlaActor != nullptr)
    {
      // reuse that actor
      UE_LOG(LogCarla, Log, TEXT("TrafficLight found"));
      return std::pair<int, FCarlaActor*>(2, CarlaActor);
    }
    else
    {
      // actor not found
      UE_LOG(LogCarla, Log, TEXT("TrafficLight not found"));
      return std::pair<int, FCarlaActor*>(0, nullptr);
    }
  }
  else if (SpawnSensors || !ActorDesc.Id.StartsWith("sensor."))
  {
    const carla::multigpu::ReplayedActorPlan Plan = carla::multigpu::PlanReplayedActor(
        MappedId, DesiredId, FReplayedActorLookup(*Episode, ActorDesc, Location));
    if (Plan.reuse)
    {
      FCarlaActor *CarlaActor = Episode->FindCarlaActor(*Plan.reuse);
      FRotator Rot = FRotator::MakeFromEuler(Rotation);
      PlaceMirror(*CarlaActor, FTransform(Rot, Location));
      return std::pair<int, FCarlaActor*>(2, CarlaActor);
    }
    // create new actor
    // create the transform
    FRotator Rot = FRotator::MakeFromEuler(Rotation);
    FTransform Trans(Rot, FVector(0, 0, 100000), FVector(1, 1, 1));
    // create as new actor
    TPair<EActorSpawnResultStatus, FCarlaActor*> Result = Episode->SpawnActorWithInfo(Trans, ActorDesc, Plan.spawn_id);
    if (Result.Key == EActorSpawnResultStatus::Success)
    {
      // relocate, keeping the scale the actor was spawned with (and moving
      // a static root, e.g. a mass-less prop, which refuses to move otherwise)
      PlaceMirror(*Result.Value, FTransform(Rot, Location));
      ALargeMapManager * LargeMapManager = UCarlaStatics::GetLargeMapManager(Episode->GetWorld());
      if (LargeMapManager)
      {
        LargeMapManager->OnActorSpawned(*Result.Value);
      }
      return std::pair<int, FCarlaActor*>(1, Result.Value);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("Actor could't be created"));
      return std::pair<int, FCarlaActor*>(0, Result.Value);
    }
  }
  else
  {
    // actor ignored
    return std::pair<int, FCarlaActor*>(0, nullptr);
  }
}

// replay event for creating actor
std::pair<int, uint32_t> FFrameData::ProcessReplayerEventAdd(
    FVector Location,
    FVector Rotation,
    CarlaRecorderActorDescription Description,
    uint32_t DesiredId,
    bool bIgnoreHero,
    bool ReplaySensors,
    std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  check(Episode != nullptr);
  FActorDescription ActorDesc;
  bool IsHero = false;

  // prepare actor description
  ActorDesc.UId = Description.UId;
  ActorDesc.Id = Description.Id;
  for (const auto &Item : Description.Attributes)
  {
    FActorAttribute Attr;
    Attr.Type = static_cast<EActorAttributeType>(Item.Type);
    Attr.Id = Item.Id;
    Attr.Value = Item.Value;
    ActorDesc.Variations.Add(Attr.Id, std::move(Attr));
    // check for hero
    if (Item.Id == "role_name" && Item.Value == "hero")
      IsHero = true;
  }

  auto result = CreateOrReuseActor(
      Location,
      Rotation,
      ActorDesc,
      DesiredId,
      ReplaySensors,
      MappedId);

  if (result.first != 0)
  {
    // disable physics and autopilot on vehicles
    if (result.second->GetActorType() == FCarlaActor::ActorType::Vehicle)
    {
      // ignore hero ?
      if (!(bIgnoreHero && IsHero))
      {
        // disable physics
        SetActorSimulatePhysics(result.second, false);
        // disable autopilot
        // SetActorAutopilot(result.second, false, false);
      }
      else
      {
        // reenable physics just in case
        SetActorSimulatePhysics(result.second, true);
      }
    }
    return std::make_pair(result.first, result.second->GetActorId());
  }
  return std::make_pair(result.first, 0);
}

// replay event for removing actor
bool FFrameData::ProcessReplayerEventDel(uint32_t DatabaseId)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(DatabaseId);
  if (CarlaActor == nullptr)
  {
    UE_LOG(LogCarla, Log, TEXT("Actor %d not found to destroy"), DatabaseId);
    return false;
  }
  Episode->DestroyActor(CarlaActor->GetActorId());
  return true;
}

// replay event for parenting actors
bool FFrameData::ProcessReplayerEventParent(
    uint32_t ChildId,
    uint32_t ParentId,
    const std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  check(Episode != nullptr);
  const std::optional<uint32_t> LocalChild = FindMirror(MappedId, ChildId);
  const std::optional<uint32_t> LocalParent = FindMirror(MappedId, ParentId);
  FCarlaActor *Child = LocalChild ? Episode->FindCarlaActor(*LocalChild) : nullptr;
  FCarlaActor *Parent = LocalParent ? Episode->FindCarlaActor(*LocalParent) : nullptr;
  if ((Child == nullptr) || (Parent == nullptr))
  {
    return false;
  }
  // Every full resync replays every attachment: only a new one registers the
  // ROS 2 parent names, which pile up otherwise ("hero/hero").
  const bool bNewParent = (Child->GetParent() != Parent->GetActorId());
  Child->SetParent(Parent->GetActorId());
  Child->SetAttachmentType(carla::rpc::AttachmentType::Rigid);
  Parent->AddChildren(Child->GetActorId());
  if (Parent->IsDormant())
  {
    // Putting a parent to sleep puts its children to sleep too.
    if (!Child->IsDormant())
    {
      Episode->PutActorToSleep(Child->GetActorId());
    }
  }
  else if (!Child->IsDormant() && (Child->GetActor() != nullptr) && (Parent->GetActor() != nullptr))
  {
    // Always rigid, keeping the child where it is: every frame sets its exact
    // pose (after its parent's), whatever attachment the primary used.
    AActor *ChildActor = Child->GetActor();
    AActor *ParentActor = Parent->GetActor();
    if (ChildActor->GetAttachParentActor() != ParentActor)
    {
      ChildActor->AttachToActor(ParentActor, FAttachmentTransformRules::KeepWorldTransform);
    }
    // Sensors read their parent's motion through their owner (the IMU). A
    // collision sensor binds its owner's hit event in SetOwner.
    if (ChildActor->GetOwner() != ParentActor)
    {
      ChildActor->SetOwner(ParentActor);
    }
  }
  if (bNewParent)
  {
    Episode->AddRosParentNames(*Child);
  }
  return true;
}

// reposition actors
bool FFrameData::ProcessReplayerPosition(CarlaRecorderPosition Pos1, CarlaRecorderPosition Pos2, double Per, double DeltaTime)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(Pos1.DatabaseId);
  FVector Location;
  FRotator Rotation;
  if(CarlaActor)
  {
    // check to assign first position or interpolate between both
    if (Per == 0.0)
    {
      // assign position 1
      Location = FVector(Pos1.Location);
      Rotation = FRotator::MakeFromEuler(Pos1.Rotation);
    }
    else
    {
      // interpolate positions
      Location = FMath::Lerp(FVector(Pos1.Location), FVector(Pos2.Location), Per);
      Rotation = FMath::Lerp(FRotator::MakeFromEuler(Pos1.Rotation), FRotator::MakeFromEuler(Pos2.Rotation), Per);
    }
    // set new transform
    FTransform Trans(Rotation, Location, FVector(1, 1, 1));
    CarlaActor->SetActorGlobalTransform(Trans, ETeleportType::None);
    return true;
  }
  return false;
}

// reposition the camera
bool FFrameData::SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation)
{
  check(Episode != nullptr);

  // get the actor to follow
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(Id);
  if (!CarlaActor)
    return false;
  // get specator pawn
  APawn *Spectator = Episode->GetSpectatorPawn();
  if (!Spectator)
   return false;

  FCarlaActor* CarlaSpectator = Episode->FindCarlaActor(Spectator);
  if (!CarlaSpectator)
    return false;

  FTransform ActorTransform = CarlaActor->GetActorGlobalTransform();
  // set the new position
  FQuat ActorRot = ActorTransform.GetRotation();
  FVector Pos = ActorTransform.GetTranslation() + (ActorRot.RotateVector(Offset));
  CarlaSpectator->SetActorGlobalTransform(FTransform(ActorRot * Rotation, Pos, FVector(1,1,1)));

  return true;
}

bool FFrameData::ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State)
{
  check(Episode != nullptr);
  FCarlaActor* CarlaActor = Episode->FindCarlaActor(State.DatabaseId);
  if(CarlaActor)
  {
    CarlaActor->SetTrafficLightState(static_cast<ETrafficLightState>(State.State));
    UTrafficLightController* Controller = CarlaActor->GetTrafficLightController();
    if(Controller)
    {
      Controller->SetElapsedTime(State.ElapsedTime);
      ATrafficLightGroup* Group = Controller->GetGroup();
      if (Group)
      {
        Group->SetFrozenGroup(State.IsFrozen);
      }
    }
     return true;
  }
  return false;
}

// set the animation for Vehicles
void FFrameData::ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle)
{
  check(Episode != nullptr);
  FCarlaActor *CarlaActor = Episode->FindCarlaActor(Vehicle.DatabaseId);
  if (CarlaActor)
  {
    FVehicleControl Control;
    Control.Throttle = Vehicle.Throttle;
    Control.Steer = Vehicle.Steering;
    Control.Brake = Vehicle.Brake;
    Control.bHandBrake = Vehicle.bHandbrake;
    Control.bReverse = (Vehicle.Gear < 0);
    Control.Gear = Vehicle.Gear;
    const auto *CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
    Control.bManualGearShift = (CarlaVehicle != nullptr) && CarlaVehicle->GetMirroredManualGearShift();
    CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
  }
}

void FFrameData::PlayVehicleParts(const std::unordered_map<uint32_t, uint32_t>& MappedId)
{
  for (const carla::multigpu::VehicleParts &Parts : MirrorVehicleParts.vehicles)
  {
    const std::optional<uint32_t> Local = FindMirror(MappedId, Parts.actor_id);
    FCarlaActor *CarlaActor = Local ? Episode->FindCarlaActor(*Local) : nullptr;
    auto *CarlaVehicle = ((CarlaActor != nullptr) && !CarlaActor->IsDormant()) ?
        Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor()) : nullptr;
    if (CarlaVehicle == nullptr)
    {
      continue;
    }
    // Without it the wheels cannot be posed, and roll on their own instead.
    if (CarlaVehicle->HasCarlaVehicleAnimation())
    {
      if (!CarlaVehicle->IsWheelAnimationOverridden())
      {
        CarlaVehicle->SetWheelAnimationOverride(true);
      }
      for (int32 i = 0; (i < static_cast<int32>(Parts.wheels.size())) && CarlaVehicle->HasWheel(i); ++i)
      {
        const EVehicleWheelLocation Location = static_cast<EVehicleWheelLocation>(i);
        CarlaVehicle->SetWheelSteerDirection(Location, Parts.wheels[i].steer);
        CarlaVehicle->SetWheelPitchAngle(Location, Parts.wheels[i].spin);
        CarlaVehicle->SetWheelSuspensionOffset(Location, Parts.wheels[i].suspension_offset);
      }
    }
    if (Parts.current_gear)
    {
      CarlaVehicle->SetMirroredDrivetrain(*Parts.current_gear, Parts.manual_gear_shift, Parts.simulating_physics);
    }
    TArray<TPair<int32, FTransform>> OpenDoors;
    for (const carla::multigpu::DoorPose &Door : Parts.open_doors)
    {
      OpenDoors.Emplace(Door.door, FTransform(
          FQuat(Door.rotation[0], Door.rotation[1], Door.rotation[2], Door.rotation[3]),
          FVector(Door.location[0], Door.location[1], Door.location[2])));
    }
    CarlaVehicle->SetMirroredDoorPoses(OpenDoors);
  }
}

// set the lights for vehicles
void FFrameData::ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(LightVehicle.DatabaseId);
  if (CarlaActor)
  {
    carla::rpc::VehicleLightState LightState(LightVehicle.State);
    CarlaActor->SetVehicleLightState(FVehicleLightState(LightState));
  }
}

void FFrameData::ProcessReplayerLightScene(CarlaRecorderLightScene LightScene)
{
  check(Episode != nullptr);
  UWorld* World = Episode->GetWorld();
  if(World)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    if (!CarlaLightSubsystem)
    {
      return;
    }
    auto* CarlaLight = CarlaLightSubsystem->GetLight(LightScene.LightId);
    if (CarlaLight)
    {
      // Through the call the primary's light changes take (the client's
      // LightManager), which several lamp blueprints handle differently from
      // the separate setters, with the primary's exact color.
      CarlaLight->SetLightState(
          LightScene.Intensity,
          LightScene.Color,
          static_cast<ELightType>(LightScene.Type),
          LightScene.bOn);
    }
  }
}

// set the animation for walkers
void FFrameData::ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker)
{
  SetWalkerSpeed(Walker.DatabaseId, Walker.Speed);
}

void FFrameData::ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(Biker.DatabaseId);
  if (CarlaActor == nullptr)
    return;
  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  check(CarlaVehicle != nullptr)
  CarlaVehicle->SetSpeedAnim(Biker.ForwardSpeed);
  CarlaVehicle->SetRotationAnim(Biker.EngineRotation);
}


// replay finish
bool FFrameData::ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, std::unordered_map<uint32_t, bool> &IsHero)
{
  // set autopilot and physics to all AI vehicles
  const FActorRegistry& Registry = Episode->GetActorRegistry();
  for (auto& It : Registry)
  {
    FCarlaActor* CarlaActor = It.Value.Get();

    // enable physics only on vehicles
    switch (CarlaActor->GetActorType())
    {

      // vehicles
      case FCarlaActor::ActorType::Vehicle:
        // check for hero
        if (!(bIgnoreHero && IsHero[CarlaActor->GetActorId()]))
        {
            // stop all vehicles
            SetActorSimulatePhysics(CarlaActor, true);
            if (auto* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor()))
            {
              CarlaVehicle->SetWheelAnimationOverride(false);
            }
            SetActorVelocity(CarlaActor, FVector(0, 0, 0));
            FVehicleControl Control;
            Control.Throttle = 0.0f;
            Control.Steer = 0.0f;
            Control.Brake = 0.0f;
            Control.bHandBrake = false;
            Control.bReverse = false;
            Control.Gear = 1;
            Control.bManualGearShift = false;
            CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
        }
        break;

      // walkers
      case FCarlaActor::ActorType::Walker:
        // stop walker
        SetWalkerSpeed(CarlaActor->GetActorId(), 0.0f);
        break;
    }
  }
  return true;
}

void FFrameData::SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity)
{
  if (!CarlaActor)
  {
    return;
  }
  CarlaActor->SetActorTargetVelocity(Velocity);
}

// set the animation speed for walkers
void FFrameData::SetWalkerSpeed(uint32_t ActorId, float Speed)
{
  check(Episode != nullptr);
  FCarlaActor * CarlaActor = Episode->FindCarlaActor(ActorId);
  if (!CarlaActor)
  {
    return;
  }
  FWalkerControl Control;
  Control.Speed = Speed;
  CarlaActor->ApplyControlToWalker(Control);
}

// enable / disable physics for an actor
bool FFrameData::SetActorSimulatePhysics(FCarlaActor* CarlaActor, bool bEnabled)
{
  if (!CarlaActor)
  {
    return false;
  }
  ECarlaServerResponse Response =
      CarlaActor->SetActorSimulatePhysics(bEnabled);
  if (Response != ECarlaServerResponse::Success)
  {
    return false;
  }
  return true;
}

void FFrameData::SetFrameCounter()
{
  FCarlaEngine::ResetFrameCounter(FrameCounter.FrameCounter);
}

FCarlaActor *FFrameData::FindTrafficSignAt(FVector Location)
{
  check(Episode != nullptr);
  auto World = Episode->GetWorld();
  check(World != nullptr);

  // get its position (truncated as int's)
  int x = static_cast<int>(Location.X);
  int y = static_cast<int>(Location.Y);
  int z = static_cast<int>(Location.Z);

  const FActorRegistry &Registry = Episode->GetActorRegistry();
  // through all actors in registry
  for (auto It = Registry.begin(); It != Registry.end(); ++It)
  {
    FCarlaActor* CarlaActor = It->Value.Get();
    if(CarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficLight || CarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficSign)
    {
      FVector vec = CarlaActor->GetActorGlobalLocation();
      int x2 = static_cast<int>(vec.X);
      int y2 = static_cast<int>(vec.Y);
      int z2 = static_cast<int>(vec.Z);
      if ((x2 == x) && (y2 == y) && (z2 == z))
      {
        // actor found
        return CarlaActor;
      }
    }
  }
  // actor not found
  return nullptr;
}

void FFrameData::AddExistingActors(void)
{
  // registring all existing actors in first frame
  const FActorRegistry &Registry = Episode->GetActorRegistry();
  for (auto& It : Registry)
  {
    const FCarlaActor* CarlaActor = It.Value.Get();
    if (CarlaActor != nullptr)
    {
      // create event
      CreateRecorderEventAdd(
          CarlaActor->GetActorId(),
          static_cast<uint8_t>(CarlaActor->GetActorType()),
          CarlaActor->GetActorGlobalTransform(),
          CarlaActor->GetActorInfo()->Description,
          false);
    }
  }
  // and their attachments, for a secondary that rebuilds from scratch
  for (auto& It : Registry)
  {
    const FCarlaActor* CarlaActor = It.Value.Get();
    if ((CarlaActor != nullptr) && (CarlaActor->GetParent() != 0u) &&
        (Registry.FindCarlaActor(CarlaActor->GetParent()) != nullptr))
    {
      AddEvent(CarlaRecorderEventParent{CarlaActor->GetActorId(), CarlaActor->GetParent()});
    }
  }
}
