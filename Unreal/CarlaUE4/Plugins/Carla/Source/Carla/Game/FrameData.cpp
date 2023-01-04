// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FrameData.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Actor/CarlaActor.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Game/CarlaEpisode.h"


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
    FCarlaActor* View = It.Value().Get();

    switch (View->GetActorType())
    {
      // save the transform for props
      case FCarlaActor::ActorType::Other:
      case FCarlaActor::ActorType::Sensor:
        AddActorPosition(View);
        break;

      // save the transform of all vehicles
      case FCarlaActor::ActorType::Vehicle:
        AddActorPosition(View);
        AddVehicleAnimation(View);
        AddVehicleLight(View);
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
        break;
    }
  }
  GetFrameCounter();
}

void FFrameData::PlayFrameData(
    UCarlaEpisode *ThisEpisode,
    std::unordered_map<uint32_t, uint32_t>& MappedId)
{

  for(const CarlaRecorderEventAdd &EventAdd : EventsAdd.GetEvents())
  {
    uint32_t OldId = EventAdd.DatabaseId;
    // Todo: check memory corruption of EventAdd.DatabaseId
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
        UE_LOG(LogCarla, Log, TEXT("actor created"));
        break;

      // actor reused from existing
      case 2:
        // mapping id (say desired Id is mapped to what)
        MappedId[OldId] = Result.second;
        UE_LOG(LogCarla, Log, TEXT("actor reused"));
        break;
    }
  }

  for (const CarlaRecorderEventDel &EventDel : EventsDel.GetEvents())
  {
    ProcessReplayerEventDel(MappedId[EventDel.DatabaseId]);
    MappedId.erase(EventDel.DatabaseId);
  }

  for (const CarlaRecorderPosition &Position : Positions.GetPositions())
  {
    CarlaRecorderPosition Pos = Position;
    auto NewId = MappedId.find(Pos.DatabaseId);
    if (NewId != MappedId.end())
    {
      Pos.DatabaseId = NewId->second;
      ProcessReplayerPosition(Pos, Pos, 0.0, 0.0);
    }
  }

  for (const CarlaRecorderStateTrafficLight &State : States.GetStates())
  {
    CarlaRecorderStateTrafficLight StateTrafficLight = State;
    StateTrafficLight.DatabaseId = MappedId[StateTrafficLight.DatabaseId];
    ProcessReplayerStateTrafficLight(StateTrafficLight);
  }

  for (const CarlaRecorderAnimVehicle &AnimVehicle : Vehicles.GetVehicles())
  {
    CarlaRecorderAnimVehicle Vehicle = AnimVehicle;
    Vehicle.DatabaseId = MappedId[Vehicle.DatabaseId];
    ProcessReplayerAnimVehicle(Vehicle);
  }

  for (const CarlaRecorderAnimWalker &AnimWalker : Walkers.GetWalkers())
  {
    CarlaRecorderAnimWalker Walker = AnimWalker;
    Walker.DatabaseId = MappedId[Walker.DatabaseId];
    ProcessReplayerAnimWalker(Walker);
  }

  for (const CarlaRecorderLightVehicle &LightVehicle : LightVehicles.GetLightVehicles())
  {
    CarlaRecorderLightVehicle Light = LightVehicle;
    Light.DatabaseId = MappedId[Light.DatabaseId];
    ProcessReplayerLightVehicle(Light);
  }

  for (const CarlaRecorderLightScene &Light : LightScenes.GetLights())
  {
    ProcessReplayerLightScene(Light);
  }

  SetFrameCounter();
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
  LightVehicles.Clear();
  LightScenes.Clear();
  Kinematics.Clear();
  BoundingBoxes.Clear();
  TriggerVolumes.Clear();
  PhysicsControls.Clear();
  TrafficLightTimes.Clear();
  FrameCounter.FrameCounter = 0;
}

