// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaReplayer.h"
#include "CarlaRecorder.h"

#include <ctime>
#include <sstream>

void CarlaReplayer::Stop(bool bKeepActors)
{
  if (Enabled)
  {
    Enabled = false;

    // destroy actors if event was recorded?
    if (!bKeepActors)
    {
      ProcessToTime(TotalTime);
    }

    File.close();

    // callback
    Helper.ProcessReplayerFinish(bKeepActors);
  }

  if (!bKeepActors)
  {
    UE_LOG(LogCarla, Log, TEXT("Replayer stop"));
  }
  else
  {
    UE_LOG(LogCarla, Log, TEXT("Replayer stop (keeping actors)"));
  }
}

bool CarlaReplayer::ReadHeader()
{
  if (File.eof())
  {
    return false;
  }

  ReadValue<char>(File, Header.Id);
  ReadValue<uint32_t>(File, Header.Size);

  return true;
}

void CarlaReplayer::SkipPacket(void)
{
  File.seekg(Header.Size, std::ios::cur);
}

std::string CarlaReplayer::GetInfo(std::string Filename)
{
  std::stringstream Info;

  // check if Replayer is busy
  if (Enabled)
  {
    // Info << "Replayer is busy replaying. Stop it first.\n";
    // return Info.str();
    Stop();
  }

  // try to open
  File.open(Filename, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename << " not found on server\n";
    return Info.str();
  }

  uint16_t i, Total;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
  CarlaRecorderEventCollision EventCollision;
  // CarlaRecorderStateTrafficLight StateTraffic;
  bool bShowFrame;

  // read Info
  RecInfo.Read(File);

  // check magic string
  // Info << "Checking Magic: " << TCHAR_TO_UTF8(*RecInfo.Magic) << std::endl;
  if (RecInfo.Magic != "CARLA_RECORDER")
  {
    Info << "File is not a CARLA recorder" << std::endl;
    return Info.str();
  }

  // show general Info
  Info << "Version: " << RecInfo.Version << std::endl;
  Info << "Map: " << TCHAR_TO_UTF8(*RecInfo.Mapfile) << std::endl;
  tm *TimeInfo = localtime(&RecInfo.Date);
  char DateStr[100];
  strftime(DateStr, 100, "%x %X", TimeInfo);
  Info << "Date: " << DateStr << std::endl << std::endl;

  // parse only frames
  while (File)
  {

    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      case static_cast<char>(CarlaRecorderPacketId::Frame):
        Frame.Read(File);
        // Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds
        // (offset 0x" << std::hex << File.tellg() << std::dec << ")\n";
        break;

      case static_cast<char>(CarlaRecorderPacketId::Event):
        bShowFrame = true;
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && bShowFrame)
        {
          Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds\n";
          bShowFrame = false;
        }
        for (i = 0; i < Total; ++i)
        {
          // add
          EventAdd.Read(File);
          Info << " Create " << EventAdd.DatabaseId << ": " << TCHAR_TO_UTF8(*EventAdd.Description.Id) <<
            " (" <<
            static_cast<int>(EventAdd.Type) << ") at (" << EventAdd.Location.X << ", " <<
            EventAdd.Location.Y << ", " << EventAdd.Location.Z << ")" << std::endl;
          for (auto &Att : EventAdd.Description.Attributes)
          {
            Info << "  " << TCHAR_TO_UTF8(*Att.Id) << " = " << TCHAR_TO_UTF8(*Att.Value) << std::endl;
          }
        }
        // del
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && bShowFrame)
        {
          Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds\n";
          bShowFrame = false;
        }
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Info << " Destroy " << EventDel.DatabaseId << "\n";
        }
        // parenting
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && bShowFrame)
        {
          Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds\n";
          bShowFrame = false;
        }
        for (i = 0; i < Total; ++i)
        {
          EventParent.Read(File);
          Info << " Parenting " << EventParent.DatabaseId << " with " << EventParent.DatabaseId <<
            " (parent)\n";
        }
        // collisions
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && bShowFrame)
        {
          Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds\n";
          bShowFrame = false;
        }
        for (i = 0; i < Total; ++i)
        {
          EventCollision.Read(File);
          Info << " Collision id " << EventCollision.Id << " between " << EventCollision.DatabaseId1;
          if (EventCollision.IsActor1Hero)
            Info << " (hero) ";
          Info << " with " << EventCollision.DatabaseId2;
          if (EventCollision.IsActor2Hero)
            Info << " (hero) ";
          Info << std::endl;
        }
        break;

      case static_cast<char>(CarlaRecorderPacketId::Position):
        // Info << "Positions\n";
        SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        // bShowFrame = true;
        // readValue<uint16_t>(File, Total);
        // if (Total > 0 && bShowFrame) {
        //  Info << "Frame " << frame.id << " at " << frame.Elapsed << "
        // seconds\n";
        //  bShowFrame = false;
        // }
        // Info << " State traffic lights: " << Total << std::endl;
        // for (i = 0; i < Total; ++i) {
        //  stateTraffic.read(File);
        //  Info << "  Id: " << stateTraffic.DatabaseId << " state: " <<
        // static_cast<char>(0x30 + stateTraffic.state) << " frozen: " <<
        // stateTraffic.isFrozen << " elapsedTime: " << stateTraffic.elapsedTime
        // << std::endl;
        //  }
        break;

      default:
        // skip packet
        Info << "Unknown packet id: " << Header.Id << " at offset " << File.tellg() << std::endl;
        SkipPacket();
        break;
    }
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

