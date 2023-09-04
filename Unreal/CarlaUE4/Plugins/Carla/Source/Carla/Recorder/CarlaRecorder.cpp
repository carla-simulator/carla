// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Lights/CarlaLight.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Traffic/TrafficLightController.h"
#include "Carla/Traffic/TrafficLightGroup.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Walker/WalkerControl.h"
#include "Carla/Walker/WalkerController.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "VehicleAnimInstance.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/VehicleLightState.h"
#include <compiler/enable-ue4-macros.h>

#include "CarlaRecorder.h"
#include "CarlaReplayerHelper.h"

#include <ctime>
#include <sstream>

ACarlaRecorder::ACarlaRecorder(void)
{
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  Disable();
}

ACarlaRecorder::ACarlaRecorder(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  Disable();
}

std::string ACarlaRecorder::ShowFileInfo(std::string Name, bool bShowAll)
{
  return Query.QueryInfo(Name, bShowAll);
}

std::string ACarlaRecorder::ShowFileCollisions(std::string Name, char Type1, char Type2)
{
  return Query.QueryCollisions(Name, Type1, Type2);
}

std::string ACarlaRecorder::ShowFileActorsBlocked(std::string Name, double MinTime, double MinDistance)
{
  return Query.QueryBlocked(Name, MinTime, MinDistance);
}

std::string ACarlaRecorder::ReplayFile(std::string Name, double TimeStart, double Duration,
    uint32_t FollowId, bool ReplaySensors)
{
  Stop();
  return Replayer.ReplayFile(Name, TimeStart, Duration, FollowId, ReplaySensors);
}

void ACarlaRecorder::SetReplayerTimeFactor(double TimeFactor)
{
  Replayer.SetTimeFactor(TimeFactor);
}

void ACarlaRecorder::SetReplayerIgnoreHero(bool IgnoreHero)
{
  Replayer.SetIgnoreHero(IgnoreHero);
}

void ACarlaRecorder::SetReplayerIgnoreSpectator(bool IgnoreSpectator)
{
  Replayer.SetIgnoreSpectator(IgnoreSpectator);
}

void ACarlaRecorder::StopReplayer(bool KeepActors)
{
  Replayer.Stop(KeepActors);
}

void ACarlaRecorder::Ticking(float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaRecorder::Ticking);
  Super::Tick(DeltaSeconds);

  if (!Episode)
    return;

  // check if recording
  if (Enabled)
  {
    PlatformTime.UpdateTime();
    VisualTime.SetTime(Episode->GetVisualGameTime());

    const FActorRegistry &Registry = Episode->GetActorRegistry();

    // through all actors in registry
    for (auto It = Registry.begin(); It != Registry.end(); ++It)
    {
      FCarlaActor* View = It.Value().Get();

      switch (View->GetActorType())
      {
        // save the transform for props
        case FCarlaActor::ActorType::Other:
          AddActorPosition(View);
          break;

        // save the transform of all vehicles
        case FCarlaActor::ActorType::Vehicle:
          AddActorPosition(View);
          AddVehicleAnimation(View);
          AddVehicleLight(View);
          AddVehicleWheelsAnimation(View);
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
            AddActorBones(View);
          }
          break;

        // save the state of each traffic light
        case FCarlaActor::ActorType::TrafficLight:
          AddTrafficLightState(View);
          break;
      }
    }

    // write all data for this frame
    Write(DeltaSeconds);
  }
  else if (Episode->GetReplayer()->IsEnabled())
  {
    // replayer
    Episode->GetReplayer()->Tick(DeltaSeconds);
  }
}

void ACarlaRecorder::Enable(void)
{
  PrimaryActorTick.bCanEverTick = true;
  Enabled = true;
}

void ACarlaRecorder::Disable(void)
{
  PrimaryActorTick.bCanEverTick = false;
  Enabled = false;
}

void ACarlaRecorder::AddActorPosition(FCarlaActor *CarlaActor)
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

void ACarlaRecorder::AddVehicleAnimation(FCarlaActor *CarlaActor)
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

void ACarlaRecorder::AddVehicleWheelsAnimation(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr)
  if (CarlaActor->IsPendingKill())
    return;
  if (CarlaActor->GetActorType() != FCarlaActor::ActorType::Vehicle)
    return;

  ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(CarlaActor->GetActor());
  if (CarlaVehicle == nullptr)
    return;

  USkeletalMeshComponent* SkeletalMesh = CarlaVehicle->GetMesh();
  if (SkeletalMesh == nullptr)
    return;

  UVehicleAnimInstance* VehicleAnim = Cast<UVehicleAnimInstance>(SkeletalMesh->GetAnimInstance());
  if (VehicleAnim == nullptr)
    return;

  const UWheeledVehicleMovementComponent* WheeledVehicleMovementComponent = VehicleAnim->GetWheeledVehicleMovementComponent();
  if (WheeledVehicleMovementComponent == nullptr)
    return;

  CarlaRecorderAnimWheels Record;
  Record.DatabaseId = CarlaActor->GetActorId();
  Record.WheelValues.reserve(WheeledVehicleMovementComponent->Wheels.Num());

  uint8 i = 0;
  for (auto Wheel : WheeledVehicleMovementComponent->Wheels)
  {
    WheelInfo Info;
    Info.Location = static_cast<EVehicleWheelLocation>(i);
    Info.SteeringAngle = CarlaVehicle->GetWheelSteerAngle(Info.Location);
    Info.TireRotation = Wheel->GetRotationAngle();
    Record.WheelValues.push_back(Info);
    ++i;
  }

  AddAnimVehicleWheels(Record);

  if (CarlaVehicle->IsTwoWheeledVehicle())
  {
    AddAnimBiker(CarlaRecorderAnimBiker
    {
      CarlaActor->GetActorId(),
      WheeledVehicleMovementComponent->GetForwardSpeed(),
      WheeledVehicleMovementComponent->GetEngineRotationSpeed() / WheeledVehicleMovementComponent->GetEngineMaxRotationSpeed()
    });
  }
}

