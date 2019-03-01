// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "Carla.h"
#include "CarlaRecorder.h"
#include "Carla/Actor/ActorDescription.h"

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

std::string ACarlaRecorder::ShowFileInfo(std::string Path, std::string Name)
{
  return Query.QueryInfo(Path + Name);
}

std::string ACarlaRecorder::ShowFileCollisions(std::string Path, std::string Name, char Type1, char Type2)
{
  return Query.QueryCollisions(Path + Name, Type1, Type2);
}

std::string ACarlaRecorder::ShowFileActorsBlocked(std::string Path, std::string Name, double MinTime, double MinDistance)
{
  return Query.QueryBlocked(Path + Name, MinTime, MinDistance);
}

std::string ACarlaRecorder::ReplayFile(std::string Path, std::string Name, double TimeStart, double Duration, uint32_t FollowId)
{
  Stop();
  return Replayer.ReplayFile(Path + Name, TimeStart, Duration, FollowId);
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
      AActor *Actor;
      switch (View.GetActorType())
      {
        // save the transform of all vehicles and walkers
        case FActorView::ActorType::Vehicle:
        case FActorView::ActorType::Walker:
          // get position of the vehicle
          Actor = View.GetActor();
          check(Actor != nullptr);
          AddPosition(CarlaRecorderPosition
          {
            View.GetActorId(),
            Actor->GetTransform().GetTranslation(),
            Actor->GetTransform().GetRotation().Euler()
          });
          break;

        // save the state of each traffic light
        case FActorView::ActorType::TrafficLight:
          // get states
          Actor = View.GetActor();
          check(Actor != nullptr);
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

std::string ACarlaRecorder::Start(FString Path, FString Name, FString MapName)
{
  // stop replayer if any in course
  if (Replayer.IsEnabled())
    Replayer.Stop();

  // stop recording
  Stop();

  NextCollisionId = 0;
  FString Filename = Path + Name;

  // binary file
  // file.open(filename.str(), std::ios::binary | std::ios::trunc |
  // std::ios::out);
  File.open(TCHAR_TO_UTF8(*Filename), std::ios::binary);
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

  return std::string(TCHAR_TO_UTF8(*Filename));
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
}

void ACarlaRecorder::Write(double DeltaSeconds)
{
  // update this frame data
  Frames.SetFrame(DeltaSeconds);

  // start
  Frames.WriteStart(File);

  // write data
  EventsAdd.Write(File);
  EventsDel.Write(File);
  EventsParent.Write(File);
  Collisions.Write(File);
  Positions.Write(File);
  States.Write(File);

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