std::string CarlaReplayer::GetInfoCollisions(std::string Filename, char Category1, char Category2)
{
  std::stringstream Info;

  // check if Replayer is busy
  if (Enabled)
  {
    // Info << "Replayer is busy replaying. Stop it first.\n";
    // return Info.str();
    Stop();
  }

  // try to open
  File.open(Filename, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename << " not found on server\n";
    return Info.str();
  }

  // other, vehicle, walkers, trafficLight, hero, any
  char Categories[] = { 'o', 'v', 'w', 't', 'h', 'a' };
  uint16_t i, Total;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
  CarlaRecorderEventCollision EventCollision;
  struct ReplayerActorInfo
  {
    uint8_t Type;
    FString Id;
  };
  std::unordered_map<uint32_t, ReplayerActorInfo> Actors;
  struct PairHash
  {
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& P) const
    {
        return (P.first * 100000) + P.second;
    }
  };
  std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash > oldCollisions, newCollisions;

  // read Info
  RecInfo.Read(File);

  // check magic string
  // Info << "Checking Magic: " << TCHAR_TO_UTF8(*RecInfo.Magic) << std::endl;
  if (RecInfo.Magic != "CARLA_RECORDER")
  {
    Info << "File is not a CARLA recorder" << std::endl;
    return Info.str();
  }

  // show general Info
  Info << "Version: " << RecInfo.Version << std::endl;
  Info << "Map: " << TCHAR_TO_UTF8(*RecInfo.Mapfile) << std::endl;
  tm *TimeInfo = localtime(&RecInfo.Date);
  char DateStr[100];
  strftime(DateStr, 100, "%x %X", TimeInfo);
  Info << "Date: " << DateStr << std::endl << std::endl;

  // header
  Info << std::setw(8) << "Time";
  Info << std::setw(6) << "Types";
  Info << std::setw(6) << "Id 1";
  Info << std::setw(35) << "Actor 1";
  Info << std::setw(6) << "Id 2";
  Info << std::setw(35) << "Actor 2";
  Info << std::endl;

  // parse only frames
  while (File)
  {

    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      case static_cast<char>(CarlaRecorderPacketId::Frame):
        Frame.Read(File);
        // exchange sets of collisions (to know when a collision is new or continue from previous frame)
        oldCollisions = std::move(newCollisions);
        newCollisions.clear();
        break;

      case static_cast<char>(CarlaRecorderPacketId::Event):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          // add
          EventAdd.Read(File);
          Actors[EventAdd.DatabaseId] = ReplayerActorInfo { EventAdd.Type, EventAdd.Description.Id };
        }
        // del
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Actors.erase(EventAdd.DatabaseId);
        }
        // parenting
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventParent.Read(File);
        }
        // collisions
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventCollision.Read(File);

          int Valid = 0;
          // get categories for both actors
          uint8_t Type1 = Categories[Actors[EventCollision.DatabaseId1].Type];
          uint8_t Type2 = Categories[Actors[EventCollision.DatabaseId2].Type];

          // filter actor 1
          if (Category1 == 'a')
            ++Valid;
          else if (Category1 == Type1)
            ++Valid;
          else if (Category1 == 'h' && EventCollision.IsActor1Hero)
            ++Valid;

          // filter actor 2
          if (Category2 == 'a')
            ++Valid;
          else if (Category2 == Type2)
            ++Valid;
          else if (Category2 == 'h' && EventCollision.IsActor2Hero)
            ++Valid;

          // only show if both actors has passed the filter
          if (Valid == 2)
          {
            // check if we need to show as a starting collision or it is a continuation one
            auto collisionPair = std::make_pair(EventCollision.DatabaseId1, EventCollision.DatabaseId2);
            if (oldCollisions.count(collisionPair) == 0)
            {
              // Info << std::setw(5) << EventCollision.Id << " ";
              Info << std::setw(8) << std::setprecision(0) << std::fixed << Frame.Elapsed;
              Info << "  " << Type1 << " " << Type2 << " ";
              Info << std::setw(6) << EventCollision.DatabaseId1;
              Info << std::setw(35) << TCHAR_TO_UTF8(*Actors[EventCollision.DatabaseId1].Id);
              Info << std::setw(6) << EventCollision.DatabaseId2;
              Info << std::setw(35) << TCHAR_TO_UTF8(*Actors[EventCollision.DatabaseId2].Id);
              //Info << std::setw(8) << Frame.Id;
              Info << std::endl;
            }
            // save current collision
            newCollisions.insert(collisionPair);
          }
        }
        break;

      case static_cast<char>(CarlaRecorderPacketId::Position):
        // Info << "Positions\n";
        SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        break;

      default:
        // skip packet
        Info << "Unknown packet id: " << Header.Id << " at offset " << File.tellg() << std::endl;
        SkipPacket();
        break;
    }
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