void ACarlaRecorder::AddWalkerAnimation(FCarlaActor *CarlaActor)
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

void ACarlaRecorder::AddTrafficLightState(FCarlaActor *CarlaActor)
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

void ACarlaRecorder::AddVehicleLight(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  FVehicleLightState LightState;
  CarlaActor->GetVehicleLightState(LightState);
  CarlaRecorderLightVehicle LightVehicle;
  LightVehicle.DatabaseId = CarlaActor->GetActorId();
  LightVehicle.State = carla::rpc::VehicleLightState(LightState).light_state;
  AddLightVehicle(LightVehicle);
}

void ACarlaRecorder::AddActorKinematics(FCarlaActor *CarlaActor)
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

void ACarlaRecorder::AddActorBoundingBox(FCarlaActor *CarlaActor)
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

void ACarlaRecorder::AddTriggerVolume(const ATrafficSignBase &TrafficSign)
{
  if (bAdditionalData)
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
}

void ACarlaRecorder::AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle)
{
  if (bAdditionalData)
  {
    CarlaRecorderPhysicsControl Control;
    Control.DatabaseId = Episode->GetActorRegistry().FindCarlaActor(&Vehicle)->GetActorId();
    Control.VehiclePhysicsControl = Vehicle.GetVehiclePhysicsControl();
    PhysicsControls.Add(Control);
  }
}

void ACarlaRecorder::AddTrafficLightTime(const ATrafficLightBase& TrafficLight)
{
  if (bAdditionalData)
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
}

void ACarlaRecorder::AddActorBones(FCarlaActor *CarlaActor)
{
  check(CarlaActor != nullptr);

  // get the bones
  FWalkerBoneControlOut Bones;
  CarlaActor->GetBonesTransform(Bones);

  CarlaRecorderWalkerBones Walker;
  Walker.DatabaseId = CarlaActor->GetActorId();
  for (auto &Bone : Bones.BoneTransforms)
  {
    FString Name = Bone.Get<0>();
    auto Transforms = Bone.Get<1>();
    FVector Loc = Transforms.Relative.GetTranslation();
    FVector Rot = Transforms.Relative.GetRotation().Euler();
    CarlaRecorderWalkerBone Entry(Name, Loc, Rot);
    Walker.Bones.push_back(Entry);
  }
  WalkersBones.Add(std::move(Walker));
}

std::string ACarlaRecorder::Start(std::string Name, FString MapName, bool AdditionalData)
{
  // stop replayer if any in course
  if (Replayer.IsEnabled())
    Replayer.Stop();

  // stop recording
  Stop();

  // reset collisions Id
  NextCollisionId = 0;

  // get the final path + filename
  std::string Filename = GetRecorderFilename(Name);

  // binary file
  File.open(Filename, std::ios::binary);
  if (!File.is_open())
  {
    return "";
  }

  // save info
  Info.Version = 1;
  Info.Magic = TEXT("CARLA_RECORDER");
  Info.Date = std::time(0);
  Info.Mapfile = MapName;

  // write general info
  Info.Write(File);

  Frames.Reset();
  PlatformTime.SetStartTime();

  Enable();

  bAdditionalData = AdditionalData;

  // add all existing actors
  AddExistingActors();

  return std::string(Filename);
}

void ACarlaRecorder::Stop(void)
{
  Disable();

  if (File)
  {
    File.close();
  }

  Clear();
}

void ACarlaRecorder::Clear(void)
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
  WalkersBones.Clear();
  Wheels.Clear();
  Bikers.Clear();
}

void ACarlaRecorder::Write(double DeltaSeconds)
{
  // update this frame data
  Frames.SetFrame(DeltaSeconds);

  // start
  Frames.WriteStart(File);
  VisualTime.Write(File);

  // events
  EventsAdd.Write(File);
  EventsDel.Write(File);
  EventsParent.Write(File);
  Collisions.Write(File);

  // positions and states
  Positions.Write(File);
  States.Write(File);

  // animations
  Vehicles.Write(File);
  Walkers.Write(File);
  LightVehicles.Write(File);
  LightScenes.Write(File);
  Wheels.Write(File);
  Bikers.Write(File);

  // additional info
  if (bAdditionalData)
  {
    Kinematics.Write(File);
    BoundingBoxes.Write(File);
    TriggerVolumes.Write(File);
    PlatformTime.Write(File);
    PhysicsControls.Write(File);
    TrafficLightTimes.Write(File);
    WalkersBones.Write(File);
  }

  // end
  Frames.WriteEnd(File);

  Clear();
}

