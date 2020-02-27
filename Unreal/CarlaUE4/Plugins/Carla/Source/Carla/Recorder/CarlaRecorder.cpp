// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Walker/WalkerControl.h"
#include "Carla/Walker/WalkerController.h"

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

std::string ACarlaRecorder::ReplayFile(std::string Name, double TimeStart, double Duration, uint32_t FollowId)
{
  Stop();
  return Replayer.ReplayFile(Name, TimeStart, Duration, FollowId);
}

inline void ACarlaRecorder::SetReplayerTimeFactor(double TimeFactor)
{
  Replayer.SetTimeFactor(TimeFactor);
}

inline void ACarlaRecorder::SetReplayerIgnoreHero(bool IgnoreHero)
{
  Replayer.SetIgnoreHero(IgnoreHero);
}

void ACarlaRecorder::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  if (!Episode)
    return;

  // check if recording
  if (Enabled)
  {
    const FActorRegistry &Registry = Episode->GetActorRegistry();

    // through all actors in registry
    for (auto It = Registry.begin(); It != Registry.end(); ++It)
    {
      FActorView View = *It;

      switch (View.GetActorType())
      {
        // save the transform of all vehicles
        case FActorView::ActorType::Vehicle:
          AddActorPosition(View);
          AddVehicleAnimation(View);
          break;

        // save the transform of all walkers
        case FActorView::ActorType::Walker:
          AddActorPosition(View);
          AddWalkerAnimation(View);
          break;

        // save the state of each traffic light
        case FActorView::ActorType::TrafficLight:
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

void ACarlaRecorder::AddActorPosition(FActorView &View)
{
  AActor *Actor = View.GetActor();
  check(Actor != nullptr);

  // get position of the vehicle
  AddPosition(CarlaRecorderPosition
  {
    View.GetActorId(),
    Actor->GetTransform().GetTranslation(),
    Actor->GetTransform().GetRotation().Euler()
  });
}

void ACarlaRecorder::AddVehicleAnimation(FActorView &View)
{
  AActor *Actor = View.GetActor();
  check(Actor != nullptr);

  if (Actor->IsPendingKill())
  {
    return;
  }

  auto Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  if (Vehicle == nullptr)
  {
    return;
  }

  FVehicleControl Control = Vehicle->GetVehicleControl();

  // save
  CarlaRecorderAnimVehicle Record;
  Record.DatabaseId = View.GetActorId();
  Record.Steering = Control.Steer;
  Record.Throttle = Control.Throttle;
  Record.Brake = Control.Brake;
  Record.bHandbrake = Control.bHandBrake;
  Record.Gear = Control.Gear;
  AddAnimVehicle(Record);
}

void ACarlaRecorder::AddWalkerAnimation(FActorView &View)
{
  AActor *Actor = View.GetActor();
  check(Actor != nullptr);

  if (!Actor->IsPendingKill())
  {
    // check to set speed in walkers
    auto Walker = Cast<APawn>(Actor);
    if (Walker)
    {
      auto Controller = Cast<AWalkerController>(Walker->GetController());
      if (Controller != nullptr)
      {
        AddAnimWalker(CarlaRecorderAnimWalker
        {
          View.GetActorId(),
          Controller->GetWalkerControl().Speed
        });
      }
    }
  }
}

void ACarlaRecorder::AddTrafficLightState(FActorView &View)
{
  AActor *Actor = View.GetActor();
  check(Actor != nullptr);

  // get states
  auto TrafficLight = Cast<ATrafficLightBase>(Actor);
  if (TrafficLight != nullptr)
  {
    AddState(CarlaRecorderStateTrafficLight
    {
      View.GetActorId(),
      TrafficLight->GetTimeIsFrozen(),
      TrafficLight->GetElapsedTime(),
      static_cast<char>(TrafficLight->GetTrafficLightState())
    });
  }
}

std::string ACarlaRecorder::Start(std::string Name, FString MapName)
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

  Enable();

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
}

void ACarlaRecorder::Write(double DeltaSeconds)
{
  // update this frame data
  Frames.SetFrame(DeltaSeconds);

  // start
  Frames.WriteStart(File);

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
    if (Episode->GetActorRegistry().Find(Actor1).GetActorInfo() != nullptr)
    {
      auto *Role = Episode->GetActorRegistry().Find(Actor1).GetActorInfo()->Description.Variations.Find("role_name");
      if (Role != nullptr)
        Collision.IsActor1Hero = (Role->Value == "hero");
    }
    Collision.DatabaseId1 = Episode->GetActorRegistry().Find(Actor1).GetActorId();

    // check actor 2
    if (Episode->GetActorRegistry().Find(Actor2).GetActorInfo() != nullptr)
    {
      auto Role = Episode->GetActorRegistry().Find(Actor2).GetActorInfo()->Description.Variations.Find("role_name");
      if (Role != nullptr)
        Collision.IsActor2Hero = (Role->Value == "hero");
    }
    Collision.DatabaseId2 = Episode->GetActorRegistry().Find(Actor2).GetActorId();

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

void ACarlaRecorder::AddAnimWalker(const CarlaRecorderAnimWalker &Walker)
{
  if (Enabled)
  {
    Walkers.Add(Walker);
  }
}

void ACarlaRecorder::AddExistingActors(void)
{
  // registring all existing actors in first frame
  FActorRegistry Registry = Episode->GetActorRegistry();
  for (auto &&View : Registry)
  {
    const AActor *Actor = View.GetActor();
    if (Actor != nullptr)
    {
      // create event
      CreateRecorderEventAdd(
          View.GetActorId(),
          static_cast<uint8_t>(View.GetActorType()),
          Actor->GetActorTransform(),
          View.GetActorInfo()->Description);
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
}