std::string CarlaReplayer::GetInfoActorsBlocked(std::string Filename, double MinTime, double MinDistance)
{
  std::stringstream Info;

  // check if Replayer is busy
  if (Enabled)
  {
    // Info << "Replayer is busy replaying. Stop it first.\n";
    // return Info.str();
    Stop();
  }

  // try to open
  File.open(Filename, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename << " not found on server\n";
    return Info.str();
  }

  // other, vehicle, walkers, trafficLight, hero, any
  uint16_t i, Total;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
  CarlaRecorderEventCollision EventCollision;
  CarlaRecorderPosition Position;
  struct ReplayerActorInfo
  {
    uint8_t Type;
    FString Id;
    FVector LastPosition;
    double Time;
    double Duration;
  };
  std::unordered_map<uint32_t, ReplayerActorInfo> Actors;
  // to be able to sort the results by the duration of each actor (decreasing order)
  std::multimap<double, std::string, std::greater<double>> Results;

  // read Info
  RecInfo.Read(File);

  // check magic string
  // Info << "Checking Magic: " << TCHAR_TO_UTF8(*RecInfo.Magic) << std::endl;
  if (RecInfo.Magic != "CARLA_RECORDER")
  {
    Info << "File is not a CARLA recorder" << std::endl;
    return Info.str();
  }

  // show general Info
  Info << "Version: " << RecInfo.Version << std::endl;
  Info << "Map: " << TCHAR_TO_UTF8(*RecInfo.Mapfile) << std::endl;
  tm *TimeInfo = localtime(&RecInfo.Date);
  char DateStr[100];
  strftime(DateStr, 100, "%x %X", TimeInfo);
  Info << "Date: " << DateStr << std::endl << std::endl;

  // header
  Info << std::setw(8) << "Time";
  Info << std::setw(6) << "Id";
  Info << std::setw(35) << "Actor";
  Info << std::setw(10) << "Duration";
  Info << std::endl;

  // parse only frames
  while (File)
  {

    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      case static_cast<char>(CarlaRecorderPacketId::Frame):
        Frame.Read(File);
        break;

      case static_cast<char>(CarlaRecorderPacketId::Event):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          // add
          EventAdd.Read(File);
          Actors[EventAdd.DatabaseId] = ReplayerActorInfo { EventAdd.Type, EventAdd.Description.Id };
        }
        // del
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Actors.erase(EventAdd.DatabaseId);
        }
        // parenting
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventParent.Read(File);
        }
        // collisions
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventCollision.Read(File);
        }
        break;

      case static_cast<char>(CarlaRecorderPacketId::Position):
        // read all positions
        ReadValue<uint16_t>(File, Total);
        for (i=0; i<Total; ++i)
        {
          Position.Read(File);
          // check if actor moved less than a distance
          if (FVector::Distance(Actors[Position.DatabaseId].LastPosition, Position.Location) < MinDistance)
          {
            // actor stopped
            if (Actors[Position.DatabaseId].Duration == 0)
              Actors[Position.DatabaseId].Time = Frame.Elapsed;
            Actors[Position.DatabaseId].Duration += Frame.DurationThis;
          }
          else
          {
            // check to show info
            if (Actors[Position.DatabaseId].Duration >= MinTime)
            {
              std::stringstream Result;
              Result << std::setw(8) << std::setprecision(0) << std::fixed << Actors[Position.DatabaseId].Time;
              Result << std::setw(6) << Position.DatabaseId;
              Result << std::setw(35) << TCHAR_TO_UTF8(*Actors[Position.DatabaseId].Id);
              Result << std::setw(10) << std::setprecision(0) << std::fixed << Actors[Position.DatabaseId].Duration;
              Result << std::endl;
              Results.insert(std::make_pair(Actors[Position.DatabaseId].Duration, Result.str()));
            }
            // actor moving
            Actors[Position.DatabaseId].Duration = 0;
            Actors[Position.DatabaseId].LastPosition = Position.Location;
          }
        }
        break;

      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        break;

      default:
        // skip packet
        Info << "Unknown packet id: " << Header.Id << " at offset " << File.tellg() << std::endl;
        SkipPacket();
        break;
    }
  }

  // show actors stopped that were not moving again
  for (auto &Actor : Actors)
  {
    // check to show info
    if (Actor.second.Duration >= MinTime)
    {
      std::stringstream Result;
      Result << std::setw(8) << std::setprecision(0) << std::fixed << Actor.second.Time;
      Result << std::setw(6) << Actor.first;
      Result << std::setw(35) << TCHAR_TO_UTF8(*Actor.second.Id);
      Result << std::setw(10) << std::setprecision(0) << std::fixed << Actor.second.Duration;
      Result << std::endl;
      Results.insert(std::make_pair(Actor.second.Duration, Result.str()));
    }
  }

  // show the result
  for (auto &Result : Results)
  {
    Info << Result.second;
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

void CarlaReplayer::Rewind(void)
{
  CurrentTime = 0.0f;
  TotalTime = 0.0f;
  TimeToStop = 0.0f;

  File.clear();
  File.seekg(0, std::ios::beg);

  // mark as header as invalid to force reload a new one next time
  Frame.Elapsed = -1.0f;
  Frame.DurationThis = 0.0f;

  MappedId.clear();

  // read geneal Info
  RecInfo.Read(File);

  // UE_LOG(LogCarla, Log, TEXT("Replayer rewind"));
}

// read last frame in File and return the Total time recorded
double CarlaReplayer::GetTotalTime(void)
{
  std::streampos Current = File.tellg();

  // parse only frames
  while (File)
  {
    // get header
    if (!ReadHeader())
    {
      break;
    }

    // check for a frame packet
    switch (Header.Id)
    {
      case static_cast<char>(CarlaRecorderPacketId::Frame):
        Frame.Read(File);
        break;
      default:
        SkipPacket();
        break;
    }
  }

  File.clear();
  File.seekg(Current, std::ios::beg);
  return Frame.Elapsed;
}

std::string CarlaReplayer::ReplayFile(std::string Filename, double TimeStart, double Duration, uint32_t ThisFollowId)
{
  std::stringstream Info;
  std::string s;

  // check to stop if we are replaying another
  if (Enabled)
  {
    Stop();
  }

  Info << "Replaying File: " << Filename << std::endl;

  // try to open
  File.open(Filename, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename << " not found on server\n";
    return Info.str();
  }

  // from start
  Rewind();

  // get Total time of recorder
  TotalTime = GetTotalTime();
  Info << "Total time recorded: " << TotalTime << std::endl;
  // set time to start replayer
  if (TimeStart < 0.0f)
  {
    TimeStart = TotalTime + TimeStart;
    if (TimeStart < 0.0f)
    {
      TimeStart = 0.0f;
    }
  }
  // set time to stop replayer
  if (Duration > 0.0f)
  {
    TimeToStop = TimeStart + Duration;
  }
  else
  {
    TimeToStop = TotalTime;
  }
  Info << "Replaying from " << TimeStart << " s - " << TimeToStop << " s (" << TotalTime << " s)" <<
      std::endl;

  // process all events until the time
  ProcessToTime(TimeStart);

  // set the follow Id
  if (ThisFollowId != 0)
    FollowId = ThisFollowId;
  else
    FollowId = 0;

  // mark as enabled
  Enabled = true;

  return Info.str();
}

void CarlaReplayer::ProcessToTime(double Time)
{
  double Per = 0.0f;
  double NewTime = CurrentTime + Time;
  bool bFrameFound = false;

  // check if we are in the right frame
  if (NewTime >= Frame.Elapsed && NewTime < Frame.Elapsed + Frame.DurationThis)
  {
    Per = (NewTime - Frame.Elapsed) / Frame.DurationThis;
    bFrameFound = true;
  }

  // process all frames until time we want or end
  while (!File.eof() && !bFrameFound)
  {
    // get header
    ReadHeader();
    // check it is a frame packet
    if (Header.Id != static_cast<char>(CarlaRecorderPacketId::Frame))
    {
      if (!File.eof())
      {
        UE_LOG(LogCarla, Log, TEXT("Replayer File error: waitting for a Frame packet"));
      }
      Stop();
      break;
    }
    // read current frame
    Frame.Read(File);

    // check if target time is in this frame
    if (Frame.Elapsed + Frame.DurationThis < NewTime)
    {
      Per = 0.0f;
    }
    else
    {
      Per = (NewTime - Frame.Elapsed) / Frame.DurationThis;
      bFrameFound = true;
    }

    // Info << "Frame: " << Frame.id << " (" << Frame.DurationThis << " / " <<
    // Frame.Elapsed << ") per: " << Per << std::endl;

    // get header
    ReadHeader();
    // check it is an events packet
    if (Header.Id != static_cast<char>(CarlaRecorderPacketId::Event))
    {
      UE_LOG(LogCarla, Log, TEXT("Replayer File error: waitting for an Event packet"));
      Stop();
      break;
    }
    ProcessEvents();

    // get header
    ReadHeader();
    // check it is a positions packet
    if (Header.Id != static_cast<char>(CarlaRecorderPacketId::Position))
    {
      UE_LOG(LogCarla, Log, TEXT("Replayer File error: waitting for a Position packet"));
      Stop();
      break;
    }
    if (bFrameFound)
    {
      ProcessPositions();
    }
    else
    {
      SkipPacket();
    }

    // get header
    ReadHeader();
    // check it is an state packet
    if (Header.Id != static_cast<char>(CarlaRecorderPacketId::State))
    {
      UE_LOG(LogCarla, Log, TEXT("Replayer File error: waitting for an State packet"));
      Stop();
      break;
    }
    if (bFrameFound)
    {
      ProcessStates();
    }
    else
    {
      SkipPacket();
    }

    // UE_LOG(LogCarla, Log, TEXT("Replayer new frame"));
  }

  // update all positions
  if (Enabled && bFrameFound)
  {
    UpdatePositions(Per);
  }

  // save current time
  CurrentTime = NewTime;

  // stop replay?
  if (CurrentTime >= TimeToStop)
  {
    // check if we need to stop the replayer and let it continue in simulation
    // mode
    if (TimeToStop < TotalTime)
    {
      Stop(true); // keep actors in scene so they continue with AI
    }
    else
    {
      Stop();
    }
  }
}

void CarlaReplayer::ProcessEvents(void)
{
  uint16_t i, Total;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
  CarlaRecorderEventCollision EventCollision;
  std::stringstream Info;

  // create events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventAdd.Read(File);

    // avoid sensor events
    if (!EventAdd.Description.Id.StartsWith("sensor."))
    {
      // show log
      Info.str("");
      Info << " Create " << EventAdd.DatabaseId << ": " << TCHAR_TO_UTF8(*EventAdd.Description.Id) << " (" <<
        EventAdd.Description.UId << ") at (" << EventAdd.Location.X << ", " <<
        EventAdd.Location.Y << ", " << EventAdd.Location.Z << ")" << std::endl;
      for (auto &Att : EventAdd.Description.Attributes)
      {
        Info << "  " << TCHAR_TO_UTF8(*Att.Id) << " = " << TCHAR_TO_UTF8(*Att.Value) << std::endl;
      }

      // UE_LOG(LogCarla, Log, "%s", Info.str().c_str());

      // auto Result = CallbackEventAdd(
      auto Result = Helper.ProcessReplayerEventAdd(
          EventAdd.Location,
          EventAdd.Rotation,
          std::move(EventAdd.Description),
          EventAdd.DatabaseId);
      switch (Result.first)
      {
        case 0:
          UE_LOG(LogCarla, Log, TEXT("actor could not be created"));
          break;
        case 1:
          if (Result.second != EventAdd.DatabaseId)
          {
            // UE_LOG(LogCarla, Log, TEXT("*actor created but with different id"));
          }
          // mapping id (recorded Id is a new Id in replayer)
          MappedId[EventAdd.DatabaseId] = Result.second;
          break;

        case 2:
          // UE_LOG(LogCarla, Log, TEXT("actor already exist, not created"));
          // mapping id (say desired Id is mapped to what)
          MappedId[EventAdd.DatabaseId] = Result.second;
          break;
      }
    }
  }

  // destroy events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventDel.Read(File);
    Info.str("");
    Info << "Destroy " << MappedId[EventDel.DatabaseId] << "\n";
    // UE_LOG(LogCarla, Log, "%s", Info.str().c_str());
    Helper.ProcessReplayerEventDel(MappedId[EventDel.DatabaseId]);
    MappedId.erase(EventDel.DatabaseId);
  }

  // parenting events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventParent.Read(File);
    Info.str("");
    Info << "Parenting " << MappedId[EventParent.DatabaseId] << " with " << MappedId[EventParent.DatabaseId] <<
      " (parent)\n";
    // UE_LOG(LogCarla, Log, "%s", Info.str().c_str());
    Helper.ProcessReplayerEventParent(MappedId[EventParent.DatabaseId], MappedId[EventParent.DatabaseIdParent]);
  }

  // collision events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventCollision.Read(File);
    Info.str("");
    Info << "Collision " << MappedId[EventCollision.DatabaseId1] << " with " << MappedId[EventCollision.DatabaseId2] << std::endl;
  }
}

