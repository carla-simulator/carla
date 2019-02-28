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
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
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
  bool bExitAtNextFrame = false;
  bool bExitLoop = false;

  // check if we are in the right frame
  if (NewTime >= Frame.Elapsed && NewTime < Frame.Elapsed + Frame.DurationThis)
  {
    Per = (NewTime - Frame.Elapsed) / Frame.DurationThis;
    bFrameFound = true;
    bExitLoop = true;
    // UE_LOG(LogCarla, Log, TEXT("Frame %f (%f) now %f per %f"), Frame.Elapsed, Frame.Elapsed + Frame.DurationThis, NewTime, Per);
  }

  // process all frames until time we want or end
  while (!File.eof() && !bExitLoop)
  {
    // get header
    ReadHeader();

    // check for a frame packet
    switch (Header.Id)
    {
      // frame
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        // only read if we are not in the right frame
        Frame.Read(File);
        // check if target time is in this frame
        if (NewTime < Frame.Elapsed + Frame.DurationThis)
        {
          Per = (NewTime - Frame.Elapsed) / Frame.DurationThis;
          bFrameFound = true;
          // UE_LOG(LogCarla, Log, TEXT("Frame %f (%f) now %f per %f"), Frame.Elapsed, Frame.Elapsed + Frame.DurationThis, NewTime, Per);
        }
        break;

      // events add
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ProcessEventsAdd();
        break;

      // events del
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ProcessEventsDel();
        break;

      // events parent
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        ProcessEventsParent();
        break;

      // collisions
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        SkipPacket();
        break;

      // positions
      case static_cast<char>(CarlaRecorderPacketId::Position):
        if (bFrameFound)
          ProcessPositions();
        else
          SkipPacket();
        break;

      // states
      case static_cast<char>(CarlaRecorderPacketId::State):
        if (bFrameFound)
          ProcessStates();
        else
          SkipPacket();
        break;

      // frame end
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        if (bFrameFound)
          bExitLoop = true;
        break;

      // unknown packet, just skip
      default:
        // skip packet
        SkipPacket();
        break;

    }
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

void CarlaReplayer::ProcessEventsAdd(void)
{
  uint16_t i, Total;
  CarlaRecorderEventAdd EventAdd;
  // std::stringstream Info;

  // process creation events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventAdd.Read(File);

    // avoid sensor events
    if (!EventAdd.Description.Id.StartsWith("sensor."))
    {
      // show log
      /*
      Info.str("");
      Info << " Create " << EventAdd.DatabaseId << ": " << TCHAR_TO_UTF8(*EventAdd.Description.Id) << " (" <<
        EventAdd.Description.UId << ") at (" << EventAdd.Location.X << ", " <<
        EventAdd.Location.Y << ", " << EventAdd.Location.Z << ")" << std::endl;
      for (auto &Att : EventAdd.Description.Attributes)
      {
        Info << "  " << TCHAR_TO_UTF8(*Att.Id) << " = " << TCHAR_TO_UTF8(*Att.Value) << std::endl;
      }

      UE_LOG(LogCarla, Log, "%s", Info.str().c_str());
      */

      // auto Result = CallbackEventAdd(
      auto Result = Helper.ProcessReplayerEventAdd(
          EventAdd.Location,
          EventAdd.Rotation,
          std::move(EventAdd.Description),
          EventAdd.DatabaseId);

      switch (Result.first)
      {
        // actor not created
        case 0:
          UE_LOG(LogCarla, Log, TEXT("actor could not be created"));
          break;

        // actor created but with different id
        case 1:
          if (Result.second != EventAdd.DatabaseId)
          {
            // UE_LOG(LogCarla, Log, TEXT("actor created but with different id"));
          }
          // mapping id (recorded Id is a new Id in replayer)
          MappedId[EventAdd.DatabaseId] = Result.second;
          break;

        // actor reused from existing
        case 2:
          // UE_LOG(LogCarla, Log, TEXT("actor already exist, not created"));
          // mapping id (say desired Id is mapped to what)
          MappedId[EventAdd.DatabaseId] = Result.second;
          break;
      }
    }
  }
}

void CarlaReplayer::ProcessEventsDel(void)
{
  uint16_t i, Total;
  CarlaRecorderEventDel EventDel;
  // std::stringstream Info;

  // process destroy events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventDel.Read(File);
    // Info.str("");
    // Info << "Destroy " << MappedId[EventDel.DatabaseId] << "\n";
    // UE_LOG(LogCarla, Log, "%s", Info.str().c_str());
    Helper.ProcessReplayerEventDel(MappedId[EventDel.DatabaseId]);
    MappedId.erase(EventDel.DatabaseId);
  }
}

void CarlaReplayer::ProcessEventsParent(void)
{
  uint16_t i, Total;
  CarlaRecorderEventParent EventParent;
  std::stringstream Info;

  // process parenting events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventParent.Read(File);
    // Info.str("");
    // Info << "Parenting " << MappedId[EventParent.DatabaseId] << " with " << MappedId[EventParent.DatabaseId] <<
      // " (parent)\n";
    // UE_LOG(LogCarla, Log, "%s", Info.str().c_str());
    Helper.ProcessReplayerEventParent(MappedId[EventParent.DatabaseId], MappedId[EventParent.DatabaseIdParent]);
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
  uint32_t NewFollowId = 0;
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
