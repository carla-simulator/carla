// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaReplayer.h"
#include "CarlaRecorder.h"

#include <ctime>
#include <sstream>

CarlaReplayer::CarlaReplayer()
{}

CarlaReplayer::~CarlaReplayer()
{
  Stop();
}

// callbacks
void CarlaReplayer::SetCallbackEventAdd(CarlaRecorderCallbackEventAdd f)
{
  CallbackEventAdd = std::move(f);
}
void CarlaReplayer::SetCallbackEventDel(CarlaRecorderCallbackEventDel f)
{
  CallbackEventDel = std::move(f);
}
void CarlaReplayer::SetCallbackEventParent(CarlaRecorderCallbackEventParent f)
{
  CallbackEventParent = std::move(f);
}
void CarlaReplayer::SetCallbackEventPosition(CarlaRecorderCallbackPosition f)
{
  CallbackPosition = std::move(f);
}
void CarlaReplayer::SetCallbackEventFinish(CarlaRecorderCallbackFinish f)
{
  CallbackFinish = std::move(f);
}
void CarlaReplayer::SetCallbackStateTrafficLight(CarlaRecorderCallbackStateTrafficLight f)
{
  CallbackStateTrafficLight = std::move(f);
}

void CarlaReplayer::Stop(bool KeepActors)
{
  if (Enabled)
  {
    Enabled = false;

    // destroy actors if event was recorded?
    if (!KeepActors)
    {
      ProcessToTime(TotalTime);
    }

    File.close();

    // callback
    if (CallbackFinish)
    {
      CallbackFinish(KeepActors);
    }
  }
  // if (!KeepActors)
  // UE_LOG(LogCarla, Log, TEXT("Replayer stop"));
  // else
  // UE_LOG(LogCarla, Log, TEXT("Replayer stop (keeping actors)"));
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
            EventAdd.Description.UId << ") at (" << EventAdd.Location.X << ", " <<
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
          Info << " Parenting " << EventParent.DatabaseId << " with " << EventDel.DatabaseId <<
            " (parent)\n";
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

std::string CarlaReplayer::ReplayFile(std::string Filename, double TimeStart, double Duration)
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
    if (TimeToStop == TotalTime)
    {
      Stop();
    }
    else
    {
      Stop(true); // keep actors in scene so they continue with AI
    }
  }
}

void CarlaReplayer::ProcessEvents(void)
{
  uint16_t i, Total;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
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

      // callback
      if (CallbackEventAdd)
      {
        // UE_LOG(LogCarla, Log, TEXT("calling callback add"));
        auto Result = CallbackEventAdd(
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
              UE_LOG(LogCarla, Log, TEXT("actor created but with different id"));
            }
            // mapping id (recorded Id is a new Id in replayer)
            MappedId[EventAdd.DatabaseId] = Result.second;
            break;

          case 2:
            UE_LOG(LogCarla, Log, TEXT("actor already exist, not created"));
            // mapping id (say desired Id is mapped to what)
            MappedId[EventAdd.DatabaseId] = Result.second;
            break;
        }
      }
      else
      {
        UE_LOG(LogCarla, Log, TEXT("callback add is not defined"));
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
    // callback
    if (CallbackEventDel)
    {
      CallbackEventDel(MappedId[EventDel.DatabaseId]);
      MappedId.erase(EventDel.DatabaseId);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("callback del is not defined"));
    }
  }

  // parenting events
  ReadValue<uint16_t>(File, Total);
  for (i = 0; i < Total; ++i)
  {
    EventParent.Read(File);
    Info.str("");
    Info << "Parenting " << MappedId[EventParent.DatabaseId] << " with " << MappedId[EventDel.DatabaseId] <<
      " (parent)\n";
    // UE_LOG(LogCarla, Log, "%s", Info.str().c_str());
    // callback
    if (CallbackEventParent)
    {
      CallbackEventParent(MappedId[EventParent.DatabaseId], MappedId[EventParent.DatabaseIdParent]);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("callback parent is not defined"));
    }
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

    // callback
    if (CallbackStateTrafficLight)
    {
      // UE_LOG(LogCarla, Log, TEXT("calling callback add"));
      StateTrafficLight.DatabaseId = MappedId[StateTrafficLight.DatabaseId];
      if (!CallbackStateTrafficLight(StateTrafficLight))
      {
        UE_LOG(LogCarla,
            Log,
            TEXT("callback state traffic light %d called but didn't work"),
            StateTrafficLight.DatabaseId);
      }
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("callback state traffic light is not defined"));
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
  std::unordered_map<int, int> TempMap;

  // map the id of all previous positions to its index
  for (i = 0; i < PrevPos.size(); ++i)
  {
    TempMap[PrevPos[i].DatabaseId] = i;
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
      // UE_LOG(LogCarla, Log, TEXT("Interpolation not possible, only one
      // position"));
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
  if (CallbackPosition)
  {
    CallbackPosition(Pos1, Pos2, Per);
  }
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