void FFrameData::Write(std::ostream& OutStream)
{
  EventsAdd.Write(OutStream);
  EventsDel.Write(OutStream);
  EventsParent.Write(OutStream);
  Positions.Write(OutStream);
  States.Write(OutStream);
  Vehicles.Write(OutStream);
  Walkers.Write(OutStream);
  LightVehicles.Write(OutStream);
  LightScenes.Write(OutStream);
  TrafficLightTimes.Write(OutStream);
  FrameCounter.Write(OutStream);
}
void FFrameData::Read(std::istream& InStream)
{
  Clear();
  while(!InStream.eof())
  {
    Header header;
    ReadValue<char>(InStream, header.Id);
    ReadValue<uint32_t>(InStream, header.Size);
    switch (header.Id)
    {
      // events add
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        EventsAdd.Read(InStream);
        break;

      // events del
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        EventsDel.Read(InStream);
        break;

      // events parent
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        EventsParent.Read(InStream);
        break;

      // positions
      case static_cast<char>(CarlaRecorderPacketId::Position):
        Positions.Read(InStream);
        break;

      // states
      case static_cast<char>(CarlaRecorderPacketId::State):
        States.Read(InStream);
        break;

      // vehicle animation
      case static_cast<char>(CarlaRecorderPacketId::AnimVehicle):
        Vehicles.Read(InStream);
        break;

      // walker animation
      case static_cast<char>(CarlaRecorderPacketId::AnimWalker):
        Walkers.Read(InStream);
        break;

      // vehicle light animation
      case static_cast<char>(CarlaRecorderPacketId::VehicleLight):
        LightVehicles.Read(InStream);
        break;

      // scene lights animation
      case static_cast<char>(CarlaRecorderPacketId::SceneLight):
        LightScenes.Read(InStream);
        break;

      case static_cast<char>(CarlaRecorderPacketId::FrameCounter):
        FrameCounter.Read(InStream);
        break;

      // unknown packet, just skip
      default:
        // skip packet
        InStream.seekg(header.Size, std::ios::cur);
        break;

    }
  }
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

  FTransform Transform = CarlaActor->GetActorGlobalTransform();
  // get position of the vehicle
  AddPosition(CarlaRecorderPosition
  {
    CarlaActor->GetActorId(),
    Transform.GetLocation(),
    Transform.GetRotation().Euler()
  });
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

  FVector Velocity, AngularVelocity;
  constexpr float TO_METERS = 1e-2;
  Velocity = TO_METERS* CarlaActor->GetActorVelocity();
  AngularVelocity = CarlaActor->GetActorAngularVelocity();
  CarlaRecorderKinematics Kinematic =
  {
    CarlaActor->GetActorId(),
    Velocity,
    AngularVelocity
   };
   AddKinematics(Kinematic);
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
  UBoxComponent* Trigger = Triggers.Top();
  auto VolumeOrigin = Trigger->GetComponentLocation();
  auto VolumeExtent = Trigger->GetScaledBoxExtent();
  CarlaRecorderActorBoundingBox TriggerVolume =
  {
    Episode->GetActorRegistry().FindCarlaActor(&TrafficSign)->GetActorId(),
    {VolumeOrigin, VolumeExtent}
  };
  TriggerVolumes.Add(TriggerVolume);
}

void FFrameData::AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle)
{
  CarlaRecorderPhysicsControl Control;
  Control.DatabaseId = Episode->GetActorRegistry().FindCarlaActor(&Vehicle)->GetActorId();
  Control.VehiclePhysicsControl = Vehicle.GetVehiclePhysicsControl();
  PhysicsControls.Add(Control);
}