void ACarlaRecorder::AddPosition(const CarlaRecorderPosition &Position)
{
  if (Enabled)
  {
    Positions.Add(Position);
  }
}

void ACarlaRecorder::AddEvent(const CarlaRecorderEventAdd &Event)
{
  if (Enabled)
  {
    EventsAdd.Add(std::move(Event));
  }
}

void ACarlaRecorder::AddEvent(const CarlaRecorderEventDel &Event)
{
  if (Enabled)
  {
    EventsDel.Add(std::move(Event));
  }
}

void ACarlaRecorder::AddEvent(const CarlaRecorderEventParent &Event)
{
  if (Enabled)
  {
    EventsParent.Add(std::move(Event));
  }
}

void ACarlaRecorder::AddCollision(AActor *Actor1, AActor *Actor2)
{
  if (Enabled)
  {
    CarlaRecorderCollision Collision;

    // some inits
    Collision.Id = NextCollisionId++;
    Collision.IsActor1Hero = false;
    Collision.IsActor2Hero = false;

    // check actor 1
    FCarlaActor *FoundActor1 = Episode->GetActorRegistry().FindCarlaActor(Actor1);
    if (FoundActor1 != nullptr) {
      if (FoundActor1->GetActorInfo() != nullptr)
      {
        auto Role = FoundActor1->GetActorInfo()->Description.Variations.Find("role_name");
        if (Role != nullptr)
          Collision.IsActor1Hero = (Role->Value == "hero");
      }
      Collision.DatabaseId1 = FoundActor1->GetActorId();
    }
    else {
      Collision.DatabaseId1 = uint32_t(-1); // actor1 is not a registered Carla actor
    }

    // check actor 2
    FCarlaActor *FoundActor2 = Episode->GetActorRegistry().FindCarlaActor(Actor2);
    if (FoundActor2 != nullptr) {
      if (FoundActor2->GetActorInfo() != nullptr)
      {
        auto Role = FoundActor2->GetActorInfo()->Description.Variations.Find("role_name");
        if (Role != nullptr)
          Collision.IsActor2Hero = (Role->Value == "hero");
      }
      Collision.DatabaseId2 = FoundActor2->GetActorId();
    }
    else {
      Collision.DatabaseId2 = uint32_t(-1); // actor2 is not a registered Carla actor
    }

    Collisions.Add(std::move(Collision));
  }
}

void ACarlaRecorder::AddState(const CarlaRecorderStateTrafficLight &State)
{
  if (Enabled)
  {
    States.Add(State);
  }
}

void ACarlaRecorder::AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle)
{
  if (Enabled)
  {
    Vehicles.Add(Vehicle);
  }
}

void ACarlaRecorder::AddAnimVehicleWheels(const CarlaRecorderAnimWheels &VehicleWheels)
{
  if (Enabled)
  {
    Wheels.Add(VehicleWheels);
  }
}

void ACarlaRecorder::AddAnimWalker(const CarlaRecorderAnimWalker &Walker)
{
  if (Enabled)
  {
    Walkers.Add(Walker);
  }
}

void ACarlaRecorder::AddAnimBiker(const CarlaRecorderAnimBiker &Biker)
{
  if (Enabled)
  {
    Bikers.Add(Biker);
  }
}

void ACarlaRecorder::AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle)
{
  if (Enabled)
  {
    LightVehicles.Add(LightVehicle);
  }
}

void ACarlaRecorder::AddEventLightSceneChanged(const UCarlaLight* Light)
{
  if (Enabled)
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
}

void ACarlaRecorder::AddKinematics(const CarlaRecorderKinematics &ActorKinematics)
{
  if (Enabled)
  {
    Kinematics.Add(ActorKinematics);
  }
}

void ACarlaRecorder::AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox)
{
  if (Enabled)
  {
    BoundingBoxes.Add(ActorBoundingBox);
  }
}

void ACarlaRecorder::AddExistingActors(void)
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
          CarlaActor->GetActorInfo()->Description);
    }
  }

  UWorld *World = GetWorld();
  if(World)
  {
    UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
    const auto& Lights = CarlaLightSubsystem->GetLights();
    for (const auto& LightPair : Lights)
    {
      UCarlaLight* Light = LightPair.Value;
      AddEventLightSceneChanged(Light);
    }
  }

}

void ACarlaRecorder::CreateRecorderEventAdd(
    uint32_t DatabaseId,
    uint8_t Type,
    const FTransform &Transform,
    FActorDescription ActorDescription)
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

  FCarlaActor* CarlaActor = Episode->FindCarlaActor(DatabaseId);
  // Other events related to spawning actors
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