void CarlaReplayer::ProcessStates(void)
{
  uint16_t i, Total;
  CarlaRecorderStateTrafficLight StateTrafficLight;
  std::stringstream Info;

  // read Total traffic light states
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    StateTrafficLight.Read(File);

    // UE_LOG(LogCarla, Log, TEXT("calling callback add"));
    StateTrafficLight.DatabaseId = MappedId[StateTrafficLight.DatabaseId];
    if (!Helper.ProcessReplayerStateTrafficLight(StateTrafficLight))
    {
      UE_LOG(LogCarla,
          Log,
          TEXT("callback state traffic light %d called but didn't work"),
          StateTrafficLight.DatabaseId);
    }
  }
}

void CarlaReplayer::ProcessPositions(void)
{
  uint16_t i, Total;

  // save current as previous
  PrevPos = std::move(CurrPos);

  // read all positions
  ReadValue<uint16_t>(File, Total);
  CurrPos.clear();
  CurrPos.reserve(Total);
  for (i = 0; i < Total; ++i)
  {
    CarlaRecorderPosition Pos;
    Pos.Read(File);
    // assign mapped Id
    auto NewId = MappedId.find(Pos.DatabaseId);
    if (NewId != MappedId.end())
    {
      Pos.DatabaseId = NewId->second;
    }
    CurrPos.push_back(std::move(Pos));
  }
}