void FFrameData::AddTrafficLightTime(const ATrafficLightBase& TrafficLight)
{
  auto DatabaseId = Episode->GetActorRegistry().FindCarlaActor(&TrafficLight)->GetActorId();
  CarlaRecorderTrafficLightTime TrafficLightTime{
    DatabaseId,
    TrafficLight.GetGreenTime(),
    TrafficLight.GetYellowTime(),
    TrafficLight.GetRedTime()
  };
  TrafficLightTimes.Add(TrafficLightTime);
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
    FCarlaActor* CarlaActor = FindTrafficLightAt(Location);
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
    // check if an actor of that type already exist with same id
    if (Episode->GetActorRegistry().Contains(DesiredId))
    {
      auto* CarlaActor = Episode->FindCarlaActor(DesiredId);
      const FActorDescription *desc = &CarlaActor->GetActorInfo()->Description;
      if (desc->Id == ActorDesc.Id)
      {
        // we don't need to create, actor of same type already exist
        // relocate
        FRotator Rot = FRotator::MakeFromEuler(Rotation);
        FTransform Trans2(Rot, Location, FVector(1, 1, 1));
        CarlaActor->SetActorGlobalTransform(Trans2);
        return std::pair<int, FCarlaActor*>(2, CarlaActor);
      }
    }
    else if (MappedId.find(DesiredId) != MappedId.end() && Episode->GetActorRegistry().Contains(MappedId[DesiredId]))
    {
      auto* CarlaActor = Episode->FindCarlaActor(MappedId[DesiredId]);
      const FActorDescription *desc = &CarlaActor->GetActorInfo()->Description;
      if (desc->Id == ActorDesc.Id)
      {
        // we don't need to create, actor of same type already exist
        // relocate
        FRotator Rot = FRotator::MakeFromEuler(Rotation);
        FTransform Trans2(Rot, Location, FVector(1, 1, 1));
        CarlaActor->SetActorGlobalTransform(Trans2);
        return std::pair<int, FCarlaActor*>(2, CarlaActor);
      }
    }
    // create new actor
    // create the transform
    FRotator Rot = FRotator::MakeFromEuler(Rotation);
    FTransform Trans(Rot, FVector(0, 0, 100000), FVector(1, 1, 1));
    // create as new actor
    TPair<EActorSpawnResultStatus, FCarlaActor*> Result = Episode->SpawnActorWithInfo(Trans, ActorDesc, DesiredId);
    if (Result.Key == EActorSpawnResultStatus::Success)
    {
      // relocate
      FTransform Trans2(Rot, Location, FVector(1, 1, 1));
      Result.Value->SetActorGlobalTransform(Trans2);
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
bool FFrameData::ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId)
{
  check(Episode != nullptr);
  FCarlaActor * Child = Episode->FindCarlaActor(ChildId);
  FCarlaActor * Parent = Episode->FindCarlaActor(ParentId);
  if(!Child)
  {
    UE_LOG(LogCarla, Log, TEXT("Parenting Child actors not found"));
    return false;
  }
  if(!Parent)
  {
    UE_LOG(LogCarla, Log, TEXT("Parenting Parent actors not found"));
    return false;
  }
  Child->SetParent(ParentId);
  Child->SetAttachmentType(carla::rpc::AttachmentType::Rigid);
  Parent->AddChildren(Child->GetActorId());
  if(!Parent->IsDormant())
  {
    if(!Child->IsDormant())
    {
      Episode->AttachActors(
          Child->GetActor(),
          Parent->GetActor(),
          static_cast<EAttachmentType>(carla::rpc::AttachmentType::Rigid));
    }
  }
  else
  {
    Episode->PutActorToSleep(Child->GetActorId());
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
    Control.bManualGearShift = false;
    CarlaActor->ApplyControlToVehicle(Control, EVehicleInputPriority::User);
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
      CarlaLight->SetLightIntensity(LightScene.Intensity);
      CarlaLight->SetLightColor(LightScene.Color);
      CarlaLight->SetLightOn(LightScene.bOn);
      CarlaLight->SetLightType(static_cast<ELightType>(LightScene.Type));
    }
  }
}

// set the animation for walkers
void FFrameData::ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker)
{
  SetWalkerSpeed(Walker.DatabaseId, Walker.Speed);
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

FCarlaActor *FFrameData::FindTrafficLightAt(FVector Location)
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
    FCarlaActor* CarlaActor = It.Value().Get();
    if(CarlaActor->GetActorType() == FCarlaActor::ActorType::TrafficLight)
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
  FActorRegistry Registry = Episode->GetActorRegistry();
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
}
