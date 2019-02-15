// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "Carla.h"
#include "CarlaRecorder.h"

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

void ACarlaRecorder::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  // check if recording
  if (Enabled && Episode)
  {
    const FActorRegistry &reg = Episode->GetActorRegistry();

    // get positions
    for (TActorIterator<ACarlaWheeledVehicle> It(GetWorld()); It; ++It)
    {
      ACarlaWheeledVehicle *Actor = *It;
      check(Actor != nullptr);
      CarlaRecorderPosition recPos {
        reg.Find(Actor).GetActorId(),
        Actor->GetTransform().GetTranslation(),
        Actor->GetTransform().GetRotation().Euler()
      };
      AddPosition(recPos);
    }

    // get states
    for (TActorIterator<ATrafficSignBase> It(GetWorld()); It; ++It)
    {
      ATrafficSignBase *Actor = *It;
      check(Actor != nullptr);
      auto TrafficLight = Cast<ATrafficLightBase>(Actor);
      if (TrafficLight != nullptr)
      {
        CarlaRecorderStateTrafficLight recTraffic {
          reg.Find(Actor).GetActorId(),
          TrafficLight->GetTimeIsFrozen(),
          TrafficLight->GetElapsedTime(),
          static_cast<char>(TrafficLight->GetTrafficLightState())
        };
        AddState(recTraffic);
      }
    }

    // write all data for this frame
    Write();
  }
  else if (Episode && Episode->GetReplayer()->IsEnabled())
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
  // reset
  Stop();

  FString Filename = Path + Name;

  // binary file
  // file.open(filename.str(), std::ios::binary | std::ios::trunc |
  // std::ios::out);
  File.open(TCHAR_TO_UTF8(*Filename), std::ios::binary);
  if (!File.is_open())
  {
    // log_error("Recorder file couldn't be created");
    return "";
  }

  // log file
  FString LogFilename = Path + Name + ".log";
  Log.open(TCHAR_TO_UTF8(*LogFilename));

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
  if (Log)
  {
    Log.close();
  }

  Clear();
}

void ACarlaRecorder::Clear(void)
{
  Events.Clear();
  Positions.Clear();
  States.Clear();

  // log << "Clear\n";
}

void ACarlaRecorder::Write(void)
{
  // update this frame data
  Frames.SetFrame();

  // write data
  Frames.Write(File, Log);
  Events.Write(File, Log);
  Positions.Write(File, Log);
  States.Write(File, Log);

  Clear();
  // log << "Write\n";
}

void ACarlaRecorder::AddPosition(const CarlaRecorderPosition &Position)
{
  if (Enabled)
  {
    Positions.AddPosition(Position);
  }
}

void ACarlaRecorder::AddEvent(const CarlaRecorderEventAdd &Event)
{
  if (Enabled)
  {
    Events.AddEvent(std::move(Event));
  }
}
void ACarlaRecorder::AddEvent(const CarlaRecorderEventDel &Event)
{
  if (Enabled)
  {
    Events.AddEvent(std::move(Event));
  }
}
void ACarlaRecorder::AddEvent(const CarlaRecorderEventParent &Event)
{
  if (Enabled)
  {
    Events.AddEvent(std::move(Event));
  }
}
void ACarlaRecorder::AddState(const CarlaRecorderStateTrafficLight &State)
{
  if (Enabled)
  {
    States.AddState(State);
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
          Actor->GetActorTransform(),
          View.GetActorInfo()->Description);
    }
  }
}

void ACarlaRecorder::CreateRecorderEventAdd(
    unsigned int DatabaseId,
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
    Transform.GetTranslation(),
    Transform.GetRotation().Euler(),
    std::move(Description)
  };
  AddEvent(std::move(RecEvent));
}