void CarlaReplayer::UpdatePositions(double Per)
{
  unsigned int i;
  uint32_t NewFollowId;
  std::unordered_map<int, int> TempMap;

  // map the id of all previous positions to its index
  for (i = 0; i < PrevPos.size(); ++i)
  {
    TempMap[PrevPos[i].DatabaseId] = i;
  }

  // get the Id of the actor to follow
  if (FollowId != 0)
  {
    auto NewId = MappedId.find(FollowId);
    if (NewId != MappedId.end())
    {
      NewFollowId = NewId->second;
      // UE_LOG(LogCarla, Log, TEXT("Following %d (%d)"), NewFollowId, FollowId);
    }
    else
      NewFollowId = 0;
  }

  // go through each actor and update
  for (i = 0; i < CurrPos.size(); ++i)
  {
    // check if exist a previous position
    auto Result = TempMap.find(CurrPos[i].DatabaseId);
    if (Result != TempMap.end())
    {
      // interpolate
      InterpolatePosition(PrevPos[Result->second], CurrPos[i], Per);
    }
    else
    {
      // assign last position (we don't have previous one)
      InterpolatePosition(CurrPos[i], CurrPos[i], 0);
    }

    // move the camera to follow this actor if required
    if (NewFollowId != 0)
    {
      if (NewFollowId == CurrPos[i].DatabaseId)
        Helper.SetCameraPosition(NewFollowId, FVector(-1000, 0, 500), FQuat::MakeFromEuler({0, -25, 0}));
        // Helper.SetCameraPosition(NewFollowId, FVector(0, 0, 2000), FQuat::MakeFromEuler({0, -70, 0}));
    }
  }
}

// interpolate a position (transform, velocity...)
void CarlaReplayer::InterpolatePosition(
    const CarlaRecorderPosition &Pos1,
    const CarlaRecorderPosition &Pos2,
    double Per)
{
  // call the callback
  Helper.ProcessReplayerPosition(Pos1, Pos2, Per);
}

// tick for the replayer
void CarlaReplayer::Tick(float Delta)
{
  // check if there are events to process
  if (Enabled)
  {
    ProcessToTime(Delta);
  }

  // UE_LOG(LogCarla, Log, TEXT("Replayer tick"));
}
